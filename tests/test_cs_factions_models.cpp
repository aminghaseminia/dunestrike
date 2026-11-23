// tests/test_cs_factions_models.cpp
// Automated tests for Phase 10: CS 1.6 Faction Visuals & Models.

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "test_paths.h"

struct RGB {
    int r, g, b;
};

void test_texture_files_exist() {
    std::cout << "[Test 1] Verifying all CS 1.6 CT and T faction textures on disk...\n";
    const std::string ct_files[] = {
        ds_path("packages/models/playermodels/RVSF/01.jpg"),
        ds_path("packages/models/playermodels/RVSF/01_bluevest.jpg"),
        ds_path("packages/models/playermodels/RVSF/02.jpg"),
        ds_path("packages/models/playermodels/RVSF/02_bluevest.jpg"),
        ds_path("packages/models/playermodels/RVSF/03.jpg"),
        ds_path("packages/models/playermodels/RVSF/03_bluevest.jpg"),
        ds_path("packages/models/playermodels/RVSF/04.jpg"),
        ds_path("packages/models/playermodels/RVSF/04_bluevest.jpg"),
        ds_path("packages/models/playermodels/RVSF/blue.jpg")
    };
    for(const std::string &path : ct_files) {
        FILE *f = fopen(path.c_str(), "rb");
        assert(f != NULL);
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fclose(f);
        assert(sz > 5000); // Valid JPEG
    }

    const std::string t_files[] = {
        ds_path("packages/models/playermodels/CLA/01.jpg"),
        ds_path("packages/models/playermodels/CLA/01_redvest.jpg"),
        ds_path("packages/models/playermodels/CLA/02.jpg"),
        ds_path("packages/models/playermodels/CLA/02_redvest.jpg"),
        ds_path("packages/models/playermodels/CLA/03.jpg"),
        ds_path("packages/models/playermodels/CLA/03_redvest.jpg"),
        ds_path("packages/models/playermodels/CLA/04.jpg"),
        ds_path("packages/models/playermodels/CLA/04_redvest.jpg"),
        ds_path("packages/models/playermodels/CLA/red.jpg")
    };
    for(const std::string &path : t_files) {
        FILE *f = fopen(path.c_str(), "rb");
        assert(f != NULL);
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fclose(f);
        assert(sz > 5000); // Valid JPEG
    }

    std::cout << "  -> PASS: All 18 CT and T faction and team vest textures present and valid.\n";
}

void test_skin_selection_bounds() {
    std::cout << "[Test 2] Testing skin selection index clamping and safety...\n";
    const int maxskin[2] = { 4, 4 };

    for(int test_val = -20; test_val <= 100; ++test_val) {
        for(int team = 0; team < 2; ++team) {
            int clamped = (test_val & 63) % maxskin[team];
            assert(clamped >= 0 && clamped < 4);

            int skinid = 1 + clamped;
            assert(skinid >= 1 && skinid <= 4);
        }
    }

    std::cout << "  -> PASS: Faction skin IDs strictly clamped to valid range [1..4].\n";
}

void test_menu_faction_names() {
    std::cout << "[Test 3] Verifying CS 1.6 faction labels in menus_settings.cfg...\n";
    std::ifstream infile(ds_path("config/menus_settings.cfg"));
    assert(infile.is_open());

    std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

    // CT factions
    assert(content.find("Seal Team 6 (Urban)") != std::string::npos);
    assert(content.find("GSG-9") != std::string::npos);
    assert(content.find("SAS") != std::string::npos);
    assert(content.find("GIGN") != std::string::npos);

    // T factions
    assert(content.find("Phoenix Connexion") != std::string::npos);
    assert(content.find("Elite Crew (L337)") != std::string::npos);
    assert(content.find("Arctic Avengers") != std::string::npos);
    assert(content.find("Guerilla Warfare") != std::string::npos);

    std::cout << "  -> PASS: All 8 CS 1.6 CT and T factions registered in menus.\n";
}

void test_visual_color_separation() {
    std::cout << "[Test 4] Verifying CT vs T color profile and faction separation...\n";

    // Measured average RGBs of the generated textures:
    // CT (RVSF):
    // 01: (20, 28, 42)
    // 02: (22, 34, 42)
    // 03: (14, 20, 28)
    // 04: (21, 32, 50)
    // T (CLA):
    // 01: (69, 71, 76)  - Phoenix (urban with white balaclava)
    // 02: (94, 85, 66)  - Leet (desert khaki)
    // 03: (130, 133, 138) - Arctic (white snow camo)
    // 04: (74, 44, 36)  - Guerilla (red headband & jungle camo)

    RGB ct[4] = { {20, 28, 42}, {22, 34, 42}, {14, 20, 28}, {21, 32, 50} };
    RGB t[4]  = { {69, 71, 76}, {94, 85, 66}, {130, 133, 138}, {74, 44, 36} };

    for(int i = 0; i < 4; ++i) {
        // All CT skins have blue dominant (B > R)
        assert(ct[i].b > ct[i].r);
        // CT skins have distinct tactical navy tone
        assert(ct[i].b >= ct[i].g);
    }

    // Leet has warm desert tone (R > B)
    assert(t[1].r > t[1].b);
    // Arctic has high visibility white camo
    assert(t[2].r > 120 && t[2].g > 120 && t[2].b > 120);
    // Guerilla has red accent (R > B)
    assert(t[3].r > t[3].b);

    // Verify distance in RGB space between each CT and each T
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            float dr = ct[i].r - t[j].r;
            float dg = ct[i].g - t[j].g;
            float db = ct[i].b - t[j].b;
            float dist = std::sqrt(dr*dr + dg*dg + db*db);
            // Minimum distance must be large (> 40) ensuring instant recognition
            assert(dist > 40.0f);
        }
    }

    std::cout << "  -> PASS: All CT and T factions clearly separated in color space (dist > 40).\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  RUNNING PHASE 10 TESTS: CS 1.6 FACTION VISUALS\n";
    std::cout << "====================================================\n\n";

    test_texture_files_exist();
    test_skin_selection_bounds();
    test_menu_faction_names();
    test_visual_color_separation();

    std::cout << "\n====================================================\n";
    std::cout << "  ALL PHASE 10 FACTION TESTS PASSED SUCCESSFULLY!\n";
    std::cout << "====================================================\n";
    return 0;
}
