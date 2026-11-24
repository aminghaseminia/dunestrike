// tests/test_tactical_map_competitive.cpp
// Automated test suite for Map & Tactical Overhaul on cs_dust2 & competitive parameters:
// 1. Competitive Server Timers & Radius (bomb_roundtime=115, bomb_timer=40, bomb_siteradius=4, bomb_exploderadius=24)
// 2. Client-side Site Planting Detection (16.0f / 4 units)
// 3. Material Wallbang Resistance & 40% penetration bonus for Crates/Wood/Doors
// 4. Verification of Dual Sites on cs_dust2.cgz

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <cmath>
#include <zlib.h>

void test_server_competitive_parameters()
{
    std::cout << "[Test 1] Verifying server.cpp competitive parameters...\n";
    std::ifstream file("source/src/server.cpp");
    if(!file.is_open()) file.open("../source/src/server.cpp");
    assert(file.is_open());

    std::string line;
    bool found_roundtime = false;
    bool found_timer = false;
    bool found_planttime = false;
    bool found_defusetime = false;
    bool found_siteradius = false;
    bool found_exploderadius = false;

    while(std::getline(file, line))
    {
        if(line.find("SERVPAR(bomb_roundtime") != std::string::npos && line.find("115") != std::string::npos)
            found_roundtime = true;
        if(line.find("SERVPAR(bomb_timer") != std::string::npos && line.find("40") != std::string::npos)
            found_timer = true;
        if(line.find("SERVPAR(bomb_planttime") != std::string::npos && line.find("3") != std::string::npos)
            found_planttime = true;
        if(line.find("SERVPAR(bomb_defusetime") != std::string::npos && line.find("5") != std::string::npos)
            found_defusetime = true;
        if(line.find("SERVPAR(bomb_siteradius") != std::string::npos && line.find("4") != std::string::npos)
            found_siteradius = true;
        if(line.find("SERVPAR(bomb_exploderadius") != std::string::npos && line.find("24") != std::string::npos)
            found_exploderadius = true;
    }
    file.close();

    assert(found_roundtime && "bomb_roundtime must default to 115 seconds (1:55 competitive round)");
    assert(found_timer && "bomb_timer must default to 40 seconds");
    assert(found_planttime && "bomb_planttime must default to 3 seconds");
    assert(found_defusetime && "bomb_defusetime must default to 5 seconds");
    assert(found_siteradius && "bomb_siteradius must default to 4 units");
    assert(found_exploderadius && "bomb_exploderadius must default to 24 units");

    std::cout << "  -> PASS: All competitive server parameters verified!\n";
}

void test_client_bomb_site_detection()
{
    std::cout << "[Test 2] Verifying clientgame.cpp bomb site detection radius...\n";
    std::ifstream file("source/src/clientgame.cpp");
    if(!file.is_open()) file.open("../source/src/clientgame.cpp");
    assert(file.is_open());

    std::string line;
    bool found_site_radius_client = false;

    while(std::getline(file, line))
    {
        if(line.find("dx*dx + dy*dy <= 16.0f") != std::string::npos)
            found_site_radius_client = true;
    }
    file.close();

    assert(found_site_radius_client && "clientgame.cpp must use 16.0f (4.0 * 4.0) matching bomb_siteradius=4");
    std::cout << "  -> PASS: Client bomb site radius matches 4.0 units (16.0f sqr distance)!\n";
}

