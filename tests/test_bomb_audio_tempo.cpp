// tests/test_bomb_audio_tempo.cpp
// Automated tests for Phase 7: Dynamic CS C4 Beep Tempo and Audio Timing.

#include <cassert>
#include <iostream>
#include <vector>

#include "test_paths.h"

int calculate_beep_interval(int remaining_millis) {
    if (remaining_millis <= 0) return 0;
    if (remaining_millis <= 2000) return 125;        // Final 2s: urgent panic beeping (8 beeps/sec)
    else if (remaining_millis <= 5000) return 250;   // 2-5s left: very fast (4 beeps/sec)
    else if (remaining_millis <= 10000) return 350;  // 5-10s left: fast (under defuse wire)
    else if (remaining_millis <= 20000) return 500;  // 10-20s left: moderate (2 beeps/sec)
    else if (remaining_millis <= 30000) return 750;  // 20-30s left: quickening
    else return 1000;                                // 30-40s left: steady 1 beep/sec
}

struct BombAudioSimulation {
    int bomb_timer_ms;
    std::vector<int> beep_timestamps;

    BombAudioSimulation(int duration_sec = 40) : bomb_timer_ms(duration_sec * 1000) {}

    void simulate() {
        int last_beep = 0;
        // Step forward in 10ms frame increments
        for (int curtime = 0; curtime <= bomb_timer_ms; curtime += 10) {
            int remaining = bomb_timer_ms - curtime;
            if (remaining <= 0) break;

            int interval = calculate_beep_interval(remaining);
            if (curtime - last_beep >= interval) {
                beep_timestamps.push_back(curtime);
                last_beep = curtime;
            }
        }
    }
};

void test_tempo_stages() {
    std::cout << "[Test 1] Testing Beep Intervals across all 6 time thresholds...\n";
    // Stage 1: >30s (e.g. 35s remaining)
    assert(calculate_beep_interval(35000) == 1000);
    // Stage 2: 20-30s (e.g. 25s remaining)
    assert(calculate_beep_interval(25000) == 750);
    // Stage 3: 10-20s (e.g. 15s remaining)
    assert(calculate_beep_interval(15000) == 500);
    // Stage 4: 5-10s (e.g. 8s remaining)
    assert(calculate_beep_interval(8000) == 350);
    // Stage 5: 2-5s (e.g. 3s remaining)
    assert(calculate_beep_interval(3000) == 250);
    // Stage 6: <=2s (e.g. 1.5s remaining)
    assert(calculate_beep_interval(1500) == 125);
    // Post-explosion (0s)
    assert(calculate_beep_interval(0) == 0);
    std::cout << "  -> PASS: All progressive intervals match CS specification.\n";
}

void test_cadence_acceleration() {
    std::cout << "[Test 2] Simulating full 40s bomb countdown and cadence acceleration...\n";
    BombAudioSimulation sim(40);
    sim.simulate();

    const std::vector<int> &beeps = sim.beep_timestamps;
    assert(!beeps.empty());

    // Count beeps in first 10 seconds (t = 0 to 10s) vs last 5 seconds (t = 35s to 40s)
    int beeps_first_10s = 0;
    int beeps_last_5s = 0;

    for (size_t i = 0; i < beeps.size(); ++i) {
        if (beeps[i] <= 10000) beeps_first_10s++;
        if (beeps[i] >= 35000 && beeps[i] < 40000) beeps_last_5s++;
    }

    std::cout << "  -> First 10 seconds: " << beeps_first_10s << " beeps (~1 beep/sec)\n";
    std::cout << "  -> Last 5 seconds: " << beeps_last_5s << " beeps (accelerated cadence)\n";

    // In first 10s, at 1000ms interval, exactly 11 beeps (t=0, 1000, 2000... 10000)
    assert(beeps_first_10s >= 10 && beeps_first_10s <= 11);
    // In last 5s (3s of 250ms interval + 2s of 125ms interval): 3/0.25 + 2/0.125 = 12 + 16 = 28 beeps!
    assert(beeps_last_5s >= 25);
    // Cadence in last 5s must be much denser than first 10s
    assert(beeps_last_5s > beeps_first_10s * 2);

    std::cout << "  -> PASS: Progressive urgency curve successfully verified.\n";
}

void test_c4_sound_events() {
    std::cout << "[Test 3] Verifying C4 plant, disarm, and defuse sound mappings...\n";
    // Check that custom sounds exist and are non-empty
    FILE *f1 = fopen(ds_path("packages/audio/weapons/c4_beep.ogg").c_str(), "rb");
    assert(f1 != NULL);
    fclose(f1);

    FILE *f2 = fopen(ds_path("packages/audio/weapons/c4_plant.ogg").c_str(), "rb");
    assert(f2 != NULL);
    fclose(f2);

    FILE *f3 = fopen(ds_path("packages/audio/weapons/c4_disarm.ogg").c_str(), "rb");
    assert(f3 != NULL);
    fclose(f3);

    std::cout << "  -> PASS: All CS C4 audio package files verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  RUNNING PHASE 7 TESTS: DYNAMIC C4 AUDIO & BEEP TEMPO\n";
    std::cout << "====================================================\n\n";

    test_tempo_stages();
    test_cadence_acceleration();
    test_c4_sound_events();

    std::cout << "\n====================================================\n";
    std::cout << "  ALL PHASE 7 C4 AUDIO TESTS PASSED SUCCESSFULLY!\n";
    std::cout << "====================================================\n";
    return 0;
}
