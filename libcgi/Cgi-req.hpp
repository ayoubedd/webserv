#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request.hpp"
#include <map>
#include <string>

namespace libcgi {
  struct Request {
    typedef std::map<std::string, std::string> CgiEnv;

    static const char *AUTH_TYPE; // not used
    static const char *CONTENT_LENGTH;
    static const char *CONTENT_TYPE;
    static const char *GATEWAY_INTERFACE;
    static const char *PATH_INFO;
    static const char *PATH_TRANSLATED;
    static const char *QUERY_STRING;
    static const char *REMOTE_ADDR;
    static const char *REMOTE_HOST;
    static const char *REMOTE_IDENT; // not used
    static const char *REQUEST_METHOD;
    static const char *SCRIPT_NAME;
    static const char *SERVER_NAME;
    static const char *SERVER_PORT;
    static const char *SERVER_PROTOCOL;
    static const char *SERVER_SOFTWARE;
    static const char *REDIRECT_STATUS;

    struct Ctx {
      std::string serverName;
      std::string scriptName;
      std::string protocol;
      std::string serverPort;
      std::string serverSoftware;
      std::string localReqPath;
    };

    std::string scriptPath;

    CgiEnv env;
    Ctx    ctx;

    void init(std::string scriptPath, std::string serverName, std::string scriptName,
              std::string localReqPath, std::string serverPort = "80",
              std::string protocol = "HTTP/1.1", std::string serverSoftware = "WebServ");
    void build(libhttp::Request *httpReq);

    void convertReqHeadersToCgiHeaders(libhttp::Headers *httpHeaders);
    void addCgiStandardHeaders(libhttp::Request *httpReq);
    void clean();
  };
} // namespace libcgi
