#include "libhttp/Writer.hpp"
#include <sys/types.h>
#include <cstring>

off_t libhttp::Writer::advanceOffSet(int fd, size_t start)
{
  return lseek(fd,start,SEEK_SET); 
}

libhttp::Writer::Writer(int fd, off_t offSet, int bufferSize):fd(fd),
  offSet(offSet),readBuffSize(bufferSize),writeBuffSize(bufferSize){}
libhttp::Writer::erorr libhttp::Writer::write()
{
  char buffer[readBuffSize];
  int sizeBufferReading;
  int sizeBufferWriting;

  if(!responses.front()->inReady || !this->isReady)
    return libhttp::Writer::erorr::WAITING;

  if(offSet == -1)
  { 
    offSet = advanceOffSet(this->responses.front()->fd, this->start);    
    if(offSet!= -1)
      offSet = 1;
  }
  if(this->responses.front()->headers.size() > 0)
  {
      sizeBufferWriting = send(this->fd, &responses.front()->headers, responses.front()->headers.size(),0);

      if(sizeBufferWriting != responses.front()->headers.size())
        responses.front()->headers.erase(responses.front()->headers.begin()+sizeBufferWriting);

      else
        responses.front()->headers.clear();
  }

  if(this->responses.front()->buffer.size() < readBuffSize)
  { 
      sizeBufferReading = ::read(responses.front()->fd,buffer,readBuffSize); 
      if(sizeBufferReading == 0 && responses.front()->buffer.size() == 0)
        return libhttp::Writer::erorr::DONE;
      if(sizeBufferReading == -1)
        return libhttp::Writer::erorr::ERORR;
  }

    this->responses.front()->buffer.insert(responses.front()->buffer.end(), buffer,buffer+sizeBufferReading);  
    sizeBufferWriting = send(this->fd, &responses.front()->buffer, responses.front()->buffer.size(),0);

    if(sizeBufferReading == -1)
      return libhttp::Writer::erorr::ERORR;

    if(sizeBufferWriting != sizeBufferReading)
    {
      this->responses.front()->buffer.clear();
      this->responses.front()->buffer.insert(responses.front()->buffer.end(),buffer+sizeBufferWriting,buffer+sizeBufferWriting);
    } 
  return libhttp::Writer::erorr::OK;
}



