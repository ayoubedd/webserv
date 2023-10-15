#include "libparse/match.hpp"
#include "libhttp/Request-target.hpp"
#include "libparse/Config.hpp"
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

static inline bool matchHostHeaderPortWithDomain(const std::string &domain,
                                                 const std::string &header) {
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

std::string libparse::findRouteRoot(const libparse::Domain     *domain,
                                    const libparse::RouteProps *route) {
  if (!route)
    return domain->root;
  if (route->root.empty() == false)
    return route->root;
  else
    return domain->root;
}

std::string libparse::findRouteIndex(const libparse::Domain     *domain,
                                     const libparse::RouteProps *route) {
  if (!route)
    return domain->root;
  if (route->index.empty() == false)
    return route->index;
  else
    return domain->index;
}

std::string libparse::findResourceInFs(const libhttp::Request &req,
                                       const libparse::Domain &domain) {
  std::pair<std::string, const libparse::RouteProps *> r =
      matchPathWithRoute(domain.routes, req.reqTarget.path);
  std::string fs = findRouteRoot(&domain, r.second);
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
  std::string index = findRouteIndex(&domain, r.second);
  if (index.empty())
    return "";
  fs = joinPath(fs, index);
  if (stat(fs.c_str(), &st) != 0)
    return "";
  return fs;
}

std::string libparse::findUploadDir(const libhttp::Request &req, const libparse::Domain &domain) {
  std::pair<std::string, const libparse::RouteProps *> route =
      matchPathWithRoute(domain.routes, req.reqTarget.path);
  if (!route.second || !route.second->upload.first)
    return "";
  std::string root = findRouteRoot(&domain, route.second);
  if (route.second->upload.second[0] == '/') {
    if (access(route.second->upload.second.c_str(), W_OK) != 0)
      return "";
    return route.second->upload.second;
  }
  std::string fs = joinPath(root, route.second->upload.second);
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
