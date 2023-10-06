#include "libhttp/Multiplexer.hpp"
#include "libhttp/Headers.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <string>

static const libparse::Domain *matchDomain(const libparse::Config &config,
                                           const std::string      &domain) {
  libparse::Domains::const_iterator iter = config.domains.find(domain);

  if (iter != config.domains.end())
    return &iter->second;
  else
    return config.defaultServer;
}

static const libparse::RouteProps *matchRoute(const libparse::Domain &domain,
                                              const std::string      &path) {
  std::string matchedRoute = libparse::matching(domain, path);

  if (matchedRoute.length())
    return &domain.routes.find(matchedRoute)->second;

  return NULL;
}

static std::string extractHost(libhttp::HeadersMap &headers) {
  libhttp::HeadersMap::iterator hostIter = headers.find("Host");

  if (hostIter != headers.end())
    return hostIter->second;

  return "";
}

static bool isRequestHandlerCgi(const libparse::RouteProps *route) {
  if (route->cgi.first.length() > 0 && route->cgi.second.length() > 0)
    return true;
  return false;
}

libhttp::MultiplexerError libhttp::multiplexer(libnet::Session        *session,
                                               const libparse::Config &config) {
  libhttp::Request           *req = session->reader.requests.front();
  std::string                 host = extractHost(req->headers.headers);
  const libparse::Domain     *domain = matchDomain(config, host);
  const libparse::RouteProps *route = matchRoute(*domain, req->reqTarget.path);

  if (isRequestHandlerCgi(route)) {

  }

  else if (req->method == "GET") {

  }

  else if (req->method == "DELETE") {

  }

  else if (req->method == "POST") {
  }

  return libhttp::MultiplexerError::UNMATCHED_HANDLER;
}
