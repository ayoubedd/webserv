#include "Config.hpp"

std::vector<tokens>  lexer(std::string content)
{
  std::vector<std::string> vecTokens;
  std::vector<tokens> tokens;
  std::string token;

  addSpace(content);
  vecTokens = split(content);
  while(vecTokens.size() > 0)
  {
    if(vecTokens.size() > 1)
    {
      if(vecTokens[1] == "{")
      {
        size_t pos = vecTokens[0].find(':');
        if(pos != std::string::npos)
          {
            setNewToken(token::DOMAINS,vecTokens[0],tokens);
            setNewToken(token::PORT,vecTokens[0].substr(pos+1),tokens);
          }
          else
          {
            setNewToken(token::DOMAINS,vecTokens[0].append(":80"),tokens);
            setNewToken(token::PORT,"80",tokens);
          }
        vecTokens.erase(vecTokens.begin());
        setNewToken(getTypeFromString(vecTokens[0]),vecTokens[0],tokens);
        vecTokens.erase(vecTokens.begin());
      continue;
      }
    }
    if(vecTokens[0] == "{")
    {
      setNewToken(token::DOMAINS,"localhost:80",tokens);
      setNewToken(token::PORT,"80",tokens);
      vecTokens.erase(vecTokens.begin());
      setNewToken(getTypeFromString(vecTokens[0]),vecTokens[0],tokens);
      vecTokens.erase(vecTokens.begin());
      continue;
    }
    if(getTypeFromString(vecTokens[0]) == token::ROUTE)
    {
      setNewToken(getTypeFromString(vecTokens[0]),vecTokens[0],tokens); 
      vecTokens.erase(vecTokens.begin());
      setNewToken(token::PATH,vecTokens[0],tokens);
      vecTokens.erase(vecTokens.begin());
      setNewToken(getTypeFromString(vecTokens[0]),vecTokens[0],tokens);
      vecTokens.erase(vecTokens.begin());
      continue;
    }
    if(getTypeFromString(vecTokens[0]) == token::CURLYBARCKETLEFT)
     {
      setNewToken(getTypeFromString(vecTokens[0]),vecTokens[0],tokens);
      vecTokens.erase(vecTokens.begin());
      continue;
     }
    if(vecTokens.size() > 1)
    { 
      if(getTypeFromString(vecTokens[0]) == token::CGI || getTypeFromString(vecTokens[0]) == token::UPLOAD)
      {
        vecTokens[1].append(" ").append(vecTokens[2]);
        setNewToken(getTypeFromString(vecTokens[0]),vecTokens[1],tokens);
        vecTokens.erase(vecTokens.begin());
        vecTokens.erase(vecTokens.begin());
        vecTokens.erase(vecTokens.begin());
        continue;
      }
      else if(getTypeFromString(vecTokens[0]) == token::METHODS)
      {
        if(vecTokens[2] == "|")
        {
          vecTokens[1].append(" ").append(vecTokens[3]);
          setNewToken(getTypeFromString(vecTokens[0]),vecTokens[1],tokens);
          vecTokens.erase(vecTokens.begin());
                  vecTokens.erase(vecTokens.begin());
        }
        else
        {
          setNewToken(getTypeFromString(vecTokens[0]),vecTokens[1],tokens);
        }
        vecTokens.erase(vecTokens.begin());
        vecTokens.erase(vecTokens.begin());
        continue;
      }
      else
      {
        setNewToken(getTypeFromString(vecTokens[0]),vecTokens[1],tokens);
        vecTokens.erase(vecTokens.begin());
        vecTokens.erase(vecTokens.begin());
        continue;
      }

    }
  }
  setNewToken(token::ENDFILE," ", tokens);
  return tokens;
}
