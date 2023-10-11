#include "libhttp/Methods.hpp"
#include "libparse/Types.hpp"

#include <cstddef>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>

const char *libhttp::Methods::GET = "GET";
const char *libhttp::Methods::DELETE = "DELETE";
const char *libhttp::Methods::POST = "POST";

off_t       advanceOffSet(int fd, size_t start);
static void ft_replace(std::string &str, const std::string &old_value,
                       const std::string &new_value) {
  size_t pos = 0;
  while ((pos = str.find(old_value, pos)) != std::string::npos) {
    str.replace(pos, old_value.length(), new_value);
    pos += new_value.length();
  }
}

bool                directoryExists(std::string &path);
bool                findResource(std::string &path);
bool                deleteDirectory(const char *path);
bool                checkRangeRequest(libhttp::Headers &headers);
std::pair<int, int> getStartandEndRangeRequest(std::string str);
void                initGetRes(libhttp::Methods::GetRes &getReq, std::string &path);
void                setRange(libhttp::Methods::GetRes &getReq, std::pair<int, int> range);
bool                checkAutoindex(std::string &name);
int                 getFileSize(const std::string &file_path);
std::string         getFileLastModification(const std::string &file_path);
std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> >
            listFilesAndDirectories(std::string &path);
std::string generateTemplate(std::string &path);
//

bool fileExists(std::string &filename) {
  std::ifstream file(filename);
  if (file)
    return true;
  return false;
}

bool isFolder(std::string &path) {
  if (path[path.length() - 1] == '/')
    return true;
  return false;
}

bool directoryExists(std::string &path) {

  DIR *dir = opendir(path.c_str());

  if (dir == nullptr) {
    return false;
  }
  closedir(dir);
  return true;
}

bool findResource(std::string &path) {
  if (!isFolder(path))
    return fileExists(path);
  return directoryExists(path);
}
bool deleteSubDirectory(const char *path) {

  struct dirent *entry;
  DIR           *dir = opendir(path);

  if (dir == nullptr) {
    return false;
  }
  while ((entry = readdir(dir))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
    else {
      std::string entryPath = std::string(path) + entry->d_name;
      struct stat statBuf;
      if (stat(entryPath.c_str(), &statBuf) == 0) {
        if (S_ISDIR(statBuf.st_mode)) {
          deleteSubDirectory(entryPath.c_str());
        } else {
          if (remove(entryPath.c_str()) != 0) {
            return false;
          }
        }
      }
    }
  }
  closedir(dir);
  if (rmdir(path) != 0)
    return false;
  return true;
}

