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
#include <string>

std::string generateTemplateFiles(std::string &path);
namespace libhttp {
    struct Methods {
    
    typedef size_t size_type;
    enum error{
      OK, 
      FILE_NOT_FOUND,
      FORBIDDEN
    };
    enum typeFile{
      FILE,
      DIR,
      NOT_FOUND
    };
    error err;
    struct GetRes {
      int fd;
      std::pair<int,int> range;
    };

 };    
  std::pair<Methods::error,Methods::GetRes> Get(Request &request);
  Methods::error Deletes(std::string &path);
  std::pair<Methods::error,Methods::GetRes> GetIfDir(std::string &path,std::string &autoFile);
}