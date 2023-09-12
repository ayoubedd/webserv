#include "libhttp/Methods.hpp"

bool fileExists(std::string &filename) {
    std::ifstream file(filename);
    if(file)
      return true;
    return false;
}

bool isFolder(std::string &path)
{
  if(path[path.length() - 1] == '/')
    return true;
  return false;
}

bool directoryExists(std::string &path) {
  DIR* dir = opendir(path.c_str());

  if (dir == nullptr) {
      return false;
  }
  closedir(dir);
  return true;
}

bool findResource(std::string &path)
{
  if(!isFolder(path))
    return fileExists(path);
  return directoryExists(path);
}

bool deleteDirectory(const char* path) {
    struct dirent* entry;
    DIR* dir = opendir(path);

    if (dir == nullptr) {
        return false;
    }
    while ((entry = readdir(dir))) {
        //if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        if (entry->d_name != "." && entry->d_name != "..")
        {
            std::string entryPath = std::string(path) + "/" + entry->d_name;
            struct stat statBuf;

            if (stat(entryPath.c_str(), &statBuf) == 0) {
                if (S_ISDIR(statBuf.st_mode)) {
                    deleteDirectory(entryPath.c_str());
                }
                else {
                    if (remove(entryPath.c_str()) != 0) {
                        return false;
                    }
                }
            }
        }
    }
    closedir(dir);
    return true;
}

bool checkRangeRequest(libhttp::Headers &headers)
{
  if(headers.headers.find("Content-Range") != headers.headers.end())
    return true;
  return false;
}

std::pair<int ,int> getStartandEndRangeRequest(std::string str)
{
  int start = 0,end = 0;

  std::stringstream strm(str.substr(6,str.find("-",6) - 6 ));
  strm >> start;
  std::stringstream strm1(str.substr(str.find("-",6) + 1,str.find("/",str.find("-",6)) - str.find("-",6)));
  strm1 >> end;
    return std::make_pair(start,end);
}
void initGetRes(libhttp::Methods::GetRes &getReq)
{
    getReq.fd = -1;
    getReq.range.first = 0;
    getReq.range.second = 0;
}


// Request Get

std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> libhttp::Get(libhttp::Request &request)
{
    std::string path;
    // you must encode and decode
    path = request.reqTarget.path;

    std::pair<int , int > range;
    libhttp::Methods::GetRes getReq;

    initGetRes(getReq);

  if(!findResource(path))
    return  std::make_pair(libhttp::Methods::error::FILE_NOT_FOUND,getReq);

  int fd = open(path.c_str(), O_RDONLY) ;
  getReq.fd = fd;
  if (fd == -1)
    return  std::make_pair(libhttp::Methods::error::FORBIDDEN,getReq);

    if(checkRangeRequest(request.headers))
    {
        range = getStartandEndRangeRequest(request.headers["Content-Range"]);
        getReq.range.first = range.first;
        getReq.range.second = range.second;
    }
    return std::make_pair(libhttp::Methods::OK,getReq);
}


// Request Delete
libhttp::Methods::error libhttp::Deletes(std::string &path)
{
    if(findResource(path))
    {
    if(isFolder(path))
    {
        if (deleteDirectory(path.c_str()))
            return libhttp::Methods::OK;
        else
            return libhttp::Methods::FORBIDDEN;
    }
    else
    {
        if (remove(path.c_str()) != 0)
            return libhttp::Methods::FORBIDDEN;
        else
            return libhttp::Methods::OK;
    }
    }
    return libhttp::Methods::FILE_NOT_FOUND;
}