bool deleteDirectory(const char *path) {

  struct dirent *entry;
  DIR           *dir = opendir(path);

  if (dir == nullptr) {
    return false;
  }
  while ((entry = readdir(dir))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
    else {
      std::string entryPath = std::string(path) + entry->d_name;
      struct stat statBuf;
      if (stat(entryPath.c_str(), &statBuf) == 0) {
        if (S_ISDIR(statBuf.st_mode)) {
          deleteSubDirectory(entryPath.c_str());
        } else {
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

bool checkRangeRequest(libhttp::Headers &headers) {
  if (headers.headers.size() == 0)
    return false;
  if (headers.headers.find(libhttp::Headers::CONTENT_RANGE) != headers.headers.end())
    return true;
  return false;
}

std::pair<int, int> getStartandEndRangeRequest(std::string str) {
  int start = 0, end = 0;

  std::stringstream strm(str.substr(6, str.find("-", 6) - 6));
  strm >> start;
  std::stringstream strm1(
      str.substr(str.find("-", 6) + 1, str.find("/", str.find("-", 6)) - str.find("-", 6)));
  strm1 >> end;
  return std::make_pair(start, end);
}

void initGetRes(libhttp::Methods::GetRes &getReq, std::string &path) {
  getReq.fd = -1;
  getReq.range.first = 0;
  getReq.range.second = getFileSize(path);
}

void setRange(libhttp::Response &response, std::pair<int, int> range) {
  // add error of reang
  if (range.second > range.first)
    return;
  response.bytesToServe = range.second - range.first;
  off_t offSet;
  offSet = advanceOffSet(response.fd, range.first);
  // error of offset
  if (offSet == -1)
    return;
}

bool checkAutoindex(std::string &name) {
  if (name[name.length() - 1] == '\\')
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
  std::string   modification_date;

  if (!file.is_open()) {
    return modification_date;
  }
  std::time_t     modification_time = std::time(0);
  struct std::tm *modification_tm = std::localtime(&modification_time);
  char            buffer[80];
  std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", modification_tm);
  modification_date = buffer;
  return modification_date;
}

std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> >
listFilesAndDirectories(std::string &path) {
  std::pair<libhttp::Methods::typeFile, libhttp::Methods::file>               pairOfFiles;
  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > vecFileAndDir;

  DIR *dir;

  struct dirent *entry;

  if ((dir = opendir(path.c_str())) == NULL) {
    pairOfFiles.first = libhttp::Methods::NOT_FOUND;
    vecFileAndDir.push_back(pairOfFiles);
    return vecFileAndDir;
  }
  while ((entry = readdir(dir)) != NULL) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }
    if (entry->d_type == DT_DIR) {
      pairOfFiles.second.name = entry->d_name;
      pairOfFiles.second.date = getFileLastModification(path + entry->d_name);
      pairOfFiles.second.size = -1;
      pairOfFiles.first = libhttp::Methods::DIR;
      vecFileAndDir.push_back(pairOfFiles);
    } else {
      pairOfFiles.second.name = entry->d_name;
      pairOfFiles.first = libhttp::Methods::FILE;
      pairOfFiles.second.date = getFileLastModification(path + entry->d_name);
      pairOfFiles.second.size = getFileSize(path + entry->d_name);
      vecFileAndDir.push_back(pairOfFiles);
    }
  }
  closedir(dir);
  return vecFileAndDir;
}

std::string generateTemplate(std::string &path) {
  std::string       templateStatic;
  std::ifstream     templateFile("static/index.html");
  std::ifstream     itemFile("static/list-item.html");
  std::stringstream buf, buffer;
  std::string       listItem;
  std::string       listItemTemplate;
  std::string       tmp;

  buffer << itemFile.rdbuf();
  itemFile.close();
  listItemTemplate = buffer.str();

  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > test;
  test = listFilesAndDirectories(path);

  for (size_t i = 0; i < test.size(); i++) {
    tmp = listItemTemplate;
    ft_replace(tmp, "{{LINK_HERE}}", path + test[i].second.name);
    ft_replace(tmp, "{{FILE_NAME}}", test[i].second.name);
    ft_replace(tmp, "{{LAST_MODIFIED}}", test[i].second.date);
    if (test[i].second.size == -1)
      ft_replace(tmp, "{{SIZE_OR_TYPE}}", "Dir");
    else
      ft_replace(tmp, "{{SIZE_OR_TYPE}}", std::to_string(test[i].second.size));
    listItem += tmp;
  }
  buf << templateFile.rdbuf();
  templateFile.close();
  templateStatic = buf.str();
  ft_replace(templateStatic, "{{INSERT_TITLE_HERE}}", path);
  ft_replace(templateStatic, "{{INSERT_PATH_HERE}}", path);
  ft_replace(templateStatic, "{{RANGE_OF_ITEMS}}", listItem);
  return templateStatic;
}

// Request Get
ssize_t libhttp::getFile(std::string &path, int status) {
  ssize_t fd;
  fd = open(path.c_str(), O_RDONLY);
  if (status == 200)
    return fd;

  std::ifstream     templateFile(path);
  std::stringstream tmp;
  std::string       buffer;
  tmp << templateFile.rdbuf();
  templateFile.close();
  buffer = tmp.str();
  ft_replace(buffer, "{{STATUS}}", std::to_string(status));
  int p[2];
  if (pipe(p) < 0)
    return -1;
  write(p[1], buffer.c_str(), buffer.length());
  return p[0];
}

void setHeaders(libhttp::Response &response, std::string contentType, int ContentLenght,
                int statusCode, std::string status) {
  std::string tmp;
  tmp = "HTTP/1.1 " + std::to_string(statusCode) + " " + status + "\r\n";
  response.buffer.insert(response.buffer.end(), tmp.c_str(), tmp.c_str() + tmp.length());
  tmp = "Content-Length: " + std::to_string(ContentLenght - 1) + "\r\n";
  response.buffer.insert(response.buffer.end(), tmp.c_str(), tmp.c_str() + tmp.length());
  tmp = "Content-Type: " + contentType + "\r\n\r\n";
  response.buffer.insert(response.buffer.end(), tmp.c_str(), tmp.c_str() + tmp.length());
}

std::vector<char > generateHeaders(int statusCode, std::string status) {
  std::string        tmp;
  std::vector<char > headers;

  tmp = "HTTP/1.1 " + std::to_string(statusCode) + " " + status + "\r\n";
  headers.insert(headers.end(), tmp.c_str(), tmp.c_str() + tmp.length());
  return headers;
}

libhttp::Response setResponse(libhttp::Response &response, int fd, size_t bytesToServe,
                              std::vector<char > buffer) {
  response.fd = fd;
  response.bytesToServe = bytesToServe;
  response.buffer = buffer;
  return response;
}

off_t advanceOffSet(int fd, size_t start) { return lseek(fd, start, SEEK_SET); }

std::pair<libhttp::Methods::error, libhttp::Response > libhttp::Get(libhttp::Request &request,
                                                                    std::string       path) {
  libhttp::Response response;

  if (!findResource(path))
    return std::make_pair(libhttp::Methods::FILE_NOT_FOUND, response);
  if (!isFolder(path)) {
    response.fd = getFile(path, 200);
    if (response.fd == -1)
      return std::make_pair(libhttp::Methods::FORBIDDEN, response);

    if (checkRangeRequest(request.headers))
      setRange(response,
               getStartandEndRangeRequest(request.headers[libhttp::Headers::CONTENT_RANGE]));
    else
      response.bytesToServe = getFileSize(path);
    setHeaders(response, libparse::getTypeFile(libparse::Types(), path), response.bytesToServe, 200,
               "OK");

    return std::make_pair(libhttp::Methods::OK, response);
  }

  std::string templateStatic;
  int         p[2];

  if (pipe(p) < 0)
    return std::make_pair(libhttp::Methods::FORBIDDEN, response);
  templateStatic = generateTemplate(path);
  write(p[1], templateStatic.c_str(), templateStatic.length());
  response.fd = p[0];

  setHeaders(response, "text/html", templateStatic.length(), 200, "OK");
  return std::make_pair(libhttp::Methods::OK, response);
}

std::pair<libhttp::Methods::error, libhttp::Response> libhttp::Delete(std::string path) {
  libhttp::Response response;

  if (findResource(path)) {
    if (isFolder(path)) {
      if (deleteDirectory(path.c_str())) {
        return std::make_pair(libhttp::Methods::OK, setResponse(response, -1, getFileSize(path),
                                                                generateHeaders(202, "OK")));
      } else
        return std::make_pair(libhttp::Methods::FORBIDDEN,
                              setResponse(response, -1, 0, generateHeaders(403, "Forbidden")));
    } else {
      if (remove(path.c_str()) != 0)
        return std::make_pair(libhttp::Methods::FORBIDDEN,
                              setResponse(response, -1, 0, generateHeaders(403, "Forbidden")));
      else
        return std::make_pair(libhttp::Methods::OK,
                              setResponse(response, -1, 0, generateHeaders(202, "OK")));
    }
  }
  return std::make_pair(libhttp::Methods::FILE_NOT_FOUND,
                        setResponse(response, -1, 0, generateHeaders(404, "FileNotFound")));
}
