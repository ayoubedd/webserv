#pragma once

#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"

namespace libparse {

  const Domain *matchReqWithServer(const libhttp::Request &req, const libparse::Config &config);

  std::pair<std::string, const RouteProps *> matchPathWithRoute(const Routes      &routes,
                                                                const std::string &path);

  std::string findRouteRoot(const libparse::Routes &routes, const libparse::RouteProps &route);
  std::string findRouteIndex(const libparse::Routes &routes, const libparse::RouteProps &route);
  std::string findResourceInFs(const libhttp::Request &req, const libparse::Domain &domain);
  std::string findUploadDir(const libhttp::Request &req, const libparse::Domain &domain);
  std::string joinPath(const std::string &s1, const std::string &s2);

} // namespace libparse
