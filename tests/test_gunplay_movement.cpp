#include <cstdio>
#include <cmath>
#include <cassert>
#include <algorithm>

// Unit test suite for Phase 3: Gunplay & Movement Inaccuracy

struct MockPlayer {
    bool onfloor;
    int timeinair;
    bool crouching;
    float vel_x, vel_y;
    int move;
    int strafe;

    MockPlayer() {
        onfloor = true;
        timeinair = 0;
        crouching = false;
        vel_x = vel_y = 0.0f;
        move = strafe = 0;
    }

    float speed() const {
        return sqrtf(vel_x * vel_x + vel_y * vel_y);
    }
};

int calculate_movement_spread(const MockPlayer &player, int base_spread) {
    float sp = (float)base_spread;

    // 1. Jumping / in air
    if(!player.onfloor && player.timeinair > 50) {
        sp = sp * 2.2f + 35.0f;
    } else {
        float spd = player.speed();
        bool is_moving = (spd > 0.08f || player.move != 0 || player.strafe != 0);

        if(is_moving) {
            if(player.crouching) {
                // Crouch-walking
                sp = sp * 1.15f + spd * 12.0f;
            } else {
                // Running
                sp = sp * 1.65f + spd * 25.0f;
            }
        } else {
            // Standing or crouching still
            if(player.crouching) {
                // Crouch accuracy bonus
                sp = sp * 0.75f;
            }
        }
    }

    return std::max(1, (int)sp);
}

int calculate_sniper_spread(const MockPlayer &player, bool scoped, int settle_time_ms, int base_spread) {
    if(scoped) {
        if(settle_time_ms <= 180) {
            int settle = std::max((base_spread * (180 - settle_time_ms)) / 180, 1);
            return calculate_movement_spread(player, settle);
        }
        if(!player.onfloor && player.timeinair > 50) return 60;
        float spd = player.speed();
        if(spd > 0.08f || player.move != 0 || player.strafe != 0) {
            return std::max(1, (int)(spd * 40.0f));
        }
        return 1;
    }
    return calculate_movement_spread(player, base_spread * 2);
}

float calculate_crosshair_scale(int current_spread, int base_spread) {
    float ratio = float(current_spread) / float(std::max(1, base_spread));
    if(ratio < 0.75f) ratio = 0.75f;
    if(ratio > 2.2f) ratio = 2.2f;
    return ratio;
}

