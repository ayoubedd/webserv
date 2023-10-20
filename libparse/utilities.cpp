#include "Config.hpp"
#include <cstddef>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <utility>

void advance(std::vector<libparse::tokens> &tokens, size_t *i);

bool checkIsEditable(std::string &fileName) {
  if (access(fileName.c_str(), R_OK) == 0)
    return true;
  return false;
}

bool checkIsInt(std::string str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (!isdigit(str[i]))
      return false;
  }
  return true;
}

int convertToInt(std::string str) {
  int               num;
  std::stringstream ss;
  ss << str;
  ss >> num;
  if (!str.empty()) {
    try {
      return atoi(str.c_str());
    } catch (...) {
      return -1;
    }
  }
  return 0;
}

bool convertStrToBool(std::string str) {
  if (str == "on")
    return true;
  return false;
}

static bool fileExists(std::string &filename) {
  if (filename.empty())
    return true;
  std::ifstream file(filename.c_str());
  if (file)
    return true;
  return false;
}

static bool directoryExists(std::string path) {
  if (path.empty())
    return true;
  DIR *dir = opendir(path.c_str());

  if (dir == NULL) {
    return false;
  }
  closedir(dir);
  return true;
}

std::pair<bool, std::string>
checkFileExistAndEditableOfCgi(std::map<std::string, std::string> cgi) {
  std::map<std::string, std::string>::iterator it = cgi.begin();
  while (it != cgi.end()) {
    if (fileExists(it->second)) {
      if (!checkIsEditable(it->second))
        return std::make_pair(false, " ");
    } else
      return std::make_pair(false, " ");
    it++;
  }
  return std::make_pair(true, " ");
}

std::pair<bool, std::string> checkFileExist(libparse::Config &config) {
  libparse::Domains            d = config.domains;
  libparse::Domains::iterator  itD;
  libparse::Routes::iterator   itR;
  std::pair<bool, std::string> res;
  std::string                  path;

  itD = d.begin();
  while (itD != d.end()) {
    if (!fileExists(itD->second.error))
      return std::make_pair(false, itD->second.error);
    itR = itD->second.routes.begin();
    while (itR != itD->second.routes.end()) {
      if (itD->second.routes[itR->first].upload.length()) {
        if (itD->second.routes[itR->first].upload[0] == '/') {
          if (!directoryExists(itD->second.routes[itR->first].upload))
            return std::make_pair(false, itD->second.routes[itR->first].upload);
        } else if (!directoryExists(itD->second.routes[itR->first].root +
                                    itD->second.routes[itR->first].upload))
          return std::make_pair(false, itD->second.routes[itR->first].root +
                                           itD->second.routes[itR->first].upload);
      }
      if (!directoryExists(itD->second.routes[itR->first].root))
        return std::make_pair(false, itD->second.routes[itR->first].root);
      path = itD->second.routes[itR->first].root + itD->second.routes[itR->first].index;
      if (!fileExists(path))
        return std::make_pair(false, itD->second.routes[itR->first].index);
      res = checkFileExistAndEditableOfCgi(itD->second.routes[itR->first].cgi);
      if (!res.first)
        return std::make_pair(false, res.second);
      itR++;
    }
    itD++;
  }
  return std::make_pair(true, " ");
}

bool checkIsPath(std::string &path) {
  if (path.empty())
    return false;
  if (path[path.length() - 1] == '/')
    return true;
  return false;
}

void advance(std::vector<libparse::tokens> &tokens, size_t *i) {
  if (*i > tokens.size()) {
    std::cerr << "Error in config File \n";
    exit(1);
  }
  (*i)++;
}

std::pair<bool, std::string> setUpLog(libparse::Config              &config,
                                      std::vector<libparse::tokens> &tokens, size_t *i) {

  if (tokens[*i].lexeme == "log_error") {
    advance(tokens, i);
    if (tokens[*i].type == libparse::tokens::ENDLINE)
      return std::make_pair(false, "log_error");
    config.log_error = tokens[*i].lexeme;
  }

  else if (tokens[*i].lexeme == "log_info") {
    advance(tokens, i);
    if (tokens[*i].type == libparse::tokens::ENDLINE)
      return std::make_pair(false, "log_info");
    config.log_info = tokens[*i].lexeme;
  }
  advance(tokens, i);
  if (tokens[*i].lexeme != "_")
    return std::make_pair(false, ";");
  return std::make_pair(true, "");
}

