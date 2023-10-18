#include "libparse/match.hpp"
#include "libhttp/Request-target.hpp"
#include "libparse/Config.hpp"
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

static inline bool matchHostHeaderPortWithDomain(const std::string &domainPort,
                                                 const std::string &header) {
  std::string::size_type i;
  std::string            host, headerPort;

  i = header.find(':');
  headerPort = ((i == std::string::npos) ? headerPort = "80" : header.substr(i));
  return headerPort == domainPort;
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
    if (matchHostHeaderPortWithDomain(start->second.port, host->second)) {
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

std::pair<std::string, const libparse::RouteProps *>
libparse::matchPathWithRoute(const libparse::Routes &routes, const std::string &path) {
  std::map<std::string, libparse::RouteProps>::const_iterator it = routes.begin();

  std::string::size_type score = 0;
  std::string            maxMatch;

  maxMatch = "";
  while (it != routes.end()) {
    score = hasPrefix(path, it->first);
    if (score > maxMatch.size())
      maxMatch = it->first;
    it++;
  }
  if (!maxMatch.size())
    return std::make_pair("", static_cast<const RouteProps *>(NULL));
  return std::make_pair(maxMatch, &routes.at(maxMatch));
}

std::string libparse::findRouteRoot(const libparse::Routes     &routes,
                                    const libparse::RouteProps &route) {
  if (!route.root.empty())
    return route.root;
  else
    return routes.find("/")->second.root;
}

std::string libparse::findRouteIndex(const libparse::Routes     &routes,
                                     const libparse::RouteProps &route) {
  if (!route.index.empty())
    return route.index;
  else
    return routes.find("/")->second.index;
}

std::string libparse::findResourceInFs(const libhttp::Request &req,
                                       const libparse::Domain &domain) {
  std::pair<std::string, const libparse::RouteProps *> r =
      matchPathWithRoute(domain.routes, req.reqTarget.path);
  std::string fs = findRouteRoot(domain.routes, *r.second);
  if (fs.empty())
    return "";
  fs = joinPath(fs, req.reqTarget.path);
  struct stat st;
  if (stat(fs.c_str(), &st) != 0)
    return "";
  if (st.st_mode & S_IFREG)
    return fs;
  if (st.st_mode & S_IFDIR && r.second && r.second->dirListening)
    return fs;
  std::string index = findRouteIndex(domain.routes, *r.second);
  if (index.empty())
    return "";
  fs = joinPath(fs, index);
  if (stat(fs.c_str(), &st) != 0)
    return "";
  return fs;
}

std::string libparse::findUploadDir(const libhttp::Request &req, const libparse::Domain &domain) {
  std::pair<std::string, const RouteProps *> route =
      matchPathWithRoute(domain.routes, req.reqTarget.path);
  if (!route.second->upload.empty())
    return "";
  std::string root = findRouteRoot(domain.routes, *route.second);
  if (route.second->upload[0] == '/') { // if the path is abs path
    if (access(route.second->upload.c_str(), W_OK) != 0)
      return "";
    return route.second->upload;
  }
  std::string fs = joinPath(root, route.second->upload);
  if (access(fs.c_str(), W_OK) != 0)
    return "";
  return fs;
}

std::string libparse::joinPath(const std::string &s1, const std::string &s2) {
  std::string path;

  if (s1.empty())
    return s2;
  if (s2.empty())
    return s1;
  path = s1;
  if (path.back() == '/' && s2.front() == '/') {
    path.pop_back();
    return path + s2;
  }
  if (path.back() != '/' && s2.front() != '/') {
    path.push_back('/');
    return path + s2;
  }
  return path + s2;
}
