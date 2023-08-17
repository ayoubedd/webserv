#include "Config.hpp"

std::vector<std::string > split(const std::string input) {
  std::vector<std::string > tokens;
  std::stringstream sstream(input);
  std::string token;

  if (input.empty())
    return tokens;
  while (sstream >> token)
    tokens.push_back(token);
  return tokens;
}

void check(std::string &str) {
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == '{') {
      str.insert(i, " ");
      i++;
    }
  }
}
libparse::token::t_type getTypeFromString(std::string typeStr) {
  if (typeStr.length() == 0)
    return libparse::token::NONO;
  else if (typeStr == "}")
    return libparse::token::CURLYBARCKETLEFT;
  else if (typeStr == "{")
    return libparse::token::CURLYBARCKETRIGTH;
  else if (typeStr == "root")
    return libparse::token::ROOT;
  else if (typeStr == "route")
    return libparse::token::ROUTE;
  else if (typeStr == "methods")
    return libparse::token::METHODS;
  else if (typeStr == "redir")
    return libparse::token::REDIR;
  else if (typeStr == "index")
    return libparse::token::INDEX;
  else if (typeStr == "error")
    return libparse::token::ERROR;
  else if (typeStr == "max_body_size")
    return libparse::token::MAXBODYSIZE;
  else if (typeStr == "dir_listing")
    return libparse::token::DIRLISTENING;
  else if (typeStr == "upload")
    return libparse::token::UPLOAD;
  else if (typeStr == "cgi")
    return libparse::token::CGI;
  else if (typeStr == "ENDDOMAIN")
    return libparse::token::DOMAINS;
  else
    return libparse::token::KEYWORD;
}

void throw_error(std::string token, std::string msg) {
  std::cout << msg << " \'" << token << "\' " << std::endl;
}
void skipEndLine(std::vector<std::string> content, size_t &i) {
  // std::cout << content[i] << "<==== \n";
  if (content[i] == "endline")
    i++;
  else {
    std::cout << content[i - 2] << "   " << content[i - 1] << "    " << content[i]
              << "   is not endline \n";
    exit(1);
  }
}

bool checkDomain(std::string domain) {
  // impleant alowed char in domain
  return true;
}

void setNewToken(libparse::token::t_type type, std::string lexeme,
                 std::vector<libparse::tokens> &token) {
  libparse::tokens tmp;
  tmp.lexeme = lexeme;
  tmp.type = type;
  token.push_back(tmp);
}

bool checkValueOfMethode(std::string value) {
  if (value == "GET" || value == "POST" || value == "HEAD" || value == "DELETE" || value == "PUT" ||
      value == "CONNECT" || value == "TRACE" || value == "PATCH")
    return true;
  return false;
}

bool checkValidKey(std::string key) {
  if (key == "error" || key == "index" || key == "max_body_size" || key == "root")
    return true;
  return false;
}

bool checkValidKeyOfRoute(std::string key) {
  if (key == "root" || key == "index" || key == "cgi" || key == "redir" || key == "dir_listing" ||
      key == "upload" || key == "methods")
    return true;
  return false;
}

int methodes(std::vector<libparse::tokens> &tokens, std::vector<std::string> content, size_t &i) {
  int error = 0;
  int j = i;
  int k = j;
  while (content[i] != "endline") {
    if (checkValueOfMethode(content[i]))
      i++;
    else
      error++, i++;
  }

  if (j == i)
    throw_error(content[j - 1], "Missing value ");

  while (j < i - 1)
    content[j].append(" ").append(content[j + 1]), j++;

  setNewToken(libparse::token::METHODS, content[k], tokens);

  return error;
}

int checkValue(std::string value) {
  // alowed char in value
  if (value.empty() || value == "endline")
    return false;
  return true;
}

int consumeToken(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content,
                 size_t &i) {
  std::string token = content[i];
  int j = ++i;
  int k = j;
  int error = 0;

  if (content[j - 1] == "methods") {
    error += methodes(tokens, content, i);
    return error;
  }

  while (content[i] != "endline") {
    checkValue(content[i]), i++;
  }

  while (j < i - 1)
    content[j].append(" ").append(content[j + 1]), j++;

  if (content[j - 1] == "cgi" || content[j - 1] == "upload") {
    if (i - j > 4)
      throw_error(content[i - 1], "to much value "), error++;
    if (j == i)
      throw_error(content[i], "Missing value "), error++;
  } else {
    if (j == i)
      throw_error(content[i], "Missing value "), error++;
    if (i - j > 2)
      throw_error(content[i - j], "to much value "), error++;
  }

  setNewToken(getTypeFromString(token), content[k], tokens);
  return error;
}

