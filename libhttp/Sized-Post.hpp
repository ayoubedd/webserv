#pragma once

#include <fstream>
#include <utility>
#include <vector>

namespace libhttp {
  struct SizedPost {
    SizedPost();

    enum State {
      READY,
      WRITTING,
      DONE,
    };

    enum Error {
      OK,
      ERROR_OPENING_FILE,
      ERROR_WRITTING_TO_FILE,
      ERROR_FILE_NOT_OPEN,
    };

    State        state;
    std::fstream file;
    std::string  filePath;
    ssize_t      contentLength;
    ssize_t      writtenBytes;

    std::pair<Error, State> write(std::vector<char> &);
    Error                   init(const std::string &filePath, ssize_t conentLength);
    void                    reset(void);
  };
} // namespace libhttp
