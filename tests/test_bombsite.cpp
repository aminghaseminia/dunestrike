#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>

// Standalone unit test for bomb site geometry & dual-site detection logic
struct Vec2 {
    float x, y;
    Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

struct BombTestState {
    int sites[2][2];
    int siteradius;

    BombTestState() {
        sites[0][0] = sites[0][1] = -1;
        sites[1][0] = sites[1][1] = -1;
        siteradius = 3;
    }

    int bombsiteat(const Vec2 &o) const {
        for(int i = 0; i < 2; ++i) {
            if(sites[i][0] < 0) continue;
            float dx = o.x - sites[i][0], dy = o.y - sites[i][1];
            if(dx*dx + dy*dy <= float(siteradius) * siteradius) return i;
        }
        return -1;
    }
};

void run_tests() {
    printf("[TEST] Starting Dual-Site Bomb Defusal Unit Tests...\n");

    BombTestState state;
    // Set Site A (index 0) at (50, 60) and Site B (index 1) at (120, 140)
    state.sites[0][0] = 50; state.sites[0][1] = 60;
    state.sites[1][0] = 120; state.sites[1][1] = 140;
    state.siteradius = 3;

    // 1. Center of Site A
    assert(state.bombsiteat(Vec2(50, 60)) == 0);
    printf("  [PASS] Center of Site A detected as site 0\n");

    // 2. Near Site A within radius 3
    assert(state.bombsiteat(Vec2(51.5f, 61.5f)) == 0); // dist^2 = 2.25 + 2.25 = 4.5 <= 9
    assert(state.bombsiteat(Vec2(50.0f, 62.9f)) == 0); // dist = 2.9 <= 3
    printf("  [PASS] Near Site A within radius detected as site 0\n");

    // 3. Center of Site B
    assert(state.bombsiteat(Vec2(120, 140)) == 1);
    printf("  [PASS] Center of Site B detected as site 1\n");

    // 4. Near Site B within radius 3
    assert(state.bombsiteat(Vec2(121.0f, 142.0f)) == 1); // dist^2 = 1 + 4 = 5 <= 9
    printf("  [PASS] Near Site B within radius detected as site 1\n");

    // 5. Far from both sites
    assert(state.bombsiteat(Vec2(0, 0)) == -1);
    assert(state.bombsiteat(Vec2(80, 100)) == -1);
    assert(state.bombsiteat(Vec2(50, 65)) == -1); // dist = 5 > 3
    assert(state.bombsiteat(Vec2(120, 145)) == -1); // dist = 5 > 3
    printf("  [PASS] Out of range coordinates correctly return -1\n");

    // 6. Boundary edge conditions
    assert(state.bombsiteat(Vec2(53.0f, 60.0f)) == 0);  // dx = 3, dy = 0 -> exact boundary
    assert(state.bombsiteat(Vec2(53.05f, 60.0f)) == -1); // slightly outside boundary
    assert(state.bombsiteat(Vec2(120.0f, 143.0f)) == 1);  // dy = 3 -> exact boundary
    assert(state.bombsiteat(Vec2(120.0f, 143.05f)) == -1); // slightly outside boundary
    printf("  [PASS] Exact boundary tests passed\n");

    // 7. Site A disabled (-1), Site B active
    BombTestState single_site;
    single_site.sites[0][0] = -1;
    single_site.sites[1][0] = 100; single_site.sites[1][1] = 100;
    assert(single_site.bombsiteat(Vec2(50, 60)) == -1);
    assert(single_site.bombsiteat(Vec2(100, 100)) == 1);
    printf("  [PASS] Handling maps with only one site passed\n");

    printf("[SUCCESS] All Dual-Site Bomb Defusal Unit Tests passed!\n");
}

int main() {
    run_tests();
    return 0;
}
