#include "libcgi/cgi.hpp"

libcgi::Cgi::Cgi(std::string scriptPath, libhttp::Request *httpReq, sockaddr *clientAddr)
    : httpReq(httpReq)
    , scriptPath(scriptPath)
    , clientAddr(clientAddr) {}
