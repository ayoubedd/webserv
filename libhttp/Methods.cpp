#include "libhttp/Methods.hpp"
#include "MultipartFormData.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/Redirect.hpp"
#include "libparse/Types.hpp"
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <utility>

const char *libhttp::Methods::GET = "GET";
const char *libhttp::Methods::DELETE = "DELETE";
const char *libhttp::Methods::POST = "POST";

bool deleteFile(const char *pathFile) {

  DIR *dir = opendir(pathFile);

  if (dir == nullptr) {
    return false;
  }
  if (remove(pathFile) != 0) {
    return false;
  }
  closedir(dir);
  return true;
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

bool findResource(std::string &path) {
  if (path.empty())
    return false;
  return (access(path.c_str(), F_OK) == 0);
}

static void ft_replace(std::string &str, const std::string &old_value,
                       const std::string &new_value) {
  size_t pos = 0;
  while ((pos = str.find(old_value, pos)) != std::string::npos) {
    str.replace(pos, old_value.length(), new_value);
    pos += new_value.length();
  }
}

size_t getFileSize(const std::string &file_path) {
  std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::ate);
  if (!file) {
    return -1;
  }
  ssize_t size = file.tellg();
  file.close();
  if (size == -1) {
    return -1;
  }
  return static_cast<size_t>(size);
}

