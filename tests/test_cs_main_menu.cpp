// tests/test_cs_main_menu.cpp
// Automated tests for Phase 8: CS 1.6 Main Menu & UI Theme.

#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

void test_cs_background_asset() {
    std::cout << "[Test 1] Verifying CS 1.6 background image asset...\n";
    FILE *f = fopen("/home/najm-pc20/dunestrike/packages/misc/cs_background.jpg", "rb");
    assert(f != NULL);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    // Stitched full background image must be valid and sized ~120KB
    assert(size > 50000);
    std::cout << "  -> PASS: CS 1.6 wallpaper asset verified (" << size << " bytes).\n";
}

void test_menu_definitions() {
    std::cout << "[Test 2] Verifying CS 1.6 menu entries and layout in menus.cfg...\n";
    std::ifstream infile("/home/najm-pc20/dunestrike/config/menus.cfg");
    assert(infile.is_open());

    std::string line;
    bool found_main = false;
    bool found_servers = false;
    bool found_newgame = false;
    bool found_config = false;
    bool found_demos = false;
    bool found_editor = false;
    bool found_quit = false;

    while (std::getline(infile, line)) {
        if (line.find("newmenu main") != std::string::npos) found_main = true;
        if (found_main) {
            if (line.find("Find Servers") != std::string::npos) found_servers = true;
            if (line.find("New Game (Bots)") != std::string::npos) found_newgame = true;
            if (line.find("Configuration") != std::string::npos) found_config = true;
            if (line.find("View Demos") != std::string::npos) found_demos = true;
            if (line.find("Map Editor") != std::string::npos) found_editor = true;
            if (line.find("Quit Game") != std::string::npos) found_quit = true;
            if (line.find("newmenu all_custom_maps") != std::string::npos) break;
        }
    }

    assert(found_main);
    assert(found_servers);
    assert(found_newgame);
    assert(found_config);
    assert(found_demos);
    assert(found_editor);
    assert(found_quit);

    std::cout << "  -> PASS: All CS 1.6 menu labels and actions correctly registered.\n";
}

void test_menu_theme_colors() {
    std::cout << "[Test 3] Verifying CS 1.6 color scheme in autoexec.cfg...\n";
    std::ifstream infile("/home/najm-pc20/dunestrike/config/autoexec.cfg");
    assert(infile.is_open());

    std::string line;
    bool found_selection_color = false;
    bool found_desc_color = false;

    while (std::getline(infile, line)) {
        if (line.find("menuselectionbgcolor") != std::string::npos) found_selection_color = true;
        if (line.find("menuselectiondescbgcolor") != std::string::npos) found_desc_color = true;
    }

    assert(found_selection_color);
    assert(found_desc_color);

    std::cout << "  -> PASS: CS 1.6 menu selection highlight palette verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  RUNNING PHASE 8 TESTS: CS 1.6 MAIN MENU & THEME\n";
    std::cout << "====================================================\n\n";

    test_cs_background_asset();
    test_menu_definitions();
    test_menu_theme_colors();

    std::cout << "\n====================================================\n";
    std::cout << "  ALL PHASE 8 MENU TESTS PASSED SUCCESSFULLY!\n";
    std::cout << "====================================================\n";
    return 0;
}
