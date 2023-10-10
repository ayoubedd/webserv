#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  std::pair<std::string, const RouteProps *> matchPathWithLocation(const Routes      &routes,
                                                                   const std::string &path);

  std::string findRouteRoot(const libparse::Domain *domain, const std::string &route);
  std::string findRouteIndex(const libparse::Domain *domain, const std::string &route);
} // namespace libparse
