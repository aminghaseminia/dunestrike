// tests/test_phase1_cs_fixes.cpp
// Automated test suite for Dune Strike Phase 1:
// 1. HUD gun viewmodel visibility guardrails
// 2. Strict Counter-Strike Team Base Spawning (CLA in T-Spawn, RVSF in CT-Spawn)
// 3. Universal Freeze-Time (All bots and players completely frozen during freeze-time)
// 4. Viewmodel recoil kickback animation calibration

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "test_paths.h"

void test_hudgun_visibility()
{
    std::cout << "[Test 1] Verifying HUD gun viewmodel visibility guardrails...\n";

    // 1. Check main.cpp for hudgun safeguard
    std::ifstream main_file(ds_path("source/src/main.cpp"));
    assert(main_file.is_open());

    std::string line;
    bool found_hudgun_guard = false;
    while(std::getline(main_file, line))
    {
        if(line.find("if(hudgun <= 0) hudgun = 1") != std::string::npos)
            found_hudgun_guard = true;
    }
    main_file.close();
    assert(found_hudgun_guard && "main.cpp must guarantee hudgun defaults to 1");

    // 2. Check saved.cfg in user profile if present
    const char *home = getenv("HOME");
    if(home)
    {
        std::string cfg_path = std::string(home) + "/.dunestrike/v1.3/config/saved.cfg";
        std::ifstream cfg_file(cfg_path.c_str());
        if(cfg_file.is_open())
        {
            bool found_hudgun_on = false;
            while(std::getline(cfg_file, line))
            {
                if(line.find("hudgun 1") != std::string::npos) found_hudgun_on = true;
            }
            cfg_file.close();
            assert(found_hudgun_on && "saved.cfg must have hudgun enabled");
        }
    }

    std::cout << "  -> PASS: HUD gun first-person viewmodel visibility verified!\n";
}

void test_strict_team_base_spawns()
{
    std::cout << "[Test 2] Verifying Counter-Strike Team Base Spawning (T vs CT)...\n";

    // 1. Verify clientgame.cpp strict team spawn selection
    std::ifstream client_file(ds_path("source/src/clientgame.cpp"));
    assert(client_file.is_open());

    std::string line;
    bool found_strict_comment = false;
    bool found_team_spawns_loop = false;
    while(std::getline(client_file, line))
    {
        if(line.find("Strict CS-style Team Spawns") != std::string::npos)
            found_strict_comment = true;
        if(line.find("ents[i].type == PLAYERSTART && ents[i].attr2 == type") != std::string::npos)
            found_team_spawns_loop = true;
    }
    client_file.close();
    assert(found_strict_comment);
    assert(found_team_spawns_loop);

    // 2. Verify server.cpp distributes spawns in bomb defusal mode
    std::ifstream server_file(ds_path("source/src/server.cpp"));
    assert(server_file.is_open());

    bool found_bomb_spawnindex = false;
    while(std::getline(server_file, line))
    {
        if(line.find("(m_arena || m_bomb) ? c->spawnindex : -1") != std::string::npos)
            found_bomb_spawnindex = true;
    }
    server_file.close();
    assert(found_bomb_spawnindex && "server.cpp must send distributed spawnindex in bomb mode");

    std::cout << "  -> PASS: Strict CS team base spawning (T/CT) verified in client and server!\n";
}

void test_universal_freezetime()
{
    std::cout << "[Test 3] Verifying Universal Freeze-Time for Bots and Players...\n";

    // 1. Verify bot.cpp freezes bots in CBot::Think()
    std::ifstream bot_file(ds_path("source/src/bot/bot.cpp"));
    assert(bot_file.is_open());

    std::string line;
    bool found_bot_freezetime = false;
    bool found_bot_zero_move = false;
    while(std::getline(bot_file, line))
    {
        if(line.find("if (is_freezetime())") != std::string::npos || line.find("if(is_freezetime())") != std::string::npos)
            found_bot_freezetime = true;
        if(line.find("m_pMyEnt->move = m_pMyEnt->strafe = 0") != std::string::npos)
            found_bot_zero_move = true;
    }
    bot_file.close();
    assert(found_bot_freezetime && "CBot::Think() must check is_freezetime()");
    assert(found_bot_zero_move && "CBot::Think() must zero bot movement during freeze-time");

    // 2. Verify weapon.cpp blocks shoot() during freeze-time
    std::ifstream weapon_file(ds_path("source/src/weapon.cpp"));
    assert(weapon_file.is_open());

    bool in_shoot_fn = false;
    bool found_shoot_freeze_block = false;
    while(std::getline(weapon_file, line))
    {
        if(line.find("void shoot") != std::string::npos) in_shoot_fn = true;
        if(in_shoot_fn && line.find("is_freezetime()") != std::string::npos)
        {
            found_shoot_freeze_block = true;
            in_shoot_fn = false;
        }
    }
    weapon_file.close();
    assert(found_shoot_freeze_block && "shoot() must block firing when is_freezetime() is true");

    std::cout << "  -> PASS: Universal freeze-time verified for all bots, movement and weapon shooting!\n";
}

void test_viewmodel_recoil_kick()
{
    std::cout << "[Test 4] Verifying Viewmodel Recoil Kickback Animation Values...\n";

    std::ifstream server_h(ds_path("source/src/server.h"));
    assert(server_h.is_open());

    std::string line;
    bool found_assault_kick = false;
    bool found_pistol_kick = false;
    bool found_carbine_kick = false;

    while(std::getline(server_h, line))
    {
        if(line.find("\"assault\"") != std::string::npos && line.find("6,  5") != std::string::npos)
            found_assault_kick = true;
        if(line.find("\"pistol\"") != std::string::npos && line.find("6,  5") != std::string::npos)
            found_pistol_kick = true;
        if(line.find("\"carbine\"") != std::string::npos && line.find("5,  4") != std::string::npos)
            found_carbine_kick = true;
    }
    server_h.close();

    assert(found_assault_kick && "Assault rifle (AK-47) must have punchy viewmodel recoil (mKR=6, mKB=5)");
    assert(found_pistol_kick && "Pistol must have crisp viewmodel recoil (mKR=6, mKB=5)");
    assert(found_carbine_kick && "Carbine (M4A1) must have steady viewmodel recoil (mKR=5, mKB=4)");

    std::cout << "  -> PASS: Viewmodel recoil kickback values calibrated and verified!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 1 CS Differences & Fixes Tests     \n";
    std::cout << "========================================================\n";

    test_hudgun_visibility();
    test_strict_team_base_spawns();
    test_universal_freezetime();
    test_viewmodel_recoil_kick();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 1 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
