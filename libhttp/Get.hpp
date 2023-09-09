namespace  libhttp {

  struct Get{
    std::string path;
    std::string location;
    std::string type;
    std::string buffer;
    size_t contentLength;
    size_t bufferSize;
    bool done;
    bool isFile;
    bool isFolder;
    size_t fd;
    size_t fileSize;
    bool error;
    bool isAlreadyInit;
    int status;
    std::string index;
  };
}

