#pragma once

#include <sys/types.h>
#include <vector>

namespace libhttp {
  struct Chunk {
    enum error {
      OK,
      INVALID_INPUT,
    };
    static std::vector<char> encode(const std::vector<char> &src, ssize_t chunkSize);
    static std::pair<libhttp::Chunk::error, std::vector<char> >
    decode(const std::vector<char> &src);
  };
} // namespace libhttp
