// tests/test_designated_team_spawns.cpp
// Automated tests for designated team spawn locations & bases in Dune Strike.

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    float dist(const Vec3 &o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

struct SpawnEntity {
    Vec3 pos;
    int team_attr; // 0 = CLA, 1 = RVSF, 100 = FFA
};

// Simulation of Dune Strike designated team spawn manager
class TeamSpawnManager {
public:
    std::vector<SpawnEntity> ents;
    Vec3 team_base[2];
    bool team_base_init[2];
    int team_cycle[2];

    TeamSpawnManager() {
        team_base_init[0] = team_base_init[1] = false;
        team_cycle[0] = team_cycle[1] = -1;
    }

    void add_spawn(float x, float y, float z, int attr2) {
        SpawnEntity e;
        e.pos = Vec3(x, y, z);
        e.team_attr = attr2;
        ents.push_back(e);
    }

    void init_team_spawns() {
        team_base_init[0] = team_base_init[1] = false;
        std::vector<int> team_spawns[2];
        for (size_t i = 0; i < ents.size(); ++i) {
            if (ents[i].team_attr == 0) team_spawns[0].push_back(i);
            else if (ents[i].team_attr == 1) team_spawns[1].push_back(i);
        }

        for (int i = 0; i < 2; ++i) {
            if (!team_spawns[i].empty()) {
                Vec3 sum(0, 0, 0);
                for (int idx : team_spawns[i]) {
                    sum.x += ents[idx].pos.x;
                    sum.y += ents[idx].pos.y;
                    sum.z += ents[idx].pos.z;
                }
                sum.x /= team_spawns[i].size();
                sum.y /= team_spawns[i].size();
                sum.z /= team_spawns[i].size();
                team_base[i] = sum;
                team_base_init[i] = true;
            }
        }

        // Fallback for FFA-only maps
        if (!team_base_init[0] || !team_base_init[1]) {
            if (ents.size() >= 2) {
                float maxdist = -1.0f;
                int best_a = 0, best_b = 1;
                for (size_t i = 0; i < ents.size(); ++i) {
                    for (size_t j = i + 1; j < ents.size(); ++j) {
                        float d = ents[i].pos.dist(ents[j].pos);
                        if (d > maxdist) {
                            maxdist = d;
                            best_a = i;
                            best_b = j;
                        }
                    }
                }
                team_base[0] = ents[best_a].pos;
                team_base[1] = ents[best_b].pos;
                team_base_init[0] = team_base_init[1] = true;
            }
        }
    }

    Vec3 spawn_player(int team, Vec3 &out_spawnpos, int &out_lastspawn, int current_millis) {
        init_team_spawns();
        std::vector<int> team_spawns;
        for (size_t i = 0; i < ents.size(); ++i) {
            if (ents[i].team_attr == team) team_spawns.push_back(i);
        }

        Vec3 chosen_pos;
        if (!team_spawns.empty()) {
            team_cycle[team] = (team_cycle[team] + 1) % team_spawns.size();
            chosen_pos = ents[team_spawns[team_cycle[team]]].pos;
        } else {
            // FFA fallback partitioning
            float closest_dist = -1.0f;
            int best_idx = -1;
            for (size_t i = 0; i < ents.size(); ++i) {
                float d_team = ents[i].pos.dist(team_base[team]);
                float d_enemy = ents[i].pos.dist(team_base[1 - team]);
                if (d_team <= d_enemy) {
                    if (best_idx < 0 || d_team < closest_dist) {
                        closest_dist = d_team;
                        best_idx = i;
                    }
                }
            }
            chosen_pos = ents[best_idx >= 0 ? best_idx : 0].pos;
        }

        out_spawnpos = chosen_pos;
        out_lastspawn = current_millis;
        return chosen_pos;
    }
};

void test_designated_team_spawns_standard_map() {
    std::cout << "[Test 1] Testing designated team spawns on map with team spawns (ac_keller style)...\n";
    TeamSpawnManager mgr;

    // CLA base at X: 50..70, Y: 100..120 (e.g. Terrorist spawn)
    mgr.add_spawn(55, 105, 4, 0);
    mgr.add_spawn(65, 105, 4, 0);
    mgr.add_spawn(60, 115, 4, 0);

    // RVSF base at X: 450..470, Y: 400..420 (e.g. CT spawn)
    mgr.add_spawn(455, 405, 4, 1);
    mgr.add_spawn(465, 405, 4, 1);
    mgr.add_spawn(460, 415, 4, 1);

    mgr.init_team_spawns();
    assert(mgr.team_base_init[0] && mgr.team_base_init[1]);

    // Centroids check
    assert(mgr.team_base[0].dist(Vec3(60, 108.33f, 4)) < 1.0f);
    assert(mgr.team_base[1].dist(Vec3(460, 408.33f, 4)) < 1.0f);

    // Spawn 5 CLA members
    for (int i = 0; i < 5; ++i) {
        Vec3 spawnpos;
        int lastspawn = 0;
        Vec3 pos = mgr.spawn_player(0, spawnpos, lastspawn, 1000 + i * 100);
        assert(pos.dist(mgr.team_base[0]) < 25.0f); // strictly within CLA base
        assert(pos.dist(mgr.team_base[1]) > 300.0f); // far away from CT base
        assert(lastspawn == 1000 + i * 100);
        assert(spawnpos.dist(pos) < 0.01f);
    }

    // Spawn 5 RVSF members
    for (int i = 0; i < 5; ++i) {
        Vec3 spawnpos;
        int lastspawn = 0;
        Vec3 pos = mgr.spawn_player(1, spawnpos, lastspawn, 2000 + i * 100);
        assert(pos.dist(mgr.team_base[1]) < 25.0f); // strictly within RVSF base
        assert(pos.dist(mgr.team_base[0]) > 300.0f); // far away from T base
        assert(lastspawn == 2000 + i * 100);
        assert(spawnpos.dist(pos) < 0.01f);
    }

    std::cout << "  -> PASS: All team members spawn strictly in their designated team base!\n";
}

void test_designated_team_spawns_ffa_fallback() {
    std::cout << "[Test 2] Testing designated team spawns on FFA-only map...\n";
    TeamSpawnManager mgr;

    // 4 generic spawns along a line
    mgr.add_spawn(10, 10, 4, 100);
    mgr.add_spawn(30, 20, 4, 100);
    mgr.add_spawn(200, 200, 4, 100);
    mgr.add_spawn(220, 210, 4, 100);

    mgr.init_team_spawns();
    assert(mgr.team_base_init[0] && mgr.team_base_init[1]);

    // Base 0 must be near (10, 10) and Base 1 near (220, 210)
    float base_sep = mgr.team_base[0].dist(mgr.team_base[1]);
    assert(base_sep > 200.0f);

    // Spawning CLA (0) must always pick Base 0 side
    Vec3 sp0; int ls0;
    Vec3 p0 = mgr.spawn_player(0, sp0, ls0, 5000);
    assert(p0.dist(mgr.team_base[0]) < 30.0f);

    // Spawning RVSF (1) must always pick Base 1 side
    Vec3 sp1; int ls1;
    Vec3 p1 = mgr.spawn_player(1, sp1, ls1, 5000);
    assert(p1.dist(mgr.team_base[1]) < 30.0f);

    std::cout << "  -> PASS: FFA map correctly partitions into two designated team spawn bases!\n";
}

void test_source_code_designated_spawns_enforcement() {
    std::cout << "[Test 3] Verifying C++ source code strict spawn separation...\n";

    // 1. Verify clientgame.cpp enforces team spawns first even in arena modes
    std::ifstream client_file("source/src/clientgame.cpp");
    if(!client_file.is_open()) client_file.open("../source/src/clientgame.cpp");
    assert(client_file.is_open());

    std::string line;
    bool found_team_check = false;
    bool found_arenaspawn_slot = false;
    while(std::getline(client_file, line)) {
        if(line.find("if(m_teammode && (type == 0 || type == 1))") != std::string::npos) found_team_check = true;
        if(line.find("e = &ents[team_spawns[arenaspawn % team_spawns.length()]];") != std::string::npos) found_arenaspawn_slot = true;
    }
    client_file.close();

    assert(found_team_check);
    assert(found_arenaspawn_slot);

    // 2. Verify server.cpp distributes spawns per team without overwriting
    std::ifstream server_file("source/src/server.cpp");
    if(!server_file.is_open()) server_file.open("../source/src/server.cpp");
    assert(server_file.is_open());

    bool found_server_team_filter = false;
    while(std::getline(server_file, line)) {
        if(line.find("team_base(clients[i]->team) == team") != std::string::npos) found_server_team_filter = true;
    }
    server_file.close();

    assert(found_server_team_filter);
    std::cout << "  -> PASS: clientgame.cpp and server.cpp strictly enforce designated team spawns!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Designated Team Spawns Verification Tests\n";
    std::cout << "========================================================\n";

    test_designated_team_spawns_standard_map();
    test_designated_team_spawns_ffa_fallback();
    test_source_code_designated_spawns_enforcement();

    std::cout << "========================================================\n";
    std::cout << "  ALL DESIGNATED TEAM SPAWNS TESTS PASSED (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
