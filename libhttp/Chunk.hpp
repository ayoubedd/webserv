#pragma once

#include <utility>
#include <vector>

namespace libhttp {
  struct Chunk {
    enum error {
      OK,
      INVALID_INPUT,
    };
    static std::vector<char> encode(const std::vector<char> &src);
    static std::vector<char> decode(const std::vector<char> &src);
  };
} // namespace libhttp
