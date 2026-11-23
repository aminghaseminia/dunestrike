// tests/test_tagging_movement.cpp
// Unit and integration tests for CS-style tagging and movement penalty on damage.

#include <cassert>
#include <cmath>
#include <iostream>

#define TAGMILLIS 450
#define TAG_HEAVY 0.42f
#define TAG_MEDIUM 0.48f
#define TAG_LIGHT 0.58f

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

struct MockPlayer {
    float vx, vy, vz;
    bool onfloor;
    int taguntil;
    float tagscale;

    MockPlayer() : vx(10.0f), vy(10.0f), vz(0.0f), onfloor(true), taguntil(0), tagscale(1.0f) {}

    void apply_tagging(int gun, int curtime_millis) {
        if (gun == GUN_GRENADE) return;

        float tagmult = TAG_MEDIUM;
        switch (gun) {
            case GUN_SNIPER:
            case GUN_ASSAULT:
            case GUN_SHOTGUN:
                tagmult = TAG_HEAVY;
                break;
            case GUN_PISTOL:
            case GUN_AKIMBO:
                tagmult = TAG_LIGHT;
                break;
            default:
                tagmult = TAG_MEDIUM;
                break;
        }

        vx *= tagmult;
        vy *= tagmult;

        if (!onfloor) {
            vx *= 0.7f;
            vy *= 0.7f;
            if (vz > 0.0f) vz = -0.1f;
        }

        taguntil = curtime_millis + TAGMILLIS;
        tagscale = tagmult;
    }

    float get_speed_multiplier(int curtime_millis) const {
        if (taguntil <= curtime_millis) return 1.0f;
        float progress = 1.0f - float(taguntil - curtime_millis) / float(TAGMILLIS);
        if (progress < 0.0f) progress = 0.0f;
        else if (progress > 1.0f) progress = 1.0f;
        return tagscale + (1.0f - tagscale) * progress;
    }
};

void test_ground_tagging_heavy() {
    MockPlayer p;
    p.vx = 10.0f; p.vy = 0.0f; p.onfloor = true;
    p.apply_tagging(GUN_ASSAULT, 1000);

    // Initial instantaneous velocity cut
    assert(std::fabs(p.vx - 4.2f) < 0.001f);
    assert(p.taguntil == 1000 + TAGMILLIS);
    assert(std::fabs(p.tagscale - TAG_HEAVY) < 0.001f);

    // Initial speed multiplier at t = 1000ms
    float speed_0 = p.get_speed_multiplier(1000);
    assert(std::fabs(speed_0 - TAG_HEAVY) < 0.001f);

    // Mid recovery at t = 1000 + 225ms
    float speed_mid = p.get_speed_multiplier(1225);
    float expected_mid = TAG_HEAVY + (1.0f - TAG_HEAVY) * 0.5f;
    assert(std::fabs(speed_mid - expected_mid) < 0.01f);

    // Full recovery at t = 1000 + TAGMILLIS
    float speed_end = p.get_speed_multiplier(1450);
    assert(std::fabs(speed_end - 1.0f) < 0.001f);

    // After recovery window
    float speed_after = p.get_speed_multiplier(1500);
    assert(std::fabs(speed_after - 1.0f) < 0.001f);

    std::cout << "[PASS] test_ground_tagging_heavy passed.\n";
}

void test_ground_tagging_light_and_medium() {
    MockPlayer p1;
    p1.apply_tagging(GUN_PISTOL, 2000);
    assert(std::fabs(p1.tagscale - TAG_LIGHT) < 0.001f);
    assert(std::fabs(p1.vx - 5.8f) < 0.001f);

    MockPlayer p2;
    p2.apply_tagging(GUN_CARBINE, 2000);
    assert(std::fabs(p2.tagscale - TAG_MEDIUM) < 0.001f);
    assert(std::fabs(p2.vx - 4.8f) < 0.001f);

    std::cout << "[PASS] test_ground_tagging_light_and_medium passed.\n";
}

void test_airborne_tagging_and_vertical_arrest() {
    MockPlayer p;
    p.vx = 10.0f; p.vy = 0.0f; p.vz = 5.0f; // jumping upwards
    p.onfloor = false;

    p.apply_tagging(GUN_SNIPER, 3000);

    // Air penalty: vx = 10.0 * TAG_HEAVY * 0.7 = 4.2 * 0.7 = 2.94
    assert(std::fabs(p.vx - 2.94f) < 0.01f);

    // Upward vertical velocity arrested
    assert(std::fabs(p.vz - (-0.1f)) < 0.001f);

    std::cout << "[PASS] test_airborne_tagging_and_vertical_arrest passed.\n";
}

void test_grenade_no_tagging() {
    MockPlayer p;
    p.vx = 10.0f; p.vy = 10.0f;
    p.apply_tagging(GUN_GRENADE, 4000);

    // Grenades use explosion physics / hitpush, not bullet tagging
    assert(p.vx == 10.0f);
    assert(p.vy == 10.0f);
    assert(p.taguntil == 0);
    assert(p.get_speed_multiplier(4000) == 1.0f);

    std::cout << "[PASS] test_grenade_no_tagging passed.\n";
}

void test_repeated_hits_refresh() {
    MockPlayer p;
    p.apply_tagging(GUN_SUBGUN, 1000);
    assert(p.taguntil == 1450);

    // Shot again at 1200ms
    p.apply_tagging(GUN_ASSAULT, 1200);
    assert(p.taguntil == 1650);
    assert(std::fabs(p.tagscale - TAG_HEAVY) < 0.001f);

    std::cout << "[PASS] test_repeated_hits_refresh passed.\n";
}

int main() {
    std::cout << "Starting CS Tagging & Movement Penalty tests...\n";
    test_ground_tagging_heavy();
    test_ground_tagging_light_and_medium();
    test_airborne_tagging_and_vertical_arrest();
    test_grenade_no_tagging();
    test_repeated_hits_refresh();
    std::cout << "All CS Tagging & Movement Penalty tests passed successfully!\n";
    return 0;
}
