// tests/test_phase3_spatial_audio.cpp
// Automated test suite for Dune Strike Phase 3:
// 1. Spatial Audio & Wall Occlusion (Raycast Los, 45% volume attenuation, 0.88 pitch muffling)
// 2. Footstep Materials (Wood, Metal, Dirt, Concrete, Water)
// 3. Soundcfg Registration and Audio File Assets

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <sys/stat.h>

#include "test_paths.h"

void test_spatial_wall_occlusion()
{
    std::cout << "[Test 1] Verifying Spatial Wall Occlusion in soundlocation.cpp...\n";

    std::ifstream sl_file(ds_path("source/src/soundlocation.cpp"));
    assert(sl_file.is_open());

    std::string line;
    bool found_raycubelos = false;
    bool found_occlusion_factor = false;
    bool found_volume_attenuation = false;
    bool found_pitch_muffling = false;

    while(std::getline(sl_file, line))
    {
        if(line.find("raycubelos(camera1->o, pos, 0.5f)") != std::string::npos) found_raycubelos = true;
        if(line.find("occlusion_factor = 0.55f") != std::string::npos) found_occlusion_factor = true;
        if(line.find("setvolume(forcedvol * occlusion_factor)") != std::string::npos ||
           line.find("setvolume(occlusion_factor)") != std::string::npos) found_volume_attenuation = true;
        if(line.find("basepitch * 0.88f") != std::string::npos) found_pitch_muffling = true;
    }
    sl_file.close();

    assert(found_raycubelos && "Must check line of sight between listener camera and 3D sound position");
    assert(found_occlusion_factor && "Must set occlusion factor to 0.55f (45% attenuation)");
    assert(found_volume_attenuation && "Must apply occlusion factor to volume/gain");
    assert(found_pitch_muffling && "Must apply 0.88f pitch muffling to occluded sounds");

    std::cout << "  -> PASS: Spatial audio wall occlusion (raycast, 0.55f vol, 0.88f pitch) verified!\n";
}

void test_footstep_materials()
{
    std::cout << "[Test 2] Verifying Footstep Materials & Texture Pattern Detection...\n";

    // 1. Verify sound.h has material enum and footstep sound enums
    std::ifstream sh_file(ds_path("source/src/sound.h"));
    assert(sh_file.is_open());

    std::string line;
    bool found_mat_enum = false;
    bool found_wood_sound = false;
    bool found_metal_sound = false;
    bool found_dirt_sound = false;

    while(std::getline(sh_file, line))
    {
        if(line.find("MAT_CONCRETE") != std::string::npos) found_mat_enum = true;
        if(line.find("S_FOOTSTEPS_WOOD") != std::string::npos) found_wood_sound = true;
        if(line.find("S_FOOTSTEPS_METAL") != std::string::npos) found_metal_sound = true;
        if(line.find("S_FOOTSTEPS_DIRT") != std::string::npos) found_dirt_sound = true;
    }
    sh_file.close();

    assert(found_mat_enum);
    assert(found_wood_sound);
    assert(found_metal_sound);
    assert(found_dirt_sound);

    // 2. Verify audiomanager.cpp implements get_floor_material
    std::ifstream am_file(ds_path("source/src/audiomanager.cpp"));
    assert(am_file.is_open());

    bool found_get_mat_fn = false;
    bool found_wood_check = false;
    bool found_metal_check = false;
    bool found_dirt_check = false;

    while(std::getline(am_file, line))
    {
        if(line.find("int get_floor_material(const vec &pos)") != std::string::npos) found_get_mat_fn = true;
        if(line.find("MAT_WOOD") != std::string::npos && line.find("stepsound = S_FOOTSTEPS_WOOD") != std::string::npos) found_wood_check = true;
        if(line.find("MAT_METAL") != std::string::npos && line.find("stepsound = S_FOOTSTEPS_METAL") != std::string::npos) found_metal_check = true;
        if(line.find("MAT_DIRT") != std::string::npos && line.find("stepsound = S_FOOTSTEPS_DIRT") != std::string::npos) found_dirt_check = true;
    }
    am_file.close();

    assert(found_get_mat_fn);
    assert(found_wood_check);
    assert(found_metal_check);
    assert(found_dirt_check);

    std::cout << "  -> PASS: Material classification and footstep sound dispatch verified!\n";
}

void test_audio_assets()
{
    std::cout << "[Test 3] Verifying Material Footstep Audio File Assets on Disk...\n";

    const char *files[] = {
        "packages/audio/player/footsteps.ogg",
        "packages/audio/player/footsteps_wood.ogg",
        "packages/audio/player/footsteps_metal.ogg",
        "packages/audio/player/footsteps_dirt.ogg"
    };

    for(int i = 0; i < 4; i++)
    {
        struct stat st;
        std::string path = files[i];
        if(stat(path.c_str(), &st) != 0)
        {
            path = std::string("../") + files[i];
            assert(stat(path.c_str(), &st) == 0 && "Audio asset file must exist");
        }
        assert(st.st_size > 1000 && "Audio asset file must be non-empty valid audio");
        std::cout << "  -> Found asset " << files[i] << " (" << st.st_size << " bytes)\n";
    }

    std::cout << "  -> PASS: All footstep audio assets verified on disk!\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  Dune Strike: Phase 3 Spatial Audio & Materials Test   \n";
    std::cout << "========================================================\n";

    test_spatial_wall_occlusion();
    test_footstep_materials();
    test_audio_assets();

    std::cout << "========================================================\n";
    std::cout << "  ALL PHASE 3 TESTS PASSED SUCCESSFULLY! (100%)         \n";
    std::cout << "========================================================\n";
    return 0;
}
