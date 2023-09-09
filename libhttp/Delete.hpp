#pragma once
#include "../libparse/Config.hpp"
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace libhttp {
   struct Delete{
    std::string path;
    int status;
    bool error;
    bool isFile;
    bool isFolder;
   };

}