int main() {
    printf("====================================================\n");
    printf("  RUNNING PHASE 3 TESTS: GUNPLAY & MOVEMENT SPREAD\n");
    printf("====================================================\n\n");

    const int RIFLE_BASE = 15;

    // Test 1: Standing still accuracy (baseline)
    printf("[Test 1] Standing still accuracy...\n");
    MockPlayer p_still;
    int sp_still = calculate_movement_spread(p_still, RIFLE_BASE);
    assert(sp_still == RIFLE_BASE);
    printf("  -> Standing still spread: %d (matches base %d). PASS\n", sp_still, RIFLE_BASE);

    // Test 2: Crouching still accuracy bonus
    printf("[Test 2] Crouching still accuracy bonus...\n");
    MockPlayer p_crouch_still;
    p_crouch_still.crouching = true;
    int sp_crouch = calculate_movement_spread(p_crouch_still, RIFLE_BASE);
    assert(sp_crouch < sp_still);
    assert(sp_crouch == (int)(RIFLE_BASE * 0.75f));
    printf("  -> Crouching still spread: %d (tightened from %d by 25%%). PASS\n", sp_crouch, sp_still);

    // Test 3: Crouch-walking slight penalty
    printf("[Test 3] Crouch-walking accuracy...\n");
    MockPlayer p_crouch_walk;
    p_crouch_walk.crouching = true;
    p_crouch_walk.move = 1;
    p_crouch_walk.vel_x = 0.35f;
    int sp_cwalk = calculate_movement_spread(p_crouch_walk, RIFLE_BASE);
    assert(sp_cwalk > sp_crouch); // more spread than crouching still
    printf("  -> Crouch-walking spread: %d. PASS\n", sp_cwalk);

    // Test 4: Running full speed inaccuracy penalty
    printf("[Test 4] Full running inaccuracy penalty...\n");
    MockPlayer p_run;
    p_run.move = 1;
    p_run.vel_x = 0.95f;
    int sp_run = calculate_movement_spread(p_run, RIFLE_BASE);
    assert(sp_run > sp_still * 2); // over 2x less accurate than standing still
    assert(sp_run > sp_cwalk);    // significantly more spread than crouch-walking
    printf("  -> Full running spread: %d (compared to still %d). PASS\n", sp_run, sp_still);

    // Test 5: Jumping / In-air bloom penalty
    printf("[Test 5] Jumping / in-air spread bloom...\n");
    MockPlayer p_jump;
    p_jump.onfloor = false;
    p_jump.timeinair = 150;
    int sp_jump = calculate_movement_spread(p_jump, RIFLE_BASE);
    assert(sp_jump > sp_run); // jumping is even worse than running
    assert(sp_jump >= (int)(RIFLE_BASE * 2.2f + 35.0f));
    printf("  -> In-air jumping spread: %d. PASS\n", sp_jump);

    // Test 6: Sniper Rifle mechanics
    printf("[Test 6] Sniper Rifle movement & scope mechanics...\n");
    const int SNIPER_BASE = 50;

    // Scoped & still -> laser accurate
    int snp_scoped_still = calculate_sniper_spread(p_still, true, 200, SNIPER_BASE);
    assert(snp_scoped_still == 1);
    printf("  -> Scoped & still spread: %d (pin-point). PASS\n", snp_scoped_still);

    // Scoped & moving -> moving penalty
    int snp_scoped_moving = calculate_sniper_spread(p_run, true, 200, SNIPER_BASE);
    assert(snp_scoped_moving > 25);
    printf("  -> Scoped & moving spread: %d (punishes run-and-gun). PASS\n", snp_scoped_moving);

    // Scoped & jumping -> jump penalty
    int snp_scoped_jump = calculate_sniper_spread(p_jump, true, 200, SNIPER_BASE);
    assert(snp_scoped_jump == 60);
    printf("  -> Scoped & jumping spread: %d. PASS\n", snp_scoped_jump);

    // No-scope -> classic penalty
    int snp_noscope = calculate_sniper_spread(p_still, false, 0, SNIPER_BASE);
    assert(snp_noscope == SNIPER_BASE * 2);
    printf("  -> No-scope spread: %d. PASS\n", snp_noscope);

    // Test 7: Dynamic Crosshair visual expansion
    printf("[Test 7] Dynamic Crosshair scaling...\n");
    float scale_still = calculate_crosshair_scale(sp_still, RIFLE_BASE);
    float scale_crouch = calculate_crosshair_scale(sp_crouch, RIFLE_BASE);
    float scale_run = calculate_crosshair_scale(sp_run, RIFLE_BASE);
    float scale_jump = calculate_crosshair_scale(sp_jump, RIFLE_BASE);

    assert(scale_still == 1.0f);
    assert(scale_crouch < 1.0f); // contracts
    assert(scale_run > 1.5f);    // expands
    assert(scale_jump >= 2.2f);  // maximum bloom

    printf("  -> Crosshair still scale:   %.2fx\n", scale_still);
    printf("  -> Crosshair crouch scale:  %.2fx (tightens)\n", scale_crouch);
    printf("  -> Crosshair running scale: %.2fx (expands)\n", scale_run);
    printf("  -> Crosshair jumping scale: %.2fx (max bloom)\n", scale_jump);
    printf("  -> PASS\n");

    printf("\n====================================================\n");
    printf("  ALL PHASE 3 GUNPLAY & MOVEMENT TESTS PASSED!\n");
    printf("====================================================\n");
    return 0;
}
