#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request-target.hpp"
#include <iostream>
#include <netinet/ip.h>
#include <vector>

namespace libhttp {
  struct Request {
    enum State { R_INIT, R_HEADERS, R_BODY, R_FIN, R_ERR };
    bool              sanitized;
    std::string       method;
    RequestTarget     reqTarget;
    std::string       version;
    Headers           headers;
    std::vector<char> body; // any data pushed here needs to be added to allBodyLen to keep track of
                            // all read Content-Length
    std::vector<char>::size_type allBodyLen;
    sockaddr_in                 *clientAddr;
    State                        state;

    Request(sockaddr_in *clientAddr);
  };
} // namespace libhttp

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req);
