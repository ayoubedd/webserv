#include "libhttp/Sized-Post.hpp"
#include <utility>

libhttp::SizedPost::SizedPost() {
  // Clean up and set state to READY
  reset();
}

libhttp::SizedPost::Error libhttp::SizedPost::init(const std::string &filePath,
                                                   ssize_t            contentLength) {
  this->contentLength = contentLength;

  file.open(filePath);
  if (file.is_open() == false) {
    reset();
    return ERROR_OPENING_FILE;
  }

  return OK;
}

std::pair<libhttp::SizedPost::Error, libhttp::SizedPost::State>
libhttp::SizedPost::write(std::vector<char> &buff) {
  switch (state) {
    case READY: {
      if (file.is_open() == false) {
        reset();
        return std::make_pair(ERROR_FILE_NOT_OPEN, state);
      }

      state = WRITTING;
    }

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
        state = DONE;
        return std::make_pair(OK, state);
      }

      break;
    }

    case DONE:
      return std::make_pair(OK, state);
  }

  return std::make_pair(OK, state);
}

void libhttp::SizedPost::reset() {
  if (file.is_open())
    file.close();

  state = READY;
  contentLength = 0;
  writtenBytes = 0;
}