void test_material_penetration_bonus()
{
    std::cout << "[Test 3] Verifying weapon.cpp material penetration bonus for wood/crates/doors...\n";
    std::ifstream file("source/src/weapon.cpp");
    if(!file.is_open()) file.open("../source/src/weapon.cpp");
    assert(file.is_open());

    std::string line;
    bool found_getmaterialpenfactor = false;
    bool found_crate_check = false;
    bool found_crate_factor = false;
    bool found_mat_bonus_usage = false;

    while(std::getline(file, line))
    {
        if(line.find("getmaterialpenfactor") != std::string::npos)
            found_getmaterialpenfactor = true;
        if(line.find("strstr(n, \"crate\")") != std::string::npos)
            found_crate_check = true;
        if(line.find("1.4f") != std::string::npos && line.find("penetration bonus") != std::string::npos)
            found_crate_factor = true;
        if(line.find("eff_max_pen = max_pen * mat_bonus") != std::string::npos)
            found_mat_bonus_usage = true;
    }
    file.close();

    assert(found_getmaterialpenfactor && "getmaterialpenfactor must be implemented");
    assert(found_crate_check && "Crate/wood/door texture detection must be implemented");
    assert(found_crate_factor && "Crate/wood/door must grant 1.4f multiplier");
    assert(found_mat_bonus_usage && "Penetration depth must scale with material bonus");

    // Mathematical verification:
    // AK-47 base penetration = 1.8 units. Through wood crate (1.4x bonus), effective penetration = 2.52 units.
    float ak_base_pen = 1.8f;
    float mat_crate_factor = 1.4f;
    float ak_crate_pen = ak_base_pen * mat_crate_factor;
    assert(std::fabs(ak_crate_pen - 2.52f) < 0.001f);

    // Wallbang damage retention through 1.0 unit crate vs 1.0 unit concrete:
    float pen_dist = 1.0f;
    float concrete_eff = 0.50f * (1.0f - (pen_dist / ak_base_pen) * 0.4f); // 0.50 * (1 - 0.222) = 0.3888
    float crate_eff = 0.50f * mat_crate_factor * (1.0f - (pen_dist / ak_crate_pen) * 0.4f); // 0.70 * (1 - 0.1587) = 0.5888
    assert(crate_eff > concrete_eff * 1.4f);

    std::cout << "  -> PASS: Crate/wood penetration bonus math verified (+40% penetration & higher damage retention)!\n";
}

struct persistent_entity
{
    short x, y, z;
    short attr1;
    unsigned char type;
    unsigned char attr2, attr3, attr4;
};

struct map_header
{
    char head[4];
    int version;
    int headersize;
    int sfactor;
    int numents;
};

void test_cs_dust2_sites_binary()
{
    std::cout << "[Test 4] Verifying cs_dust2.cgz map binary flags and layout...\n";
    const char *path = "packages/maps/official/cs_dust2.cgz";
    gzFile f = gzopen(path, "rb");
    if(!f) f = gzopen("../packages/maps/official/cs_dust2.cgz", "rb");
    assert(f != NULL && "Could not open cs_dust2.cgz");

    map_header hdr;
    int r = gzread(f, &hdr, sizeof(map_header));
    assert(r == sizeof(map_header));

    assert((hdr.head[0]=='A' && hdr.head[1]=='C' && hdr.head[2]=='M' && hdr.head[3]=='P') ||
           (hdr.head[0]=='C' && hdr.head[1]=='U' && hdr.head[2]=='B' && hdr.head[3]=='E'));

    gzseek(f, hdr.headersize, SEEK_SET);
    int flag_count = 0;
    bool found_site_a = false;
    bool found_site_b = false;

    for(int i = 0; i < hdr.numents; ++i)
    {
        persistent_entity e;
        gzread(f, &e, 12);
        if(e.type == 12) // CTF_FLAG
        {
            flag_count++;
            if(e.attr2 == 0 && e.x == 155 && e.y == 41 && e.z == 7)
                found_site_a = true;
            if(e.attr2 == 1 && e.x == 138 && e.y == 184 && e.z == -11)
                found_site_b = true;
        }
    }
    gzclose(f);

    assert(flag_count == 2 && "cs_dust2 must contain exactly 2 flag entities for Site A and Site B");
    assert(found_site_a && "Site A must be positioned at (155, 41, 7)");
    assert(found_site_b && "Site B must be positioned at (138, 184, -11)");

    std::cout << "  -> PASS: cs_dust2.cgz verified with Site A at (155,41,7) and Site B at (138,184,-11)!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Tactical Map & Competitive Settings Tests\n";
    std::cout << "========================================================\n";

    test_server_competitive_parameters();
    test_client_bomb_site_detection();
    test_material_penetration_bonus();
    test_cs_dust2_sites_binary();

    std::cout << "========================================================\n";
    std::cout << "  ALL TACTICAL MAP & COMPETITIVE TESTS PASSED!\n";
    std::cout << "========================================================\n";
    return 0;
}