bool checkIsKeyServer(libparse::token::t_type type) {
  if (type == libparse::tokens::ERROR || type == libparse::tokens::MAXBODYSIZE ||
      type == libparse::tokens::MAXHEADERSSIZE)
    return true;
  return false;
}

bool checkIsKeyRoute(libparse::token::t_type type) {
  if (libparse::tokens::ROOT == type || libparse::tokens::METHODS == type ||
      libparse::tokens::REDIR == type || type == libparse::tokens::INDEX ||
      type == libparse::tokens::DIRLISTENING || type == libparse::tokens::UPLOAD ||
      type == libparse::tokens::CGI)
    return true;
  return false;
}

bool checkIsKeyRouteStr(std::string key) {
  if (key == "root" || key == "methods" || key == "redir" || key == "index" ||
      key == "dir_listing" || key == "cgi" || key == "upload")
    return true;
  return false;
}

void skipWithSpace(std::string &content, size_t *i) {
  while (content[*i] == ' ' || content[*i] == '\t' || content[*i] == '\n')
    (*i)++;
}

std::string getWord(std::string &content, size_t *i) {
  size_t      j = *i;
  std::string value;

  while ((content[*i] != ' ' && content[*i] != '\t' && content[*i] != '\n' && content[*i] != ';') &&
         (*i < content.size()))
    (*i)++;

  value = content.substr(j, *i - j);
  return value;
}

bool isWhiteSpace(char c) {
  if (c == ' ' || c == '\t' || c == '\n')
    return true;
  return false;
}

bool checkMethod(std::string method) {
  if (method == "GET" || method == "POST" || method == "DELETE")
    return true;
  return false;
}

std::pair<bool, std::string> setUpMethods(libparse::Routes &route, std::string &nameRoute,
                                          std::vector<libparse::tokens> &tokens, size_t *i) {
  advance(tokens, i);
  std::vector<std::string > vec;
  int                       j = (*i);
  while (tokens[*i].type != libparse::tokens::ENDLINE && (*i) - j < 4) {
    if (checkMethod(tokens[*i].lexeme))
      vec.push_back(tokens[*i].lexeme);
    else
      return std::make_pair(false, tokens[*i].lexeme);
    advance(tokens, i);
  }
  if (tokens[*i].type != libparse::tokens::ENDLINE)
    return std::make_pair(false, tokens[*i].lexeme);

  route[nameRoute].methods = vec;
  return std::make_pair(true, " ");
}
bool checkIsOneOff(std::string &str) {
  if (str != "on" && str != "off")
    return false;
  return true;
}

std::pair<bool, std::string> setUpToken(libparse::Routes &route, std::string &nameRoute,
                                        std::vector<libparse::tokens> &tokens, size_t *i) {
  std::string token = tokens[*i].lexeme;
  advance(tokens, i);
  if (tokens[*i].type != libparse::tokens::ENDLINE) {
    if (token == "index")
      route[nameRoute].index = tokens[*i].lexeme;
    else if (token == "redir")
      route[nameRoute].redir = tokens[*i].lexeme;
    else if (token == "dir_listing") {
      if (!checkIsOneOff(tokens[*i].lexeme))
        return std::make_pair(false, tokens[*i].lexeme);

      route[nameRoute].dirListening = convertStrToBool(tokens[*i].lexeme);
    } else if (token == "upload") {
      if (!checkIsPath(tokens[*i].lexeme))
        return std::make_pair(false, tokens[*i].lexeme);
      route[nameRoute].upload = tokens[*i].lexeme;
    } else if (token == "root") {
      if (!checkIsPath(tokens[*i].lexeme))
        return std::make_pair(false, tokens[*i].lexeme);
      route[nameRoute].root = tokens[*i].lexeme;
    }
  } else
    return std::make_pair(false, tokens[*i].lexeme);
  advance(tokens, i);
  if (tokens[*i].type != libparse::tokens::ENDLINE)
    return std::make_pair(false, tokens[*i].lexeme);
  advance(tokens, i);
  return std::make_pair(true, " ");
}

