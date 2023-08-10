#include "Config.hpp"
#include "TestParser.hpp"

libparse::Domains initMap(std::string namefile)
{
  std::string contentFile;
  libparse::Domains domains;
  std::vector<tokens> tokens;
  std::vector<std::string> tokenStr;

  contentFile =  readFile(namefile);
  check(contentFile);
  tokenStr = split(contentFile);
  lexer(tokens,tokenStr);
  domains = parser(tokens);
  return domains;
}
