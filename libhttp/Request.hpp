#pragma once

#include "libhttp/Headers.hpp"
#include <iostream>

namespace libhttp {
  struct Request {
    std::string method;
    std::string path;
    std::string version;
    Headers headers;
    std::string body;
  };
} // namespace libhttp

std::ostream& operator<<(std::ostream &os, const libhttp::Request &req);