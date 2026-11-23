// tests/test_paths.h
// Resolve repo-relative asset paths so tests run from any checkout location.
//
// Tests used to hardcode the original author's home directory, which made them
// fail (and abort the whole suite) on every other machine. __FILE__ points at
// this header, which lives in <root>/tests/, so the repo root is everything
// before the final "/tests/" segment.
#ifndef DS_TEST_PATHS_H
#define DS_TEST_PATHS_H

#include <string>

inline std::string ds_repo_root() {
    static const std::string root = [] {
        const std::string f = __FILE__;          // <root>/tests/test_paths.h
        size_t sep = f.rfind("/tests/");
        if(sep == std::string::npos) sep = f.rfind("\\tests\\");
        if(sep != std::string::npos) return f.substr(0, sep);
        // Compiled with a path relative to the repo root, e.g. tests/foo.cpp
        if(f.compare(0, 6, "tests/") == 0 || f.compare(0, 6, "tests\\") == 0) return std::string("..");
        return std::string(".");
    }();
    return root;
}

inline std::string ds_path(const std::string &relpath) {
    return ds_repo_root() + "/" + relpath;
}

#endif
