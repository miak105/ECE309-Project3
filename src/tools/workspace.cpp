// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    
    // Step 1: Reject an empty path
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }

    // Step 2: Reject null characters
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "path contains a null character";
        return false;
    }

    // Step 3: Reject absolute paths
    fs::path p{std::string(user_path)};

    if (p.is_absolute() || p.has_root_path()) {
        *reason = "absolute paths are not allowed";
        return false;
    }

    // Step 4: Normalize the path
    fs::path normalized = p.lexically_normal();

    // Step 5: Prevent escaping the workspace
    if (normalized.begin() != normalized.end() &&
        *normalized.begin() == "..") {

        *reason = "path escapes the workspace";
        return false;
    }

    // Step 6: Build the final path
    if (normalized == ".") {
        *out = root();
    } else {
        *out = root() / normalized;
    }

    return true;
}
