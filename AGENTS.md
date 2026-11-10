# AGENTS.md — Dune Strike (aminghaseminia/dunestrike)

Read this before touching builds, releases, the app-drawer entry, or the local server.

## Golden rule: rebuild after every pull

This repo is wired so that "pull → rebuild" is the only update workflow:

```sh
cd ~/Projects/dunestrike
git pull --rebase
./build-linux.sh          # production build of client + server → bin_unix/
systemctl --user restart dunestrike-server   # only if the server service is running
```

Everything below exists to make that flow safe. Do not break it.

- A pull/merge is **not live on the game server** until the **server binary** is rebuilt and `dunestrike-server` is restarted. `./build-linux.sh` builds both client and server; `make server_install` alone is enough when only the server host needs updating.
- The server host must run the same protocol/build as the clients. After the protocol bump in this tree, old server binaries will reject new clients.

## Local Linux build

- Build dependencies (sudo apt): `clang g++ libsdl2-dev libsdl2-image-dev libopenal-dev libvorbis-dev zlib1g-dev libx11-dev`.
- Always build with `PRODUCTION` defined (`build-linux.sh` passes `-DPRODUCTION`).
  - Production: `DS_VERSION 1400`, master port `28760`.
  - Plain `make` (no flag): `DS_VERSION -1400`, master port `28758` — a dev/test build, wrong for releases.
- `bin_unix/` is git-ignored (binaries never committed). The launcher uses `bin_unix/dunestrike_client` / `dunestrike_server`.
- `build-linux.sh` is the canonical rebuild command (configures+builds bundled enet on first run, then `make client server install`). Keep it working.

## Local macOS build (this machine)

- Homebrew build dependencies: `brew install sdl2 sdl2_image libvorbis` (OpenAL is a system framework).
- `source/src/Makefile_local` is git-ignored and supplies the macOS link overrides (no X11; OpenGL/OpenAL frameworks; SDL for the standalone server). Keep it present on this machine.
- Production rebuild:
  ```sh
  cd source/src
  make clean
  make CXXFLAGS="-O3 -fomit-frame-pointer -Wall -fsigned-char -DPRODUCTION" client server -j"$(sysctl -n hw.ncpu)"
  make install
  ```
- Output binaries are installed into `bin_unix/` exactly like the Linux build. Do not build the Mac client without `-DPRODUCTION` for normal testing.

## App drawer entry — refresh after every build or rebuild (mandatory)

- On macOS the client drawer entry is `~/Applications/Dune Strike.app`. Its `Contents/MacOS/DuneStrike` launcher `cd`s into this repo and execs `bin_unix/dunestrike_client`, so it always runs the latest build. On Linux the equivalent entry is `~/.local/share/applications/dunestrike.desktop` pointing at `dunestrike.sh`.
- **After every build/rebuild, refresh the existing app-drawer entry and verify there is exactly one Dune Strike client entry.** Do not create a new versioned `.app` or `.desktop` file for each version.
  - macOS duplicates check: `find /Applications "$HOME/Applications" -maxdepth 2 -iname 'Dune Strike.app'`
  - Linux duplicates check: `find "$HOME/.local/share/applications" -iname 'dunestrike*.desktop'`
  - macOS LaunchServices refresh: `/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "$HOME/Applications/Dune Strike.app"`
- If stale duplicates exist, remove them so the drawer cannot launch an old copy. `Dune Strike Server.command` is the LAN server launcher, not a second client — keep it.
- If the repo moves, update the launcher path in the app entry (and the Linux/systemd unit). Otherwise the entry needs no maintenance after pulls/rebuilds.
- **Never copy/install the client binary to a separate location for the launcher** — that freezes it at an old version.
- Icons: macOS `Contents/Resources/dunestrike.icns`; Linux `~/.local/share/icons/hicolor/256x256/apps/dunestrike.png` (converted from `packages/misc/icon.png`). Regenerate only if the source icon changes.
- Do not delete/replace the app entry when "cleaning up"; it is intentional state outside the repo.

## Dedicated server — systemd user service

