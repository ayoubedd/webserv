#include "libhttp/Methods.hpp"

std::vector<std::pair<libhttp::Methods::typeFile, std::string>> listFilesAndDirectories(std::string &path);
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

bool checkAutoindex(std::string &name)
{
  if(name[name.length() -1 ] == '\\')
    return false;
  return true;
}

std::string generateTemplateFiles(std::string &path)
{
  std::vector<std::pair<libhttp::Methods::typeFile, std::string>> pairOfTypeAndFiles;
  std::string templ;
  std::string head="<!DOCTYPE html> \
                    <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> \
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
    <title>Document</title>  </head>  <body> ";

  templ+=head;
  pairOfTypeAndFiles = listFilesAndDirectories(path);
  for(size_t i = 0; i < pairOfTypeAndFiles.size(); i++)
  {
    templ+=pairOfTypeAndFiles[i].second;
    if(pairOfTypeAndFiles[i].first == libhttp::Methods::DIR)
      templ+="\\";
    templ+="<br>";
    templ+="<br>";
  }
  templ+="</body> </html>";
  return templ;
}

std::vector<std::pair<libhttp::Methods::typeFile, std::string>> listFilesAndDirectories(std::string &path)
{
  std::pair<libhttp::Methods::typeFile , std::string> pairOfTypeAndName;
  std::vector<std::pair<libhttp::Methods::typeFile , std::string>> vecFileAndDir;
  DIR *dir;
  struct dirent *entry;

    // Open the directory
    if ((dir = opendir(path.c_str())) == NULL) {
     pairOfTypeAndName.first = libhttp::Methods::NOT_FOUND;
     pairOfTypeAndName.second = "";
      vecFileAndDir.push_back(pairOfTypeAndName);
      return vecFileAndDir;
    }
    // Read directory entries
    while ((entry = readdir(dir)) != NULL)
    {
      // Skip . and .. entries
      if (entry->d_name == "." || entry->d_name == "..") {
          continue;
      }
      // Check if it's a directory
      if (entry->d_type == DT_DIR) {
        pairOfTypeAndName.second = entry->d_name;
        pairOfTypeAndName.first = libhttp::Methods::DIR;
        vecFileAndDir.push_back(pairOfTypeAndName);
      } 
      // Check if it's a regular file
      else 
      {
        pairOfTypeAndName.second = entry->d_name;
        pairOfTypeAndName.first = libhttp::Methods::FILE;
        vecFileAndDir.push_back(pairOfTypeAndName);
      }
    }
    closedir(dir);
    return vecFileAndDir;
}

std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> GetIfDir(std::string &path,std::string &autoFile)
{
  std::pair<int , int > range;
  libhttp::Methods::GetRes getReq;
  int p[2];
  std::string buffer;

  // open file for auto index;
  if(checkAutoindex(autoFile))
  {
    getReq.fd = open(path.c_str(),O_RDONLY);
    if(getReq.fd == -1)
        return std::make_pair(libhttp::Methods::FORBIDDEN,getReq);

    return std::make_pair(libhttp::Methods::OK,getReq);
  }
  if (pipe(p) < 0)
      return std::make_pair(libhttp::Methods::FORBIDDEN,getReq);

  buffer = generateTemplateFiles(path);
  std::cout << buffer ;
  write(p[1],buffer.c_str(),buffer.length());
  getReq.fd = p[0];

  return std::make_pair(libhttp::Methods::OK,getReq);
}

// Request Get

std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> libhttp::Get(libhttp::Request &request)
{
  std::string path;
  // you must encode and decode
  path = request.reqTarget.path;
  std::pair<int , int > range;
  libhttp::Methods::GetRes getReq;

  // init sturct of get Request by -1,0,-1
  initGetRes(getReq);
  // check Resource 
  if(!findResource(path))
    return  std::make_pair(libhttp::Methods::error::FILE_NOT_FOUND,getReq);
  
  getReq.fd = open(path.c_str(), O_RDONLY);
  if (getReq.fd == -1)
    return  std::make_pair(libhttp::Methods::error::FORBIDDEN,getReq);

  // handle bytes range get request
  if(checkRangeRequest(request.headers))
  {
    range = getStartandEndRangeRequest(request.headers["Content-Range"]);
    getReq.range.first = range.first;
    getReq.range.second = range.second;
  }
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
