#include "Config.hpp"
#include "utilities.hpp"
void setDefeaultSever(libparse::Domains &domains, std::string server)
{
  for(libparse::Domains::iterator it = domains.begin(); it != domains.end() ; it++)
    it->second.defaultServer = server;
}

void libparse::parser(std::string filename, libparse::Domains &domains) {
  std::vector<std::string> content;
  std::string contentFile;
  std::vector<libparse::tokens> tokens;
  size_t i = 0;

  contentFile = readFile(filename);
  check(contentFile);
  content = split(contentFile);
  content.push_back("endifle");
  lexer(tokens, content);
  domains = setTokenInStruct(tokens);
  if(checkDefaulfServer(content,i))
    setDefeaultSever(domains,content[1]);
}
