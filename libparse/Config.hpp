#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <map>
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
    ENDROUTE,
    ENDDOMAIN,
    PATH,
    NONO
  }type;
  std::string lexeme;  
}tokens;
libparse::Domains initMap(std::string namefile);
std::string readFile(std::string filename);
void cleanUp(std::vector<tokens> &tokens);
bool CheckDomain(std::vector<tokens> &tokens, std::vector<std::string> content, size_t &i);
void setNewToken(token::t_type type, std::string lexeme, std::vector<tokens> &token);
std::vector<std::string >split(const std::string input);
void  check(std::string &str);
int lexer(std::vector<tokens> &tokens, std::vector<std::string> content);



libparse::Domains parser(std::vector<tokens> tokens);
void consumeOutRoute(std::string strRoute, libparse::RouteProps &routeProps, libparse::Domain &domain, std::vector<tokens> &tokens);
void setDefautValue(libparse::RouteProps &routeProps, libparse::Domain &domain);
void consume(libparse::Domain &domain, std::vector<tokens> &tokens);
std::string consumeDomain(libparse::Domain &domain, std::vector<tokens> &tokens);
void consumeRoute(libparse::RouteProps &routeProps, libparse::Domain &domain, std::vector<tokens> &tokens);
