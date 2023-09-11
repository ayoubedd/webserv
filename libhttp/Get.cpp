#include "Get.hpp"
#include <utility>
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

std::pair<libhttp::Get::t_error,int> Get(std::string path)
{
  if(!findResource(path))
    return  std::make_pair(libhttp::Get::FILE_NOT_FOUND,-1);

  int fd = open(path.c_str(), O_RDONLY) ;
  if (fd == -1)
        return std::make_pair(libhttp::Get::FORBIDDEN,-1);

  return std::make_pair(libhttp::Get::OK,fd);
}
