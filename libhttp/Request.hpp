#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request-target.hpp"
#include "libnet/SessionState.hpp"
#include <iostream>
#include <netinet/ip.h>
#include <vector>

// TODO: update allBodyLen in chunked and multiform upload
namespace libhttp {
  struct Request {
    bool              sanitized;
    std::string       method;
    RequestTarget     reqTarget;
    std::string       version;
    Headers           headers;
    std::vector<char> body; // any data pushed here needs to be added to allBodyLen to keep track of
                            // all read Content-Length
    std::vector<char>::size_type allBodyLen;
    sockaddr_in                 *clientAddr;
    libnet::SessionState         state;

    Request(sockaddr_in *clientAddr);
    void expandeRefererHeaderInPath();
  };
} // namespace libhttp

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req);
