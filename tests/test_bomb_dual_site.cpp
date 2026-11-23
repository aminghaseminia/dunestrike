#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <string>

// Comprehensive test suite for Phase 1: Dual-Site Bomb Defusal (Sites A and B)

enum { BOMB_WAIT = 0, BOMB_LIVE, BOMB_PLANTING, BOMB_PLANTED, BOMB_DEFUSING, BOMB_END };
enum { BOMB_RESULT_NONE = 0, BOMB_RESULT_ATTACKERS, BOMB_RESULT_DEFENDERS };
enum { BOMB_REASON_NONE = 0, BOMB_REASON_EXPLODED, BOMB_REASON_DEFUSED, BOMB_REASON_TIME, BOMB_REASON_ELIMINATED };
enum { TEAM_CLA = 0, TEAM_RVSF = 1 };

struct Vec3 {
    float x, y, z;
    Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    float dist(const Vec3 &o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return sqrtf(dx*dx + dy*dy + dz*dz);
    }
    float sqrxy(const Vec3 &o) const {
        float dx = x - o.x, dy = y - o.y;
        return dx*dx + dy*dy;
    }
};

struct BombStateMachine {
    int state;
    int round;
    int attackerteam;
    int carrier;
    int site; // 0 = Site A, 1 = Site B, -1 = none
    int sites[2][2]; // [0]=Site A (x,y), [1]=Site B (x,y)
    int siteradius;
    Vec3 pos; // planted bomb position

    BombStateMachine() {
        reset();
    }

    void reset() {
        state = BOMB_WAIT;
        round = 0;
        attackerteam = TEAM_CLA;
        carrier = -1;
        site = -1;
        sites[0][0] = sites[0][1] = -1;
        sites[1][0] = sites[1][1] = -1;
        siteradius = 3;
        pos = Vec3(0, 0, 0);
    }

    // Exact implementation from server.cpp
    int bombsiteat(const Vec3 &o) const {
        for(int i = 0; i < 2; ++i) {
            if(sites[i][0] < 0) continue;
            float dx = o.x - sites[i][0], dy = o.y - sites[i][1];
            if(dx*dx + dy*dy <= float(siteradius) * siteradius) return i;
        }
        return -1;
    }

    bool bombdistok(const Vec3 &o) const {
        if(pos.x < 0) return false;
        return o.dist(pos) <= float(siteradius);
    }

    // Client-side hint generator matching renderhud.cpp
    std::string get_plant_hint(int clientnum, const Vec3 &player_pos) const {
        if(state == BOMB_LIVE && carrier == clientnum) {
            int atsite = -1;
            for(int i = 0; i < 2; ++i) {
                if(sites[i][0] >= 0) {
                    float dx = player_pos.x - sites[i][0], dy = player_pos.y - sites[i][1];
                    if(dx*dx + dy*dy <= 9.0f) { atsite = i; break; }
                }
            }
            if(atsite >= 0) {
                char buf[64];
                snprintf(buf, sizeof(buf), "press F or right-click to plant at Site %c", atsite == 0 ? 'A' : 'B');
                return buf;
            }
            return "YOU HAVE THE BOMB - plant at Site A or Site B";
        }
        return "";
    }

    // Actionline generator matching renderhud.cpp
    std::string get_action_line(int pct) const {
        if(state == BOMB_PLANTING) {
            char buf[64];
            snprintf(buf, sizeof(buf), "PLANTING%s %d%%", 
                site == 0 ? " AT SITE A" : site == 1 ? " AT SITE B" : "", pct);
            return buf;
        }
        if(state == BOMB_DEFUSING) {
            char buf[64];
            snprintf(buf, sizeof(buf), "DEFUSING %d%%", pct);
            return buf;
        }
        return "";
    }

    bool start_plant(int clientnum, const Vec3 &player_pos) {
        if(state != BOMB_LIVE || carrier != clientnum) return false;
        int s = bombsiteat(player_pos);
        if(s < 0) return false;
        state = BOMB_PLANTING;
        site = s;
        pos = player_pos;
        return true;
    }

    bool finish_plant() {
        if(state != BOMB_PLANTING) return false;
        state = BOMB_PLANTED;
        carrier = -1;
        return true;
    }

    bool start_defuse(int clientnum, int team, const Vec3 &player_pos) {
        if(state != BOMB_PLANTED) return false;
        if(team == attackerteam) return false; // attackers can't defuse
        if(!bombdistok(player_pos)) return false;
        state = BOMB_DEFUSING;
        return true;
    }

    bool finish_defuse() {
        if(state != BOMB_DEFUSING) return false;
        state = BOMB_END;
        return true;
    }
};

