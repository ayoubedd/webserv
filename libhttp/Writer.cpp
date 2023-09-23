#include "libhttp/Writer.hpp"

int libhttp::Writer::advanceOffSet(int fd, size_t start)
{
  int readSize = 0;
  char buffer[start+1];
  readSize = ::read(fd,buffer,start);
  return readSize; 
}

int libhttp::Writer::read(int fd, char *buffer,int bufferSize)
{
 return ::read(fd,buffer,bufferSize);
}
