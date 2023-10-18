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
    std::string                        path;
    std::string                        root;
    std::string                        index;
    std::vector<std::string>           methods;
    bool                               dirListening;
    std::string                        upload;
    std::map<std::string, std::string> cgi;
    std::string                        redir;
  };

  struct Domain {
    std::string error;
    ssize_t     maxBodySize;
    ssize_t     maxHeaserSize;
    std::string root;
    std::string index;
    std::string port;
    Routes      routes;
  };

  struct Config {
    Domain     *defaultServer;
    std::string log_info;
    std::string log_error;
    Domains     domains;

    bool init();
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
      MAXHEADERSSIZE,
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

  void                         lexer(std::vector<libparse::tokens> &tokens, std::string &content);
  std::string                  readFile(std::string filename);
  std::pair<bool, std::string> parser(libparse::Config              &config,
                                      std::vector<libparse::tokens> &tokens);
  bool                         checkConfig(const std::string &fileName, libparse::Config &config);
}; // namespace libparse
