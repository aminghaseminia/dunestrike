#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

// Test suite for Phase 2: Halftime & Match Structure (MR format, team swapping, scores, match point)

enum { TEAM_CLA = 0, TEAM_RVSF = 1, TEAM_SPECT = 2 };

struct MockClient {
    int clientnum;
    int team;
    std::string name;
    int frags;
    int deaths;

    MockClient(int id, int t, const char *n) : clientnum(id), team(t), name(n), frags(0), deaths(0) {}
};

struct MatchManager {
    int bomb_roundlimit;
    int bomb_halftime;
    int bomb_rounddelay;

    int round;
    int attackerteam;
    int wins[2];
    int halftime;
    bool forceintermission;
    std::string last_server_msg;
    std::vector<MockClient> clients;

    MatchManager(int roundlimit = 8, int htime = 0) {
        bomb_roundlimit = roundlimit;
        bomb_halftime = htime;
        bomb_rounddelay = 5;
        reset();
    }

    void reset() {
        round = 0;
        attackerteam = TEAM_CLA;
        wins[0] = wins[1] = 0;
        halftime = 0;
        forceintermission = false;
        last_server_msg = "";
    }

    int get_halftimerounds() const {
        return bomb_halftime > 0 ? bomb_halftime : (bomb_roundlimit > 1 ? bomb_roundlimit - 1 : 1);
    }

    void start_round() {
        if(forceintermission) return;
        round++;
        attackerteam = TEAM_CLA; // CLA always attacks; teams swap at halftime
        int htr = get_halftimerounds();
        const char *halfstr = round <= htr ? "First Half" : "Second Half";
        char buf[128];
        snprintf(buf, sizeof(buf), "Round %d (%s): %s attack, %s defend", 
            round, halfstr, attackerteam == TEAM_CLA ? "CLA" : "RVSF", attackerteam == TEAM_CLA ? "RVSF" : "CLA");
        last_server_msg = buf;
    }

    void end_round(int winner_team) {
        if(winner_team == TEAM_CLA || winner_team == TEAM_RVSF) {
            wins[winner_team]++;
        }

        int htr = get_halftimerounds();

        // 1. Check Win
        if(wins[0] >= bomb_roundlimit || wins[1] >= bomb_roundlimit) {
            int winner = wins[0] >= bomb_roundlimit ? TEAM_CLA : TEAM_RVSF;
            char buf[128];
            snprintf(buf, sizeof(buf), "Match Over! %s wins the match (%d - %d)!", 
                winner == TEAM_CLA ? "CLA" : "RVSF", wins[winner], wins[1 - winner]);
            last_server_msg = buf;
            forceintermission = true;
            return;
        }

        // 2. Check Draw after full regulation
        if(round >= 2 * htr) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Match Over! It's a DRAW (%d - %d) after %d rounds!", 
                wins[0], wins[1], round);
            last_server_msg = buf;
            forceintermission = true;
            return;
        }

        // 3. Check Halftime
        if(!halftime && round == htr) {
            halftime = 1;
            last_server_msg = "HALFTIME! SWITCHING SIDES";
            // Swap scores
            std::swap(wins[0], wins[1]);
            // Swap player teams
            for(auto &c : clients) {
                if(c.team == TEAM_CLA) c.team = TEAM_RVSF;
                else if(c.team == TEAM_RVSF) c.team = TEAM_CLA;
            }
            return;
        }

        // 4. Match Point check
        if(wins[0] == bomb_roundlimit - 1) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[MATCH POINT] CLA is on match point (%d - %d)!", wins[0], wins[1]);
            last_server_msg = buf;
        } else if(wins[1] == bomb_roundlimit - 1) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[MATCH POINT] RVSF is on match point (%d - %d)!", wins[1], wins[0]);
            last_server_msg = buf;
        }
    }
};

