#include "Config.hpp"
#include <algorithm>
#include <string>

// path route
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
// /dev /des
std::string libparse::matching(libparse::Domain domain, std::string path) {
  std::map<std::string, libparse::RouteProps> routes = domain.routes;
  std::map<std::string, libparse::RouteProps>::iterator it = routes.begin();

  std::string::size_type score = 0;
  std::string maxMatch;

  while (it != routes.end()) {

    score = hasPrefix(path, it->first);
    if (score > maxMatch.size())
      maxMatch = it->first;
    it++;
  }
  return maxMatch;
}
