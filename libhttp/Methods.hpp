#pragma once
#include "libparse/Config.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Response.hpp"

#include <utility>
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

namespace libhttp {
    struct Methods {
    enum error{
      OK, 
      FILE_NOT_FOUND,
      FORBIDDEN
    };
    struct file{
      std::string name;
      std::string date;
      ssize_t size;
    };
    enum typeFile{
      FILE,
      DIR,
      NOT_FOUND
    };
    error err;
    struct GetRes {
      int fd;
      std::vector<char > headers;
      std::pair<int,int> range;
    };

 };    
  std::pair<Methods::error,Response > Get(Request &request, std::string path);
  std::pair<Methods::error,Response > Delete(std::string path);
  ssize_t getFile(std::string &path,int status);
}
std::string generateTemplate(std::string &path);