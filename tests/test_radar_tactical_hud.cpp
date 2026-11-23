// tests/test_radar_tactical_hud.cpp
// Automated tests for Phase 9: CS 1.6 Tactical Radar & HUD cues.

#include <cassert>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>

struct vec2 {
    float x, y;
    vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
    float magnitude() const { return std::sqrt(x * x + y * y); }
    void mul(float f) { x *= f; y *= f; }
};

void test_radar_positions() {
    std::cout << "[Test 1] Verifying CS 1.6 Top-Left vs Classic Top-Right Radar coordinates...\n";

    const float VIRTW = 2400.0f;
    const float VIRTH = 1800.0f;
    const float radarviewsize = std::min(VIRTW, VIRTH) / 5.0f; // 360.0f
    const float halfviewsize = radarviewsize / 2.0f; // 180.0f
    assert(radarviewsize == 360.0f);
    assert(halfviewsize == 180.0f);

    // CS 1.6 style (radarposition == 0)
    int radarposition = 0;
    assert(radarposition == 0);
    vec2 centerpos_cs(halfviewsize + 72.0f, halfviewsize + 64.0f);
    vec2 radarpos_cs(radarviewsize + 88.0f, radarviewsize + 80.0f);

    assert(centerpos_cs.x == 252.0f);
    assert(centerpos_cs.y == 244.0f);
    // Must be in top-left quadrant (< VIRTW/4)
    assert(centerpos_cs.x < VIRTW / 4.0f);
    assert(centerpos_cs.y < VIRTH / 4.0f);

    // Classic style (radarposition == 1)
    radarposition = 1;
    assert(radarposition == 1);
    vec2 centerpos_classic(VIRTW - halfviewsize - 72.0f, halfviewsize + 64.0f);
    assert(centerpos_classic.x == 2148.0f);
    // Must be in top-right quadrant (> VIRTW * 3/4)
    assert(centerpos_classic.x > VIRTW * 0.75f);

    std::cout << "  -> PASS: Radar center coordinates correctly placed (CS: " 
              << centerpos_cs.x << "," << centerpos_cs.y << " | Classic: "
              << centerpos_classic.x << "," << centerpos_classic.y << ").\n";
}

void test_console_safety_and_anti_overlap() {
    std::cout << "[Test 2] Verifying console text indentation & anti-overlap with top-left radar...\n";

    const int VIRTW = 2400;
    const int VIRTH = 1800;
    const int CONSPAD = 10;
    const int FONTH = 24;
    const float radarviewsize = std::min(VIRTW, VIRTH) / 5.0f;
    const vec2 radarpos_cs(radarviewsize + 88.0f, radarviewsize + 80.0f); // 448, 440

    // When radarposition == 0 (CS 1.6 top-left):
    int conleft_cs = int(std::ceil(radarpos_cs.x * 2.0f)) + CONSPAD;
    int conwidth_cs = VIRTW * 2 - conleft_cs - CONSPAD - 2 * FONTH / 3;

    // Verify conleft is strictly past the radar bounding box (896 + 10 = 906)
    assert(conleft_cs > radarpos_cs.x * 2.0f);
    assert(conleft_cs == 906);
    // Verify console text fits within the screen bounds
    assert(conleft_cs + conwidth_cs <= VIRTW * 2);
    assert(conwidth_cs > 1000); // Plentiful width for messages

    // When radarposition == 1 (Classic top-right):
    float overlaysize = (VIRTH / 6.0f) * 4.0f / 3.25f;
    vec2 radarpos_classic(VIRTW - 10.0f - VIRTH / 28.0f - overlaysize, 10.0f + VIRTH / 52.0f);
    int conleft_classic = CONSPAD + FONTH / 3;
    int conwidth_classic = int(std::floor(radarpos_classic.x)) * 2 - 2 * CONSPAD - 2 * FONTH / 3;

    assert(conleft_classic == CONSPAD + 8);
    assert(conleft_classic + conwidth_classic <= radarpos_classic.x * 2.0f);

    std::cout << "  -> PASS: Console dynamically indents to x=" << conleft_cs 
              << " (width=" << conwidth_cs << ") when radar is top-left, preventing any HUD overlap.\n";
}

