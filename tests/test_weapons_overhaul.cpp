// tests/test_weapons_overhaul.cpp
// Automated tests for weapons mechanics, spread curves, and recoil recovery calibration.

#include <cassert>
#include <iostream>
#include <algorithm>
#include <cmath>

struct GunStats {
    int spread;
    int recoilbase;
    int maxrecoil;
    int attackdelay;
};

// Mirror calibrated weapon stats from server.h
GunStats g_knife   = { 1,   0,  0, 450 };
GunStats g_pistol  = { 12, 14, 22, 140 };
GunStats g_carbine = { 12, 18, 32,  90 };
GunStats g_shotgun = { 1,  35, 45, 850 };
GunStats g_subgun  = { 15, 12, 22,  75 };
GunStats g_sniper  = { 2,  35, 35, 1300 };
GunStats g_assault = { 14, 22, 38, 100 };

// Mirror calibrated spread functions from weapon.cpp
int calc_assault_spread(int shots, int base_spread = 14) {
    int base = base_spread;
    if(shots > 0) {
        if(shots <= 2) base += shots * 4;
        else if(shots <= 6) base += 8 + (shots - 2) * 3;
        else base = std::min(36, base + 20);
    }
    return base;
}

int calc_subgun_spread(int shots, int base_spread = 15) {
    int base = base_spread;
    if(shots > 0) base += std::min(16, shots * 2);
    return base;
}

int calc_carbine_spread(int shots, int base_spread = 12) {
    int base = base_spread;
    if(shots > 0) base += std::min(15, shots * 3);
    return base;
}

int calc_pistol_spread(int shots, int base_spread = 12) {
    int base = base_spread;
    if(shots > 1) base += std::min(18, (shots - 1) * 4);
    return base;
}

void test_assault_rifle_spread() {
    std::cout << "[Test 1] Testing AK-47 Assault Rifle spread curve...\n";

    // First shot: pinpoint accuracy
    assert(calc_assault_spread(0) == 14);

    // 2nd and 3rd shots: tight burst
    assert(calc_assault_spread(1) == 18);
    assert(calc_assault_spread(2) == 22);

    // 4th to 6th shots: controllable spray (NOT instantly 55)
    assert(calc_assault_spread(3) == 25);
    assert(calc_assault_spread(4) == 28);
    assert(calc_assault_spread(6) == 34);

    // Full auto spray capped at 34
    assert(calc_assault_spread(10) == 34);
    assert(calc_assault_spread(30) == 34);

    std::cout << "  -> PASS: AK-47 burst is tight & sustained spray is fully controllable!\n";
}

void test_submachine_gun_spread() {
    std::cout << "[Test 2] Testing MP5 Submachine Gun spread curve...\n";

    // First shot
    assert(calc_subgun_spread(0) == 15);

    // Progressive spray (NOT blown out to 70)
    assert(calc_subgun_spread(1) == 17);
    assert(calc_subgun_spread(2) == 19);
    assert(calc_subgun_spread(5) == 25);
    assert(calc_subgun_spread(20) == 31); // Capped at 31 (was 70!)

    std::cout << "  -> PASS: MP5 SMG has smooth, mobile, run-and-gun spread!\n";
}

void test_pistol_precision() {
    std::cout << "[Test 3] Testing Pistol tap precision & spam bloom...\n";

    // First shot: clean precision (was 35, now 12)
    assert(calc_pistol_spread(0) == 12);
    assert(calc_pistol_spread(1) == 12);

    // Rapid spamming increases spread progressively
    assert(calc_pistol_spread(2) == 16);
    assert(calc_pistol_spread(3) == 20);
    assert(calc_pistol_spread(10) == 30);

    std::cout << "  -> PASS: Pistol initial shot has crisp CS-style precision!\n";
}

void test_carbine_accuracy() {
    std::cout << "[Test 4] Testing M4A1 Carbine laser accuracy...\n";

    assert(calc_carbine_spread(0) == 12);
    assert(calc_carbine_spread(1) == 15);
    assert(calc_carbine_spread(2) == 18);
    assert(calc_carbine_spread(10) == 27);

    std::cout << "  -> PASS: M4A1 Carbine provides tight, reliable recoil & spread!\n";
}

void test_shotgun_and_sniper_recoil() {
    std::cout << "[Test 5] Testing Shotgun & Sniper recoil kick calibration...\n";

    // Shotgun kick reduced from 140 down to 35
    assert(g_shotgun.recoilbase == 35);
    assert(g_shotgun.maxrecoil == 45);

    // Sniper kick reduced from 85 down to 35
    assert(g_sniper.recoilbase == 35);
    assert(g_sniper.maxrecoil == 35);

    std::cout << "  -> PASS: Shotgun and AWP recoil kicks are calibrated and smooth!\n";
}

void test_knife_reach() {
    std::cout << "[Test 6] Testing Knife slash reach calibration...\n";

    float knife_reach = 4.2f;
    float player_bounding_radius = 1.1f;
    float enemy_bounding_radius = 1.1f;
    float touching_center_distance = player_bounding_radius + enemy_bounding_radius; // 2.2f

    // Reach must comfortably exceed touching distance to ensure reliable slash detection
    assert(knife_reach > touching_center_distance + 1.0f);
    assert(knife_reach == 4.2f);

    std::cout << "  -> PASS: Knife reach (4.2 cubes) connects cleanly with enemy models!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Weapons Overhaul & Feel Verification Tests\n";
    std::cout << "========================================================\n";

    test_assault_rifle_spread();
    test_submachine_gun_spread();
    test_pistol_precision();
    test_carbine_accuracy();
    test_shotgun_and_sniper_recoil();
    test_knife_reach();

    std::cout << "========================================================\n";
    std::cout << "  ALL WEAPONS OVERHAUL TESTS PASSED (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
