#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$DIR/.."

echo "========================================"
echo "  Dune Strike Automated Test Suite"
echo "========================================"

# 1. Compile unit tests
echo "[BUILD] Compiling unit tests..."
clang++ -O2 -Wall "$DIR/test_bombsite.cpp" -o "$DIR/test_bombsite"
clang++ -O2 -Wall "$DIR/test_bomb_dual_site.cpp" -o "$DIR/test_bomb_dual_site"
clang++ -O2 -Wall "$DIR/test_halftime_match.cpp" -o "$DIR/test_halftime_match"
clang++ -O2 -Wall "$DIR/test_gunplay_movement.cpp" -o "$DIR/test_gunplay_movement"
clang++ -O2 -Wall "$DIR/test_tagging_movement.cpp" -o "$DIR/test_tagging_movement"
clang++ -O2 -Wall "$DIR/test_weapon_damage_model.cpp" -o "$DIR/test_weapon_damage_model"
clang++ -O2 -Wall "$DIR/test_bomb_audio_tempo.cpp" -o "$DIR/test_bomb_audio_tempo"
clang++ -O2 -Wall "$DIR/test_cs_main_menu.cpp" -o "$DIR/test_cs_main_menu"

# 2. Run unit tests
echo "[RUN] Running test_bombsite..."
"$DIR/test_bombsite"

echo "[RUN] Running test_bomb_dual_site..."
"$DIR/test_bomb_dual_site"

echo "[RUN] Running test_halftime_match..."
"$DIR/test_halftime_match"

echo "[RUN] Running test_gunplay_movement..."
"$DIR/test_gunplay_movement"

echo "[RUN] Running test_tagging_movement..."
"$DIR/test_tagging_movement"

echo "[RUN] Running test_weapon_damage_model..."
"$DIR/test_weapon_damage_model"

echo "[RUN] Running test_bomb_audio_tempo..."
"$DIR/test_bomb_audio_tempo"

echo "[RUN] Running test_cs_main_menu..."
"$DIR/test_cs_main_menu"

echo ""
echo "========================================"
echo "  ALL TESTS PASSED SUCCESSFULLY!"
echo "========================================"
