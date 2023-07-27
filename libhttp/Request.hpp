#pragma once

#include "Headers.hpp"

namespace libhttp {
  struct Request {
    std::string method;
    std::string path;
    std::string version;
    Headers headers;
    std::string body;
  };
} // namespace libhttp
