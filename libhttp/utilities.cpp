#include "utilities.hpp"
#include <utility>
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

bool fileExists(std::string &filename) {
    std::ifstream file(filename);
    if(file)
      return true;
    return false;
}

bool isFolder(std::string &path)
{
  if(path[path.length() - 1] == '\\')
    return true;
  return false;
}

bool directoryExists(std::string &path) {
  struct stat sb;

   if (stat(path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
     return true;
  return false;

}

bool findResource(std::string &path)
{
  if(!isFolder(path))
    return fileExists(path);
  return directoryExists(path);
}

