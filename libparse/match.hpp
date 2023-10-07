#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  const RouteProps *matchPathWithLocation(const libparse::Domain &domain, const std::string &path);
} // namespace libparse
