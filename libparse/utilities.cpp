#include "Config.hpp"
#include <iostream>

void throwError(std::string token, std::string msg)
{
  std::cout << msg <<"  "<< token << std::endl;
}

bool convertStrToBool(std::string str)
{
  if(str == "on")
    return true;
  return false;
}

void  check(std::string &str)
{ 
    for(size_t i = 0 ; i < str.length() ; i++)
    {
      if(str[i] == '{')
      {
        str.insert(i," ");
        i++;
      }
    }
}

void setNewToken(token::t_type type, std::string lexeme, std::vector<tokens> &token) 
{ 
  tokens tmp;
  tmp.lexeme = lexeme;
  tmp.type = type;
  token.push_back(tmp);
}

token::t_type getTypeFromString(std::string typeStr)
{
    if (typeStr == "}") return token::CURLYBARCKETLEFT;
    else if (typeStr == "{") return token::CURLYBARCKETRIGTH;
    else if (typeStr == "root") return token::ROOT;
    else if (typeStr == "route") return token::ROUTE;
    else if (typeStr == "methods") return token::METHODS;
    else if (typeStr == "redir") return token::REDIR;
    else if (typeStr == "index") return token::INDEX;
    else if (typeStr == "error") return token::ERROR;
    else if (typeStr == "max_body_size") return token::MAXBODYSIZE;
    else if (typeStr == "dir_listing") return token::DIRLISTENING;
    else if (typeStr == "upload") return token::UPLOAD;
    else if (typeStr == "cgi") return token::CGI;
    else return token::KEYWORD; 
}

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

void  addSpace(std::string &str)
{ 
    for(size_t i = 0 ; i < str.length() ; i++)
    {
      if(str[i] == '{')
      {
        str.insert(i," ");
        i++;
      }
    }
}

std::string consumeDomain(libparse::Domain &domain, std::vector<tokens> &tokens)
{
    std::string strDomain;
    strDomain = tokens[0].lexeme;
    tokens.erase(tokens.begin());
    if(tokens[0].type == token::PORT)
    {
      domain.port = tokens[0].lexeme;
      tokens.erase(tokens.begin());
    }
    if(tokens[0].type == token::CURLYBARCKETRIGTH)
      tokens.erase(tokens.begin()); 
    else
      throwError(tokens[0].lexeme, "ERORR:unexpected token");
    return strDomain;
}

std::string consumeRouteProps(libparse::RouteProps &routeProps, std::vector<tokens> &tokens) 
{
  std::vector<std::string> tmpSplit;
  tokens.erase(tokens.begin());
  if(tokens[0].type == token::PATH)
  {
    routeProps.path = tokens[0].lexeme;
    tokens.erase(tokens.begin());
  }
  if(tokens[0].type == token::CURLYBARCKETRIGTH)
    tokens.erase(tokens.begin());

  while (tokens[0].type != token::CURLYBARCKETLEFT)
  {
    if(tokens[0].type == token::ROOT)
    {
      routeProps.root = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::INDEX)
    {
      routeProps.index = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::DIRLISTENING)
    {
      routeProps.dirListening =  convertStrToBool(tokens[0].lexeme);
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::METHODS)
    {
      tmpSplit = split(tokens[0].lexeme); 
      routeProps.methods = tmpSplit;
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::UPLOAD)
    {
      tmpSplit = split(tokens[0].lexeme); 
       routeProps.upload = std::make_pair(convertStrToBool(tmpSplit[0]), tmpSplit[1]);
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::CGI)
    {
      tmpSplit = split(tokens[0].lexeme); 
      routeProps.cgi =std::make_pair(tmpSplit[0], tmpSplit[1]);  
      tokens.erase(tokens.begin());
      continue;
    }
    if(tokens[0].type == token::REDIR)
    {
      routeProps.redir = tokens[0].lexeme;
      tokens.erase(tokens.begin());
      continue;
    }
    else
    {
      throwError(tokens[0].lexeme, "Error:unexpected token");
      tokens.erase(tokens.begin());
      continue;
    }
  } 
  if(token::CURLYBARCKETLEFT == tokens[0].type)
  {
    tokens.erase(tokens.begin());
    return routeProps.path;
  }
  else
  {
    throwError(tokens[0].lexeme, "Error:unexpected token");
  } 
  return routeProps.path;
}

void consume(libparse::Domain& domain,std::vector<tokens> &tokens)
{
    std::stringstream str;
    int maxBodySize;
    if(tokens[0].type == token::ERROR)
      {
        domain.error = tokens[0].lexeme;
        tokens.erase(tokens.begin());
      }
      if(tokens[0].type == token::MAXBODYSIZE)
      {
        str << tokens[0].lexeme.c_str();
       str  >> maxBodySize;
        domain.max_body_size = maxBodySize;
        tokens.erase(tokens.begin());
      }
        if(tokens[0].type == token::ROOT)
      {  
         domain.root = tokens[0].lexeme;
        tokens.erase(tokens.begin());
      }
      if(tokens[0].type == token::INDEX)
      {
        domain.index = tokens[0].lexeme;
        tokens.erase(tokens.begin());
      }
}
