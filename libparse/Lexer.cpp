#include "Config.hpp"
#include "utilities.hpp"

void setNewToken(libparse::token::t_type type, std::string lexeme,
                 std::vector<libparse::tokens> &token) {
  libparse::tokens tmp;
  tmp.lexeme = lexeme;
  tmp.type = type;
  token.push_back(tmp);
}

libparse::token::t_type getTypeFromString(std::string typeStr) {
  if (typeStr.length() == 0)
    return libparse::token::NONO;
  else if (typeStr == "}")
    return libparse::token::CURLYBARCKETLEFT;
  else if (typeStr == "{")
    return libparse::token::CURLYBARCKETRIGTH;
  else if (typeStr == "root")
    return libparse::token::ROOT;
  else if (typeStr == "route")
    return libparse::token::ROUTE;
  else if (typeStr == "methods")
    return libparse::token::METHODS;
  else if (typeStr == "redir")
    return libparse::token::REDIR;
  else if (typeStr == "index")
    return libparse::token::INDEX;
  else if (typeStr == "error")
    return libparse::token::ERROR;
  else if (typeStr == "max_body_size")
    return libparse::token::MAXBODYSIZE;
  else if (typeStr == "max_header_size")
    return libparse::token::MAXHEADERSSIZE;
  else if (typeStr == "dir_listing")
    return libparse::token::DIRLISTENING;
  else if (typeStr == "upload")
    return libparse::token::UPLOAD;
  else if (typeStr == "cgi")
    return libparse::token::CGI;
  else if (typeStr == "default")
    return libparse::token::DEFAULT;
  else if(typeStr == "log_info")
    return libparse::tokens::LOGINFO;
  else if(typeStr == "log_error")
    return libparse::tokens::LOGERROR; 
  else
    return libparse::token::KEYWORD;
}

void libparse::lexer(std::vector<libparse::tokens> &tokens,std::string &content)
{
    size_t i = 0;
    std::string key;

    while(i < content.length()) 
    {
      if(content[i] == '{')
        setNewToken(libparse::token::CURLYBARCKETRIGTH,"{",tokens);
      else if(content[i] == '}')
        setNewToken(libparse::token::CURLYBARCKETLEFT,"}",tokens);
      else if(content[i] == ';')
        setNewToken(libparse::token::ENDLINE,"_",tokens);
      else if(isWhiteSpace(content[i]))
        skipWithSpace(content,&i),i--;
      else
      {      
        key = getWord(content,&i);
        setNewToken(getTypeFromString(key),key,tokens);
        i--;
      }    
     i++;
    }
    setNewToken(libparse::token::ENDFILE," ",tokens);
}
