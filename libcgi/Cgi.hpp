#pragma once

#include "libcgi/Cgi-req.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Request.hpp"
#include <string>
#include <sys/socket.h>

namespace libcgi {

  struct Cgi {
    static char temp[35];
    enum error {
      OK,
      FAILED_OPEN_FILE,
      FAILED_OPEN_SCRIPT,
      FAILED_EXEC_PERM,
      FAILED_OPEN_PIPE,
      FAILED_OPEN_DIR,
      FAILED_FORK,
      FAILED_WRITE,
      FAILED_READ,
      MALFORMED
    };

    libhttp::Request    *httpReq;
    std::string          scriptPath;
    sockaddr_in         *clientAddr;
    CgiRequest           req;
    Respons              res;
    libnet::SessionState state;
    int                  fd[2];
    pid_t                pid;
    size_t               bodySize;
    size_t               bufferSize;
    int                  cgiInput;

    Cgi(libhttp::Request *httpReq, std::string scriptPath, sockaddr_in *clientInfo,
        size_t bufferSize = 8192);

    error init(std::string serverName, std::string scriptName, std::string localReqPath,
               std::string serverPort = "80", std::string protocol = "HTTP/1.1",
               std::string serverSoftware = "WebServ");

    /**
     * this function does not ensure that all bytes were written
     */
    error write(std::vector<char> &body);
    error exec();
    error read();

    /**
     *  this funcion should be run only when status is CGI_FIN
     */
    void clean();

    std::pair<error, libnet::SessionState> handleCgiBuff(char *ptr, size_t len);
  };
} // namespace libcgi
