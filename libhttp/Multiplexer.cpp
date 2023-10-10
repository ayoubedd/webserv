#include "libhttp/Multiplexer.hpp"
#include "libhttp/Headers.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <string>

static bool isRequestHandlerCgi(const libparse::RouteProps *route) {
  if (route->cgi.second != "defautl path")
    return true;
  return false;
}

libhttp::MultiplexerError libhttp::multiplexer(libnet::Session        *session,
                                               const libparse::Config &config) {
  libhttp::Request       *req = session->reader.requests.front();
  const libparse::Domain *domain = libparse::matchReqWithServer(*req, config);
  const std::pair<std::string, const libparse::RouteProps *> route =
      libparse::matchPathWithLocation(domain->routes, req->reqTarget.path);

  if (isRequestHandlerCgi(route.second)) {

  }

  else if (req->method == "GET") {

  }

  else if (req->method == "DELETE") {

  }

  else if (req->method == "POST") {

  }

  return libhttp::MultiplexerError::UNMATCHED_HANDLER;
}
