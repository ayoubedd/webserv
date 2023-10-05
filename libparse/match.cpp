#include "libparse/match.hpp"
#include <algorithm>
#include <string>

bool matchHostHeaderWithDomain(const std::string &domain, const std::string &header) {
  std::string::size_type i;

  i = domain.find(':');
  if (i == std::string::npos)
    return domain == header;
  return domain.substr(0, i) == header;
}

const libparse::Domain *matchReqWithServer(const libhttp::Request &req,
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
    std::cout << start->first << " == " << host->second << std::endl;
    if (matchHostHeaderWithDomain(start->first, host->second)) {
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

std::string libparse::matchPathWithLocaiton(libparse::Domain domain, std::string path) {
  std::map<std::string, libparse::RouteProps>           routes = domain.routes;
  std::map<std::string, libparse::RouteProps>::iterator it = routes.begin();

  std::string::size_type score = 0;
  std::string            maxMatch;

  while (it != routes.end()) {

    score = hasPrefix(path, it->first);
    if (score > maxMatch.size())
      maxMatch = it->first;
    it++;
  }
  return maxMatch;
}
