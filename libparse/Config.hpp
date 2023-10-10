#pragma once

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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
    std::size_t max_body_size;
    std::string root;
    std::string index;
    std::string port;
    Routes      routes;
    std::string defaultServer;
  };

  struct Config {
    Domain *defaultServer;
    std::string log_error;
    std::string log_info;
    std::string defaultS;
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
      ENDLINE,
      PATH,
      DEFAULT,
      LOGERROR,
      LOGINFO,
      SEMICOLON,
      NONO
    } type;
    std::string lexeme;
  } tokens;

  void        lexer(std::vector<tokens> &tokens, std::vector<std::string> content);
  std::string readFile(std::string filename);
  std::string matching(libparse::Domain domain, std::string path);
  void        parser(std::string filename, libparse::Config &config);
}; // namespace libparse
