#include "libhttp/Methods.hpp"

#include <iostream>
#include <fstream>
#include <ctime>

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
    closedir(dir);
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

void initGetRes(libhttp::Methods::GetRes &getReq)
{
    getReq.fd = -1;
    getReq.range.first = 0;
    getReq.range.second = -1;
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

std::string get_file_last_modification(const std::string &file_path) {
    std::ifstream file(file_path.c_str());
    std::string modification_date;
  
    if (!file) {
        return modification_date;
    }
    std::time_t modification_time = std::time(0);
    struct std::tm *modification_tm = std::localtime(&modification_time);
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", modification_tm);
    return modification_date;
}

std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > listFilesAndDirectories(std::string &path)
{
  std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> pairOfFiles;
  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > vecFileAndDir;
  
  DIR *dir;
  
  struct dirent *entry;

    // Open the directory
    if ((dir = opendir(path.c_str())) == NULL) {
     pairOfFiles.first = libhttp::Methods::NOT_FOUND;
      vecFileAndDir.push_back( pairOfFiles);
      return vecFileAndDir;
    }
    // Read directory entries
    while ((entry = readdir(dir)) != NULL)
    {
      // Skip . and .. entries
      if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name ,"..") ){
          continue;
      }
      // Check if it's a directory
      if (entry->d_type == DT_DIR) {
        pairOfFiles.second.name = entry->d_name;
        pairOfFiles.second.date = get_file_last_modification(path+entry->d_name);
        pairOfFiles.second.size = getFileSize(path+entry->d_name);
        pairOfFiles.first = libhttp::Methods::DIR;
        vecFileAndDir.push_back( pairOfFiles);
      }
      // Check if it's a regular file
      else 
      {
        pairOfFiles.second.name = entry->d_name;
        pairOfFiles.first = libhttp::Methods::FILE;
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
std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> libhttp::Get(libhttp::Request &request,std::string path)
{
  // you must encode and decode
  std::pair<ssize_t , ssize_t > range;
  libhttp::Methods::GetRes getReq;

  // init sturct of get Request by -1,0,-1
  initGetRes(getReq);
  // check Resource 
  if(!findResource(path))
    return  std::make_pair(libhttp::Methods::error::FILE_NOT_FOUND,getReq);

  if(!isFolder(path))
  {
    getReq.fd = open(path.c_str(), O_RDONLY);
    if (getReq.fd == -1)
        return  std::make_pair(libhttp::Methods::error::FORBIDDEN,getReq);
  // handle bytes range get request
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
  // handle simple get request 
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
