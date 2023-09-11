#pragma once

#include "libhttp/Request.hpp"
#include <fstream>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace libhttp {
  struct ChunkDecoder {
    ChunkDecoder(const std::string &tmpDir = "/tmp/webserv/chunk");

    enum Error {
      OK,               // Nothing to worry about
      MALFORMED,        // The request body is malformed
      CANNOT_OPEN_FILE, // Faillure opening the output file
      RERUN,            // There more data left to operate on
      NO_ENOUGH_DATA,   // No enough data to complete the current action
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
    std::string                  tmpDir;
    ErrorStatusPair              decode(libhttp::Request &req);
    void                         reset(void);
  };

} // namespace libhttp
