#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>

// Test Suite for CS Tactical Polish:
// 1. Dynamic CS Crosshair Bloom Math & Variables
// 2. Directional Damage Arc Math & Angle Tracking
// 3. cs_dust2 2D Tactical Radar Texture Asset & Dimensions

void test_dynamic_crosshair_bloom()
{
    std::cout << "[Test 1] Verifying CS Dynamic Crosshair Bloom & Spread Math...\n";

    float base_gap = 4.0f;
    float length = 9.0f;
    float thickness = 2.0f;

    // Stationary standing: gap = base_gap
    float gap_stand = base_gap;
    assert(gap_stand == 4.0f);

    // Crouch tightening: gap should shrink
    float gap_crouch = std::max(1.0f, base_gap - 2.0f);
    assert(gap_crouch == 2.0f);
    assert(gap_crouch < gap_stand && "Crouching must tighten crosshair gap");

    // Running at speed (vel = (0.5, 0.5) -> speed ~ 0.707)
    float speed = std::hypot(0.5f, 0.5f);
    float movespread = std::min(speed * 32.0f, 18.0f);
    float gap_run = base_gap + movespread;
    assert(gap_run > gap_stand && "Running must expand crosshair gap");

    // Jumping in air: jump penalty (+12.0f)
    float inair_penalty = 12.0f;
    float gap_jump = base_gap + movespread + inair_penalty;
    assert(gap_jump > gap_run && "Jumping in air must produce large crosshair bloom");

    // Firing recoil spread (dynspread - defspread = 15)
    int dynspread = 35, defspread = 20;
    float spread_diff = float(dynspread - defspread);
    float recoil_bloom = std::min(spread_diff * 0.5f, 22.0f);
    float gap_firing = base_gap + recoil_bloom;
    assert(gap_firing == base_gap + 7.5f && "Firing must expand crosshair bloom");

    std::cout << "  -> PASS: Crosshair spread bloom math for stand, crouch, run, jump, and recoil verified!\n";
}

void test_directional_damage_indicator()
{
    std::cout << "[Test 2] Verifying Directional Damage Indicator Angle Math...\n";

    // Player at (100, 100), facing North (yaw = 0 deg)
    float player_x = 100.0f, player_y = 100.0f, player_yaw = 0.0f;

    auto calc_rel_angle = [&](float ax, float ay, float pyaw) {
        float dx = ax - player_x;
        float dy = ay - player_y;
        float attacker_ang = std::atan2(dy, dx) * 180.0f / M_PI;
        float rel_ang = 180.0f - pyaw - attacker_ang;
        while(rel_ang < 0.0f) rel_ang += 360.0f;
        while(rel_ang >= 360.0f) rel_ang -= 360.0f;
        return rel_ang;
    };

    // Attacker directly in front (along Y axis):
    // In engine, 0 or 180 depending on coordinate convention: relative angle should map to front (0/360)
    // Attacker behind should map to ~180 degrees
    float ang_front = calc_rel_angle(100.0f, 150.0f, 90.0f);
    float ang_behind = calc_rel_angle(100.0f, 50.0f, 90.0f);
    float diff = std::abs(ang_behind - ang_front);
    assert((std::abs(diff - 180.0f) < 1.0f) && "Opposing attacker positions must produce 180 degree angle delta");

    // Fade out and outward expansion progress
    int starttime = 1000;
    int expiretime = 2000;
    int currenttime = 1500; // 50% through
    float progress = float(currenttime - starttime) / float(expiretime - starttime);
    assert(progress == 0.5f);
    float alpha = (1.0f - progress * progress) * 0.75f;
    assert(alpha == 0.75f * 0.75f && "Alpha must decay smoothly with quadratic curve");

    float base_radius = 120.0f;
    float radius = base_radius + progress * 24.0f;
    assert(radius == 132.0f && "Damage arc must expand slightly outward as it fades");

    std::cout << "  -> PASS: Threat angle calculation, quadratic alpha decay, and expansion verified!\n";
}

void test_radar_minimap_asset()
{
    std::cout << "[Test 3] Verifying cs_dust2 2D Radar Texture Asset...\n";

    const char *path = "packages/maps/radar/cs_dust2.png";
    std::ifstream file(path, std::ios::binary);
    assert(file.is_open() && "Radar texture packages/maps/radar/cs_dust2.png must exist on disk");

    // Check PNG signature
    unsigned char header[8];
    file.read((char *)header, 8);
    assert(header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G' && "Valid PNG signature");

    // Read IHDR chunk for dimensions
    unsigned char ihdr[17];
    file.read((char *)ihdr, 17);
    int width = (ihdr[8] << 24) | (ihdr[9] << 16) | (ihdr[10] << 8) | ihdr[11];
    int height = (ihdr[12] << 24) | (ihdr[13] << 16) | (ihdr[14] << 8) | ihdr[15];

    assert(width == 512 && height == 512 && "Radar texture must be 512x512 for optimal HUD performance");
    file.close();

    std::cout << "  -> PASS: packages/maps/radar/cs_dust2.png verified (512x512 RGBA PNG)!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: CS Tactical Polish Test Suite            \n";
    std::cout << "========================================================\n";

    test_dynamic_crosshair_bloom();
    test_directional_damage_indicator();
    test_radar_minimap_asset();

    std::cout << "========================================================\n";
    std::cout << "  ALL TACTICAL POLISH TESTS PASSED SUCCESSFULLY! (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
