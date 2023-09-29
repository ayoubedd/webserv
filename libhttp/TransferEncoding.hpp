#pragma once

#include "libhttp/Chunk.hpp"

namespace libhttp {
  struct TransferEncoding {
    libhttp::Chunk chunk;
  };
}; // namespace libhttp
