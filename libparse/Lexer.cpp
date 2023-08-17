#include "Config.hpp"
#include "utilities.hpp"

void libparse::lexer(std::vector<tokens> &tokens, std::vector<std::string> content) {
  size_t i = 0;
  size_t j = 0;

  while (i < content.size()) {
    if (content[i] == "{" && i != 0) {
      if (config(tokens, content, i))
        cleanUp(tokens);
    }
    i++;
  }
  setNewToken(libparse::token::ENDFILE, " ", tokens);
}
