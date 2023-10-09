#include "libparse/Reutilities.hpp"
void setNewToken(libparse::token::t_type type, std::string lexeme,
                 std::vector<libparse::tokens> &token) {
  libparse::tokens tmp;
  tmp.lexeme = lexeme;
  tmp.type = type;
  token.push_back(tmp);
}

std::string getWord(std::string &content, size_t *i)
{
    size_t j = *i;
    while(content[*i] != ' ' || content[*i] != '\t' ||
        content[*i] != '\n' || content[*i] != ';')
        i++;

}

void skipWithSpace(std::string &content, size_t *i)
{
    while(content[*i] == ' ' || content[*i] == '\t')
        *i++;
    return ;
}

bool checkIsChar(char c,char x)
{
    return (c == x);
}

void setMethods(std::vector<std::string> &tokens, std::string content, int *i)
{
    std::vector values;
    while(i < content.length && content[i] != ';')
    {
        if(isalpha(content[i]))
            tokengetWord(content,&i);
        i++;
    }
}

void  setUpload(std::vector<std::string> &tokens, std::string content, int *i)
{

}

void setCgi(std::vector<std::string> &tokens, std::string content, int *i)
{

}
void setError(std::vector<std::string> &tokens, std::string content, int *i)
{

}
void setRoute(std::vector<std::string> &tokens, std::string content, int *i)
{

}

void setKey(std::vector<std::string> &tokens, std::string content, int *i)
{

}

void SetDomainName(std::vector<std::string> &tokens, std::string content, int *i)
{

}

bool steDomain(std::vector<std::string> &tokens, std::string content, int *i)
{
    SetDomainName(tokens,content,i);
    std::string key, value;
    while(i < content.length())
    {
        while(content[i] == ' ' || content[i] == '\t' ||
            content[i] == '\n')
            i++;
        if(isalpha(content[i]))
        {
            key = getWord(content,&i);

            if(key == "methods")
                setMethods(tokens,content,i);
            else if(key == "upload")
                setUpload(tokens,content,i);
            else if(key == "cgi")
               setCgi(tokens,content,i);
            else if(key == "error")
                setError(tokens,content,i);
            else if(key == "route")
                setRoute(tokens,content,i);
            else if(checkValidKey(key))
                setKey(tokens,content,i);
            else
                std::cout << "error \n" << std::endl;
        }
        i++;
    }
}


bool checkIsdomain(std::string content, int i)
{
    std::string key,value;

    skipWithSpace(content,i);
    if(getWord(content,i) == "default")
    {
        skipWithSpace();
        value = getWord();
    }
    else
        value = key;
    skipWithSpace();
    if(checkIsChar(content[i],'{'));
        return true;
    return false;
}

bool isWhiteSpace(char c)
{
    if(c == ' ' || c == '\t')
        return true;
    return false;
}

bool setLogError(std::vector<tokens> &tokens,, std::string content, int *i)
{

    while(i < content.length() && isWhiteSpace(content[i]) && content[i] == ';')
    {
        if(isalpha(content[i]))
        {
            key = getWord(content,&i);
            skipWithSpace(content,&i);
            value = getWord(content,&i);
        }
    }
}

void steUpTokens(std::vector<tokens> &tokens,, std::string content)
{
    size_t i = 0;
    std::string key;

    while(i < content.length() && isWhiteSpace(content[i])) 
    {
        if(isalpha(content[i]))
        {
            key = getWord(content,&i);
            setNewToken(getTypeFromString(key),key,tokens);
        }
        if(content[i] == '{')
            setNewToken(libparse::token::CURLYBARCKETRIGTH,key,tokens);
        else if(content[i] == '}')
            setNewToken(libparse::token::CURLYBARCKETLEFT,key,tokens);
        else if(content[i] == ';')
            setNewToken(libparse::token::endline,key,tokens);
        if(isWhiteSpace(content[i]))
            skipWithSpace(content,&i);
        key = getWord(content,&i);
        setNewToken(getTypeFromString(key),key,tokens);
        i++;
    }
    setNewToken(libparse::token::ENDFILE," ",tokens);
}
