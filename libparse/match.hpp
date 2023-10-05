#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  std::string matchPathWithLocaiton(libparse::Domain domain, std::string path);
} // namespace libparse
