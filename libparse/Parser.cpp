#include "Config.hpp"
#include "utilities.hpp"
#include <fcntl.h>
#include <unistd.h>

void createTmpWebserv(void)
{
  createFolder("tmp");
  createFolder("tmp/webserv")
  ;
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
  // if(!valideConfig(config.domains))
  // {
  //   std::cout << "File Not Exist or Executable " << std::endl;
  // exit(1);
  // }
  createTmpWebserv();
}
