#pragma once
#include "libhttp/Headers.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include "libparse/Config.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace libhttp {
  struct Methods {
    static const char *GET;
    static const char *POST;
    static const char *DELETE;

    enum error { OK, FILE_NOT_FOUND, FORBIDDEN,OUT_RANGE,REDIR};
    struct file {
      std::string name;
      std::string date;
      ssize_t     size;
      int         fd;
    };
    enum typeFile { FILE, DIR, NOT_FOUND };
    error err;
  };
  std::pair<Methods::error, Response *> Get(Request &request, std::string path);
  std::pair<Methods::error, Response *> Delete(std::string path);
  ssize_t                              getFile(std::string &path, int status);
} // namespace libhttp
std::string generateTemplate(std::string &path);
