#pragma once

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define MAX_REQ_BODY_SIZE 4096

namespace libparse {
  struct RouteProps;
  struct Domain;
  typedef std::map<std::string, RouteProps> Routes;
  typedef std::map<std::string, Domain>     Domains;

  struct RouteProps {
    std::string                         path;
    std::string                         root;
    std::string                         index;
    std::vector<std::string>            methods;
    bool                                dirListening;
    std::pair<bool, std::string>        upload;
    std::pair<std::string, std::string> cgi;
    std::string                         redir;
  };

  struct Domain {
    std::string error;
    ssize_t     maxBodySize;
    ssize_t     maxHeaserSize;
    std::string root;
    std::string index;
    std::string port;
    Routes      routes;
    std::string defaultServer;
  };

  struct Config {
    Domain *defaultServer;
    Domains domains;
  };

  typedef struct token {

    enum t_type {
      CURLYBARCKETLEFT,
      CURLYBARCKETRIGTH,
      KEYWORD,
      ROOT,
      ROUTE,
      METHODS,
      REDIR,
      INDEX,
      ERROR,
      MAXBODYSIZE,
      DIRLISTENING,
      UPLOAD,
      CGI,
      DOMAINS,
      PORT,
      ENDFILE,
      ENDROUTE,
      ENDDOMAIN,
      PATH,
      NONO
    } type;
    std::string lexeme;
  } tokens;

  void        lexer(std::vector<tokens> &tokens, std::vector<std::string> content);
  std::string readFile(std::string filename);
  void        parser(std::string filename, libparse::Config &config);
}; // namespace libparse
