#include "Config.hpp"


int lexer(std::vector<tokens> &tokens, std::vector<std::string> content)
{
  int numberDomain = 0;
  size_t i = 0;

  while (i < content.size())
  {
    if (content[i] == "{" && i != 0)
    {
      if (!CheckDomain(tokens, content, i))
        cleanUp(tokens);
      else
        numberDomain++;
    }
    i++;
  }
  setNewToken(token::ENDFILE," ",tokens);
  return numberDomain;
}


