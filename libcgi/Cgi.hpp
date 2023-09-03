#pragma once

#include "libcgi/Cgi-req.hpp"
#include "libhttp/Request.hpp"
#include <string>
#include <sys/socket.h>

namespace libcgi {

  struct CgiResult {
    int pid;
    int fd;
  };

  struct Cgi {
    enum error { OK, FAILED_OPEN_FILE, FILED_OPEN_PIPE, FAILED_FORK };

    libhttp::Request *httpReq;
    std::string       scriptPath;
    sockaddr         *clientAddr;
    CgiRequest        cgiReq;

    Cgi(std::string scriptPath, libhttp::Request *httpReq, sockaddr *clientInfo);

    error init(std::string serverName, std::string scriptName, std::string localReqPath,
               std::string serverPort = "80", std::string protocol = "HTTP/1.1",
               std::string serverSoftware = "WebServ");

    std::pair<error, CgiResult> exec();
  };
} // namespace libcgi
