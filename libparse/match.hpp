#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  std::pair<std::string, const RouteProps *> matchPathWithLocation(const Routes      &routes,
                                                                   const std::string &path);

  std::string findRouteRoot(const libparse::Domain *domain, const libparse::RouteProps *route);
  std::string findRouteIndex(const libparse::Domain *domain, const libparse::RouteProps *route);
  std::string findResourceInFs(const libhttp::Request &req, const libparse::Domain &domain);
} // namespace libparse
