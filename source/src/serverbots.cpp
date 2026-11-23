//
// serverbots.cpp: server-managed AI bot processes
//
// This file is only compiled into the dedicated/server build. It owns the
// lifecycle of the bot clients: spawn, stop, restart, private profile dirs and
// per-bot logs. The AI itself runs in the normal headless client binary, so no
// client-side physics/bot code needs to be linked into the server.
//

#include "cube.h"
#include "serverbots.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#endif

extern servercommandline scl;
extern bool isdedicated;
extern int servmillis;
extern int mandatory_auth;
extern bool usemaster;

struct serverbotprocess
{
    bool used;
    int slot;
    int skill;
    int killat;
    int nextspawn;
#ifndef WIN32
    pid_t pid;
#else
    int pid;
#endif
};

static serverbotprocess botprocs[SERVERBOTS_MAX];
static int configured_count = 0;
static int configured_skill = 2;
static int pending_count = -1;
static int pending_skill = -1;
static int last_think = 0;
static int last_config_error = 0;

static const char *skillnames[5] = { "best", "good", "medium", "worse", "bad" };

static int clampskill(int skill)
{
    return skill < 0 ? 0 : (skill > 4 ? 4 : skill);
}

int serverbots_skillfromname(const char *name)
{
    if(!name || !*name) return 2;
    loopi(5) if(!strcasecmp(name, skillnames[i])) return i;
    int n = atoi(name);
    return n >= 0 && n <= 4 ? n : -1;
}

const char *serverbots_skillname(int skill)
{
    return skillnames[clampskill(skill)];
}

int serverbots_active()
{
    int count = 0;
    loopi(SERVERBOTS_MAX) if(botprocs[i].used) count++;
    return count;
}

int serverbots_configured()
{
    return configured_count;
}

bool serverbots_isbotname(const char *name)
{
    if(!name || !*name || !bot_prefix[0]) return false;
    int plen = (int)strlen(bot_prefix);
    if(strncmp(name, bot_prefix, plen)) return false;
    const char *suffix = name + plen;
    if(!*suffix) return false;
    loopi((int)strlen(suffix)) if(suffix[i] < '0' || suffix[i] > '9') return false;
    int number = atoi(suffix);
    return number >= 1 && number <= SERVERBOTS_MAX;
}

static void makehome(int slot, char *out, int outlen)
{
    const char *home = getenv("HOME");
#ifdef WIN32
    if(!home || !*home) home = getenv("USERPROFILE");
#endif
    if(!home || !*home) home = ".";
    snprintf(out, outlen, "%s/.dunestrike/serverbots/bot-%d", home, slot + 1);
}

static void makelogfile(int slot, char *out, int outlen)
{
    snprintf(out, outlen, "logs/bots/bot-%d.log", slot + 1);
}

static void writebotskill(int slot, int skill)
{
    char dir[1024], file[1200];
    makehome(slot, dir, sizeof(dir));
    createdir(dir);
    snprintf(file, sizeof(file), "%s/aibotskill.txt", dir);
    FILE *f = fopen(file, "w");
    if(!f) return;
    fprintf(f, "%s\n", serverbots_skillname(skill));
    fclose(f);
}

#ifndef WIN32
static void spawnbot(int slot)
{
    char home[1024], logfile[1200], gamedir[1024], botname[64], skill[32];
    makehome(slot, home, sizeof(home));
    makelogfile(slot, logfile, sizeof(logfile));
    if(!getcwd(gamedir, sizeof(gamedir))) copystring(gamedir, ".", sizeof(gamedir));

    createdir("logs");
    createdir("logs/bots");
    createdir(home);
    writebotskill(slot, configured_skill);

    string slotstr;
    itoa(slotstr, slot + 1);
    copystring(botname, bot_prefix, sizeof(botname));
    concatstring(botname, slotstr, sizeof(botname));
    copystring(skill, serverbots_skillname(configured_skill), sizeof(skill));

    char homearg[1100], portarg[64], serverarg[64], namearg[128], teamarg[64], skillarg[64];
    snprintf(homearg, sizeof(homearg), "--home=%s", home);
    snprintf(portarg, sizeof(portarg), "--aibotport=%d", scl.serverport);
    snprintf(serverarg, sizeof(serverarg), "--aibotserver=127.0.0.1");
    snprintf(namearg, sizeof(namearg), "--aibotname=%s", botname);
    snprintf(teamarg, sizeof(teamarg), "--aibotteam=auto");
    snprintf(skillarg, sizeof(skillarg), "--aibotskill=%s", skill);

    char *argv[12];
    int argc = 0;
    argv[argc++] = bot_clientpath;
    argv[argc++] = homearg;
    argv[argc++] = (char *)"--aibot";
    argv[argc++] = serverarg;
    argv[argc++] = portarg;
    argv[argc++] = namearg;
    argv[argc++] = teamarg;
    argv[argc++] = skillarg;
    argv[argc] = NULL;

    int logfd = open(logfile, O_WRONLY|O_CREAT|O_APPEND, 0644);

    pid_t pid = fork();
    if(pid == 0)
    {
        setsid();
        if(chdir(gamedir) != 0) _exit(126);
        if(logfd >= 0)
        {
            dup2(logfd, STDOUT_FILENO);
            dup2(logfd, STDERR_FILENO);
            close(logfd);
        }
        execv(bot_clientpath, argv);
        _exit(127);
    }

    if(logfd >= 0) close(logfd);

    if(pid < 0)
    {
        mlog(DSLOG_WARNING, "serverbots: could not spawn bot %d", slot + 1);
        botprocs[slot].nextspawn = servmillis + 5000;
        return;
    }

    botprocs[slot].used = true;
    botprocs[slot].slot = slot;
    botprocs[slot].pid = pid;
    botprocs[slot].skill = configured_skill;
    botprocs[slot].killat = 0;
    botprocs[slot].nextspawn = 0;
    mlog(DSLOG_INFO, "serverbots: started bot %s (pid %d, skill %s)", botname, (int)pid, skill);
}
#else
static void spawnbot(int slot)
{
    mlog(DSLOG_WARNING, "serverbots: process spawning is not implemented on this platform");
}
#endif

