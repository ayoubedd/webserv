#include "libhttp/Writer.hpp"
#include <iostream>
#include <sys/types.h>
#include <cstring>
#include <vector>

off_t libhttp::Writer::advanceOffSet(int fd, size_t start)
{
  return lseek(fd,start,SEEK_SET); 
}

libhttp::Writer::Writer(int fd, off_t offSet, int bufferSize, int sock):fd(fd),
  offSet(offSet),readBuffSize(bufferSize),sock(sock),writeBuffSize(bufferSize)
{

}

// void printh(std::vector<char> v)
// {
//   std::cout << "---------------\n";
//   for(size_t i = 0 ; i < v.size();i++)
//     std::cout << v[i] ;
//   std::cout << "---------------\n";
// }


libhttp::Writer::erorr libhttp::Writer::write()
{
  char buffer[readBuffSize];
  int sizeBufferReading = 0;
  int sizeBufferWriting = 0;

  if(!responses.front()->inReady || !this->isReady)
    return libhttp::Writer::WAITING;
  if(offSet == -1)
  { 
    offSet = advanceOffSet(this->responses.front()->fd, this->start);    
    if(offSet!= -1)
      offSet = 1;
  }
  if(this->responses.front()->headers.size() > 0)
  {
      sizeBufferWriting = send(this->sock, &responses.front()->headers.front(), responses.front()->headers.size(),0);

      if(sizeBufferWriting != (int)responses.front()->headers.size())
        responses.front()->headers.erase(responses.front()->headers.begin()+sizeBufferWriting);

      else
        responses.front()->headers.clear();
  }
  std::cout << this->responses.front()->headers.size() << std::endl;

  if(this->responses.front()->buffer.size() < this->readBuffSize)
  { 
      sizeBufferReading = ::read(this->fd,buffer,readBuffSize); 
      if(sizeBufferReading == 0 && responses.front()->buffer.size() == 0)
        return libhttp::Writer::DONE;
      if(sizeBufferReading == -1)
        return libhttp::Writer::ERORR;
  }

    this->responses.front()->buffer.insert(responses.front()->buffer.end(), buffer,buffer+sizeBufferReading);
    sizeBufferWriting = send(this->sock, &responses.front()->buffer.front(), responses.front()->buffer.size(),0);
    if(sizeBufferReading == -1)
      return libhttp::Writer::ERORR;

    if(sizeBufferWriting != sizeBufferReading)
    {
      this->responses.front()->buffer.clear();
      this->responses.front()->buffer.insert(responses.front()->buffer.end(),buffer+sizeBufferWriting,buffer+sizeBufferWriting);
    } 
  return libhttp::Writer::OK;
}