std::pair<bool, std::string> setUpCgi(libparse::Routes &route, std::string &nameRoute,
                                      std::vector<libparse::tokens> &tokens, size_t *i,
                                      std::map<std::string, std::string > &cgi) {
  std::map<std::string, std::string>::iterator it = cgi.begin();
  std::string                                  key;
  while (it != cgi.end())
    it++;

  advance(tokens, i);
  if (tokens[*i].type == libparse::tokens::ENDLINE)
    return std::make_pair(false, tokens[*i].lexeme);

  key = tokens[*i].lexeme;
  advance(tokens, i);
  if (tokens[*i].type == libparse::tokens::ENDLINE)
    return std::make_pair(false, tokens[*i].lexeme);
  route[nameRoute].cgi[key] = tokens[*i].lexeme;
  advance(tokens, i);
  if (tokens[*i].type != libparse::tokens::ENDLINE)
    return std::make_pair(false, tokens[*i].lexeme);
  return std::make_pair(true, " ");
}

void setUpRouteInConfig(libparse::Config &config, std::string &nameDomain, std::string &nameRoute,
                        libparse::Routes &route) {
  config.domains[nameDomain].routes[nameRoute].cgi = route[nameRoute].cgi;
  config.domains[nameDomain].routes[nameRoute].index = route[nameRoute].index;
  config.domains[nameDomain].routes[nameRoute].root = route[nameRoute].root;
  config.domains[nameDomain].routes[nameRoute].dirListening = route[nameRoute].dirListening;
  config.domains[nameDomain].routes[nameRoute].redir = route[nameRoute].redir;
  config.domains[nameDomain].routes[nameRoute].methods = route[nameRoute].methods;
  config.domains[nameDomain].routes[nameRoute].upload = route[nameRoute].upload;
}

std::pair<bool, std::string> setUpRout(libparse::Config &config, std::string &nameDomain,
                                       std::vector<libparse::tokens> &tokens, size_t *i) {
  std::pair<bool, std::string>       res;
  libparse::Routes                   route;
  std::map<std::string, std::string> cgi;
  libparse::RouteProps               routeProps;
  std::string                        nameRoute;

  advance(tokens, i);
  nameRoute = tokens[*i].lexeme;
  if (nameRoute[0] != '/')
    return std::make_pair(false, tokens[*i].lexeme);
  advance(tokens, i);
  if (tokens[*i].lexeme != "{")
    return std::make_pair(false, tokens[*i].lexeme);
  advance(tokens, i);
  while (tokens[*i].type != libparse::tokens::CURLYBARCKETLEFT) {
    if (checkIsKeyRoute(tokens[*i].type)) {
      if (tokens[*i].type == libparse::tokens::METHODS) {
        res = setUpMethods(route, nameRoute, tokens, i);
        if (!res.first)
          return std::make_pair(false, res.second);
        advance(tokens, i);
      } else if (tokens[*i].type == libparse::tokens::CGI) {
        res = setUpCgi(route, nameRoute, tokens, i, cgi);
        if (!res.first)
          return std::make_pair(false, res.second);
        advance(tokens, i);
      } else {
        res = setUpToken(route, nameRoute, tokens, i);
        if (!res.first)
          return std::make_pair(false, res.second);
      }
    } else {
      return std::make_pair(false, tokens[*i].lexeme);
    }
    continue;
  }
  setUpRouteInConfig(config, nameDomain, nameRoute, route);
  advance(tokens, i);
  return std::make_pair(true, tokens[*i].lexeme);
}

bool checkDomain(std::string &nameDomain) {
  if (nameDomain.empty())
    return false;
  if (nameDomain[0] == '.' || nameDomain[0] == '-' || nameDomain[nameDomain.length() - 1] == '.')
    return false;
  for (size_t i = 0; i < nameDomain.length(); i++) {
    if (!isalpha(nameDomain[i]) && !isdigit(nameDomain[i]) && nameDomain[i] != '.' &&
        nameDomain[i] != '-')
      return false;
  }
  return true;
}

