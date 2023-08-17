#include "Config.hpp"
#include "utilities.hpp"

void libparse::parser(std::string filename, libparse::Domains &domains,
                      std::vector<libparse::tokens> &tokens) {
  std::vector<std::string> content;
  std::string contentFile;

  contentFile = readFile(filename);
  check(contentFile);
  content = split(contentFile);
  content.push_back("endifle");
  lexer(tokens, content);
  domains = setTokenInStruct(tokens);
}
