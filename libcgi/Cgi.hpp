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
    enum error {
      OK,
      FAILED_OPEN_FILE,
      FAILED_EXEC_PERM,
      FILED_OPEN_PIPE,
      FAILED_FORK,
      FAILED_WRITE
    };

    libhttp::Request *httpReq;
    std::string       scriptPath;
    sockaddr_in      *clientAddr;
    CgiRequest        cgiReq;
    int               fd[2];
    pid_t             pid;
    size_t            bodySize;

    Cgi(libhttp::Request *httpReq, std::string scriptPath, sockaddr_in *clientInfo);

    error init(std::string serverName, std::string scriptName, std::string localReqPath,
               std::string serverPort = "80", std::string protocol = "HTTP/1.1",
               std::string serverSoftware = "WebServ");

    error                                  write(std::vector<char> &body);
    std::pair<error, CgiResult>            exec();
    std::pair<error, libnet::SessionState> read(libnet::SessionState state);
  };
} // namespace libcgi
