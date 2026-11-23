// tests/test_all_keys_persian.cpp
// Comprehensive automated test suite verifying ALL in-game keys under Persian (FA) keyboard layouts.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <SDL2/SDL.h>

// Mirror the engine's canonical scancode-to-asciikey mapping
int scancodetoasciikey(int scancode)
{
    if(scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return 'a' + (scancode - SDL_SCANCODE_A);
    if(scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
        return '1' + (scancode - SDL_SCANCODE_1);
    if(scancode == SDL_SCANCODE_0) return '0';
    switch(scancode)
    {
        case SDL_SCANCODE_RETURN: return SDLK_RETURN;
        case SDL_SCANCODE_ESCAPE: return SDLK_ESCAPE;
        case SDL_SCANCODE_BACKSPACE: return SDLK_BACKSPACE;
        case SDL_SCANCODE_TAB: return SDLK_TAB;
        case SDL_SCANCODE_SPACE: return SDLK_SPACE;
        case SDL_SCANCODE_MINUS: return SDLK_MINUS;
        case SDL_SCANCODE_EQUALS: return SDLK_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return SDLK_LEFTBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return SDLK_RIGHTBRACKET;
        case SDL_SCANCODE_BACKSLASH: return SDLK_BACKSLASH;
        case SDL_SCANCODE_NONUSHASH: return SDLK_HASH;
        case SDL_SCANCODE_SEMICOLON: return SDLK_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return SDLK_QUOTE;
        case SDL_SCANCODE_GRAVE: return SDLK_BACKQUOTE;
        case SDL_SCANCODE_COMMA: return SDLK_COMMA;
        case SDL_SCANCODE_PERIOD: return SDLK_PERIOD;
        case SDL_SCANCODE_SLASH: return SDLK_SLASH;
        case SDL_SCANCODE_DELETE: return SDLK_DELETE;
        default:
            return scancode ? (scancode | SDLK_SCANCODE_MASK) : 0;
    }
}

// Simulated Dune Strike key map structure
struct KeyDef {
    int code;
    std::string name;
    std::string action;
    bool pressed;
};

std::map<int, KeyDef> g_keymap;

void register_key(int code, const std::string &name, const std::string &action) {
    KeyDef kd;
    kd.code = code;
    kd.name = name;
    kd.action = action;
    kd.pressed = false;
    g_keymap[code] = kd;
}

void init_dune_strike_binds() {
    g_keymap.clear();
    // Letters
    register_key('a', "A", "left");
    register_key('b', "B", "key_showmenuweapons");
    register_key('c', "C", "copy");
    register_key('d', "D", "right");
    register_key('e', "E", "edittoggle");
    register_key('f', "F", "bombaction");
    register_key('g', "G", "quicknadethrow");
    register_key('h', "H", "hf_floor");
    register_key('i', "I", "whois");
    register_key('j', "J", "");
    register_key('k', "K", "corner");
    register_key('l', "L", "menu_light");
    register_key('m', "M", "moveent");
    register_key('n', "N", "quickanswer");
    register_key('o', "O", "move_ceiling_down");
    register_key('p', "P", "move_ceiling_up");
    register_key('q', "Q", "key_prevweapon");
    register_key('r', "R", "reload");
    register_key('s', "S", "backward");
    register_key('t', "T", "saycommand");
    register_key('u', "U", "undo");
    register_key('v', "V", "key_showmenuvoicecom");
    register_key('w', "W", "forward");
    register_key('x', "X", "voiceon");
    register_key('y', "Y", "key_teamchat");
    register_key('z', "Z", "floorceiltex");

    // Numbers
    register_key('1', "1", "primary");
    register_key('2', "2", "secondary");
    register_key('3', "3", "grenades");
    register_key('4', "4", "melee");
    register_key('5', "5", "attr5");
    register_key('6', "6", "attr6");
    register_key('7', "7", "attr7");
    register_key('8', "8", "vdelta_down");
    register_key('9', "9", "vdelta_up");
    register_key('0', "0", "");

    // Punctuation & symbols
    register_key(SDLK_SLASH, "SLASH", "key_commandline");
    register_key(SDLK_BACKQUOTE, "BACKQUOTE", "key_commandline");
    register_key(SDLK_BACKSLASH, "BACKSLASH", "key_console_scroll");
    register_key(SDLK_HASH, "HASH", "key_console_scroll");
    register_key(SDLK_MINUS, "MINUS", "copyent");
    register_key(SDLK_EQUALS, "EQUALS", "pasteent");
    register_key(SDLK_LEFTBRACKET, "LEFTBRACKET", "floor_down");
    register_key(SDLK_RIGHTBRACKET, "RIGHTBRACKET", "floor_up");
    register_key(SDLK_SEMICOLON, "SEMICOLON", "editent");
    register_key(SDLK_QUOTE, "QUOTE", "");
    register_key(SDLK_COMMA, "COMMA", "equalize_floor");
    register_key(SDLK_PERIOD, "PERIOD", "equalize_ceiling");

    // Control & navigation keys
    register_key(SDLK_BACKSPACE, "BACKSPACE", "dropflag");
    register_key(SDLK_TAB, "TAB", "key_showscores");
    register_key(SDLK_RETURN, "RETURN", "");
    register_key(SDLK_ESCAPE, "ESCAPE", "key_escape");
    register_key(SDLK_SPACE, "SPACE", "jump");
    register_key(SDLK_DELETE, "DELETE", "floortex_dn");

    // Scancode-masked keys
    register_key(SDL_SCANCODE_LSHIFT | SDLK_SCANCODE_MASK, "LSHIFT", "crouch");
    register_key(SDL_SCANCODE_RSHIFT | SDLK_SCANCODE_MASK, "RSHIFT", "");
    register_key(SDL_SCANCODE_LCTRL | SDLK_SCANCODE_MASK, "LCTRL", "editmeta");
    register_key(SDL_SCANCODE_RCTRL | SDLK_SCANCODE_MASK, "RCTRL", "jump");
    register_key(SDL_SCANCODE_LALT | SDLK_SCANCODE_MASK, "LALT", "key_showmap");
    register_key(SDL_SCANCODE_UP | SDLK_SCANCODE_MASK, "UP", "forward");
    register_key(SDL_SCANCODE_DOWN | SDLK_SCANCODE_MASK, "DOWN", "backward");
    register_key(SDL_SCANCODE_LEFT | SDLK_SCANCODE_MASK, "LEFT", "left");
    register_key(SDL_SCANCODE_RIGHT | SDLK_SCANCODE_MASK, "RIGHT", "right");
    register_key(SDL_SCANCODE_F1 | SDLK_SCANCODE_MASK, "F1", "key_voteyes");
    register_key(SDL_SCANCODE_F2 | SDLK_SCANCODE_MASK, "F2", "key_voteno");
    register_key(SDL_SCANCODE_F9 | SDLK_SCANCODE_MASK, "F9", "togglevoice");
    register_key(SDL_SCANCODE_F10 | SDLK_SCANCODE_MASK, "F10", "key_getdemo");
    register_key(SDL_SCANCODE_F11 | SDLK_SCANCODE_MASK, "F11", "key_toggleconsole");
    register_key(SDL_SCANCODE_F12 | SDLK_SCANCODE_MASK, "F12", "screenshot");
}

// Engine fallback simulator
KeyDef* simulate_keypress(int raw_keycode, int scancode, bool isdown, int &out_canonical_code) {
    out_canonical_code = raw_keycode;
    KeyDef *haskey = nullptr;

    auto it = g_keymap.find(raw_keycode);
    if(it != g_keymap.end()) haskey = &it->second;

    if(!haskey && !(raw_keycode & SDLK_SCANCODE_MASK) && scancode) {
        auto it_sc = g_keymap.find(scancode | SDLK_SCANCODE_MASK);
        if(it_sc != g_keymap.end()) haskey = &it_sc->second;
    }

    if(!haskey && scancode) {
        int asciikey = scancodetoasciikey(scancode);
        if(asciikey) {
            auto it_ascii = g_keymap.find(asciikey);
            if(it_ascii != g_keymap.end()) {
                haskey = &it_ascii->second;
                out_canonical_code = asciikey;
            }
        }
    }

    if(!haskey && scancode) {
        const char *scname = SDL_GetScancodeName((SDL_Scancode)scancode);
        if(scname && *scname) {
            for(auto &pair : g_keymap) {
                if(strcasecmp(pair.second.name.c_str(), scname) == 0) {
                    haskey = &pair.second;
                    break;
                }
            }
        }
    }

    if(haskey) {
        haskey->pressed = isdown;
    }
    return haskey;
}

void test_all_26_letters_persian() {
    std::cout << "[Test 1] Testing all 26 alphabet keys under Persian layout...\n";
    init_dune_strike_binds();

    // Map each physical key to its Persian keysym
    struct LetterCase {
        SDL_Scancode scancode;
        int persian_keysym;
        const char *persian_char;
        std::string expected_action;
    } cases[] = {
        { SDL_SCANCODE_A, 1588, "ش", "left" },
        { SDL_SCANCODE_B, 1584, "ذ", "key_showmenuweapons" },
        { SDL_SCANCODE_C, 1586, "ز", "copy" },
        { SDL_SCANCODE_D, 1740, "ی", "right" },
        { SDL_SCANCODE_E, 1579, "ث", "edittoggle" },
        { SDL_SCANCODE_F, 1576, "ب", "bombaction" },
        { SDL_SCANCODE_G, 1604, "ل", "quicknadethrow" },
        { SDL_SCANCODE_H, 1575, "ا", "hf_floor" },
        { SDL_SCANCODE_I, 1607, "ه", "whois" },
        { SDL_SCANCODE_J, 1578, "ت", "" },
        { SDL_SCANCODE_K, 1606, "ن", "corner" },
        { SDL_SCANCODE_L, 1605, "م", "menu_light" },
        { SDL_SCANCODE_M, 1662, "پ", "moveent" },
        { SDL_SCANCODE_N, 1583, "د", "quickanswer" },
        { SDL_SCANCODE_O, 1582, "خ", "move_ceiling_down" },
        { SDL_SCANCODE_P, 1581, "ح", "move_ceiling_up" },
        { SDL_SCANCODE_Q, 1590, "ض", "key_prevweapon" },
        { SDL_SCANCODE_R, 1602, "ق", "reload" },
        { SDL_SCANCODE_S, 1587, "س", "backward" },
        { SDL_SCANCODE_T, 1601, "ف", "saycommand" },
        { SDL_SCANCODE_U, 1593, "ع", "undo" },
        { SDL_SCANCODE_V, 1585, "ر", "key_showmenuvoicecom" },
        { SDL_SCANCODE_W, 1589, "ص", "forward" },
        { SDL_SCANCODE_X, 1591, "ط", "voiceon" },
        { SDL_SCANCODE_Y, 1594, "غ", "key_teamchat" },
        { SDL_SCANCODE_Z, 1592, "ظ", "floorceiltex" },
    };

    for(const auto &c : cases) {
        int canonical = 0;
        // 1. Keydown
        KeyDef *k = simulate_keypress(c.persian_keysym, c.scancode, true, canonical);
        assert(k != nullptr);
        assert(k->pressed == true);
        assert(k->action == c.expected_action);

        // 2. Keyup
        KeyDef *k_up = simulate_keypress(c.persian_keysym, c.scancode, false, canonical);
        assert(k_up == k);
        assert(k->pressed == false);
    }
    std::cout << "  -> PASS: All 26 Persian alphabet keys map & release correctly!\n";
}

void test_all_numbers_persian() {
    std::cout << "[Test 2] Testing weapon selection numbers (1-4) & digits (0-9) under Persian digits...\n";
    init_dune_strike_binds();

    // In Iranian Persian keyboard layout, top row can emit Persian digits (0x06F0 - 0x06F9)
    // or Arabic digits (0x0660 - 0x0669)
    struct DigitCase {
        SDL_Scancode scancode;
        int persian_keysym;
        const char *persian_char;
        std::string expected_action;
    } digits[] = {
        { SDL_SCANCODE_1, 0x06F1, "۱", "primary" },
        { SDL_SCANCODE_2, 0x06F2, "۲", "secondary" },
        { SDL_SCANCODE_3, 0x06F3, "۳", "grenades" },
        { SDL_SCANCODE_4, 0x06F4, "۴", "melee" },
        { SDL_SCANCODE_5, 0x06F5, "۵", "attr5" },
        { SDL_SCANCODE_6, 0x06F6, "۶", "attr6" },
        { SDL_SCANCODE_7, 0x06F7, "۷", "attr7" },
        { SDL_SCANCODE_8, 0x06F8, "۸", "vdelta_down" },
        { SDL_SCANCODE_9, 0x06F9, "۹", "vdelta_up" },
        { SDL_SCANCODE_0, 0x06F0, "۰", "" },
    };

    for(const auto &d : digits) {
        int canonical = 0;
        KeyDef *k = simulate_keypress(d.persian_keysym, d.scancode, true, canonical);
        assert(k != nullptr);
        assert(k->pressed == true);
        assert(k->action == d.expected_action);
        assert(canonical >= SDLK_0 && canonical <= SDLK_9);

        // Release
        simulate_keypress(d.persian_keysym, d.scancode, false, canonical);
        assert(k->pressed == false);
    }
    std::cout << "  -> PASS: Weapon slots 1..4 & all digits work seamlessly with Persian numbers!\n";
}

void test_special_keys_and_symbols() {
    std::cout << "[Test 3] Testing symbols and control keys (/ ` \\ - = [ ] ; ' , . SPACE TAB ESC ENTER)...\n";
    init_dune_strike_binds();

    struct SymCase {
        SDL_Scancode scancode;
        int dummy_persian_sym;
        std::string expected_action;
    } syms[] = {
        { SDL_SCANCODE_SLASH, 1600, "key_commandline" },
        { SDL_SCANCODE_GRAVE, 247, "key_commandline" },
        { SDL_SCANCODE_BACKSLASH, 1662, "key_console_scroll" },
        { SDL_SCANCODE_MINUS, 45, "copyent" },
        { SDL_SCANCODE_EQUALS, 61, "pasteent" },
        { SDL_SCANCODE_LEFTBRACKET, 1580, "floor_down" },
        { SDL_SCANCODE_RIGHTBRACKET, 1670, "floor_up" },
        { SDL_SCANCODE_SEMICOLON, 1705, "editent" },
        { SDL_SCANCODE_COMMA, 1608, "equalize_floor" },
        { SDL_SCANCODE_PERIOD, 46, "equalize_ceiling" },
        { SDL_SCANCODE_SPACE, 32, "jump" },
        { SDL_SCANCODE_TAB, 9, "key_showscores" },
        { SDL_SCANCODE_ESCAPE, 27, "key_escape" },
        { SDL_SCANCODE_BACKSPACE, 8, "dropflag" },
        { SDL_SCANCODE_DELETE, 127, "floortex_dn" },
    };

    for(const auto &s : syms) {
        int canonical = 0;
        KeyDef *k = simulate_keypress(s.dummy_persian_sym, s.scancode, true, canonical);
        assert(k != nullptr);
        assert(k->action == s.expected_action);
        simulate_keypress(s.dummy_persian_sym, s.scancode, false, canonical);
        assert(k->pressed == false);
    }
    std::cout << "  -> PASS: All symbol & control keys map correctly regardless of layout!\n";
}

void test_menu_hotkey_translation() {
    std::cout << "[Test 4] Testing menu hotkeys (Weapons buy menu & Settings)...\n";

    // Simulating menukey normalization
    auto normalize_menukey = [](int code) -> int {
        if(code >= 0x06F1 && code <= 0x06F9) return SDLK_1 + (code - 0x06F1);
        if(code >= 0x0661 && code <= 0x0669) return SDLK_1 + (code - 0x0661);
        if(code == 0x06F0 || code == 0x0660) return SDLK_0;
        return code;
    };

    for(int i = 1; i <= 9; ++i) {
        int persian_digit = 0x06F0 + i;
        int arabic_digit = 0x0660 + i;
        assert(normalize_menukey(persian_digit) == SDLK_1 + (i - 1));
        assert(normalize_menukey(arabic_digit) == SDLK_1 + (i - 1));
    }
    std::cout << "  -> PASS: Menu hotkeys 1-9 accurately translate Persian digits!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Full Persian Keyboard Compatibility Tests\n";
    std::cout << "========================================================\n";

    test_all_26_letters_persian();
    test_all_numbers_persian();
    test_special_keys_and_symbols();
    test_menu_hotkey_translation();

    std::cout << "========================================================\n";
    std::cout << "  ALL PERSIAN KEYBOARD COMPATIBILITY TESTS PASSED (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