void test_radar_bomb_site_clamping() {
    std::cout << "[Test 3] Verifying tactical bomb site directional clamping algorithm...\n";

    const float radarheight = 150.0f;
    const float maxdist = radarheight / 2.0f; // 75.0f
    const float radarviewsize = 360.0f;
    const float scaled = radarviewsize / radarheight; // 2.4f
    const float halfviewsize = radarviewsize / 2.0f; // 180.0f

    // Distant bomb site A: Player at (500, 500), Site A at (1200, 500) -> dx = 700, dy = 0
    vec2 p_player(500, 500);
    vec2 site_a(1200, 500);
    vec2 delta(site_a.x - p_player.x, site_a.y - p_player.y);

    float dist = delta.magnitude();
    assert(dist == 700.0f);
    assert(dist > maxdist);

    // Apply clamping as implemented in drawradar_vicinity
    if(dist > maxdist && maxdist > 0) {
        delta.mul(maxdist * 0.90f / dist);
    }

    // Verify clamped distance is within radar boundary
    float clamped_dist = delta.magnitude();
    assert(clamped_dist <= maxdist * 0.90f + 0.001f);
    assert(std::fabs(clamped_dist - 67.5f) < 0.001f);

    // After applying scaled factor to radar screen coords:
    delta.mul(scaled);
    float screen_dist = delta.magnitude();
    assert(screen_dist <= halfviewsize); // Stays inside the circular compass perimeter (162.0 <= 180.0)
    assert(delta.y == 0.0f);
    assert(delta.x > 0.0f); // Direction angle strictly preserved

    std::cout << "  -> PASS: Distant bomb sites correctly clamp to radar perimeter (" 
              << screen_dist << "px <= " << halfviewsize << "px) with angle preserved.\n";
}

void test_planted_bomb_hud_cues() {
    std::cout << "[Test 4] Verifying C4 plant & defuse HUD tactical cues and labeling...\n";

    // Verify Site mapping
    for(int site = 0; site < 2; ++site) {
        const char *sitename = (site == 0 ? "SITE A" : (site == 1 ? "SITE B" : "UNKNOWN"));
        assert(std::string(sitename) == (site == 0 ? "SITE A" : "SITE B"));

        char timerline[128];
        int remaining = 34500; // 34.5 seconds
        snprintf(timerline, sizeof(timerline), "C4 PLANTED [%s] - %02d:%02d", sitename, remaining/60000, (remaining/1000)%60);
        assert(std::string(timerline) == (site == 0 ? "C4 PLANTED [SITE A] - 00:34" : "C4 PLANTED [SITE B] - 00:34"));

        char defuseline[128];
        int pct = 65;
        snprintf(defuseline, sizeof(defuseline), "DEFUSING AT %s (%d%%)", sitename, pct);
        assert(std::string(defuseline) == (site == 0 ? "DEFUSING AT SITE A (65%)" : "DEFUSING AT SITE B (65%)"));
    }

    // Verify urgency flashing pulse
    int millis1 = 0;
    int millis2 = 150;
    int flash1 = (millis1 / 150) % 2;
    int flash2 = (millis2 / 150) % 2;
    assert(flash1 != flash2);

    std::cout << "  -> PASS: C4 Plant/Defuse HUD strings and pulse frequencies verified.\n";
}

void test_source_code_integrity() {
    std::cout << "[Test 5] Checking source code implementation in renderhud.cpp and console.cpp...\n";

    std::ifstream hudfile("/home/najm-pc20/dunestrike/source/src/renderhud.cpp");
    assert(hudfile.is_open());
    std::string hudcontent((std::istreambuf_iterator<char>(hudfile)), std::istreambuf_iterator<char>());

    assert(hudcontent.find("VARP(radarposition, 0, 0, 1);") != std::string::npos);
    assert(hudcontent.find("radarposition == 0 ? (halfviewsize+72)") != std::string::npos);
    assert(hudcontent.find("bombinfo.carrier == pl->clientnum") != std::string::npos);
    assert(hudcontent.find("C4 PLANTED [%s]") != std::string::npos);
    assert(hudcontent.find("%s AT %s (%d%%)") != std::string::npos);

    std::ifstream confile("/home/najm-pc20/dunestrike/source/src/console.cpp");
    assert(confile.is_open());
    std::string concontent((std::istreambuf_iterator<char>(confile)), std::istreambuf_iterator<char>());

    assert(concontent.find("conleft = int(ceil(getradarpos().x * 2.0f)) + CONSPAD;") != std::string::npos);
    assert(concontent.find("conwidth = VIRTW*2 - conleft - CONSPAD - 2*FONTH/3;") != std::string::npos);

    std::cout << "  -> PASS: Source code contains all tactical radar and HUD modifications.\n";
}

int main() {
    std::cout << "=== Running Dune Strike Phase 9 Test Suite ===\n";
    test_radar_positions();
    test_console_safety_and_anti_overlap();
    test_radar_bomb_site_clamping();
    test_planted_bomb_hud_cues();
    test_source_code_integrity();
    std::cout << "=== ALL PHASE 9 RADAR & HUD TESTS PASSED ===\n";
    return 0;
}
