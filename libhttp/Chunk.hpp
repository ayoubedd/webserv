#pragma once

#include "libhttp/Request.hpp"
#include <fstream>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace libhttp {
  struct ChunkDecoder {
    ChunkDecoder(void);

    enum Error {
      OK,                     // Nothing to worry about
      MALFORMED,              // The request body is malformed
      CANNOT_OPEN_FILE,       // Faillure opening the output file
      RERUN,                  // There more data left to operate on
      NO_ENOUGH_DATA,         // No enough data to complete the current action
      ERROR_WRITTING_TO_FILE, // Error Writting to output file
    };

    enum Status {
      READY,         // The decoder is ready to consume a chunked body
      CHUNK_START,   // Reader is at the start of a chunk
      READING_CHUNK, // The reader is in the process of reading a chunk
      DONE,          // The reader is done
    };

    typedef std::pair<Error, Status> ErrorStatusPair;

    Status                       status;
    std::vector<char>::size_type chunkSize;
    std::vector<char>::size_type remainingBytes;
    std::fstream                 file;
    std::string                  filePath;
    ErrorStatusPair              decode(libhttp::Request &req, const std::string &uploadRoot);
    void                         reset(libhttp::ChunkDecoder::Status = READY);
  };

  struct ChunkEncoder {
    ChunkEncoder(void);

    enum error {
      OK,
      FAILURE_WRITTING,
    };

    std::vector<char>::size_type remainingBytes;

    error encode(std::vector<char> &buff, int fd);
    void  reset(void);
  };

  struct Chunk {
    ChunkDecoder decoder;
    ChunkEncoder encoder;
  };
} // namespace libhttp
