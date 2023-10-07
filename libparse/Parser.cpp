#include "Config.hpp"
#include "utilities.hpp"
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

bool checkRoutes(libparse::Routes routes, std::string root)
{
  for(libparse::Routes::iterator it = routes.begin(); it != routes.end(); it++)
  {

  }
  return true;
}

bool IsExist(std::string path)
{
  if (access(path.c_str(),F_OK) == 0) 
    return true;
  return false;
}

bool IsExecutable(std::string path)
{

  struct stat            s;

  if (stat(path.c_str(), &s) == 0)
    return true;
  return false;
}

bool checkIsValideFile(libparse::Domain domain)
{
  std::string temp;
  temp = domain.error;

  if(!IsExist(domain.error) || !IsExist(domain.root) || !IsExist(domain.root+domain.index))
    return false;

  if(!checkRoutes(domain.routes,domain.root))
    return false;
}

static bool valideConfig(libparse::Domains &domain)
{
  for(libparse::Domains::iterator it = domain.begin(); it != domain.end();it++)
  {
    if(checkIsValideFile(it->second))
      return false;
  }
  return true;
}
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

void libparse::parser(std::string filename, libparse::Config &config) {
  std::vector<std::string>      content;
  std::string                   contentFile;
  std::vector<libparse::tokens> tokens;
  size_t                        i = 0;

  contentFile = readFile(filename);
  check(contentFile);
  content = split(contentFile);
  content.push_back("endifle");
  lexer(tokens, content);
  config.domains = setTokenInStruct(tokens);
  if (checkDefaulfServer(content, i))
    config.defaultServer = getDefeaultSever(config.domains, content[1]);
}
