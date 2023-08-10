#include "Config.hpp"

bool checkValidToken(std::string token);
bool setToken(std::vector<tokens> &tokens, std::string token, std::string lexeme);
void setNewToken(token::t_type type, std::string lexeme, std::vector<tokens> &token);


std::vector<std::string >split(const std::string input)
{
  std::vector<std::string > tokens;
  std::stringstream sstream(input);
  std::string token;

  if(input.empty())
    return tokens;
  while (sstream >> token)
    tokens.push_back(token);
  return tokens;
}

void  check(std::string &str)
{ 
    for(int i = 0 ; i < str.length() ; i++)
    {
      if(str[i] == '{')
      {
        str.insert(i," ");
        i++;
      }
    }
}
bool checkValue(std::string value)
{
  if (!checkValidToken(value))
    return true;
  return false;
}

token::t_type getTypeFromString(std::string typeStr)
{
  if (typeStr.length() == 0)
    return token::NONO;
  else if (typeStr == "}")
    return token::CURLYBARCKETLEFT;
  else if (typeStr == "{")
    return token::CURLYBARCKETRIGTH;
  else if (typeStr == "root")
    return token::ROOT;
  else if (typeStr == "route")
    return token::ROUTE;
  else if (typeStr == "methods")
    return token::METHODS;
  else if (typeStr == "redir")
    return token::REDIR;
  else if (typeStr == "index")
    return token::INDEX;
  else if (typeStr == "error")
    return token::ERROR;
  else if (typeStr == "max_body_size")
    return token::MAXBODYSIZE;
  else if (typeStr == "dir_listing")
    return token::DIRLISTENING;
  else if (typeStr == "upload")
    return token::UPLOAD;
  else if (typeStr == "cgi")
    return token::CGI;
  else if (typeStr == "ENDDOMAIN")
    return token::DOMAINS;
  else
    return token::KEYWORD;
}

bool setToken(std::vector<tokens> &tokens, std::string token, std::string lexeme)
{
  if (checkValidToken(lexeme))
    std::cout << "Error: Missing value \'" << token << "\'" << std::endl;
  if (!checkValue(lexeme))
    std::cout << "ERORR:Unexpected value \'" << token << "\'";
  setNewToken(getTypeFromString(token), lexeme, tokens);
  return true;
}

bool checkValidToken(std::string token)
{
  if (token == "error" || token == "index" || token == "max_body_size" ||
      token == "root" || token == "cgi" || token == "redir" ||
      token == "dir_listing" || token == "upload" || token == "methods")
    return true;
  return false;
}

void setNewToken(token::t_type type, std::string lexeme, std::vector<tokens> &token)
{
  tokens tmp;
  tmp.lexeme = lexeme;
  tmp.type = type;
  token.push_back(tmp);
}

void setDomain(std::vector<tokens> &tokens, std::string domain)
{
  size_t pos = domain.find(':');
  if (pos != std::string::npos)
  {
    setNewToken(token::DOMAINS, domain, tokens);
    setNewToken(token::PORT, domain.substr(pos + 1), tokens);
  }
  else
  {
    setNewToken(token::DOMAINS, domain.append(":80"), tokens);
    setNewToken(token::PORT, "80", tokens);
  }
}

bool CheckDomain(std::vector<tokens> &tokens, std::vector<std::string> content, size_t &i)
{
  int error = 0;

  setDomain(tokens, content[i - 1]);
  setNewToken(token::CURLYBARCKETRIGTH, content[i], tokens), i++;
  while (checkValidToken(content[i]))
    setToken(tokens, content[i], content[i + 1]), i += 2;
  while (content[i] == "route" && i < content.size())
  {
    setNewToken(token::ROUTE, " ", tokens), i++;
    if (checkValue(content[i]))
      setNewToken(token::PATH, content[i], tokens), i++;

    if (content[i] != "{")
      std::cout << "Error:Invalid route ,Missing \'{\'" << std::endl, error++;

    setNewToken(token::CURLYBARCKETRIGTH, content[i], tokens), i++;

    while (checkValidToken(content[i]) && content[i] != "}" && i < content.size())
    {
      if (content[i] == "methods")
      {
        int j = i++;
        if (!checkValue(content[i]))
        {
          std::cout << "Error:Missing value \'" << content[i - 1] << "\'" << std::endl, error++;
          continue;
        }
        while (checkValue(content[i + 1]) && i< content.size())
          content[j + 1].append(" ").append(content[i + 1]), i++;
        setToken(tokens, "methods", content[j + 1]), i++;
        continue;
      }
      if (content[i] == "cgi" || content[i] == "upload")
      {
        if (!checkValue(content[i + 1]))
        {
          std::cout << "Error:Missing value \'" << content[i] << "\'" << std::endl, error++;
          continue;
        }
        else
        {
          content[i + 1].append(" ").append(content[i + 2]);
          setToken(tokens, content[i], content[i + 1]), i += 3;
          continue;
        }
      }
      if(!checkValidToken(content[i]) && content[i] != "}")
        std::cout << "Error:Invalid token \'" << content[i] << "\'" << std::endl, error++, i++;
      if (content[i] != "}" && i < content.size())
      {
        if (!checkValue(content[i + 1]))
          std::cout << "Error:Missing value \'" << content[i] << "\'" << std::endl, error++, i++;
        else
          setToken(tokens, content[i], content[i + 1]), i += 2;
        continue;
      }
    }
    if(!checkValidToken(content[i]) && content[i] != "}")
        std::cout << "Error:Invalid token \'" << content[i] << "\'" << std::endl, error++, i++;
    if (content[i] != "}" && i < content.size())
      std::cout << "Error:Invalid route ,Missing \'}\'" << std::endl, error++;
    else
    {
      setNewToken(token::CURLYBARCKETLEFT, content[i], tokens);
      setNewToken(token::ENDROUTE, " ", tokens), i++;
    }
  }
  if (content[i] != "}" && i < content.size())
    std::cout << "Error:Invalid route ,Missing \'}\'" << std::endl, error++;
   
  setNewToken(token::CURLYBARCKETLEFT, content[i], tokens),i++;
  setNewToken(token::ENDDOMAIN, " ", tokens);
  if (error == 0)
    return true;
  return false;
}

