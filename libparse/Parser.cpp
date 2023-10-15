#include "Config.hpp"
#include "utilities.hpp"

// [X]check is finched by /
// [X]check file is exsite 
//      [] check .py or .php existub
// [X] check allowd char in domain
// check i is not out of range
// [X] check size body and headres int 
//  [] check is int

// [X] setUp name domain and port
// [X] setup default server

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
    size_t i = 0;
    std::pair<bool , std::string> res;

    while((tokens[i].lexeme == "log_error" || tokens[i].lexeme == "log_info") && i < 4 && i < tokens.size())
    {
      res = setUpLog(config,tokens,&i); 
        if(!res.first)
          return res;
      i++;
    }
    while(tokens[i].type != libparse::tokens::ENDFILE && i < tokens.size())
    {
      if(tokens[i].lexeme == "default")
      {
        res = setUpDefaultSever(config,tokens,&i);
        if(!res.first)
          return res;
        config.defaultServer = &config.domains.begin()->second;
      }
      else
      {
        res = SetUpServer(config,tokens,&i);;
        if(!res.first)
          return res;
      }
    continue;
    }
  return std::make_pair(true,"");
}

bool libparse::checkConfig(std::string &fileName,libparse::Config &config)
{
  std::string                   contentFile;
  std::vector<libparse::tokens> tokens;
  std::pair<bool,std::string> res;

  contentFile = libparse::readFile(fileName);
  libparse::lexer(tokens,contentFile);
  res = parser(config,tokens);
  if(!res.first)
  {
    std::cout << "Error : "<< res.second <<std::endl;
    return res.first;
  }
  res = checkFileExist(config);
  if(!res.first)
  {
    std::cout << "File or Dir Not Exist " << res.second << std::endl;
    return res.first;
  }
  return true;
}
