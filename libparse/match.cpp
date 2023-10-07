#include "libparse/match.hpp"
#include "libparse/Config.hpp"
#include <algorithm>
#include <string>

bool matchHostHeaderPortWithDomain(const std::string &domain, const std::string &header) {
  std::string::size_type i;
  std::string            host;

  i = header.find(':');
  host = header;
  if (i == std::string::npos)
    host += ":80";
  return domain == host;
}

const libparse::Domain *libparse::matchReqWithServer(const libhttp::Request &req,
                                                     const libparse::Config &config) {
  libhttp::HeadersMap::const_iterator host;
  libparse::Domains::const_iterator   start, end;

  host = req.headers.headers.find(libhttp::Headers::HOST);
  if (host == req.headers.headers.end()) {
    return config.defaultServer;
  }
  start = config.domains.begin();
  end = config.domains.end();
  while (start != end) {
    if (matchHostHeaderPortWithDomain(start->first, host->second)) {
      return &start->second;
    }
    start++;
  }
  return config.defaultServer;
}

static inline std::string::size_type hasPrefix(std::string str, std::string prefix) {
  std::string::size_type i;

  i = 0;
  while (i < prefix.size() && i < str.size()) {
    if (str[i] != prefix[i])
      break;
    i++;
  }
  if (i == prefix.size())
    return i;
  return 0;
}

const libparse::RouteProps *libparse::matchPathWithLocation(const libparse::Domain &domain,
                                                            const std::string      &path) {
  std::map<std::string, libparse::RouteProps>           routes = domain.routes;
  std::map<std::string, libparse::RouteProps>::iterator it = routes.begin();

  std::string::size_type score = 0;
  std::string            maxMatch;

  maxMatch = "";
  while (it != routes.end()) {
    score = hasPrefix(path, it->first);
    if (score > maxMatch.size())
      maxMatch = it->first;
    it++;
  }
  std::cerr << maxMatch << std::endl;
  if (!maxMatch.size())
    return NULL;
  return &domain.routes.at(maxMatch);
}