int main() {
    printf("====================================================\n");
    printf("  RUNNING PHASE 2 TESTS: HALFTIME & MATCH STRUCTURE\n");
    printf("====================================================\n\n");

    // Test 1: Halftime calculation across standards
    printf("[Test 1] Testing Halftime calculation across MR formats...\n");
    MatchManager mm_default(8, 0); // Short match / default: first to 8 wins
    assert(mm_default.get_halftimerounds() == 7);
    printf("  -> bomb_roundlimit=8: halftime after 7 rounds (MR7). PASS\n");

    MatchManager mm_cs2(13, 0); // CS2 competitive MR12: first to 13 wins
    assert(mm_cs2.get_halftimerounds() == 12);
    printf("  -> bomb_roundlimit=13: halftime after 12 rounds (MR12). PASS\n");

    MatchManager mm_cs16(16, 0); // CS 1.6 competitive MR15: first to 16 wins
    assert(mm_cs16.get_halftimerounds() == 15);
    printf("  -> bomb_roundlimit=16: halftime after 15 rounds (MR15). PASS\n");

    MatchManager mm_override(13, 6); // Explicit halftime override
    assert(mm_override.get_halftimerounds() == 6);
    printf("  -> bomb_halftime=6 override: halftime after 6 rounds. PASS\n");

    // Test 2: Full Match Lifecycle simulation (first half, halftime swap, second half, win)
    printf("\n[Test 2] Simulating Match Lifecycle (MR7: First to 8 wins)...\n");
    MatchManager match(8, 0);
    // Add players: Team Alpha on CLA (team 0), Team Bravo on RVSF (team 1)
    match.clients.push_back(MockClient(0, TEAM_CLA, "Player_Alpha1"));
    match.clients.push_back(MockClient(1, TEAM_CLA, "Player_Alpha2"));
    match.clients.push_back(MockClient(2, TEAM_RVSF, "Player_Bravo1"));
    match.clients.push_back(MockClient(3, TEAM_RVSF, "Player_Bravo2"));

    // First Half: Rounds 1 to 7
    // Suppose Team Alpha wins 5 rounds and Team Bravo wins 2 rounds.
    for(int r = 1; r <= 7; ++r) {
        match.start_round();
        assert(match.attackerteam == TEAM_CLA);
        assert(match.round == r);
        // Alpha (CLA) wins rounds 1, 2, 3, 5, 6. Bravo (RVSF) wins rounds 4, 7.
        int winner = (r == 4 || r == 7) ? TEAM_RVSF : TEAM_CLA;
        match.end_round(winner);
    }

    // Check Halftime trigger
    printf("  -> Verifying Halftime Trigger at end of Round 7...\n");
    assert(match.halftime == 1);
    assert(match.last_server_msg == "HALFTIME! SWITCHING SIDES");
    // Team Alpha had 5 wins on CLA, Team Bravo had 2 wins on RVSF.
    // At halftime, player teams swapped!
    assert(match.clients[0].team == TEAM_RVSF); // Alpha now on RVSF
    assert(match.clients[1].team == TEAM_RVSF);
    assert(match.clients[2].team == TEAM_CLA);  // Bravo now on CLA
    assert(match.clients[3].team == TEAM_CLA);
    // And scores swapped so Alpha keeps 5 wins (now on RVSF) and Bravo keeps 2 wins (now on CLA)
    assert(match.wins[TEAM_RVSF] == 5);
    assert(match.wins[TEAM_CLA] == 2);
    printf("  -> Halftime executed: Alpha moved to RVSF (5 wins), Bravo moved to CLA (2 wins). PASS\n");

    // Second Half: Round 8 begins
    match.start_round();
    assert(match.round == 8);
    assert(match.attackerteam == TEAM_CLA); // Bravo (now on CLA) attacks from T Spawn!
    printf("  -> Round 8 (Second Half) started: Bravo attacks as CLA, Alpha defends as RVSF. PASS\n");

    // Round 8: Alpha (RVSF) wins round -> score becomes RVSF 6 - 2 CLA
    match.end_round(TEAM_RVSF);
    assert(match.wins[TEAM_RVSF] == 6);

    // Round 9: Alpha (RVSF) wins round -> score becomes RVSF 7 - 2 CLA -> Match Point!
    match.start_round();
    match.end_round(TEAM_RVSF);
    assert(match.wins[TEAM_RVSF] == 7);
    assert(match.last_server_msg.find("[MATCH POINT]") != std::string::npos);
    printf("  -> Match Point accurately triggered at 7-2: '%s'. PASS\n", match.last_server_msg.c_str());

    // Round 10: Alpha (RVSF) wins round -> reaches 8 wins -> Match Over!
    match.start_round();
    match.end_round(TEAM_RVSF);
    assert(match.wins[TEAM_RVSF] == 8);
    assert(match.forceintermission == true);
    assert(match.last_server_msg == "Match Over! RVSF wins the match (8 - 2)!");
    printf("  -> Match termination accurately triggered: '%s'. PASS\n", match.last_server_msg.c_str());

    // Test 3: Regulation Draw Simulation (7 - 7 tie in MR7)
    printf("\n[Test 3] Simulating Regulation Draw (7 - 7 tie)...\n");
    MatchManager draw_match(8, 0);
    // First Half: 4-3
    for(int r = 1; r <= 7; ++r) {
        draw_match.start_round();
        draw_match.end_round(r <= 4 ? TEAM_CLA : TEAM_RVSF); // CLA 4 - 3 RVSF
    }
    assert(draw_match.halftime == 1);
    // After halftime swap: RVSF 4 - 3 CLA
    assert(draw_match.wins[TEAM_RVSF] == 4);
    assert(draw_match.wins[TEAM_CLA] == 3);

    // Second Half: 7 more rounds (rounds 8 to 14)
    // CLA wins 4 rounds, RVSF wins 3 rounds -> Total: RVSF 7 - 7 CLA
    for(int r = 8; r <= 14; ++r) {
        draw_match.start_round();
        draw_match.end_round(r <= 11 ? TEAM_CLA : TEAM_RVSF);
    }
    assert(draw_match.round == 14);
    assert(draw_match.wins[0] == 7 && draw_match.wins[1] == 7);
    assert(draw_match.forceintermission == true);
    assert(draw_match.last_server_msg == "Match Over! It's a DRAW (7 - 7) after 14 rounds!");
    printf("  -> Regulation Draw accurately triggered: '%s'. PASS\n", draw_match.last_server_msg.c_str());

    printf("\n====================================================\n");
    printf("  ALL PHASE 2 HALFTIME & MATCH TESTS PASSED!\n");
    printf("====================================================\n");
    return 0;
}
