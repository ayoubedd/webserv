#pragma once
#include "libparse/Config.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Headers.hpp"

#include <utility>
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace libhttp {
    struct Methods {
    typedef size_t size_type;
    enum error{
      OK, 
      FILE_NOT_FOUND,
      FORBIDDEN
    };

    error err;

    struct GetRes {
      int fd;
      std::pair<size_type, size_type> range;
    };
        static std::pair<error,GetRes> Get(std::string path);
        error Deletes(std::string &path);
 };
}