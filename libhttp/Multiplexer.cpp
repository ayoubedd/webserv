#include "libhttp/Multiplexer.hpp"
#include "libhttp/Headers.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <string>

static const libparse::Domain *matchDomain(const libparse::Domains &domains,
                                           const std::string       &domain) {
  libparse::Domains::const_iterator iter = domains.find(domain);

  if (iter != domains.end())
    return &iter->second;
  else
    return &domains.begin()->second;
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

static bool isRequestHandlerCgi() { return false; }

void libhttp::multiplexer(libnet::Session *session, const libparse::Domains &domains) {
  libhttp::Request       *req = session->reader.requests.front();
  std::string             host = extractHost(req->headers.headers);
  const libparse::Domain *domain = matchDomain(domains, host);

  if (domain == NULL) {
    // config file contains zero domains
    return;
  }

  const libparse::RouteProps *route = matchRoute(*domain, req->reqTarget.path);

  if (route == NULL) {
    // No matched route
    return;
  }

  if (isRequestHandlerCgi()) {

  }

  if (req->method == "GET" || req->method == "DELETE") {

  }

  else if (req->method == "POST") {

  }

  else {
    // Unsupported Method
  }
}