int domain(std::vector<libparse::tokens> &tokens, std::string domain) {
  int error = 0;
  size_t pos = domain.find(':');
  if (checkDomain(domain)) {
    if (pos != std::string::npos) {
      setNewToken(libparse::token::DOMAINS, domain, tokens);
      setNewToken(libparse::token::PORT, domain.substr(pos + 1), tokens);
    } else {
      setNewToken(libparse::token::DOMAINS, domain.append(":80"), tokens);
      setNewToken(libparse::token::PORT, "80", tokens);
    }
  } else
    throw_error(domain, "invalid domain "), error++;
  return error;
}

int configRout(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content,
               size_t &i) {
  int error = 0;

  setNewToken(libparse::token::ROUTE, " ", tokens), i++;
  if (checkValue(content[i]))
    setNewToken(libparse::token::PATH, content[i], tokens), i++;
  else
    std::cout << "Error:Invalid root ,Missing path" << std::endl, error++;

  if (content[i] != "{")
    std::cout << "Error:Invalid route ,Missing \'{\'" << std::endl, error++;

  else
    setNewToken(libparse::token::CURLYBARCKETRIGTH, content[i], tokens), i++;

  skipEndLine(content, i);

  while (i < content.size() && content[i] != "}") {
    if (checkValidKeyOfRoute(content[i])) {
      consumeToken(tokens, content, i);
      skipEndLine(content, i);
      continue;
    } else
      std::cout << "Error:Invalid token \'" << content[i] << "\'" << std::endl, error++, i++;
  }
  if (content[i] == "}") {
    setNewToken(libparse::token::CURLYBARCKETLEFT, content[i], tokens), i++;
    setNewToken(libparse::token::ENDROUTE, " ", tokens);
    skipEndLine(content, i);
    return error;
  } else
    std::cout << content[i] << "i have error \n", error++;
  return error;
}

int configOutRout(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content,
                  size_t &i) {
  int error = 0;
  int j = ++i;
  int k = j;

  while (content[i] != "endline")
    checkValue(content[i]), i++;

  while (j < i - 1)
    content[j].append(" ").append(content[j + 1]), j++;

  if (j == i)
    throw_error(content[k - 1], "Missing value "), error++;
  if (i - j > 1)
    throw_error(content[i - 1], "to much value "), error++;
  setNewToken(getTypeFromString(content[k - 1]), content[k], tokens), skipEndLine(content, i);
  return error;
}

int config(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content, size_t &i) {
  int error = 0;
  error += domain(tokens, content[i - 1]);
  setNewToken(libparse::token::CURLYBARCKETRIGTH, content[i], tokens), i++;
  skipEndLine(content, i);

  while (checkValidKey(content[i]))
    error += configOutRout(tokens, content, i);

  while (content[i] == "route" && i < content.size())
    error += configRout(tokens, content, i);

  if (content[i] == "}") {
    setNewToken(libparse::token::CURLYBARCKETLEFT, content[i], tokens), i++;
    setNewToken(libparse::token::ENDDOMAIN, " ", tokens);
    skipEndLine(content, i);
    return error;
  } else
    std::cout << "Error:Invalid token \'" << content[i - 2] << " " << content[i - 1] << "   "
              << content[i] << "\'" << std::endl,
        error++;
  return error;
}

void cleanUp(std::vector<libparse::tokens> &tokens) {
  int i = 0;
  while (tokens[tokens.size() - 1].type != libparse::token::DOMAINS)
    tokens.pop_back();
  tokens.pop_back();
}

// std::string getTypeFromInt(int type) {
//     switch (type) {
//         case 0: return "CURLYBARCKETLEF";
//         case 1: return "CURLYBARCKETRIGTH";
//         case 2: return "KEYWORD";
//         case 3: return "ROOT";
//         case 4: return "ROUTE";
//         case 5: return "METHODS";
//         case 6: return "REDIR";
//         case 7: return "INDEX";
//         case 8: return "ERROR";
//         case 9: return "MAXBODYSIZE";
//         case 10: return "DIRLISTENING";
//         case 11: return "UPLOAD";
//         case 12: return "CGI";
//         case 13: return "DOMAINS";
//         case 14: return "PORT";
//         case 15: return "ENDFILE";
//         case 16: return "ENDROUTE";
//         case 17: return "ENDDOMAIN";
//         case 18: return "PATH";
//         default: return "NONO";
//     }
// }

bool convertStrToBool(std::string str) {
  if (str == "on")
    return true;
  return false;
}

bool CheckValueOfOutRout(libparse::token::t_type type) {
  if (type == libparse::token::ERROR || type == libparse::token::INDEX ||
      type == libparse::token::ROOT || type == libparse::token::MAXBODYSIZE)
    return true;
  return false;
}

bool CheckValueOfInRout(libparse::token::t_type type) {
  if (type == libparse::token::PATH || type == libparse::token::ROOT ||
      type == libparse::token::INDEX || type == libparse::token::DIRLISTENING ||
      type == libparse::token::METHODS || type == libparse::token::UPLOAD ||
      type == libparse::token::CGI || type == libparse::token::REDIR)
    return true;
  return false;
}

