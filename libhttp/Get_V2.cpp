#include "Get_V2.hpp"
#include <stdlib.h>
#include <unistd.h>

bool directoryExists(std::string &path) {
  struct stat sb;

   if (stat(path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
     return true;
  return false;

}

bool fileExists(std::string &filename) {
    std::ifstream file(filename);
    if(file)
      return true;
    return false;
}

static void setTypeResource(libhttp::Get &get)
{
  if(get.path[get.path.length() - 1] == '\\')
    get.isFolder = true, get.isFile = false;
  else
    get.isFile = true, get.isFolder= false;
}

static bool findResource(libhttp::Get &get)
{
  setTypeResource(get);
  if(get.isFile)
    return fileExists(get.path);
  return directoryExists(get.path);
}

void openDir(libhttp::Get &get)
{ 
  std::map<std::string , std::string> dirContents;
  std::map<std::string , std::string>::iterator it;
  std::string templateDir;
  std::string newLine,openTag, closeTag;

  int pipefd[2];
  if (pipe(pipefd) == -1) 
  {
        perror("pipe");
        return ;
  }
  openTag ="<h3>";
  closeTag = "</h3>";
  newLine = "<br>";
  
  templateDir ="<!DOCTYPE html><html lang=\"en\"><head> \
                <meta charset=\"UTF-8\"><meta name=\"viewport\"\
                content=\"width=device-width, initial-scale=1.0\">\
                <title>Index</title></head><body><h1>"+get.path+"</h1>";

  for(it = dirContents.begin() ; it != dirContents.end(); it++)
  {
    if(it->first == "dir")
      templateDir+=openTag+(it->second)+"//"+ closeTag;
    else
      templateDir+=openTag+(it->second)+ closeTag;
  }
  templateDir+="</body></html>";
  write(pipefd[0],templateDir.c_str(), templateDir.length());
  get.error = false;
  get.status = 200;

}

void openFile(libhttp::Get &get)
{
  // open file 
  get.fd = open(get.path.c_str(), O_RDONLY) ;
  if (get.fd == -1)
	{
    get.done = true;
    get.error = true;
    get.status = 403;
    return ;
	}
  get.status = 200;
  
}

static void init(libhttp::Get &get,std::string path)
{
  get.path = path; 
  get.done = false;
  get.error = false;
  get.isFile = false;
  get.isFolder = false;
  get.status = 0;
}

libhttp::Get Get(std::string path)
{
  libhttp::Get get;

  init(get,path);

  if(!findResource(get))
  {
    // 404 not found  
  }
  else
  {
    if(get.isFile)
      openFile(get); 
    else
      openDir(get);
  }

  return get;
}