static void starbot(int slot)
{
#ifndef WIN32
    if(botprocs[slot].used && !botprocs[slot].killat)
    {
        kill(botprocs[slot].pid, SIGTERM);
        botprocs[slot].killat = servmillis + 2000;
        mlog(DSLOG_INFO, "serverbots: stopping bot %d (pid %d)", slot + 1, (int)botprocs[slot].pid);
    }
#endif
}

static void applybots()
{
    if(!isdedicated) return;

    if(usemaster || mandatory_auth)
    {
        if(servmillis - last_config_error > 30000)
        {
            last_config_error = servmillis;
            mlog(DSLOG_WARNING, "serverbots: cannot connect bots because the server requires authentication (mandatory_auth/usemaster)");
        }
        return;
    }

    int maxcount = bot_max;
    if(maxcount > SERVERBOTS_MAX) maxcount = SERVERBOTS_MAX;
    if(maxcount > scl.maxclients - 1) maxcount = scl.maxclients - 1;
    if(maxcount < 0) maxcount = 0;
    if(configured_count > maxcount) configured_count = maxcount;
    if(configured_count < 0) configured_count = 0;

    loopi(SERVERBOTS_MAX)
    {
        if(i < configured_count)
        {
            if(!botprocs[i].used && servmillis >= botprocs[i].nextspawn)
                spawnbot(i);
            else if(botprocs[i].used)
                writebotskill(i, configured_skill);
        }
        else if(botprocs[i].used)
            starbot(i);
    }
}

void serverbots_setconfig(int count, int skill, bool immediate)
{
    if(count < 0) count = 0;
    if(count > SERVERBOTS_MAX) count = SERVERBOTS_MAX;
    configured_count = count;
    configured_skill = clampskill(skill);
    if(immediate) applybots();
}

void serverbots_setpending(int count, int skill)
{
    pending_count = count < 0 ? 0 : (count > SERVERBOTS_MAX ? SERVERBOTS_MAX : count);
    pending_skill = clampskill(skill);
}

void serverbots_onstartgame()
{
    if(pending_count >= 0)
    {
        configured_count = pending_count;
        configured_skill = pending_skill;
        pending_count = pending_skill = -1;
    }
    applybots();
}

void serverbots_init()
{
    loopi(SERVERBOTS_MAX)
    {
        botprocs[i].used = false;
        botprocs[i].slot = i;
        botprocs[i].skill = 2;
        botprocs[i].killat = 0;
        botprocs[i].nextspawn = 0;
        botprocs[i].pid = 0;
    }

    configured_count = bot_default_count;
    configured_skill = clampskill(bot_default_skill);
    mlog(DSLOG_INFO, "serverbots: manager ready (default %d bot(s), skill %s, max %d)",
         configured_count, serverbots_skillname(configured_skill), bot_max);
}

void serverbots_think()
{
    if(!isdedicated) return;
    if(servmillis - last_think < 250) return;
    last_think = servmillis;

#ifndef WIN32
    loopi(SERVERBOTS_MAX)
    {
        if(!botprocs[i].used) continue;

        if(botprocs[i].killat && servmillis >= botprocs[i].killat)
        {
            kill(botprocs[i].pid, SIGKILL);
            botprocs[i].killat = servmillis + 1000;
        }

        int status = 0;
        pid_t r = waitpid(botprocs[i].pid, &status, WNOHANG);
        if(r == botprocs[i].pid)
        {
            mlog(DSLOG_INFO, "serverbots: bot %d exited (pid %d)", i + 1, (int)botprocs[i].pid);
            botprocs[i].used = false;
            botprocs[i].pid = 0;
            botprocs[i].killat = 0;
            botprocs[i].nextspawn = servmillis + 5000;
        }
    }
#endif

    if(configured_count > serverbots_active()) applybots();
}

void serverbots_shutdown()
{
#ifndef WIN32
    loopi(SERVERBOTS_MAX)
    {
        if(!botprocs[i].used) continue;
        kill(botprocs[i].pid, SIGTERM);
        botprocs[i].killat = servmillis + 2000;
    }

    for(int round = 0; round < 20; round++)
    {
        bool any = false;
        loopi(SERVERBOTS_MAX)
        {
            if(!botprocs[i].used) continue;
            int status = 0;
            pid_t r = waitpid(botprocs[i].pid, &status, WNOHANG);
            if(r == botprocs[i].pid)
            {
                botprocs[i].used = false;
                botprocs[i].pid = 0;
                continue;
            }
            any = true;
            if(round >= 10) kill(botprocs[i].pid, SIGKILL);
        }
        if(!any) break;
        usleep(100000);
    }
#endif
}
