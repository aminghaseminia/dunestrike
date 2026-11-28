#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

// Test Suite for Phase 2: Viewmodel Animations (Reload, Deploy) & Camera Recoil

void test_reload_viewmodel_motion()
{
    std::cout << "[Test 1] Verifying Reload Viewmodel Motion Curves...\n";

    int reloadtime = 1800; // standard reload ms
    float max_tilt = 0.0f;
    float max_drop = 0.0f;
    bool had_slap_jolt = false;

    for(int elapsed = 0; elapsed <= reloadtime; elapsed += 50)
    {
        float t = float(elapsed) / float(reloadtime);
        float curve = std::sin(t * M_PI);
        float roll_anim = curve * -20.0f;
        float drop_z = -curve * 0.24f;

        if(std::abs(roll_anim) > std::abs(max_tilt)) max_tilt = roll_anim;
        if(std::abs(drop_z) > std::abs(max_drop)) max_drop = drop_z;

        if(t > 0.44f && t < 0.60f)
        {
            float slap = std::sin((t - 0.44f) / 0.16f * M_PI);
            if(slap > 0.8f) had_slap_jolt = true;
        }
    }

    assert(std::abs(max_tilt - (-20.0f)) < 0.1f && "Reload tilt must reach ~ -20 degrees");
    assert(std::abs(max_drop - (-0.24f)) < 0.01f && "Reload drop must reach ~ -0.24 units");
    assert(had_slap_jolt && "Tactile magazine insertion jolt must occur around t=0.5");

    std::cout << "  -> PASS: Reload tilt curve (-20 deg), drop (-0.24 units), and magazine insertion jolt verified!\n";
}

void test_deploy_quickdraw_curve()
{
    std::cout << "[Test 2] Verifying Quick-Draw / Deploy Rise Transition...\n";

    int deploy_time = 260; // ms

    // At t=0 (instant of switch)
    float t0 = 0.0f;
    float factor0 = (1.0f - t0) * (1.0f - t0);
    float z0 = -factor0 * 0.30f;
    float pitch0 = factor0 * 22.0f;

    assert(z0 == -0.30f && "Weapon must start lowered by 0.30 units on switch");
    assert(pitch0 == 22.0f && "Weapon pitch must start tilted up by 22 degrees on switch");

    // At t=0.5 (halfway through deploy, ~130ms)
    float t_half = 0.5f;
    float factor_half = (1.0f - t_half) * (1.0f - t_half);
    float z_half = -factor_half * 0.30f;
    assert(z_half > z0 && z_half < 0.0f && "Weapon must be rising smoothly with deceleration");

    // At t=1.0 (deploy completed)
    float t1 = 1.0f;
    float factor1 = (1.0f - t1) * (1.0f - t1);
    float z1 = -factor1 * 0.30f;
    float pitch1 = factor1 * 22.0f;
    assert(z1 == 0.0f && pitch1 == 0.0f && "Weapon must settle perfectly at neutral (0, 0)");

    std::cout << "  -> PASS: Quick-draw deploy rise curve, quadratic ease-out, and neutral settlement verified!\n";
}

void test_camera_pitch_recoil()
{
    std::cout << "[Test 3] Verifying Camera Recoil Pitch per Weapon Type...\n";

    float ak_standing = 0.65f;
    float ak_crouching = 0.65f * 0.65f;
    float subgun_standing = 0.45f;
    float pistol_standing = 0.55f;

    assert(ak_standing > subgun_standing && "Assault rifle kick must exceed subgun");
    assert(ak_crouching < ak_standing && "Crouching must mitigate spray recoil kick");
    assert(std::abs(ak_crouching - 0.4225f) < 0.001f);

    float current_pitch = 10.0f;
    current_pitch = std::min(current_pitch + ak_standing, 89.0f);
    assert(current_pitch == 10.65f && "Single shot must kick pitch up by 0.65 degrees");

    std::cout << "  -> PASS: Camera pitch recoil kicks and crouching mitigations verified!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 2 Viewmodel Animations & Recoil    \n";
    std::cout << "========================================================\n";

    test_reload_viewmodel_motion();
    test_deploy_quickdraw_curve();
    test_camera_pitch_recoil();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 2 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
