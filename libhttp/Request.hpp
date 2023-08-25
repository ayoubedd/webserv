#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Request-target.hpp"
#include <iostream>
#include <vector>

namespace libhttp {
  struct Request {
    std::string method;
    RequestTarget reqTarget;
    std::string version;
    Headers headers;
    std::vector<char> body;
  };
} // namespace libhttp

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req);