void cleanUp(std::vector<tokens> &tokens)
{
  int i = 0;
  while (tokens[tokens.size() - 1].type != token::DOMAINS)
    tokens.pop_back();
  tokens.pop_back();
}

// parsr

void throwError(std::string token, std::string msg)
{
  std::cout << msg << "  " << token << std::endl;
  exit(1);
}

bool convertStrToBool(std::string str)
{
  if (str == "on")
    return true;
  return false;
}

void consumeOutRoute(std::string strRoute, libparse::RouteProps &routeProps, libparse::Domain &domain, std::vector<tokens> &tokens)
{
  std::vector<std::string> tmpSplit;
  while (tokens.begin() != tokens.end() && tokens[0].type != token::ENDROUTE)
  {
    if (tokens[0].type == token::PATH)
    {
      routeProps.path = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::ROOT)
    {
      routeProps.root = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::INDEX)
    {
      routeProps.index = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::DIRLISTENING)
    {
      routeProps.dirListening = convertStrToBool(tokens[0].lexeme);
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::METHODS)
    {
      tmpSplit = split(tokens[0].lexeme);
      routeProps.methods = tmpSplit;
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::UPLOAD)
    {
      tmpSplit = split(tokens[0].lexeme);
      routeProps.upload = std::make_pair(convertStrToBool(tmpSplit[0]), tmpSplit[1]);
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::CGI)
    {
      tmpSplit = split(tokens[0].lexeme);
      routeProps.cgi = std::make_pair(tmpSplit[0], tmpSplit[1]);
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::REDIR)
    {
      routeProps.redir = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if (tokens[0].type == token::CURLYBARCKETLEFT)
    {
      tokens.erase(tokens.begin());
      if (token::ENDROUTE != tokens[0].type)
        throwError(tokens[0].lexeme, "222222222:");
      else
      {
        tokens.erase(tokens.begin());
        domain.routes[strRoute] = routeProps;
        break;
      }
    }
    else
    {
      throwError(tokens[0].lexeme, "1111111");
      tokens.erase(tokens.begin());
      continue;
    }
  }
}

void consumeRoute(libparse::RouteProps &routeProps, libparse::Domain &domain, std::vector<tokens> &tokens)
{
  std::vector<std::string> tmpSplit;
  tokens.erase(tokens.begin());
  if (tokens[0].type == token::PATH)
  {
    routeProps.path = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == token::CURLYBARCKETRIGTH)
    tokens.erase(tokens.begin());
  else
    throwError("{", "in route");
}

std::string consumeDomain(libparse::Domain &domain, std::vector<tokens> &tokens)
{
  std::string strDomain;
  strDomain = tokens[0].lexeme;
  tokens.erase(tokens.begin());

  if (tokens[0].type == token::PORT)
  {
    domain.port = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == token::CURLYBARCKETRIGTH)
    tokens.erase(tokens.begin());
  else
    throwError(tokens[0].lexeme, "error:CURLYBARCKETRIGTH not open ");
  return strDomain;
}

void consume(libparse::Domain &domain, std::vector<tokens> &tokens)
{
  if (tokens[0].type == token::ERROR)
  {
    domain.error = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == token::MAXBODYSIZE)
  {
    domain.max_body_size = std::stoi(tokens[0].lexeme);
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == token::ROOT)
  {
    domain.root = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if (tokens[0].type == token::INDEX)
  {
    domain.index = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
}

void setDefautValue(libparse::RouteProps &routeProps, libparse::Domain &domain)
{
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

