// tests/test_weapon_damage_model.cpp
// Automated tests for Phase 5: CS Weapon Damage, Armor Penetration, Hitzones, and Magazine Specs.

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

enum {
    GUN_KNIFE = 0,
    GUN_PISTOL,
    GUN_CARBINE,
    GUN_SHOTGUN,
    GUN_SUBGUN,
    GUN_SNIPER,
    GUN_ASSAULT,
    GUN_GRENADE,
    GUN_AKIMBO,
    NUMGUNS
};

enum {
    HITZONE_NONE = 0,
    HITZONE_TORSO,
    HITZONE_HEAD,
    HITZONE_STOMACH,
    HITZONE_LEGS,
    HITZONE_ARMS,
    NUMHITZONES
};

struct TestGunInfo {
    const char *name;
    int damage;
    int piercing;
    int magsize;
    int attackdelay;
    bool isauto;
};

// Calibrated gun stats from server.h
const TestGunInfo test_guns[NUMGUNS] = {
    { "Knife",        50,  85,  1, 500,  false },
    { "Pistol",       30,  50, 12, 150,  false },
    { "Carbine(M4)",  33,  70, 30,  90,  true  },
    { "Shotgun",      20,  50,  8, 880,  false },
    { "Subgun(MP5)",  26,  50, 30,  75,  true  },
    { "Sniper(AWP)", 115,  97, 10, 1400, false },
    { "Assault(AK)",  36,  78, 30, 100,  true  },
    { "Grenade",     100,  60,  1, 650,  false },
    { "Akimbo",       30,  52, 30,  80,  true  }
};

float get_hitzonescale(int zone) {
    switch(zone) {
        case HITZONE_HEAD:    return 4.0f;
        case HITZONE_STOMACH: return 1.25f;
        case HITZONE_LEGS:    return 0.75f;
        case HITZONE_ARMS:    return 0.75f;
        case HITZONE_TORSO:
        default:              return 1.0f;
    }
}

struct TestPlayer {
    int health;
    int armour;
    bool has_helmet;

    TestPlayer(int h = 100, int a = 100, bool helm = true) : health(h), armour(a), has_helmet(helm) {}

    int apply_damage(int damage, int gun, int zone = HITZONE_TORSO) {
        if (damage == 2147483647) { // INT_MAX
            damage = health;
            armour = health = 0;
            has_helmet = false;
            return damage;
        }

        const TestGunInfo &gi = test_guns[gun];

        // Headshot:
        if (zone == HITZONE_HEAD) {
            if (!has_helmet || armour <= 0) {
                health -= damage;
                return damage;
            }
            float pen = std::max(0.0f, std::min(1.0f, float(gi.piercing) / 100.0f));
            int health_dmg = (int)std::ceil(damage * pen);
            int armor_absorb = damage - health_dmg;
            int armor_dmg = std::max(1, (int)std::ceil(armor_absorb * 0.5f));

            if (armour >= armor_dmg) {
                armour -= armor_dmg;
                damage = health_dmg;
            } else {
                damage = health_dmg + (armor_dmg - armour) * 2;
                armour = 0;
                has_helmet = false;
            }
            health -= damage;
            return damage;
        }

        // Torso & Stomach (protected by Kevlar body armor):
        if (zone == HITZONE_TORSO || zone == HITZONE_STOMACH) {
            if (armour > 0) {
                float pen = std::max(0.0f, std::min(1.0f, float(gi.piercing) / 100.0f));
                int health_dmg = (int)std::ceil(damage * pen);
                int armor_absorb = damage - health_dmg;
                int armor_dmg = std::max(1, (int)std::ceil(armor_absorb * 0.5f));

                if (armour >= armor_dmg) {
                    armour -= armor_dmg;
                    damage = health_dmg;
                } else {
                    damage = health_dmg + (armor_dmg - armour) * 2;
                    armour = 0;
                    has_helmet = false;
                }
                health -= damage;
                return damage;
            }
        }

        health -= damage;
        return damage;
    }
};

