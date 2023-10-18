#include "core/Initialization.hpp"
#include "libcgi/Cgi.hpp"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static bool isDirExistAndOk(const std::string &path) {
  if (access(path.c_str(), F_OK | R_OK | W_OK | X_OK))
    return false;
  return true;
}

bool WebServ::initializeFsEnv(void) {
  if (isDirExistAndOk("/tmp/webserv/") == false) {
    std::cerr << "Webserv: create '/tmp/webserv/' with the appropriate permissions." << std::endl;
    return true;
  }

  if (isDirExistAndOk(libcgi::Cgi::blueprint) == false) {
    std::cerr << "Webserv: create '" << libcgi::Cgi::blueprint
              << "' with the appropriate permissions." << std::endl;
    return true;
  }

  return false;
}
