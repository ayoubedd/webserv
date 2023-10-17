#include "Config.hpp"
#include "utilities.hpp"

// [X] setUp name domain and port
// [X] setup default server
// [X]check is finched by /
// [X]check file is exsite 
// [X] check allowd char in domain
// [X] check size body and headres int 

// [X] cgi map
// [X] check port.
// [X] {}
// [X] check allow defaut on 
// [X] check leaks file discr

// [] check i is not out of range.
// [X] check rout / and index root.
// [] diplcute domain with the same port
// [] check max int size


static bool strStartWith(const std::string str, const std::string prefix) {
  std::string::size_type i;

  for (i = 0; i < prefix.size(); i++) {
    if (prefix[i] != str[i])
      return false;
  }
  return true;
}

libparse::Domain *getDefeaultSever(libparse::Domains &domains, std::string server) {
  for (libparse::Domains::iterator it = domains.begin(); it != domains.end(); it++) {
    if (strStartWith(it->first, server))
      return &it->second;
  };
  return NULL;
}

std::pair<bool , std::string> libparse::parser(libparse::Config &config,std::vector<libparse::tokens> &tokens)
{
    if(!tokens.size())
      return std::make_pair(true,"");
    size_t i = 0;
    bool check = true;
    std::pair<bool , std::string> res;
    while((tokens[i].lexeme == "log_error" || tokens[i].lexeme == "log_info") && i < 4 && i < tokens.size())
    {
      res = setUpLog(config,tokens,&i); 
        if(!res.first)
          return res;
      i++;
    }
    while(tokens[i].type != libparse::tokens::ENDFILE && i < tokens.size() -1)
    {
      if(tokens[i].lexeme == "default")
      {
        if(!check)
          return std::make_pair(false,"you must one defaul sever");
        check = false;
        res = setUpDefaultSever(config,tokens,&i);
        if(!res.first)
          return res;
        config.defaultServer = &config.domains.begin()->second;
      }
      else
      {
        res = SetUpServer(config,tokens,&i);
        if(!res.first)
          return res;
      }
    continue;
  }
  return std::make_pair(true,"");
}

std::pair<bool, std::string> checkDuplicateNameDomain(libparse::Config &config)
{
  libparse::Domains::iterator itD;
    libparse::Domains::iterator it;
  itD = config.domains.begin();
  std::string nameDomain;
  while (itD != config.domains.end()) 
  {
    nameDomain = itD->first;
    it = ++itD;
    while(it != config.domains.end())
    {
      if(it->first == nameDomain)
        return std::make_pair(false,it->first);
      it++;
    }
  }
  return std::make_pair(true,"");
}

std::pair<bool , std::string> checkDefaultRout(libparse::Config &config)
{
  libparse::Domains::iterator itD ;
  libparse::Routes::iterator itR = config.defaultServer->routes.find("/");
  
  if(itR == config.defaultServer->routes.end())
    return std::make_pair(false,"defautl server");
  itD = config.domains.begin();
  itR = itD->second.routes.begin();

  while (itD != config.domains.end()) 
  {
    itR = itD->second.routes.find("/");
    if(itR == itD->second.routes.end())
      return std::make_pair(false,itD->first);
    if(itR->second.index.empty())
      return std::make_pair(false,"index is empty in the server :"+itD->first);
    if(itR->second.root.empty())
      return std::make_pair(false,"root is empty in the server :"+itD->first);
    itD++;
  }
  return std::make_pair(true,"");
}

bool libparse::checkConfig(std::string &fileName,libparse::Config &config)
{
  std::string                   contentFile;
  std::vector<libparse::tokens> tokens;
  std::pair<bool,std::string> res;

  contentFile = libparse::readFile(fileName);
  if(contentFile.empty())
    return false;
  libparse::lexer(tokens,contentFile);
  res = parser(config,tokens);
  if(!res.first)
  {
    std::cout << "Error invalide Token : "<< res.second <<std::endl;
    return res.first;
  }
  res = checkFileExist(config);
  if(!res.first)
  {
    std::cout << "File Not Exist or Not Editable : "<< res.second <<std::endl;
    return res.first;
  }
  res = checkDefaultRout(config);
  if(!res.first)
  {
    std::cout << "Error Default Route "<< res.second <<std::endl;
    return res.first;
  }
  res = checkDuplicateNameDomain(config);
  if(!res.first)
  {
     std::cout << "Error Mulitple name domain "<< res.second <<std::endl;
    return res.first;
  }
  return true;
}
