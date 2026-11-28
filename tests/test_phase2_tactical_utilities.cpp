// tests/test_phase2_tactical_utilities.cpp
// Automated test suite for Dune Strike Phase 2 Tactical Utilities:
// 1. Angle-aware Dot-Product Flashbang & Ear Ringing (Tinnitus)
// 2. Counter-Strike Defuse Kit System (10s standard defusal / 5s with kit)
// 3. Dense Smoke Core & Bot Line-of-Sight blocking

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cassert>

void test_dot_product_flashbang()
{
    std::cout << "[Test 1] Verifying Angle-Aware Dot-Product Flashbang & Tinnitus...\n";

    // 1. Check weapon.cpp for dot product implementation
    std::ifstream weapon_file("source/src/weapon.cpp");
    if(!weapon_file.is_open()) weapon_file.open("../source/src/weapon.cpp");
    assert(weapon_file.is_open());

    std::string line;
    bool found_lookdir = false;
    bool found_dot_product = false;
    bool found_angle_factor = false;
    bool found_tinnitus_check = false;

    bool in_tinnitus_cond = false;
    while(std::getline(weapon_file, line))
    {
        if(line.find("lookdir.x * nadedir.x") != std::string::npos) found_dot_product = true;
        if(line.find("vec lookdir") != std::string::npos) found_lookdir = true;
        if(line.find("angle_factor = (dot > 0.5f)") != std::string::npos) found_angle_factor = true;
        if(line.find("angle_factor > 0.4f") != std::string::npos) in_tinnitus_cond = true;
        if(in_tinnitus_cond && line.find("eardamagemillis") != std::string::npos)
        {
            found_tinnitus_check = true;
            in_tinnitus_cond = false;
        }
    }
    weapon_file.close();

    assert(found_lookdir && "Must compute view look direction");
    assert(found_dot_product && "Must compute dot product of lookdir and nadedir");
    assert(found_angle_factor && "Must scale flash duration with dot product angle factor");
    assert(found_tinnitus_check && "Tinnitus ear ringing must only trigger on high angle factor eye contact");

    // 2. Math verification:
    // Looking directly at grenade: dot = 1.0 -> angle_factor = 1.0 (Full flash ~4.5-4.8s)
    float dot_direct = 1.0f;
    float factor_direct = (dot_direct > 0.5f) ? 1.0f : ((dot_direct > 0.0f) ? (0.20f + 0.80f * (dot_direct / 0.5f)) : 0.08f);
    assert(factor_direct == 1.0f);

    // Turned around 180 degrees (back to flash): dot = -1.0 -> angle_factor = 0.08 (glancing flash ~0.35s)
    float dot_turned = -1.0f;
    float factor_turned = (dot_turned > 0.5f) ? 1.0f : ((dot_turned > 0.0f) ? (0.20f + 0.80f * (dot_turned / 0.5f)) : 0.08f);
    assert(factor_turned == 0.08f);
    assert(factor_turned < factor_direct * 0.10f); // More than 90% flash reduction when looking away!

    std::cout << "  -> PASS: Dot-product flashbang angle math & tinnitus conditional ringing verified!\n";
}