std::pair<bool, std::string> setUpKey(libparse::Config &config, std::string nameDomain,
                                      std::vector<libparse::tokens> &tokens, size_t *i) {
  int num;
  if (tokens[*i].type == libparse::token::MAXBODYSIZE) {
    advance(tokens, i);
    if (tokens[*i].type == libparse::token::ENDLINE)
      return std::make_pair(false, "max_body_size");
    if (!checkIsInt(tokens[*i].lexeme))
      return std::make_pair(false, "max_body_size");
    num = convertToInt(tokens[*i].lexeme);
    if (num == -1)
      return std::make_pair(false, "max_body_size");
    config.domains[nameDomain].maxBodySize = num;
    advance(tokens, i);
    if (tokens[*i].type != libparse::token::ENDLINE)
      return std::make_pair(false, "max_body_size");
    advance(tokens, i);
    return std::make_pair(true, "");
  } else if (tokens[*i].type == libparse::token::ERROR) {
    advance(tokens, i);
    if (tokens[*i].type == libparse::token::ENDLINE)
      return std::make_pair(false, "error");
    config.domains[nameDomain].error = tokens[*i].lexeme;
    advance(tokens, i);
    if (tokens[*i].type != libparse::token::ENDLINE)
      return std::make_pair(false, "error");
    advance(tokens, i);
    return std::make_pair(true, "");
  } else if (tokens[*i].type == libparse::token::MAXHEADERSSIZE) {
    advance(tokens, i);
    if (tokens[*i].type == libparse::token::ENDLINE)
      return std::make_pair(false, "max_header_size");
    config.domains[nameDomain].maxHeaserSize = convertToInt(tokens[*i].lexeme);
    advance(tokens, i);
    if (tokens[*i].type != libparse::token::ENDLINE)
      return std::make_pair(false, "max_header_size");
    advance(tokens, i);
    return std::make_pair(true, "");
  }
  return std::make_pair(false, "");
}
std::pair<bool, std::string> setUpPort(libparse::Config &config, std::string &nameDomain) {
  size_t      pos = nameDomain.find(':');
  int         port;
  std::string domain;

  if (pos != std::string::npos) {
    domain = nameDomain.substr(0, pos);
    if (!checkDomain(domain))
      return std::make_pair(false, domain);
    if (pos < nameDomain.length() - 1)
      config.domains[nameDomain].port = nameDomain.substr(pos + 1);
    else {
      nameDomain.append(":80");
      config.domains[nameDomain].port = "80";
    }
    if (!checkIsInt(config.domains[nameDomain].port))
      return std::make_pair(false, config.domains[nameDomain].port);
    port = convertToInt(config.domains[nameDomain].port);
    if (port < 0 || port > 65535)
      return std::make_pair(false, config.domains[nameDomain].port);
    return std::make_pair(true, config.domains[nameDomain].port);
  } else {
    if (!checkDomain(nameDomain))
      return std::make_pair(false, nameDomain);
    nameDomain.append(":80");
    config.domains[nameDomain].port = "80";
    return std::make_pair(true, config.domains[nameDomain].port);
  }
  return std::make_pair(false, nameDomain);
}

std::pair<bool, std::string> SetUpServer(libparse::Config              &config,
                                         std::vector<libparse::tokens> &tokens, size_t *i) {
  std::string                  nameDomain, strRoute;
  libparse::Routes             route;
  std::pair<bool, std::string> res;

  nameDomain = tokens[*i].lexeme;
  res = setUpPort(config, nameDomain);
  if (!res.first)
    return std::make_pair(false, res.second);
  advance(tokens, i);
  if (tokens[*i].type != libparse::tokens::CURLYBARCKETRIGTH)
    return std::make_pair(false, "{");
  advance(tokens, i);
  while (tokens[*i].type != libparse::tokens::CURLYBARCKETLEFT) {
    if (checkIsKeyServer(tokens[*i].type)) {
      res = setUpKey(config, nameDomain, tokens, i);
      if (!res.first)
        return std::make_pair(false, res.second);
    } else if (tokens[*i].type == libparse::tokens::ROUTE) {
      res = setUpRout(config, nameDomain, tokens, i);
      if (!res.first)
        return std::make_pair(false, res.second);
    } else {
      std::cout << "Error in server " << tokens[*i].lexeme << std::endl;
      return std::make_pair(false, tokens[*i].lexeme);
    }
    continue;
  }
  advance(tokens, i);
  return std::make_pair(true, nameDomain);
}

std::pair<bool, std::string> setUpDefaultSever(libparse::Config              &config,
                                               std::vector<libparse::tokens> &tokens, size_t *i) {
  std::pair<bool, std::string> res;
  advance(tokens, i);
  return SetUpServer(config, tokens, i);
}