std::string getFileLastModification(const std::string &file_path) {

  char        buffer[100];
  struct stat stats;
  struct tm  *tm;

  if (stat(file_path.c_str(), &stats) == 0) {
    tm = gmtime(&stats.st_mtime);
    strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
  }
  return std::string(buffer);
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

libhttp::Response *setResponse(libhttp::Response *response, int fd, size_t bytesToServe) {
  response->fd = fd;
  response->bytesToServe = bytesToServe;
  return response;
}

std::vector<char> *generateHeaders(int statusCode, std::string status) {
  std::string        tmp;
  std::vector<char> *headers = new std::vector<char>;

  tmp = "HTTP/1.1 " + std::to_string(statusCode) + " " + status + "\r\n";
  headers->insert(headers->end(), tmp.c_str(), tmp.c_str() + tmp.length());
  return headers;
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
  ssize_t size;

  std::vector<std::pair<libhttp::Methods::typeFile, libhttp::Methods::file> > test;
  test = listFilesAndDirectories(path);

  for (size_t i = 0; i < test.size(); i++) {
    tmp = listItemTemplate;
    if (test[i].second.size == -1)
      ft_replace(tmp, "{{LINK_HERE}}", test[i].second.name + "/");
    else
      ft_replace(tmp, "{{LINK_HERE}}", test[i].second.name);
    ft_replace(tmp, "{{LINK_HERE}}", test[i].second.name);
    ft_replace(tmp, "{{FILE_NAME}}", test[i].second.name);
    ft_replace(tmp, "{{LAST_MODIFIED}}", test[i].second.date);
    if (test[i].second.size == -1)
      ft_replace(tmp, "{{SIZE_OR_TYPE}}", "Dir");
    else {
      std::string temp;
      size = test[i].second.size;
      temp = std::to_string(size);
      if (size > 1000000) {
        size %= 1000000;
        temp = std::to_string(size) + "M";
        if (size > 1000) {
          size %= 1000;
          temp = std::to_string(size) + "G";
        }
      }
      ft_replace(tmp, "{{SIZE_OR_TYPE}}", temp);
    }
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

bool isFolder(std::string &path) {

  if (path.length() == 0)
    return false;
  if (path[path.length() - 1] == '/')
    return true;
  return false;
}

bool isFile(std::string &path) { return !(isFolder(path)); }

void initFile(libhttp::Methods::file &file, std::string fileName) {
  file.fd = open(fileName.c_str(), O_RDONLY);
  if (file.fd == -1)
    return;

  std::ifstream inFile(fileName.c_str(), std::ios::binary | std::ios::ate);
  if (!inFile.is_open()) {
    return;
  }
  ssize_t size = inFile.tellg();
  inFile.close();
  file.size = static_cast<size_t>(size);
  if (size == -1) {
    return;
  }
  file.size = static_cast<size_t>(size);
  file.date = "";
}

bool checkRangeRequest(libhttp::Headers &headers) {

  if (headers.headers.size() == 0)
    return false;

  if (headers.headers.find("Range") != headers.headers.end())
    return true;
  return false;
}

bool checkRange(libhttp::Methods::file &file, std::pair<int, int> range) {

  if (range.second < range.first) {
    return false;
  }
  if (range.second > file.size) {
    return false;
  }
  return true;
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

off_t advanceOffSet(int fd, size_t start) { return lseek(fd, start, SEEK_SET); }

bool setRange(libhttp::Response *response, std::pair<int, int> range) {
  response->bytesToServe = range.second - range.first;
  // off_t offSet;
  return (advanceOffSet(response->fd, range.first) != -1);
}

void setHeaders(libhttp::Response *response, std::string contentType, size_t ContentLenght,
                int statusCode, std::string status) {
  std::string tmp;
  tmp = "HTTP/1.1 " + std::to_string(statusCode) + " " + status + "\r\n";
  response->buffer->insert(response->buffer->end(), tmp.c_str(), tmp.c_str() + tmp.length());
  tmp = "Content-Length: " + std::to_string(ContentLenght) + "\r\n";
  response->buffer->insert(response->buffer->end(), tmp.c_str(), tmp.c_str() + tmp.length());
  tmp = "Content-Type: " + contentType + "\r\n";
  response->buffer->insert(response->buffer->end(), tmp.c_str(), tmp.c_str() + tmp.length());
  tmp = "Accept-Ranges: bytes\r\n\r\n";
  response->buffer->insert(response->buffer->end(), tmp.c_str(), tmp.c_str() + tmp.length());
}

std::pair<libhttp::Methods::error, libhttp::Response *> libhttp::Get(libhttp::Request &request,
                                                                     std::string       path) {
  libhttp::Response     *response = NULL;
  libhttp::Methods::file file;

  if (isFile(path)) {
    initFile(file, path);
    if (path.empty())
      return std::make_pair(libhttp::Methods::FORBIDDEN,
                            libhttp::redirect(request.reqTarget.path + "/"));
    if (file.fd == -1)
      return std::make_pair(libhttp::Methods::REDIR,
                            libhttp::redirect(request.reqTarget.path + "/"));
    if (checkRangeRequest(request.headers)) {

      std::pair<int, int> range = getStartandEndRangeRequest(request.headers["Range"]);
      if (!range.second)
        range.second = file.size;
      if (!checkRange(file, range))
        return std::make_pair(libhttp::Methods::OUT_RANGE, nullptr);
      response = new Response();
      response->fd = file.fd;
      setRange(response, range);
      setHeaders(response, libparse::getTypeFile(libparse::Types(), path),
                 range.second - range.first, 200, "OK");
    } else {
      response = new Response();
      setHeaders(response, libparse::getTypeFile(libparse::Types(), path), file.size, 200, "OK");
    }
    response->fd = file.fd;
    return std::make_pair(libhttp::Methods::OK, response);
  }
  std::string fileName, templateStatic;
  fileName = libhttp::generateFileName("/tmp/webserv/dir_listing");
  int fdStatic = open(fileName.c_str(), O_RDWR | O_CREAT, 0644);
  if (fdStatic == -1)
    return std::make_pair(libhttp::Methods::FORBIDDEN, nullptr);

  response = new Response();
  templateStatic = generateTemplate(path);
  write(fdStatic, templateStatic.c_str(), templateStatic.length());
  response->fd = fdStatic;
  setHeaders(response, "text/html", templateStatic.length(), 200, "OK");
  lseek(fdStatic, 0, SEEK_SET);
  std::remove(fileName.c_str());
  return std::make_pair(libhttp::Methods::OK, response);
}

std::pair<libhttp::Methods::error, libhttp::Response *> libhttp::Delete(std::string path) {
  libhttp::Response *response = new Response(nullptr);

  if (findResource(path)) {
    if (isFolder(path)) {
      if (deleteDirectory(path.c_str())) {
        response->buffer = generateHeaders(202, "OK");
        return std::make_pair(libhttp::Methods::OK, setResponse(response, -1, getFileSize(path)));
      } else {
        response->buffer = generateHeaders(403, "Forbidden");
        return std::make_pair(libhttp::Methods::FORBIDDEN, setResponse(response, -1, 0));
      }
    } else {
      if (remove(path.c_str()) != 0) {
        response->buffer = generateHeaders(403, "Forbidden");
        return std::make_pair(libhttp::Methods::FORBIDDEN, setResponse(response, -1, 0));
      } else {
        response->buffer = generateHeaders(202, "OK");
        return std::make_pair(libhttp::Methods::OK, setResponse(response, -1, 0));
      }
    }
  }
  response->buffer = generateHeaders(404, "FileNotFound");
  return std::make_pair(libhttp::Methods::FILE_NOT_FOUND, setResponse(response, -1, 0));
}
