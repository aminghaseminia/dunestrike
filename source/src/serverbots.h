//
// serverbots.h: server-managed AI bot processes
//
// The dedicated server owns the bot lifecycle. It spawns headless client
// processes on the server host, gives each one a private profile and log, and
// stops/restarts them as the configured bot count changes. Human clients never
// launch or manage bots.
//

#ifndef SERVERBOTS_H
#define SERVERBOTS_H

#define SERVERBOTS_MAX 16

extern int bot_max, bot_default_count, bot_default_skill;
extern char bot_prefix[16];
extern char bot_clientpath[256];

void serverbots_init();
void serverbots_shutdown();
void serverbots_think();
void serverbots_setconfig(int count, int skill, bool immediate = true);
void serverbots_setpending(int count, int skill);
void serverbots_onstartgame();
int serverbots_active();
int serverbots_configured();
const char *serverbots_skillname(int skill);
int serverbots_skillfromname(const char *name);
bool serverbots_isbotname(const char *name);

#endif
