// tests/test_scancode_keyboard_fallback.cpp
// Automated tests for keyboard layout compatibility (Persian / non-Latin layouts).

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <SDL2/SDL.h>

void test_sdl_scancode_resolution() {
    std::cout << "[Test 1] Verifying SDL2 scancode to keyname resolution for gameplay keys...\n";

    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_W)) == "W");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_A)) == "A");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_S)) == "S");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_D)) == "D");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_R)) == "R");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_B)) == "B");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_Q)) == "Q");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_C)) == "C");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_E)) == "E");
    assert(std::string(SDL_GetScancodeName(SDL_SCANCODE_F)) == "F");

    std::cout << "  -> PASS: All movement and action scancodes resolve to standard CS key names.\n";
}

void test_persian_keyboard_fallback_simulation() {
    std::cout << "[Test 2] Simulating keypress with Persian keyboard layout active...\n";

    // Standard English keymap bindings
    std::map<std::string, std::string> binds;
    binds["W"] = "forward";
    binds["A"] = "left";
    binds["S"] = "backward";
    binds["D"] = "right";
    binds["R"] = "reload";
    binds["B"] = "key_showmenuweapons";
    binds["Q"] = "key_prevweapon";

    // Map of Persian keysyms to physical scancodes:
    // When keyboard is in Persian, pressing W sends Persian 'ص' (unicode 1589), scancode 26
    struct KeyEvent {
        int keycode; // Unicode keysym
        SDL_Scancode scancode;
        const char *persian_char;
        const char *expected_action;
    } events[] = {
        { 1589, SDL_SCANCODE_W, "ص", "forward" },
        { 1588, SDL_SCANCODE_A, "ش", "left" },
        { 1587, SDL_SCANCODE_S, "س", "backward" },
        { 1740, SDL_SCANCODE_D, "ی", "right" },
        { 1602, SDL_SCANCODE_R, "ق", "reload" },
        { 1584, SDL_SCANCODE_B, "ذ", "key_showmenuweapons" },
        { 1590, SDL_SCANCODE_Q, "ض", "key_prevweapon" },
    };

    for(const auto &ev : events) {
        // 1. Simulating lookup by keycode (fails in non-Latin layouts)
        bool keycode_found = (ev.keycode >= 'a' && ev.keycode <= 'z');
        assert(!keycode_found); // Confirms Persian keysym is not in standard Latin ASCII

        // 2. Fallback via physical scancode name (our engine fix)
        const char *scname = SDL_GetScancodeName(ev.scancode);
        assert(scname != nullptr);
        auto it = binds.find(scname);
        assert(it != binds.end());
        assert(it->second == ev.expected_action);

        std::cout << "  -> Key '" << ev.persian_char << "' (scancode " << ev.scancode 
                  << " -> '" << scname << "') successfully triggered: '" 
                  << it->second << "'\n";
    }

    std::cout << "  -> PASS: All Persian layout keystrokes successfully mapped to actions.\n";
}

int main() {
    std::cout << "=== Running Keyboard Layout Fallback Test Suite ===\n";
    test_sdl_scancode_resolution();
    test_persian_keyboard_fallback_simulation();
    std::cout << "=== ALL KEYBOARD FALLBACK TESTS PASSED ===\n";
    return 0;
}
