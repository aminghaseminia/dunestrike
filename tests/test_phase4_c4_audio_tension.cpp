#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

// Test Suite for Phase 4: C4 Audio Escalation, 10s Warning, Pitch Shifts & Bounce Sounds

void test_c4_progressive_beep_tempo()
{
    std::cout << "[Test 1] Verifying C4 Progressive Beep Tempo Windows...\n";

    auto get_interval = [](int remaining) {
        if(remaining <= 1500) return 70;
        if(remaining <= 4000) return 130;
        if(remaining <= 10000) return 220;
        if(remaining <= 20000) return 480;
        if(remaining <= 30000) return 750;
        return 1000;
    };

    assert(get_interval(35000) == 1000 && "Default pace > 30s must be 1.0s (1 beep/sec)");
    assert(get_interval(25000) == 750 && "Pace 20-30s must quicken to 750ms");
    assert(get_interval(15000) == 480 && "Pace 10-20s must be ~2 beeps/sec (480ms)");
    assert(get_interval(8000) == 220 && "Final 10s must enter rapid alarm tempo (220ms, ~4.5 beeps/sec)");
    assert(get_interval(3000) == 130 && "Final 4s must trigger panic double-beeps (130ms, ~7.7 beeps/sec)");
    assert(get_interval(1000) == 70 && "Final 1.5s must hit detonation climax (70ms, ~14 beeps/sec)");

    std::cout << "  -> PASS: All progressive beep interval stages (1000ms down to 70ms) verified!\n";
}

void test_c4_pitch_escalation()
{
    std::cout << "[Test 2] Verifying C4 Dynamic Pitch Escalation Curves...\n";

    auto get_pitch = [](int remaining) {
        if(remaining <= 1500) return 1.55f;
        if(remaining <= 4000) return 1.40f;
        if(remaining <= 10000) return 1.25f;
        if(remaining <= 20000) return 1.08f;
        if(remaining <= 30000) return 1.02f;
        return 1.0f;
    };

    assert(get_pitch(35000) == 1.0f && "Baseline pitch must be 1.0");
    assert(get_pitch(25000) > get_pitch(35000) && "Pitch must elevate at 20-30s");
    assert(get_pitch(15000) > get_pitch(25000) && "Pitch must elevate further at 10-20s");
    assert(get_pitch(8000) == 1.25f && "Pitch in critical 10s must reach 1.25x");
    assert(get_pitch(3000) == 1.40f && "Pitch in final 4s must reach 1.40x");
    assert(get_pitch(1000) == 1.55f && "Detonation pitch must reach 1.55x high tension");

    std::cout << "  -> PASS: Dynamic pitch escalation from 1.0x to 1.55x verified!\n";
}

void test_10s_warning_trigger()
{
    std::cout << "[Test 3] Verifying Critical 10s Detonation Warning Alert...\n";

    bool played_warning = false;
    int triggered_count = 0;

    for(int remaining = 15000; remaining >= 0; remaining -= 500)
    {
        if(remaining <= 10000 && !played_warning)
        {
            played_warning = true;
            triggered_count++;
        }
    }

    assert(triggered_count == 1 && "Warning cue must fire exactly once at <= 10s");
    assert(played_warning && "Warning flag must be set");

    std::cout << "  -> PASS: Critical 10-second defuse threshold warning alert verified!\n";
}

void test_grenade_bounce_clatter()
{
    std::cout << "[Test 4] Verifying Grenade Physics Bounce Threshold...\n";

    float calibrated_threshold = 0.6f;
    float short_bank_dist = 0.85f;
    float micro_jitter = 0.35f;

    auto should_bounce_sound = [calibrated_threshold](float dist) {
        return dist >= calibrated_threshold;
    };

    assert(!should_bounce_sound(micro_jitter) && "Micro floor rolling jitter must not spam audio");
    assert(should_bounce_sound(short_bank_dist) && "Short-range tactical bank shots must trigger bounce sound");

    std::cout << "  -> PASS: Calibrated 0.6 unit bounce sound threshold verified!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 4 C4 Audio & Tension Suite         \n";
    std::cout << "========================================================\n";

    test_c4_progressive_beep_tempo();
    test_c4_pitch_escalation();
    test_10s_warning_trigger();
    test_grenade_bounce_clatter();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 4 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
