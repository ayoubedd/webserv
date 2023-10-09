#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"



void skipWithSpace(std::string &content, size_t *i)
{
    while(content[*i] == ' ' || content[*i] == '\t' || content[*i] == '\n')
        (*i)++;
}

std::string getWord(std::string &content, size_t *i)
{
  size_t j = *i;
  std::string value;

  while((content[*i] != ' ' && content[*i] != '\t' &&
    content[*i] != '\n' && content[*i] != ';') && (*i < content.size()))
      (*i)++;
      
  value =  content.substr(j,*i-j);
  return value;
}
bool isWhiteSpace(char c)
{
    if(c == ' ' || c == '\t' || c == '\n')
        return true;
    return false;
}

void steUpTokens(std::vector<libparse::tokens> &tokens,std::string content)
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
        setNewToken(libparse::token::SEMICOLON,"_",tokens);
      else if(isWhiteSpace(content[i]))
        skipWithSpace(content,&i),i--;
      else if(isalpha(content[i]))
      {      
        key = getWord(content,&i);
        setNewToken(getTypeFromString(key),key,tokens);
        i--;
      }    
     i++;
    }
    setNewToken(libparse::token::ENDFILE," ",tokens);
}

// std::string help(std::vector<libparse::tokens> tokens, int start, int end)
// {
//   std::string values;

//   for(size_t i = start, i < end ;i++)
//   {
//     values = " " + tokens[i].lexeme;
//   }
//   return values;
// }


// bool checkIsKey(libparse::type type)
// {
//   if(libparse::ROOT || libparse::METHODS || libparse::REDIR
//     || libparse::INDEX || libparse::ERROR || libparse:: MAXBODYSIZE || libparse:: DIRLISTENING
//       || libparse:: UPLOAD || libparse::CGI || libparse::LOGERROR || libparse::LOGINFO)
//        return true;
//   return false;
//}

bool isToken()
{

}

void consumeServer(std::vector<libparse::tokens> &tokens, size_t *i)
{
}

void consumeLog(std::vector<libparse::tokens> &tokens, size_t *i)
{

}

void consumeToken(std::vector<libparse::tokens> &tokens, size_t *i)
{
  
}

bool Lexer(std::vector<libparse::tokens> &tokens)
{
  std::vector<libparse::tokens> tmp;
    size_t i = 0;
    while(tokens[i].type != libparse::tokens::ENDFILE)
    {
      if(tokens[i].lexeme == "log_error" || tokens[i].lexeme == "log_info")
      {
        consumeLog(tokens,&i);
      }
      else if(tokens[i].lexeme == "default")
      {
        consumeToken(tokens,&i);
        consumeServer(tokens,&i);
      }
      else
        consumeServer(tokens,&i);;
      continue;
    }

}

bool test(std::vector<libparse::tokens> tokens)
{
    std::vector<libparse::tokens> tokenss;
    size_t i = 0;
    size_t j= 0;
    while(tokens[i].type != libparse::tokens::ENDFILE)
    {
        if(checkIsKey(tokens[i].type))
        {
          j = i++;
          while(tokens[i].type != libparse::ENDLINE && tokens[i].type != libparse::ENDFILE)
                i++;
          
          setNewToken(libparse::tokens::METHODS, help(tokens,j,i), tokenss);

        }
        else if(tokens[i].type ==libparse::DEFAULT)
        {
          j =i++;
          if(tokens[i].type != libparse::KEYWORD)
            std::cout << "error \n";
          i++;
          if(tokens[i].type != libparse::CURLYBARCKETLEFT)
            std::cout << "error \n";
          setNewToken(libparse::tokens::DOMAIN,tokens[i-1], tokenss);
        }
        else if(tokens[i].type ==libparse::CURLYBARCKETRIGTH)
        {
          // set name domain
          if(i != 0)
          {
          if(tokens[i-1].type == libparse::KEYWORD)
              setNewToken(libparse::tokens::DOMAIN,tokens[i-1], tokenss);
          }
          else
              std::cout << "error \n";
          
        }
        else if(tokens[i].type ==libparse::ROUTE)
        {
          // set route

        }
        i++;
    }
    printTokens(tokenss);
}

void printTokens(  std::vector<libparse::tokens> tokens) {

  for(auto i = 0; i < tokens.size(); i++)
    std::cout << tokens[i].type << "|" << tokens[i].lexeme <<"|"<< std::endl;

}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }
  std::vector<libparse::tokens> tokens;
  std::string content = libparse::readFile(argv[1]);
  steUpTokens(tokens,content);
  //test(tokens);
  printTokens(tokens);
  return 0;
}
