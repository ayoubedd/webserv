#include "libhttp/Writer.hpp"
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

libhttp::Writer::Writer(int sock, int bufferSize) {
  this->sock = sock;
  this->readWriteBufferSize = bufferSize;
}

libhttp::Writer::~Writer() {
  // if (responses.empty() == true)
  //   return;

  // while (responses.empty() != true) {
  //   libhttp::Response *response = responses.front();
  //   delete response;
  //   responses.pop();
  // }
}

libhttp::Writer::erorr libhttp::Writer::write(bool permitedToRead) {
  // Check Responses queue is not empty
  if (responses.empty() == true)
    return libhttp::Writer::OK;

  libhttp::Response *response = responses.front();

  // Should read from the file only if:
  // - there is a file.
  // - we haven't exceeded the amount of bytes.
  // - the current bufferd data is not enough.
  bool shouldReadFromFd;

  shouldReadFromFd =
      permitedToRead && response->fd != -1 && // Only read if there a body (aka a fd).
      ((response->bytesToServe != -1 && response->bytesToServe > response->readBytes &&
        response->buffer->size() <
            readWriteBufferSize) || // Byte range && and readBytes is less than bytesToServe.
       (response->bytesToServe == -1 &&
        response->buffer->size() <
            readWriteBufferSize)); // A regular body, with left bytes to be read.

  if (shouldReadFromFd == true) {
    // Deciding how much bytes to read from file.
    // if the current buffer is less than a full MSS:
    //  - its a full readWriteBufferSize (it might over than full a full MSS, syscalls are much
    //  expensive than memory.).
    //  - the mount of bytes left to be read from the fd. (in case of byte range).
    ssize_t bytesToRead;

    // Satisfying a full MSS
    if (response->bytesToServe != -1)
      bytesToRead = (ssize_t)readWriteBufferSize < (response->bytesToServe - response->readBytes)
                        ? readWriteBufferSize
                        : (response->bytesToServe - response->readBytes);
    else
      bytesToRead = readWriteBufferSize;

    char    buffer[bytesToRead];
    ssize_t readBytes = ::read(response->fd, buffer, bytesToRead);

    // Reading Failure
    if (readBytes == -1)
      return libhttp::Writer::ERORR_READING_FROM_FD;

    response->readBytes += readBytes;

    if (readBytes == 0)
      response->doneReading = true;

    else
      response->buffer->insert(response->buffer->end(), buffer, buffer + readBytes);
  }

  size_t bytesToWrite = readWriteBufferSize < response->buffer->size() ? readWriteBufferSize
                                                                       : response->buffer->size();

  ssize_t writtenBytes = ::send(sock, &(*response->buffer)[0], bytesToWrite, 0);

  if (writtenBytes == -1)
    return libhttp::Writer::ERORR_WRITTING_TO_FD;

  response->buffer->erase(response->buffer->begin(), response->buffer->begin() + writtenBytes);

  // Should drop the response only if:
  // - Reached the end of file.
  // - End of range.
  bool shoudPopResponse =
      (response->fd == -1 ||                                     // There is not input
       (response->bytesToServe == response->readBytes) ||        // Range read
       (response->fd > 0 && response->doneReading == true) ||    // Input read (read returned 0)
       (response->fd == -2 && response->doneReading == true)) && // Cgi reader done
      response->buffer->size() == 0;                             // And buffer size is zero

  if (shoudPopResponse == true) {
    delete response;
    responses.pop();
  }

  return libhttp::Writer::OK;
}