void setDefautlValue(libparse::Domain &domain, libparse::RouteProps &routeProps) {
  std::vector<std::string> v;
  routeProps.root = "default root";
  routeProps.index = "index.html";
  v.push_back("GET");
  v.push_back("POST");
  routeProps.methods = v;
  routeProps.redir = "defautl redir";
  routeProps.upload = std::make_pair("off", "default path");
  routeProps.cgi = std::make_pair("php", "defautl path");
  routeProps.dirListening = "off";

  domain.error = "default error";
  domain.index = "index.html";
  domain.max_body_size = 1028;
  domain.root = "default root";
  domain.port = "80";
}

std::string SetDomain(std::vector<libparse::tokens> &tokens, libparse::Domain &domain) {
  std::string strDomain;
  strDomain = tokens[0].lexeme;
  tokens.erase(tokens.begin());

  if (tokens[0].type == libparse::token::PORT) {
    domain.port = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::CURLYBARCKETRIGTH)
    tokens.erase(tokens.begin());
  return strDomain;
}

void SetRoute(std::vector<libparse::tokens> &tokens, libparse::Domain &domain) {
  if (tokens[0].type == libparse::token::ERROR) {
    domain.error = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::MAXBODYSIZE) {
    domain.max_body_size = std::stoi(tokens[0].lexeme);
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::ROOT) {
    domain.root = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::INDEX) {
    domain.index = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
}

void SetValue(std::vector<libparse::tokens> &tokens, libparse::RouteProps &routeProps) {

  std::vector<std::string> tmpSplit;
  if (tokens[0].type == libparse::token::PATH) {
    routeProps.path = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::ROOT) {
    routeProps.root = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::INDEX) {
    routeProps.index = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::DIRLISTENING) {
    routeProps.dirListening = convertStrToBool(tokens[0].lexeme);
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::METHODS) {
    tmpSplit = split(tokens[0].lexeme);
    routeProps.methods = tmpSplit;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::UPLOAD) {
    tmpSplit = split(tokens[0].lexeme);
    routeProps.upload = std::make_pair(convertStrToBool(tmpSplit[0]), tmpSplit[1]);
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::CGI) {
    tmpSplit = split(tokens[0].lexeme);
    routeProps.cgi = std::make_pair(tmpSplit[0], tmpSplit[1]);
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::token::REDIR) {
    routeProps.redir = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
}

void SetConfigInRoute(std::vector<libparse::tokens> &tokens, libparse::RouteProps &routeProps,
                      libparse::Domain &domain) {
  tokens.erase(tokens.begin());
  if (tokens[0].type == libparse::token::PATH) {
    routeProps.path = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == libparse::token::CURLYBARCKETRIGTH)
    tokens.erase(tokens.begin());
  while (tokens[0].type != libparse::token::ENDROUTE) {
    if (CheckValueOfInRout(tokens[0].type)) {
      SetValue(tokens, routeProps);
      continue;
    }
    if (tokens[0].type == libparse::token::CURLYBARCKETLEFT) {
      tokens.erase(tokens.begin());
      if (tokens[0].type == libparse::token::ENDROUTE) {
        tokens.erase(tokens.begin());
        domain.routes[routeProps.path] = routeProps;
        break;
      } else
        std::cout << "error";
    } else
      std::cout << "error \n";
  }
  if (tokens[0].type == libparse::token::ENDROUTE)
    tokens.erase(tokens.begin());
}

libparse::Domains setTokenInStruct(std::vector<libparse::tokens> &tokens) {
  libparse::Domains domains;
  libparse::Domains tmp;
  libparse::Routes routes;
  struct libparse::Domain domain;
  struct libparse::RouteProps routeProps;

  std::string strDomain;
  std::string strRoute;
  std::vector<std::string> tmpSplit;

  if (tokens.size() == 0)
    return domains;
  while (tokens[0].type != libparse::token::ENDFILE) {
    if (tokens[0].type == libparse::token::DOMAINS) {
      setDefautlValue(domain, routeProps);
      strDomain = SetDomain(tokens, domain);
      continue;
    }
    if (CheckValueOfOutRout(tokens[0].type)) {
      SetRoute(tokens, domain);
      continue;
    }
    if (tokens[0].type == libparse::token::ROUTE) {
      SetConfigInRoute(tokens, routeProps, domain);
      continue;
    }
    if (tokens[0].type == libparse::token::CURLYBARCKETLEFT) {
      tokens.erase(tokens.begin());
      if (tokens[0].type == libparse::token::ENDDOMAIN) {
        domains[strDomain] = domain;
        tokens.erase(tokens.begin());
      }
      continue;
    }
  }
  return domains;
}