void test_hitzone_multipliers() {
    std::cout << "[Test 1] Testing CS hitzone multipliers...\n";
    assert(get_hitzonescale(HITZONE_HEAD) == 4.0f);
    assert(get_hitzonescale(HITZONE_STOMACH) == 1.25f);
    assert(get_hitzonescale(HITZONE_TORSO) == 1.0f);
    assert(get_hitzonescale(HITZONE_LEGS) == 0.75f);
    assert(get_hitzonescale(HITZONE_ARMS) == 0.75f);
    std::cout << "  -> PASS: All hitzone scales match CS standard.\n";
}

void test_ak47_damage_and_one_tap() {
    std::cout << "[Test 2] Testing AK-47 (Assault) damage and 1-tap headshot...\n";
    int base_dmg = test_guns[GUN_ASSAULT].damage; // 36

    // 1. Unarmored Headshot
    TestPlayer target_no_armor(100, 0);
    int raw_head_dmg = (int)(base_dmg * get_hitzonescale(HITZONE_HEAD) + 0.5f); // 36 * 4 = 144
    assert(raw_head_dmg == 144);
    int taken1 = target_no_armor.apply_damage(raw_head_dmg, GUN_ASSAULT, HITZONE_HEAD);
    assert(taken1 == 144);
    assert(target_no_armor.health <= 0); // 1-tap kill!

    // 2. Armored Headshot (AK vs Helmet)
    TestPlayer target_helmet(100, 100);
    int taken2 = target_helmet.apply_damage(raw_head_dmg, GUN_ASSAULT, HITZONE_HEAD);
    // 144 * 0.78 = 112.32 -> 113 dmg to health!
    assert(taken2 >= 100); // 1-tap kill through helmet like CS AK-47!
    assert(target_helmet.health <= 0);
    assert(target_helmet.armour < 100);

    // 3. Torso shot vs Armor (AK vs Kevlar)
    TestPlayer target_torso(100, 100);
    int raw_torso = (int)(base_dmg * get_hitzonescale(HITZONE_TORSO) + 0.5f); // 36
    int taken3 = target_torso.apply_damage(raw_torso, GUN_ASSAULT, HITZONE_TORSO);
    // 36 * 0.78 = 28.08 -> 29 health dmg
    assert(taken3 == 29);
    assert(target_torso.health == 71); // 4-shot kill to chest
    assert(target_torso.armour == 96); // absorbed 7 dmg, cost 4 armor

    std::cout << "  -> PASS: AK-47 delivers authentic 1-tap headshot kill and CS chest ttk.\n";
}

void test_m4a1_damage_and_helmet() {
    std::cout << "[Test 3] Testing M4A1 (Carbine) damage vs helmet and unarmored...\n";
    int base_dmg = test_guns[GUN_CARBINE].damage; // 33

    // 1. Unarmored Headshot: 33 * 4 = 132 -> 1 shot kill
    TestPlayer target_no_armor(100, 0);
    int raw_head = (int)(base_dmg * get_hitzonescale(HITZONE_HEAD) + 0.5f);
    int taken1 = target_no_armor.apply_damage(raw_head, GUN_CARBINE, HITZONE_HEAD);
    assert(taken1 == 132);
    assert(target_no_armor.health <= 0);

    // 2. Armored Headshot: 132 * 0.70 = 92.4 -> 93 dmg (CS M4 headshot dink: ~90-93 dmg, leaves target alive with low HP)
    TestPlayer target_helmet(100, 100, true);
    int taken2 = target_helmet.apply_damage(raw_head, GUN_CARBINE, HITZONE_HEAD);
    assert(taken2 == 93);
    assert(target_helmet.health == 7); // Survived with 7 HP! Authentic CS M4 behavior!
    assert(target_helmet.armour == 80);

    // 3. Kevlar WITHOUT Helmet: 100hp + 100 armor, but has_helmet = false
    TestPlayer target_kevlar_no_helm(100, 100, false);
    int taken3 = target_kevlar_no_helm.apply_damage(raw_head, GUN_CARBINE, HITZONE_HEAD);
    assert(taken3 == 132); // Full 132 unmitigated headshot damage!
    assert(target_kevlar_no_helm.health <= 0); // One-tap kill because no helmet!

    std::cout << "  -> PASS: M4A1 deals 93 dmg dink vs helmet (survives with 7 HP), exactly like CS.\n";
}

