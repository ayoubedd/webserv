#include "Config.hpp"
#include "utilities.hpp"

void libparse::parser(std::string filename, libparse::Domains &domains) {
  std::vector<std::string> content;
  std::string contentFile;
  std::vector<libparse::tokens> tokens;

  contentFile = readFile(filename);
  check(contentFile);
  content = split(contentFile);
  content.push_back("endifle");
  lexer(tokens, content);
  domains = setTokenInStruct(tokens);
}