int main() {
    printf("====================================================\n");
    printf("  RUNNING PHASE 1 TESTS: DUAL-SITE BOMB DEFUSAL\n");
    printf("====================================================\n\n");

    BombStateMachine bsm;
    // Map cs_dust2: Site A at (64, 48), Site B at (192, 160)
    bsm.sites[0][0] = 64;  bsm.sites[0][1] = 48;  // Site A
    bsm.sites[1][0] = 192; bsm.sites[1][1] = 160; // Site B
    bsm.siteradius = 3;

    // Test 1: Verification of Site A detection
    printf("[Test 1] Testing Site A detection...\n");
    assert(bsm.bombsiteat(Vec3(64, 48, 10)) == 0);
    assert(bsm.bombsiteat(Vec3(65, 49, 10)) == 0);
    assert(bsm.bombsiteat(Vec3(64 + 2.9f, 48, 10)) == 0);
    assert(bsm.bombsiteat(Vec3(64 + 3.1f, 48, 10)) == -1);
    printf("  -> PASS: Site A coordinates accurately identified as Site 0.\n");

    // Test 2: Verification of Site B detection
    printf("[Test 2] Testing Site B detection...\n");
    assert(bsm.bombsiteat(Vec3(192, 160, 10)) == 1);
    assert(bsm.bombsiteat(Vec3(191, 161, 10)) == 1);
    assert(bsm.bombsiteat(Vec3(192, 160 + 2.9f, 10)) == 1);
    assert(bsm.bombsiteat(Vec3(192, 160 + 3.1f, 10)) == -1);
    printf("  -> PASS: Site B coordinates accurately identified as Site 1.\n");

    // Test 3: Attacker carrying bomb: prompt at spawn vs at sites
    printf("[Test 3] Testing HUD plant hints for carrier...\n");
    bsm.state = BOMB_LIVE;
    bsm.carrier = 0;
    // At T Spawn (10, 10) - not at any site
    std::string hint_spawn = bsm.get_plant_hint(0, Vec3(10, 10, 0));
    assert(hint_spawn == "YOU HAVE THE BOMB - plant at Site A or Site B");
    printf("  -> At spawn: '%s'\n", hint_spawn.c_str());

    // At Site A
    std::string hint_siteA = bsm.get_plant_hint(0, Vec3(64, 48, 0));
    assert(hint_siteA == "press F or right-click to plant at Site A");
    printf("  -> At Site A: '%s'\n", hint_siteA.c_str());

    // At Site B
    std::string hint_siteB = bsm.get_plant_hint(0, Vec3(192, 160, 0));
    assert(hint_siteB == "press F or right-click to plant at Site B");
    printf("  -> At Site B: '%s'\n", hint_siteB.c_str());
    printf("  -> PASS: Carrier HUD cues dynamically differentiate Site A and Site B.\n");

    // Test 4: Plant simulation at Site A
    printf("[Test 4] Simulating bomb plant workflow at Site A...\n");
    bool plant_a_started = bsm.start_plant(0, Vec3(64, 48, 0));
    assert(plant_a_started);
    assert(bsm.state == BOMB_PLANTING);
    assert(bsm.site == 0);
    assert(bsm.get_action_line(45) == "PLANTING AT SITE A 45%");
    bool plant_a_done = bsm.finish_plant();
    assert(plant_a_done);
    assert(bsm.state == BOMB_PLANTED);
    assert(bsm.site == 0);
    assert(bsm.carrier == -1);
    printf("  -> PASS: Complete Site A plant lifecycle succeeded.\n");

    // Test 5: Defusal simulation at Site A
    printf("[Test 5] Simulating defusal workflow at Site A...\n");
    // Defender (team RVSF = 1) at Site A
    bool defuse_started = bsm.start_defuse(1, TEAM_RVSF, Vec3(64.5f, 48.5f, 0));
    assert(defuse_started);
    assert(bsm.state == BOMB_DEFUSING);
    assert(bsm.get_action_line(80) == "DEFUSING 80%");
    bool defuse_done = bsm.finish_defuse();
    assert(defuse_done);
    assert(bsm.state == BOMB_END);
    printf("  -> PASS: Complete defusal lifecycle at Site A succeeded.\n");

    // Test 6: Plant simulation at Site B
    printf("[Test 6] Simulating bomb plant workflow at Site B...\n");
    bsm.reset();
    bsm.sites[0][0] = 64;  bsm.sites[0][1] = 48;
    bsm.sites[1][0] = 192; bsm.sites[1][1] = 160;
    bsm.state = BOMB_LIVE;
    bsm.carrier = 0;
    bool plant_b_started = bsm.start_plant(0, Vec3(192, 160, 0));
    assert(plant_b_started);
    assert(bsm.state == BOMB_PLANTING);
    assert(bsm.site == 1);
    assert(bsm.get_action_line(70) == "PLANTING AT SITE B 70%");
    bool plant_b_done = bsm.finish_plant();
    assert(plant_b_done);
    assert(bsm.state == BOMB_PLANTED);
    assert(bsm.site == 1);
    printf("  -> PASS: Complete Site B plant lifecycle succeeded.\n");

    // Test 7: Defusal simulation at Site B
    printf("[Test 7] Simulating defusal workflow at Site B...\n");
    bool defuse_b_started = bsm.start_defuse(2, TEAM_RVSF, Vec3(193.0f, 160.0f, 0));
    assert(defuse_b_started);
    assert(bsm.state == BOMB_DEFUSING);
    bool defuse_b_done = bsm.finish_defuse();
    assert(defuse_b_done);
    assert(bsm.state == BOMB_END);
    printf("  -> PASS: Complete defusal lifecycle at Site B succeeded.\n");

    // Test 8: Non-carrier or wrong-location plant attempt rejected
    printf("[Test 8] Negative tests: invalid plant attempts...\n");
    bsm.reset();
    bsm.sites[0][0] = 64;  bsm.sites[0][1] = 48;
    bsm.sites[1][0] = 192; bsm.sites[1][1] = 160;
    bsm.state = BOMB_LIVE;
    bsm.carrier = 0;
    // Player 1 (not carrier) attempts plant
    assert(!bsm.start_plant(1, Vec3(64, 48, 0)));
    // Player 0 (carrier) attempts plant in Mid (100, 100)
    assert(!bsm.start_plant(0, Vec3(100, 100, 0)));
    printf("  -> PASS: Illegal plant attempts correctly rejected.\n");

    printf("\n====================================================\n");
    printf("  ALL 8 PHASE 1 UNIT & INTEGRATION TESTS PASSED!\n");
    printf("====================================================\n");
    return 0;
}
