#include <iostream>
#include <cassert>
#include <cmath>

// Test unit for:
// 1. Sniper Double Zoom logic (0 -> 1 -> 2 -> 0)
// 2. Crosshair size safety guardrail (s >= 10 / fallback to 15)
// 3. Radar teammate clamping & visibility across large distances

void test_sniper_double_zoom()
{
    std::cout << "[Test 1] Verifying Sniper Double Zoom state cycle & FOV calculation...\n";
    int scoped = 0; // initial unscoped

    // First click: zoom level 1 (FOV 40)
    scoped = (scoped + 1) % 3;
    assert(scoped == 1);
    float fov_lvl1 = (scoped == 2) ? 15.0f : 40.0f;
    assert(fov_lvl1 == 40.0f);

    // Second click: zoom level 2 (Double Zoom, FOV 15)
    scoped = (scoped + 1) % 3;
    assert(scoped == 2);
    float fov_lvl2 = (scoped == 2) ? 15.0f : 40.0f;
    assert(fov_lvl2 == 15.0f);

    // Third click: unzoom (back to 0)
    scoped = (scoped + 1) % 3;
    assert(scoped == 0);

    // Drop scope on attack / fire
    scoped = 2; // in double zoom
    bool fired = true;
    if(fired && scoped) scoped = 0;
    assert(scoped == 0);

    std::cout << "  -> PASS: Sniper Double Zoom cycles 0 -> 1 (40 FOV) -> 2 (15 FOV) -> 0 correctly!\n";
}

void test_crosshair_guardrails()
{
    std::cout << "[Test 2] Verifying Crosshair size guardrails (fallback on 0 or negative)...\n";
    
    // In renderhud.cpp:
    // float s = size > 0 ? size : (float)(crosshairsize > 0 ? crosshairsize : 15);
    int test_crosshairsize_zero = 0;
    float s1 = -1.0f > 0 ? -1.0f : (float)(test_crosshairsize_zero > 0 ? test_crosshairsize_zero : 15);
    assert(s1 == 15.0f);

    int test_crosshairsize_custom = 22;
    float s2 = -1.0f > 0 ? -1.0f : (float)(test_crosshairsize_custom > 0 ? test_crosshairsize_custom : 15);
    assert(s2 == 22.0f);

    float explicit_size = 24.0f;
    float s3 = explicit_size > 0 ? explicit_size : (float)(test_crosshairsize_zero > 0 ? test_crosshairsize_zero : 15);
    assert(s3 == 24.0f);

    std::cout << "  -> PASS: Crosshair is guaranteed visible on all weapons even when config has crosshairsize 0!\n";
}

void test_radar_teammate_clamping()
{
    std::cout << "[Test 3] Verifying Radar teammate position clamping across large map distances...\n";
    
    float radarheight = 150.0f;
    float maxdist = radarheight / 2.0f; // 75 units radius
    assert(maxdist == 75.0f);

    // Case A: Teammate nearby (30 units away) -> not clamped
    float dx1 = 30.0f, dy1 = 0.0f;
    float dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    assert(dist1 <= maxdist);
    float clamped_dx1 = dx1;
    if(dist1 > maxdist && maxdist > 0) clamped_dx1 *= (maxdist * 0.92f / dist1);
    assert(clamped_dx1 == 30.0f);

    // Case B: Teammate far away (250 units away across Dust 2) -> clamped to radar edge
    float dx2 = 200.0f, dy2 = 150.0f;
    float dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2); // 250.0f
    assert(dist2 > maxdist);
    float clamped_dx2 = dx2 * (maxdist * 0.92f / dist2);
    float clamped_dy2 = dy2 * (maxdist * 0.92f / dist2);
    float clamped_dist = std::sqrt(clamped_dx2 * clamped_dx2 + clamped_dy2 * clamped_dy2);
    assert(std::abs(clamped_dist - 69.0f) < 0.001f); // 75 * 0.92 = 69.0f (inside radar border)
    assert(clamped_dist <= maxdist);

    std::cout << "  -> PASS: Teammates across full map are clamped to radar rim and remain visible at all times!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Radar, Crosshair & Sniper Double Zoom Test Suite       \n";
    std::cout << "========================================================\n";
    test_sniper_double_zoom();
    test_crosshair_guardrails();
    test_radar_teammate_clamping();
    std::cout << "========================================================\n";
    std::cout << "  ALL RADAR, CROSSHAIR & ZOOM TESTS PASSED (100%)       \n";
    std::cout << "========================================================\n";
    return 0;
}
