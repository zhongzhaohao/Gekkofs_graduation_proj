//
// Created by evie on 9/4/17.
//

#include <preload/preload_util.hpp>
#include <cstring>

bool is_fs_path(const char* path) {
    return strstr(path, root_path.c_str()) == path;
}