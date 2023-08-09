#pragma once

#include <map>
#include <string>
#include <vector>

// initDomains
namespace libparse {
  struct RouteProps;
  struct Domain;

  typedef std::map<std::string, RouteProps> Routes;
  typedef std::map<std::string, Domain> Domains;

  struct RouteProps {
    std::string path;
    std::string root;
    std::string index;
    std::string redir;
    std::vector<std::string> methods;
    bool dirListening;
    std::pair<bool, std::string> upload;
    std::pair<std::string, std::string> cgi;
  };

  struct Domain {
    std::string error;
    std::size_t max_body_size;
    std::string root;
    std::string index;
    std::string port;
    Routes routes;
  };
}; // namespace libparse
