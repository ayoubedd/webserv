#include "Delete.hpp"

static bool directoryExists(std::string &path) {
  struct stat sb;

   if (stat(path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
     return true;
  return false;

}

static bool fileExists(std::string &filename) {
    std::ifstream file(filename);
    if(file)
      return true;
    return false;
}

static void setTypeResource(libhttp::Delete &Delete)
{
  if(Delete.path[Delete.path.length() - 1] == '\\')
    Delete.isFolder = true, Delete.isFile = false;
  else
    Delete.isFile = true, Delete.isFolder= false;
}

static bool findResource(libhttp::Delete & Delete)
{
  setTypeResource(Delete);
  if(Delete.isFile)
    return fileExists(Delete.path);
  if(Delete.isFolder)
    return directoryExists(Delete.path);
}

static void init(libhttp::Delete &Delete, std::string path)
{
  Delete.path = path;
}

void Delete(libhttp::Delete &Delete, std::string &path)
{
  init(Delete,path);
  if(findResource(Delete))
  {
    if(Delete.isFile)
    {
       
    }
    if(Delete.isFolder)
    {
      
    }
  }
  else
  {
    // not found 404
  }
}
