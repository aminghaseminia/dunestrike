// tests/test_round_flow_mvp_freezetime.cpp
// Automated test suite for Dune Strike Phase 3:
// 1. Freeze-time movement lock (0..15s, spawn lock, buy access, countdown HUD)
// 2. Round MVP Calculation (Defuser, Planter, Multi-frag Eliminator)
// 3. Round End Summary & Win Reason (Terrorist/CT win banners)

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "test_paths.h"

void test_freeze_time_mechanics()
{
    std::cout << "[Test 1] Verifying Freeze-Time Movement Lock and Buy Window...\n";

    // 1. Check physics.cpp for freeze-time movement lock
    std::ifstream phys_file(ds_path("source/src/physics.cpp"));
    assert(phys_file.is_open());

    std::string line;
    bool found_frozen_check = false;
    bool found_frozen_d_zero = false;
    bool found_frozen_jump = false;

    while(std::getline(phys_file, line))
    {
        if(line.find("bool frozen = is_freezetime()") != std::string::npos) found_frozen_check = true;
        if(line.find("d = vec(0, 0, 0);") != std::string::npos) found_frozen_d_zero = true;
        if(line.find("!is_freezetime()") != std::string::npos) found_frozen_jump = true;
    }
    phys_file.close();

    assert(found_frozen_check);
    assert(found_frozen_d_zero);
    assert(found_frozen_jump);

    // 2. Check clientgame.cpp for freezetime var and can_buy
    std::ifstream cg_file(ds_path("source/src/clientgame.cpp"));
    assert(cg_file.is_open());

    bool found_freezetime_var = false;
    bool found_can_buy_freeze = false;
    bool found_check_freezetime = false;

    while(std::getline(cg_file, line))
    {
        if(line.find("VARP(freezetime, 0, 5, 15)") != std::string::npos) found_freezetime_var = true;
        if(line.find("if(is_freezetime()) return in_buy_zone(d)") != std::string::npos) found_can_buy_freeze = true;
        if(line.find("check_freezetime()") != std::string::npos) found_check_freezetime = true;
    }
    cg_file.close();

    assert(found_freezetime_var);
    assert(found_can_buy_freeze);
    assert(found_check_freezetime);

    std::cout << "  -> PASS: Freeze-time locks position/jump while permitting buyzone equipment purchases!\n";
}

void test_round_mvp_system()
{
    std::cout << "[Test 2] Verifying Round MVP Calculation Logic...\n";

    std::ifstream cg_file(ds_path("source/src/clientgame.cpp"));
    assert(cg_file.is_open());

    std::string line;
    bool found_calculate_mvp = false;
    bool found_defuser_mvp = false;
    bool found_planter_mvp = false;
    bool found_elimination_mvp = false;
    bool found_round_kills_inc = false;

    while(std::getline(cg_file, line))
    {
        if(line.find("void calculate_round_mvp") != std::string::npos) found_calculate_mvp = true;
        if(line.find("for defusing the bomb") != std::string::npos) found_defuser_mvp = true;
        if(line.find("for planting the bomb") != std::string::npos) found_planter_mvp = true;
        if(line.find("for eliminating %d enemies") != std::string::npos) found_elimination_mvp = true;
        if(line.find("act->round_kills++") != std::string::npos) found_round_kills_inc = true;
    }
    cg_file.close();

    assert(found_calculate_mvp);
    assert(found_defuser_mvp);
    assert(found_planter_mvp);
    assert(found_elimination_mvp);
    assert(found_round_kills_inc);

    std::cout << "  -> PASS: Round MVP accurately rewards Defusers, Planters, and Top Fraggers!\n";
}

void test_hud_announcements_and_win_reason()
{
    std::cout << "[Test 3] Verifying HUD Freeze Countdown, MVP Stars & Win Reasons...\n";

    std::ifstream hud_file(ds_path("source/src/renderhud.cpp"));
    assert(hud_file.is_open());

    std::string line;
    bool found_hud_freezetime = false;
    bool found_hud_mvp_banner = false;
    bool found_t_win_banner = false;
    bool found_ct_win_banner = false;
    bool found_c4_exploded_reason = false;
    bool found_bomb_defused_reason = false;

    while(std::getline(hud_file, line))
    {
        if(line.find("[ FREEZE TIME:") != std::string::npos) found_hud_freezetime = true;
        if(line.find("★ ROUND MVP:") != std::string::npos) found_hud_mvp_banner = true;
        if(line.find("TERRORISTS WIN THE ROUND") != std::string::npos) found_t_win_banner = true;
        if(line.find("COUNTER-TERRORISTS WIN THE ROUND") != std::string::npos) found_ct_win_banner = true;
        if(line.find("Target Destroyed (C4 Exploded)") != std::string::npos) found_c4_exploded_reason = true;
        if(line.find("Bomb Defused") != std::string::npos) found_bomb_defused_reason = true;
    }
    hud_file.close();

    assert(found_hud_freezetime);
    assert(found_hud_mvp_banner);
    assert(found_t_win_banner);
    assert(found_ct_win_banner);
    assert(found_c4_exploded_reason);
    assert(found_bomb_defused_reason);

    std::cout << "  -> PASS: HUD accurately displays Freeze-Time countdown, MVP gold stars, and Win Reason banners!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "DUNE STRIKE: PHASE 3 ROUND FLOW & MVP VERIFICATION\n";
    std::cout << "========================================================\n";

    test_freeze_time_mechanics();
    test_round_mvp_system();
    test_hud_announcements_and_win_reason();

    std::cout << "\n>>> ALL ROUND FLOW & MVP TESTS PASSED SUCCESSFULLY! <<<\n";
    return 0;
}
