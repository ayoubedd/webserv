#include "libhttp/Methods.hpp"
#include "libparse/Types.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdlib.h>
#include <string>

bool directoryExists(std::string &path);
bool findResource(std::string &path);
bool deleteDirectory(const char* path);
bool checkRangeRequest(libhttp::Headers &headers);
std::pair<int ,int> getStartandEndRangeRequest(std::string str);
void initGetRes(libhttp::Methods::GetRes &getReq,std::string &path);
void setRange(libhttp::Methods::GetRes &getReq, std::pair<int, int> range);
bool checkAutoindex(std::string &name);
int getFileSize(const std::string &file_path);
std::string getFileLastModification(const std::string &file_path);
std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > listFilesAndDirectories(std::string &path);
std::string generateTemplate(std::string &path);
//

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
      if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name ,".."))
          continue ;
      else {
        std::string entryPath = std::string(path) + entry->d_name;
        // std::cout << entryPath << std::endl;
        struct stat statBuf;
        if (stat(entryPath.c_str(), &statBuf) == 0) {
          if (S_ISDIR(statBuf.st_mode)) {
              deleteDirectory(entryPath.c_str());
          }
          else {
            std::cout << entryPath << std::endl;
            if (remove(entryPath.c_str()) != 0) {
              return false;
          }
        }
      }
    }
    }
    closedir(dir);
    if(rmdir(path) != 0)
      return false;
    return true;
}

bool checkRangeRequest(libhttp::Headers &headers)
{
  if(headers.headers.find(libhttp::Headers::Content_Range) != headers.headers.end())
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

void initGetRes(libhttp::Methods::GetRes &getReq,std::string &path)
{
    getReq.fd = -1;
    getReq.range.first = 0;
    getReq.range.second = getFileSize(path);
}

void setRange(libhttp::Methods::GetRes &getReq, std::pair<int, int> range)
{
    getReq.range.first = range.first;
    getReq.range.second = range.second;
}

bool checkAutoindex(std::string &name)
{
  if(name[name.length() -1 ] == '\\')
    return false;
  return true;
}

int getFileSize(const std::string &file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::ate);
    if (!file) {
        return -1;
    }
    std::ifstream::pos_type size = file.tellg();
    file.close();
    if (size == -1) {
        return -1;
    }
    return static_cast<int>(size);
}

std::string getFileLastModification(const std::string &file_path) {
    std::ifstream file(file_path.c_str());
    std::string modification_date;
 
    if (!file.is_open()) {
        return modification_date;
    }
    std::time_t modification_time = std::time(0);
    struct std::tm *modification_tm = std::localtime(&modification_time);
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", modification_tm);
    modification_date = buffer;
    return modification_date;
}

std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > listFilesAndDirectories(std::string &path)
{
  std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> pairOfFiles;
  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > vecFileAndDir;
  
  DIR *dir;
  
  struct dirent *entry;

    if ((dir = opendir(path.c_str())) == NULL) {
     pairOfFiles.first = libhttp::Methods::NOT_FOUND;
      vecFileAndDir.push_back( pairOfFiles);
      return vecFileAndDir;
    }
    while ((entry = readdir(dir)) != NULL)
    {
      if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name ,"..") ){
          continue;
      }
      if (entry->d_type == DT_DIR) {
        pairOfFiles.second.name = entry->d_name;
        pairOfFiles.second.date = getFileLastModification(path+entry->d_name);
        pairOfFiles.second.size = -1;
        pairOfFiles.first = libhttp::Methods::DIR;
        vecFileAndDir.push_back( pairOfFiles);
      }
      else 
      {
        pairOfFiles.second.name = entry->d_name;
        pairOfFiles.first = libhttp::Methods::FILE;
        pairOfFiles.second.date = getFileLastModification(path+entry->d_name);
        pairOfFiles.second.size = getFileSize(path+entry->d_name);
        vecFileAndDir.push_back( pairOfFiles);
      }
    }
    closedir(dir);
    return vecFileAndDir;
}


std::string generateTemplate(std::string &path)
{
  std::string templateStatic;
  std::ifstream templateFile("static/index.html");
  std::ifstream itemFile("static/list-item.html");
  std::stringstream buf,buffer;
  std::string listItem;
  std::string listItemTemplate;
  std::string tmp;
  
  buffer << itemFile.rdbuf();
  itemFile.close();
  listItemTemplate = buffer.str();

  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > test;
  test = listFilesAndDirectories(path);
  
  for(size_t i = 0; i < test.size() ;i++)
  {
    tmp = listItemTemplate;
    ft_replace(tmp,"{{LINK_HERE}}",path + test[i].second.name);
    ft_replace(tmp,"{{FILE_NAME}}",test[i].second.name);
    ft_replace(tmp,"{{LAST_MODIFIED}}",test[i].second.date);
    if(test[i].second.size == -1)
      ft_replace(tmp,"{{SIZE_OR_TYPE}}","Dir");
    else
      ft_replace(tmp,"{{SIZE_OR_TYPE}}",std::to_string(test[i].second.size));
    listItem+=tmp;
  }
  buf << templateFile.rdbuf();
  templateFile.close();
  templateStatic = buf.str();
  ft_replace(templateStatic,"{{INSERT_TITLE_HERE}}",path);
  ft_replace(templateStatic,"{{INSERT_PATH_HERE}}",path);
  ft_replace(templateStatic,"{{RANGE_OF_ITEMS}}",listItem);
  return templateStatic;
}

// Request Get
ssize_t libhttp::getFile(std::string &path,int status)
{
  ssize_t fd;
  fd = open(path.c_str(), O_RDONLY);
  if(status == 200)
    return fd;

  std::ifstream templateFile(path);
  std::stringstream tmp;
  std::string buffer;
  tmp << templateFile.rdbuf();
  templateFile.close();
  buffer = tmp.str();
  ft_replace(buffer,"{{STATUS}}",std::to_string(status));
  int p[2];
  if (pipe(p) < 0)
      return -1;
  write(p[1],buffer.c_str(),buffer.length());
  return p[0];
}

std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> libhttp::Get(libhttp::Request &request,std::string path)
{
  libhttp::Methods::GetRes getReq;

  if(!findResource(path))
    return  std::make_pair(libhttp::Methods::error::FILE_NOT_FOUND,getReq);

  initGetRes(getReq,path);

  if(!isFolder(path))
  {
    getReq.fd = getFile(path,200);
    if (getReq.fd == -1)
        return  std::make_pair(libhttp::Methods::error::FORBIDDEN,getReq);

  getReq.contentType =libparse::getTypeFile(libparse::Types(),path);
    if(checkRangeRequest(request.headers))
        setRange(getReq,getStartandEndRangeRequest(request.headers[libhttp::Headers::Content_Range]));

    return std::make_pair(libhttp::Methods::OK,getReq);
  }

  std::string templateStatic;
  int p[2];

  if (pipe(p) < 0)
      return std::make_pair(libhttp::Methods::FORBIDDEN,getReq);
  templateStatic = generateTemplate(path);
  write(p[1],templateStatic.c_str(),templateStatic.length());
  getReq.fd = p[0];
  getReq.contentType = "text/html";
  getReq.range.first = 0;
  getReq.range.second = templateStatic.length();

  return std::make_pair(libhttp::Methods::OK,getReq);
}

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
