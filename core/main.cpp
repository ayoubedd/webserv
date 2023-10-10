#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include "libparse/TestParser.hpp"
#include <iostream>
#include <utility>
#include <vector>

// bool checkIsKey(libparse::tokens type)
// {
//   if(libparse::tokens::ROOT == type || libparse::tokens::METHODS == type || libparse::tokens::REDIR == type
//     || libparse::tokens::INDEX || libparse::tokens::ERROR || libparse::tokens::MAXBODYSIZE || libparse::tokens:: DIRLISTENING
//       || libparse::tokens::UPLOAD || libparse::tokens::CGI || libparse::tokens::LOGERROR || libparse::tokens::LOGINFO)
//        return true;
//   return false;
// }

bool checkIsKeyServer(libparse::token::t_type type)
{
  if(libparse::tokens::ROOT == type || libparse::tokens::METHODS == type || libparse::tokens::REDIR == type
    || libparse::tokens::INDEX || type == libparse::tokens::ERROR || type == libparse::tokens::MAXBODYSIZE )
    return true;
  return false;

}

bool checkIsKeyRoute(libparse::token::t_type type)
{
  if(libparse::tokens::ROOT == type || libparse::tokens::METHODS == type || libparse::tokens::REDIR == type
    || type == libparse::tokens::INDEX || type == libparse::tokens:: DIRLISTENING || type == libparse::tokens::UPLOAD 
    || type == libparse::tokens::CGI)
       return true;
  return false;

}

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


void Lexer(std::vector<libparse::tokens> &tokens,std::string content)
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


bool setUpLog(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i){

  if(tokens[*i].lexeme == "log_error")
  {
    (*i)++;
    if(tokens[*i].type == libparse::tokens::ENDLINE)
      return false;
    config.log_error = tokens[*i].lexeme;
  }

  else if(tokens[*i].lexeme == "log_info")
  {
    (*i)++;
    if(tokens[*i].type == libparse::tokens::ENDLINE )
      return false;
    config.log_info = tokens[*i].lexeme;
  }
  (*i)++;
  if(tokens[*i].lexeme != "_")
    return false;

  return true;
}

bool checkMethod(std::string method)
{
  if(method == "GET" || method == "POST" || method == "DELETE")
    return true;
  return false;
}

bool setUpMethods(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  (*i)++;
  std::vector<std::string > vec;

   while(tokens[*i].type != libparse::tokens::ENDLINE)
    {
      if(checkMethod(tokens[*i].lexeme))
        vec.push_back(tokens[*i].lexeme);
      else
      return false;
       (*i)++; 
    }
   // setUp methods config

   return true;
}

bool setUpCgi(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
    (*i)++; 
    int j = *i; 
    while(tokens[*i].type != libparse::tokens::ENDLINE)
    {
            
    }
    if(*i - j > 3)
      return  false;
    (*i)++;
  return true;
}

bool setUpRout(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  // cousum route
  // cousum path
  // {
   while(tokens[*i].type != libparse::tokens::CURLYBARCKETLEFT)
    {
      if(!checkIsKeyRoute(tokens[*i].type))
      {
        if(tokens[*i].type == libparse::tokens::METHODS)
          setUpMethods(config,tokens,i);
        if(tokens[*i].type == libparse::tokens::CGI)
          setUpMethods(config,tokens,i);
      }
      else
      {
        std::cout << "Error " << tokens[*i].lexeme << std::endl;
        return false;
      }
    }
   return true;
}

bool setUpDomain(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  if(!checkDomain(tokens[*i].lexeme))
    return false;
  // setup value domain
  return true; 
}

bool setUpKey(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  return true;
}

bool SetUpServer(libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  std::string strDomain, strRoute;
  libparse::Routes route;
    if(!setUpDomain(config,tokens,i))
        return false;
    (*i)++;
    if(tokens[*i].type != libparse::tokens::CURLYBARCKETRIGTH)
      return false;

   while(tokens[*i].type != libparse::tokens::ENDFILE)
  {
      if(checkIsKeyServer(tokens[*i].type))
      {
        if(!setUpKey(config,tokens,i))
          return false;
      }
      if(tokens[*i].type != libparse::tokens::ROUTE)
      {
        if(!setUpRout(config,tokens,i))
          return false;
      }
      else
      {
        std::cout << "Error " << tokens[*i].lexeme << std::endl;
        return false;
      }
  }
   return true;
}

bool setUpDefaultSever( libparse::Config &config,std::vector<libparse::tokens> &tokens, size_t *i)
{
  return true;
}

bool parser(libparse::Config &config,std::vector<libparse::tokens> &tokens)
{
    size_t i = 0;
    while(tokens[i].type != libparse::tokens::ENDFILE)
    {
      if(tokens[i].lexeme == "log_error" || tokens[i].lexeme == "log_info")
      {
        if(!setUpLog(config,tokens,&i))
          return false;
      }
      else if(tokens[i].lexeme == "default")
      {
        setUpDefaultSever(config,tokens,&i);
        SetUpServer(config,tokens,&i);
      }
      else
        SetUpServer(config,tokens,&i);
      i++;
    }
  return true;
}


void printTokens(  std::vector<libparse::tokens> tokens) {

  for(auto i = 0; i < tokens.size(); i++)
    std::cout << getTypeFromInt(tokens[i].type) << "|" << tokens[i].lexeme <<"|"<< std::endl;

}


void printConfig(libparse::Config &config)
{
  std::cout << "|"<<config.log_error << "|"<<std::endl;
  std::cout << "|"<<config.log_info << "|"<<std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }
  std::vector<libparse::tokens> tokens;
  std::string content = libparse::readFile(argv[1]);
  libparse::Config config;
  Lexer(tokens,content);
  printTokens(tokens);
  // int a = parser(config,tokens);
  // std::cout << "=====> "  << a << std::endl;
  // printConfig(config);
  return 0;
}




    // if(tokens[*i].type == libparse::token::ROOT)
    // {

    // }
    // if(tokens[*i].type == libparse::tokens::METHODS)
    // {

    // }
    // if(tokens[*i].type == libparse::tokens::REDIR)
    // {

    // }
    // if(tokens[*i].type == libparse::INDEX)
    // {

    // }
    // if(tokens[*i].lexeme == libparse::ERROR)
    // {

    // }
    // if(libparse:: MAXBODYSIZE)
    // if(libparse:: DIRLISTENING)
    // if(libparse:: UPLOAD )
    // if(libparse::CGI)
    // if(libparse::LOGERROR) 
    // if(libparse::LOGINFO)