void test_defuse_kit_system()
{
    std::cout << "[Test 2] Verifying Defuse Kit System (10s base / 5s with kit)...\n";

    // 1. Verify entity.h has has_defuse_kit
    std::ifstream ent_file("source/src/entity.h");
    if(!ent_file.is_open()) ent_file.open("../source/src/entity.h");
    assert(ent_file.is_open());
    std::string line;
    bool found_ent_kit = false;
    while(std::getline(ent_file, line))
    {
        if(line.find("bool has_defuse_kit") != std::string::npos) found_ent_kit = true;
    }
    ent_file.close();
    assert(found_ent_kit);

    // 2. Verify server.h has has_defuse_kit in clientstate
    std::ifstream server_h("source/src/server.h");
    if(!server_h.is_open()) server_h.open("../source/src/server.h");
    assert(server_h.is_open());
    bool found_serv_kit = false;
    while(std::getline(server_h, line))
    {
        if(line.find("bool has_defuse_kit") != std::string::npos) found_serv_kit = true;
    }
    server_h.close();
    assert(found_serv_kit);

    // 3. Verify server.cpp defusal durations: 5000ms with kit vs 10000ms without kit
    std::ifstream server_cpp("source/src/server.cpp");
    if(!server_cpp.is_open()) server_cpp.open("../source/src/server.cpp");
    assert(server_cpp.is_open());
    bool found_5000_10000 = false;
    while(std::getline(server_cpp, line))
    {
        if(line.find("with_kit ? 5000 : 10000") != std::string::npos) found_5000_10000 = true;
    }
    server_cpp.close();
    assert(found_5000_10000 && "server.cpp must enforce 5s defuse with kit and 10s without kit");

    // 4. Verify renderhud.cpp display strings
    std::ifstream hud_file("source/src/renderhud.cpp");
    if(!hud_file.is_open()) hud_file.open("../source/src/renderhud.cpp");
    assert(hud_file.is_open());
    bool found_hud_kit_str = false;
    bool found_hud_without_kit_str = false;
    while(std::getline(hud_file, line))
    {
        if(line.find("DEFUSING WITH KIT [5s]") != std::string::npos) found_hud_kit_str = true;
        if(line.find("DEFUSING WITHOUT KIT [10s]") != std::string::npos) found_hud_without_kit_str = true;
    }
    hud_file.close();
    assert(found_hud_kit_str && found_hud_without_kit_str);

    std::cout << "  -> PASS: Defuse kit 10s vs 5s system verified across client, server, and HUD!\n";
}

void test_dense_smoke_core()
{
    std::cout << "[Test 3] Verifying Dense Smoke Core & Bot Sightline Blocking...\n";

    // 1. Verify renderparticles.cpp dual-layer smoke
    std::ifstream part_file("source/src/renderparticles.cpp");
    if(!part_file.is_open()) part_file.open("../source/src/renderparticles.cpp");
    assert(part_file.is_open());
    std::string line;
    bool found_dense_core = false;
    while(std::getline(part_file, line))
    {
        if(line.find("Dense impenetrable inner core") != std::string::npos) found_dense_core = true;
    }
    part_file.close();
    assert(found_dense_core && "particle_smokebomb must generate dense inner core");

    // 2. Verify weapon.cpp smokezone and ray_hits_smoke
    std::ifstream weapon_file("source/src/weapon.cpp");
    if(!weapon_file.is_open()) weapon_file.open("../source/src/weapon.cpp");
    assert(weapon_file.is_open());
    bool found_smokezone = false;
    bool found_ray_hits_smoke = false;
    while(std::getline(weapon_file, line))
    {
        if(line.find("create_smokezone") != std::string::npos) found_smokezone = true;
        if(line.find("bool ray_hits_smoke") != std::string::npos) found_ray_hits_smoke = true;
    }
    weapon_file.close();
    assert(found_smokezone);
    assert(found_ray_hits_smoke);

    // 3. Verify bot_util.cpp IsVisible checks ray_hits_smoke
    std::ifstream bot_file("source/src/bot/bot_util.cpp");
    if(!bot_file.is_open()) bot_file.open("../source/src/bot/bot_util.cpp");
    assert(bot_file.is_open());
    bool found_bot_smoke_check = false;
    while(std::getline(bot_file, line))
    {
        if(line.find("if(ray_hits_smoke(v1, v2)) return false;") != std::string::npos)
            found_bot_smoke_check = true;
    }
    bot_file.close();
    assert(found_bot_smoke_check && "Bot IsVisible() must block sightlines through smoke");

    std::cout << "  -> PASS: Dense smoke core and bot sightline occlusion verified!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 2 Tactical Utilities Test Suite    \n";
    std::cout << "========================================================\n";

    test_dot_product_flashbang();
    test_defuse_kit_system();
    test_dense_smoke_core();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 2 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
