
std::map<std::string ,std::string> directoryContents(libhttp::Get &get);
void readContent(libhttp::Get &get);
void openFile(libhttp::Get &get);
bool directoryExists(std::string &path);
bool fileExists(std::string &filename);
void generatorTemplateDirListing(libhttp::Get &get);
static void setTypeResource(libhttp::Get &get);
static bool findResource(libhttp::Get &get);

static void init(libhttp::get &get,std::string path)
{
  get.path = path; 
  get.done = false;
  get.isfile = false;
  get.isfile = false;
  get.buffersize = 1024;
  get.error = false;
  get.isalreadyinit = true;
}

libhttp::Get Get(std::string &path)
{
  
    init(get,path);
    if(!findResource(get))
    {
      // 404 not found resources
       
      get.done = true;
      get.error = true;
      get.status = 404;
      return  "";
    }
    else
    {
      if(get.isFile)
      {
        // read and return buffer 
        openFile(get);
      }
      if(get.isFolder)
      {
        // genrate template for list files and direcotry 
        generatorTemplateDirListing(get);
        return get.buffer;
      }
    }
  return "";
}

static bool findResource(libhttp::Get &get)
{
  setTypeResource(get);
  if(get.isFile)
    return fileExists(get.path);
  if(get.isFolder)
    return directoryExists(get.path);
}

static void setTypeResource(libhttp::Get &get)
{
  if(get.path[get.path.length() - 1] == '\\')
    get.isFolder = true, get.isFile = false;
  else
    get.isFile = true, get.isFolder= false;
}

std::map<std::string ,std::string> directoryContents(libhttp::Get &get)
{
  DIR* dir = opendir(get.path.c_str());
  struct dirent* entry;
  std::map<std::string , std::string> dirContents;
  
  if (!dir) {
    // set true value of error 
    get.status = 401;
    get.error = true;
    return dirContents;
    }
   while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_DIR) {
        dirContents["dir"]=entry->d_name;
      }
      else
        dirContents["file"]=entry->d_name;
   }
   return dirContents;
}

void readContent(libhttp::Get &get)
{
  // read  and ser buffer 
  char buffer[1025];
  get.contentLength = read(get.fd,buffer,get.bufferSize);
  if(get.contentLength == 0)
    get.done = true, get.status = 200;
  get.buffer = buffer;
}

void openFile(libhttp::Get &get)
{
 // open file and read set buffer 
  char buffer[1025];
   get.fd = open(get.path.c_str(), O_RDONLY) ;
  if (get.fd == -1)
	{
    get.done = true;
    get.error = true;
    get.status = 403;
	}
  else
    get.contentLength = read(get.fd,buffer,get.bufferSize);
  get.buffer = buffer;
}

bool directoryExists(std::string &path) {
  struct stat sb;

   if (stat(path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
     return true;
  return false;

}

bool fileExists(std::string &filename) {
    std::ifstream file(filename);
    if(file)
      return true;
    return false;
}

void setTypeResource(libhttp::Get &get);

void SetTypeFile(libhttp::Get &get)
{
  std::string type;
  type = get.path.substr(get.path.rfind(".") + 1, get.path.size() - get.path.rfind("."));

  if (type == "html")
		get.type = "text/html";
	else if (type == "css")
		get.type = "text/css";
	else if (type == "js")
		get.type  = "text/javascript";
	else if (type == "jpeg" || type == "jpg")
		get.type  = "image/jpeg";
	else if (type == "png")
		get.type  = "image/png";
	else if (type == "bmp")
		get.type  = "image/bmp";
	else
	get.type  = "text/plain";
}

void generatorTemplateDirListing(libhttp::Get &get)
{
  std::map<std::string , std::string> dirContents;
  std::map<std::string , std::string>::iterator it;
  std::string templateDir;
  std::string newLine,openTag, closeTag;

  openTag ="<h3>";
  closeTag = "</h3>";
  newLine = "<br>";
  
  templateDir ="<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Index</title></head><body><h1>"+get.path+"</h1>";
  for(it = dirContents.begin() ; it != dirContents.end(); it++)
  {
    if(it->first == "dir")
      templateDir+=openTag+(it->second)+"//"+ closeTag;
    else
      templateDir+=openTag+(it->second)+ closeTag;
  }
  templateDir+="</body></html>";
  get.buffer = templateDir;
  get.error = false;
  get.status = 200;
  // genert template of dirnlisting 
}
