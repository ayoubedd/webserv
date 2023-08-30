#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request-target.hpp"
#include <iostream>
#include <netinet/ip.h>
#include <vector>

namespace libhttp {
  struct Request {
    std::string       method;
    RequestTarget     reqTarget;
    std::string       version;
    Headers           headers;
    std::vector<char> body;
    sockaddr_in      *clientAddr;
  };
} // namespace libhttp

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req);
