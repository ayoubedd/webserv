#pragma once

#include "../libhttp/Request.hpp"
#include "../libhttp/Response.hpp"
#include "../libparse/Config.hpp"
#include "libhttp/Error-generate.hpp"
#include <fcntl.h>
#include <unistd.h>

namespace Webserv {
  struct Logger {
    static const int INFO = 0x01;
    static const int ERROR = 0x02;
    enum error { OK, DUP, OPEN };
    static const char *LOGGERLINE;
    static const char *IPV4;
    static const char *METHOD;
    static const char *VERSIONHTTP;
    static const char *PATH;
    static const char *TIME;
    static void        log(const libhttp::Request &request, int option = 1);
  };
} // namespace Webserv
