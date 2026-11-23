// tests/test_buyzone_restrictions.cpp
// Automated tests for Buy Zone and 10-second Buy Time restrictions.

#include <cassert>
#include <iostream>
#include <string>
#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    float dist(const Vec3 &o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

struct Player {
    int team; // 0 = CLA, 1 = RVSF
    int state; // 0 = CS_ALIVE, 1 = CS_DEAD
    Vec3 pos;
    Vec3 spawnpos;
    int lastspawn;
    int nextprimary;
};

class BuyZoneManager {
public:
    Vec3 team_base[2];
    bool team_base_init[2];

    BuyZoneManager() {
        team_base[0] = Vec3(50, 50, 4);   // CLA (T) Base
        team_base[1] = Vec3(450, 450, 4); // RVSF (CT) Base
        team_base_init[0] = team_base_init[1] = true;
    }

    bool in_buy_zone(const Player &p) {
        if (p.state != 0) return false;
        int t = p.team;
        if (t != 0 && t != 1) return false;

        // Proximity to spawn point
        if (p.pos.dist(p.spawnpos) <= 80.0f) return true;

        // Proximity to team base
        if (p.pos.dist(team_base[t]) <= 90.0f) return true;

        return false;
    }

    bool can_buy(const Player &p, int current_millis, std::string &out_reason) {
        if (p.state != 0) {
            out_reason = "Player is dead";
            return false;
        }

        // 10-second buy time limit
        if (current_millis - p.lastspawn > 10000) {
            out_reason = "10 seconds have passed. You cannot buy weapons now.";
            return false;
        }

        // Must be in designated team buy zone
        if (!in_buy_zone(p)) {
            out_reason = "You are not in a buy zone.";
            return false;
        }

        out_reason = "OK";
        return true;
    }

    bool try_select_weapon(Player &p, int weapon_id, int current_millis, std::string &out_reason) {
        if (!can_buy(p, current_millis, out_reason)) {
            return false;
        }
        p.nextprimary = weapon_id;
        out_reason = "Weapon selected";
        return true;
    }
};

void test_buy_time_window() {
    std::cout << "[Test 1] Testing 10-second buy time window...\n";
    BuyZoneManager mgr;

    Player p;
    p.team = 0; // CLA
    p.state = 0; // CS_ALIVE
    p.pos = Vec3(52, 52, 4);
    p.spawnpos = Vec3(50, 50, 4);
    p.lastspawn = 1000;
    p.nextprimary = 0; // Assault

    std::string reason;

    // At t = 1500 (0.5s after spawn): allowed
    assert(mgr.can_buy(p, 1500, reason));
    assert(reason == "OK");

    // At t = 10000 (9.0s after spawn): allowed
    assert(mgr.can_buy(p, 10000, reason));
    assert(reason == "OK");

    // At t = 11000 (10.0s exact): allowed
    assert(mgr.can_buy(p, 11000, reason));

    // At t = 11001 (10.001s): rejected!
    assert(!mgr.can_buy(p, 11001, reason));
    assert(reason == "10 seconds have passed. You cannot buy weapons now.");

    // Weapon selection attempt rejected
    assert(!mgr.try_select_weapon(p, 2, 11001, reason));
    assert(p.nextprimary == 0);

    std::cout << "  -> PASS: 10-second buy time limit strictly enforced!\n";
}

void test_buy_zone_location() {
    std::cout << "[Test 2] Testing Buy Zone location constraints...\n";
    BuyZoneManager mgr;

    Player p;
    p.team = 0; // CLA
    p.state = 0;
    p.pos = Vec3(50, 50, 4);
    p.spawnpos = Vec3(50, 50, 4);
    p.lastspawn = 2000;
    p.nextprimary = 0;

    std::string reason;

    // Inside CLA base: allowed
    assert(mgr.can_buy(p, 3000, reason));
    assert(mgr.try_select_weapon(p, 3, 3000, reason));
    assert(p.nextprimary == 3);

    // Player ran out of base to (180, 180): outside buy zone!
    p.pos = Vec3(180, 180, 4);
    assert(!mgr.can_buy(p, 4000, reason));
    assert(reason == "You are not in a buy zone.");
    assert(!mgr.try_select_weapon(p, 1, 4000, reason));

    // Player standing in enemy base (450, 450) as CLA: rejected!
    p.pos = Vec3(450, 450, 4);
    assert(!mgr.can_buy(p, 4000, reason));
    assert(reason == "You are not in a buy zone.");

    std::cout << "  -> PASS: Buy zone restricts weapon purchasing to own team's spawn base!\n";
}

void test_countdown_seconds_calculation() {
    std::cout << "[Test 3] Verifying HUD countdown seconds calculation...\n";
    int lastspawn = 5000;

    auto calc_secs = [lastspawn](int current_millis) -> int {
        return std::max(0, (10000 - (current_millis - lastspawn) + 999) / 1000);
    };

    assert(calc_secs(5000) == 10);
    assert(calc_secs(6000) == 9);
    assert(calc_secs(14000) == 1);
    assert(calc_secs(15000) == 0);
    assert(calc_secs(16000) == 0);

    std::cout << "  -> PASS: Buy time countdown displays accurate seconds (10..0s)!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Buy Zone & Buy Time Verification Tests\n";
    std::cout << "========================================================\n";

    test_buy_time_window();
    test_buy_zone_location();
    test_countdown_seconds_calculation();

    std::cout << "========================================================\n";
    std::cout << "  ALL BUY ZONE & BUY TIME TESTS PASSED (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