- Unit: `~/.config/systemd/user/dunestrike-server.service`, **enabled**, and the user has `Linger=yes` → starts at boot without login.
- `WorkingDirectory` = this repo; `ExecStart` = repo `dunestrike-server.sh` (voice chat `-R` + `config/servercmdline.txt`).
- **After any rebuild of the server binary, run `systemctl --user restart dunestrike-server`** — the service keeps running the old inode otherwise (nothing breaks, but the new build is not live until restart). `build-linux.sh` does not restart it automatically on purpose.
- Runs **LAN-only** (no `-m` masterserver → no `-Y` private key required). Port **UDP 28763**.
- `ufw` is active on this machine: local play works via loopback; other LAN devices need `sudo ufw allow 28763/udp`.
- Useful: `systemctl --user status|restart dunestrike-server`, `journalctl --user -u dunestrike-server -f` (console output may be block-buffered; game file logs are in `logs/`).

## CI release pipeline (`.github/workflows/build-release.yml`)

- Pull requests do **not** produce server/client release binaries. The release jobs run on **push to `main`**, a `v*` tag, or `workflow_dispatch`; merge/push first, then the server and client assets are rebuilt.
- Trigger: **push to `main`** rebuilds Linux + Windows + macOS and updates the versioned release automatically (the workflow moves the `v<version>` tag to the new commit). Pushing a tag `v*` or running `workflow_dispatch` builds an exact version. Assets are uploaded with `--clobber`, so re-runs are safe.
- Keep the "Enable production build" sed step — without it the shipped binaries are dev builds (negative version, wrong master port).
- **Windows job** (mingw cross, i686/32-bit — matches the shipped `bin_win32` DLLs):
  - `-rdynamic` must be stripped from `source/src/Makefile` (Linux-only flag, the Makefile `override`s it in).
  - Link against bundled import libs in `source/lib`: `-lvorbisfile` (file is `vorbisfile.lib`, **not** `libvorbisfile`), `-lSDL2main`, `-lOpenAL32`, `-lzdll`.
  - `-static-libgcc/-static-libstdc++` belong in **link flags only** (CLIENT_LIBS/SERVER_LIBS) — putting them in `CXXFLAGS` breaks the PCH rule.
  - `libgcc_s_dw2-1.dll` + `libwinpthread-1.dll` must be copied from the toolchain into `bin_win32/` before zipping, or the exe will not start on Windows.
  - The standalone server links SDL2 too (`DS_USE_SDL_THREADS` is forced on `WIN32`/`__APPLE__` in `platform.h`).
- **macOS job**: `macos-latest` (arm64); brew SDL2/libvorbis; server needs SDL cflags in `SERVER_INCLUDES`; Homebrew dylibs are bundled into the zip with `@executable_path/libs` rewrites (otherwise the archive only runs on machines with Homebrew).
- **Packaging**: `git archive` of the tree already contains `bin_unix/` — when adding built binaries, remove the old directory first or you get `bin_unix/bin_unix/` nesting.
- Tag hygiene: after moving/creating a tag, verify with `git ls-remote origin refs/tags/v<ver>` that it points at the intended commit — a stale tag rebuilds old code while you think you fixed it.

## Git / GitHub auth gotchas (this machine)

- The local SSH key authenticates to GitHub as **`majidasgari`**, which has **no access** to this repo — SSH pushes fail.
- Push over **HTTPS**: remote push URL is set to `https://github.com/aminghaseminia/dunestrike.git` with the `gh` credential helper (`gh auth setup-git`).
- The `gh` token needs the **`workflow` scope** to push anything under `.github/workflows/` (`gh auth refresh -h github.com -s workflow` if rejected).
- Concurrent edits are normal: `git pull --rebase origin main` before pushing; the owner pushes their own fixes too. If your commit duplicates theirs and becomes empty during rebase, just drop it (`git rebase --skip`) and confirm with `git diff <tag> main`.

## Origin and version

Dune Strike is an altered distribution built on the Cube engine and the
AssaultCube codebase. See `CREDITS.md` for the required origin and license
notices. `DS_VERSION 1400` corresponds to game version 1.4.0.0. `cube.h` keeps
`//#define PRODUCTION` commented; enable it per-build as described above.
