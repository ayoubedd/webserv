#pragma once

#include "libcgi/Cgi-req.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Request.hpp"
#include <string>
#include <sys/socket.h>

namespace libcgi {

  struct Cgi {
    static char temp[35];
    enum Error {
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

    enum State { INIT, READING_HEADERS, READING_BODY, FIN, ERR };

    sockaddr_in *clientAddr;
    Request      req;
    Respons      res;
    State        state;
    int          fd[2];
    pid_t        pid;
    size_t       bodySize;
    size_t       bufferSize;
    int          cgiInput;
    bool         shouldRead;

    Cgi(sockaddr_in *clientInfo, size_t bufferSize = 8192);

    Error init(libhttp::Request *httpReq, std::string scriptPath, std::string serverName,
               std::string localReqPath, std::string serverPort = "80",
               std::string protocol = "HTTP/1.1", std::string serverSoftware = "WebServ");

    /**
     * this function does not ensure that all bytes were written
     */
    Error write(std::vector<char> &body);
    Error exec();
    Error read();

    /**
     *  this funcion should be run only when status is CGI_FIN
     */
    void clean();

    std::pair<Error, State> handleCgiBuff(char *ptr, size_t len);
  };
} // namespace libcgi
