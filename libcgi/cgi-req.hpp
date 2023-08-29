#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request.hpp"
#include <map>
#include <string>

namespace libcgi {
  struct CgiRequest {
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

    typedef std::map<std::string, std::string> CgiEnv;

    CgiEnv env;

    void convertReqHeadersToCgiHeaders(libhttp::Headers *httpHeaders);
    void addCgiStandardHeaders(libhttp::Request *httpReq);
  };
} // namespace libcgi
