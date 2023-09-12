#include "libhttp/Get.hpp"
#include <utility>
#include <fcntl.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include "libhttp/Request.hpp"
#include "libhttp/Headers.hpp"

bool checkRangeRequest(libhttp::Headers &headers)
{
  if(headers.headers.find("Content-Range") != headers.headers.end())
    return true;
  return false;
}
std::pair<int ,int> getStartandEndRangeRequest(std::string str)
{
  int j=0 ;
  int start,end,total;
  std::string tmp;


  if(strncmp(str.substr(0,5), "bytes",5) != 0)
    return ;
  tmp = str.find("-",6);
  std::stringstream strm(str.substr(6,str.find("-",6) - 6 ));
  strm >> start;
  std::stringstream strm(str.substr(str.find("-",6),str.find("/",str.find("-",6)) - str.find("-",6)));
  strm >> end;
  std::stringstream strm(str.substr(str.rfind("/")));
  strm >> total;

}


std::pair<libhttp::Get::error,libhttp::Get::GetRes> Get(std::string path)
{
  if(!findResource(path))
    return  std::make_pair(libhttp::Get::FILE_NOT_FOUND,-1);

  int fd = open(path.c_str(), O_RDONLY) ;
  if (fd == -1)
        return std::make_pair(libhttp::Get::FORBIDDEN,-1);

  return std::make_pair(libhttp::Get::OK,fd);
}
