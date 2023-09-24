#pragma once

#include "libcgi/Cgi-req.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Request.hpp"
#include <string>
#include <sys/socket.h>

namespace libcgi {

  struct Cgi {
    enum error {
      OK,
      FAILED_OPEN_FILE,
      FAILED_EXEC_PERM,
      FAILED_OPEN_PIPE,
      FAILED_FORK,
      FAILED_WRITE,
      FAILED_READ
    };

    libhttp::Request    *httpReq;
    std::string          scriptPath;
    sockaddr_in         *clientAddr;
    CgiRequest           req;
    Respons              res;
    int                  fd[2];
    pid_t                pid;
    size_t               bodySize;
    libnet::SessionState state;

    Cgi(libhttp::Request *httpReq, std::string scriptPath, sockaddr_in *clientInfo);

    error init(std::string serverName, std::string scriptName, std::string localReqPath,
               std::string serverPort = "80", std::string protocol = "HTTP/1.1",
               std::string serverSoftware = "WebServ");

    error write(std::vector<char> &body);
    error exec();
    error read();
    void  clean();

    libnet::SessionState handleCgiBuff(char *ptr, size_t len);
  };
} // namespace libcgi
