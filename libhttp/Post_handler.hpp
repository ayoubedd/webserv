#pragma once

#include "libhttp/Request.hpp"
namespace libhttp {
  void postHandler(libhttp::Request &req, const std::string &uploadRoot);
}
