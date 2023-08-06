#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <map>

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

typedef struct token{

  enum t_type{
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
    PATH,
    NONO
  }type;
  std::string lexeme;  
}tokens;

std::vector<tokens>  lexer(std::string content);
libparse::Domains parser(std::string filename);
std::string readFile(std::string filename);
token::t_type getTypeFromString(std::string typeStr);
std::vector<tokens>  lexer(std::string content);
libparse::Domains parser(std::vector<tokens> tokens);
std::vector<std::string >split(const std::string input);
bool convertStrToBool(std::string str);
std::vector<std::string >split(const std::string input);
void  addSpace(std::string &str);
void throwError(std::string token, std::string msg);
std::string readFile(std::string filename);
void setNewToken(token::t_type type, std::string lexeme, std::vector<tokens> &token);
std::string consumeDomain(libparse::Domain &domain, std::vector<tokens> &tokens);
std::string consumeRouteProps(libparse::RouteProps &routeProps, std::vector<tokens> &tokens);
void consume(libparse::Domain& domain,std::vector<tokens> &tokens);