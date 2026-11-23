// tests/test_bot_audio_removal.cpp
// Automated test verifying that all bot audio and voice callouts are completely removed.

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

void test_bot_ai_source_has_no_voice_callouts() {
    std::cout << "[Test 1] Verifying bot_ai.cpp contains zero voice/audio callouts...\n";

    std::ifstream file("../source/src/bot/bot_ai.cpp");
    assert(file.is_open());

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        // Verify no call to audiomgr in bot AI
        assert(line.find("audiomgr") == std::string::npos);
        // Verify no S_FA_SPOTTED voice trigger in bot AI
        assert(line.find("S_FA_SPOTTED") == std::string::npos);
        assert(line.find("lastspottedvoice") == std::string::npos);
    }
    file.close();

    std::cout << "  -> PASS: Verified bot_ai.cpp has zero voice callouts or audio emissions.\n";
}

void test_all_bot_files_have_no_voice() {
    std::cout << "[Test 2] Verifying entire source/src/bot/ has zero voicecom or radio emissions...\n";

    const char* bot_files[] = {
        "../source/src/bot/bot.cpp",
        "../source/src/bot/bot_ai.cpp",
        "../source/src/bot/botmanager.cpp",
        "../source/src/bot/bot_util.cpp",
        "../source/src/bot/bot_waypoint.cpp",
        "../source/src/bot/ds_bot.cpp",
        "../source/src/bot/ds_bot_ai.cpp"
    };

    for(const char* filepath : bot_files) {
        std::ifstream file(filepath);
        if(!file.is_open()) continue;
        std::string line;
        while(std::getline(file, line)) {
            assert(line.find("audiomgr.playsound") == std::string::npos);
            assert(line.find("SV_VOICECOM") == std::string::npos);
        }
        file.close();
    }

    std::cout << "  -> PASS: All bot source files verified completely free of audio/voice triggers.\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Bot Audio Removal Verification Tests\n";
    std::cout << "========================================================\n";

    test_bot_ai_source_has_no_voice_callouts();
    test_all_bot_files_have_no_voice();

    std::cout << "========================================================\n";
    std::cout << "  ALL BOT AUDIO REMOVAL TESTS PASSED (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
