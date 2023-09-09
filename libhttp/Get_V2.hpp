#pragma once
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../libparse/Config.hpp"

namespace  libhttp {

  struct Get{
    std::string path;
    std::string type;
    size_t contentLength;
    bool done;
    bool isFile;
    bool isFolder;
    size_t fd;
    size_t fileSize;
    bool error;
    int status;
  };
}

