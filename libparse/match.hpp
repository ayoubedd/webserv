#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  std::pair<std::string, const RouteProps *> matchPathWithLocation(const Routes      &routes,
                                                                   const std::string &path);
} // namespace libparse