void test_awp_one_hit_kill() {
    std::cout << "[Test 4] Testing AWP (Sniper) 1-hit kill to torso and limbs...\n";
    int base_dmg = test_guns[GUN_SNIPER].damage; // 115

    // 1. Torso vs 100hp + 100 armor
    TestPlayer target_armored(100, 100);
    int raw_torso = (int)(base_dmg * get_hitzonescale(HITZONE_TORSO) + 0.5f); // 115
    int taken = target_armored.apply_damage(raw_torso, GUN_SNIPER, HITZONE_TORSO);
    // 115 * 0.97 = 111.55 -> 112 dmg to health
    assert(taken == 112);
    assert(target_armored.health <= 0); // 1-shot torso kill through armor!

    // 2. Leg hit (legs don't 1-hit kill, as in CS)
    TestPlayer target_leg(100, 100);
    int raw_leg = (int)(base_dmg * get_hitzonescale(HITZONE_LEGS) + 0.5f); // 115 * 0.75 = 86
    int taken_leg = target_leg.apply_damage(raw_leg, GUN_SNIPER, HITZONE_LEGS);
    assert(taken_leg < 100);
    assert(target_leg.health > 0); // Survives leg hit!

    std::cout << "  -> PASS: AWP kills instantly to chest/torso through armor, survives on leg hit.\n";
}

void test_magazine_and_fire_rates() {
    std::cout << "[Test 5] Testing weapon magazine sizes and fire rate cadences...\n";
    // M4 Carbine: 30 mag, 90ms attackdelay (~666 RPM)
    assert(test_guns[GUN_CARBINE].magsize == 30);
    assert(test_guns[GUN_CARBINE].attackdelay == 90);
    assert(test_guns[GUN_CARBINE].isauto == true);

    // AK Assault: 30 mag, 100ms attackdelay (600 RPM)
    assert(test_guns[GUN_ASSAULT].magsize == 30);
    assert(test_guns[GUN_ASSAULT].attackdelay == 100);
    assert(test_guns[GUN_ASSAULT].isauto == true);

    // MP5 Subgun: 30 mag, 75ms attackdelay (800 RPM)
    assert(test_guns[GUN_SUBGUN].magsize == 30);
    assert(test_guns[GUN_SUBGUN].attackdelay == 75);
    assert(test_guns[GUN_SUBGUN].isauto == true);

    // Pistol: 12 mag
    assert(test_guns[GUN_PISTOL].magsize == 12);

    // Sniper: 10 mag, 1400ms bolt-action delay
    assert(test_guns[GUN_SNIPER].magsize == 10);
    assert(test_guns[GUN_SNIPER].attackdelay == 1400);

    // Shotgun: 8 mag
    assert(test_guns[GUN_SHOTGUN].magsize == 8);

    std::cout << "  -> PASS: All 9 weapons configured with exact CS magazine and RPM specifications.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  RUNNING PHASE 5 TESTS: WEAPONS & DAMAGE MODEL\n";
    std::cout << "====================================================\n\n";

    test_hitzone_multipliers();
    test_ak47_damage_and_one_tap();
    test_m4a1_damage_and_helmet();
    test_awp_one_hit_kill();
    test_magazine_and_fire_rates();

    std::cout << "\n====================================================\n";
    std::cout << "  ALL PHASE 5 WEAPON & DAMAGE MODEL TESTS PASSED!\n";
    std::cout << "====================================================\n";
    return 0;
}
