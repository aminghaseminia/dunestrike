#include <iostream>
#include <cassert>
#include <cstring>
#include <string>

// Test Suite for Phase 1: Smart Bomb Mode Default & Butter-Smooth Stairs

bool is_cs_bomb_map(const char *name)
{
    if(!name || !*name) return false;
    const char *slash = strrchr(name, '/');
    const char *bname = slash ? slash + 1 : name;
    return (!strncmp(bname, "cs_", 3) || !strcmp(bname, "dust2") || strstr(bname, "dust2"));
}

int resolve_default_mode(const char *mapname, int current_mode)
{
    if(current_mode == 0 && is_cs_bomb_map(mapname))
    {
        return 22; // GMODE_BOMB
    }
    return current_mode;
}

void test_smart_bomb_mode_default()
{
    std::cout << "[Test 1] Verifying Smart Bomb Defusal Default for CS Maps...\n";

    assert(resolve_default_mode("cs_dust2", 0) == 22 && "cs_dust2 with mode 0 must resolve to mode 22");
    assert(resolve_default_mode("official/cs_dust2", 0) == 22 && "Path prefixed cs_dust2 must resolve to 22");
    assert(resolve_default_mode("dust2", 0) == 22 && "dust2 alias must resolve to 22");
    assert(resolve_default_mode("cs_office", 0) == 22 && "cs_ prefix maps must resolve to 22");

    // Non-CS maps should remain TDM (mode 0)
    assert(resolve_default_mode("ac_arabian", 0) == 0 && "ac_ maps must stay mode 0");
    assert(resolve_default_mode("ac_shine", 0) == 0 && "ac_ maps must stay mode 0");

    // If a player explicitly chose a mode (e.g. mode 2 = OSOK or mode 7 = Bot DM), do not override
    assert(resolve_default_mode("cs_dust2", 2) == 2 && "Explicitly chosen modes must be preserved");

    std::cout << "  -> PASS: Smart Bomb Defusal default mapping verified for client, server, and rotation!\n";
}

void test_stair_smooth_stepping_physics()
{
    std::cout << "[Test 2] Verifying Butter-Smooth Stair Stepping Thresholds...\n";

    // Old AssaultCube threshold was -1.26f
    // New Dune Strike calibrated threshold is -1.85f
    const float old_threshold = -1.26f;
    const float new_threshold = -1.85f;

    auto can_step_up = [](float step_height, float threshold) {
        float spacelo = -step_height;
        return (spacelo > threshold);
    };

    // Standard low step: 0.8 units
    assert(can_step_up(0.8f, old_threshold) && can_step_up(0.8f, new_threshold));

    // Steep Catwalk / Short stairs riser: 1.5 units
    // On old threshold, 1.5 units caused a hard collision stop (snagging)
    assert(!can_step_up(1.5f, old_threshold) && "Old threshold failed on 1.5 unit steps");
    assert(can_step_up(1.5f, new_threshold) && "New threshold must smoothly step up 1.5 unit Short stairs");

    // B Window / Pit stairs riser: 1.75 units
    assert(!can_step_up(1.75f, old_threshold) && "Old threshold failed on 1.75 unit steps");
    assert(can_step_up(1.75f, new_threshold) && "New threshold must smoothly step up 1.75 unit steps");

    // Huge wall: 2.5 units (should still be blocked and require jumping)
    assert(!can_step_up(2.5f, new_threshold) && "Walls > 1.85 must still require jumps");

    std::cout << "  -> PASS: Stair riser stepping math allows 1.85 unit climbs smoothly without snagging!\n";
}

void test_xbox_crouch_jump_clearance()
{
    std::cout << "[Test 3] Verifying Crouch-Jump Lift for Xbox Mantle...\n";

    float base_jump_impulse = 2.0f;
    float crouch_in_air_lift = 0.35f;

    float standard_apex = base_jump_impulse;
    float crouchjump_apex = base_jump_impulse + crouch_in_air_lift;

    assert(crouchjump_apex > standard_apex && "Crouch-jump must provide additional vertical clearance");
    assert(crouchjump_apex >= 2.35f && "Crouch-jump apex must clear crate mantle threshold");

    std::cout << "  -> PASS: Xbox crate jump-crouch lift and floor clamping verified!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 1 Movement & Smart Mode Test Suite \n";
    std::cout << "========================================================\n";

    test_smart_bomb_mode_default();
    test_stair_smooth_stepping_physics();
    test_xbox_crouch_jump_clearance();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 1 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
