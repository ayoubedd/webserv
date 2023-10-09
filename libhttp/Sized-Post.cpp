#include "libhttp/Sized-Post.hpp"
#include <cstdio>
#include <utility>

libhttp::SizedPost::SizedPost() {
  // Clean up and set state to READY
  state = libhttp::SizedPost::READY;
  reset();
}

libhttp::SizedPost::Error libhttp::SizedPost::init(const std::string &filePath,
                                                   ssize_t            contentLength) {
  this->contentLength = contentLength;

  file.open(filePath, std::fstream::out);
  if (file.is_open() == false) {
    reset();
    return ERROR_OPENING_FILE;
  }

  this->filePath = filePath;

  return libhttp::SizedPost::OK;
}

std::pair<libhttp::SizedPost::Error, libhttp::SizedPost::State>
libhttp::SizedPost::write(std::vector<char> &buff) {
  if (state == READY) {
    if (file.is_open() == false) {
      reset();
      return std::make_pair(ERROR_FILE_NOT_OPEN, state);
    }

    state = WRITTING;
  }

  switch (state) {
    case READY:
      break;
    case WRITTING: {
      file.write(&buff[0], buff.size());
      writtenBytes += buff.size();
      buff.clear();

      if (file.bad() == true) {
        reset();
        return std::make_pair(ERROR_WRITTING_TO_FILE, state);
      }

      // Done wrtting
      if (contentLength == writtenBytes) {
        reset();
        return std::make_pair(OK, DONE);
      }

      break;
    }

    case DONE:
      return std::make_pair(OK, state);
  }

  return std::make_pair(OK, state);
}

void libhttp::SizedPost::reset() {
  if (file.is_open() == true)
    file.close();

  // If reseted while writting and didn't finish writting
  // should removed the file.
  if (state == WRITTING && contentLength != writtenBytes)
    std::remove(filePath.c_str());

  state = READY;
  contentLength = 0;
  writtenBytes = 0;
  filePath = "";
}
