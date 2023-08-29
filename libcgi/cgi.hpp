#pragma once

#include "libhttp/Request.hpp"
#include <string>
#include <sys/socket.h>

namespace libcgi {
  struct Cgi {
    libhttp::Request *httpReq;
    std::string       scriptPath;
    sockaddr         *clientAddr;

    Cgi(std::string scriptPath, libhttp::Request *httpReq, sockaddr *clientInfo);
  };
} // namespace libcgi
