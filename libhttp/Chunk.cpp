#include "libhttp/Chunk.hpp"
#include "libhttp/Request.hpp"
#include <sstream>
#include <sys/types.h>
#include <utility>
#include <vector>

libhttp::ChunkDecoder::ChunkDecoder(const std::string &tmpDir) {
  status = READY;
  chunkSize = 0;
  this->tmpDir = tmpDir;
}

static std::pair<libhttp::ChunkDecoder::Error, std::vector<char>::size_type>
extractChunkSize(std::vector<char> &vec) {
  std::vector<char>::const_iterator begin = vec.begin();
  std::vector<char>::const_iterator end = vec.end();
  std::vector<char>::const_iterator tmpBegin = begin;

  ssize_t     chunkSize = 0;
  std::string hexLowerCase("abcdef");
  std::string hexUpperCase("ABCDEF");

  while (tmpBegin != end &&
         (std::isdigit(*tmpBegin) || hexLowerCase.find(*tmpBegin) != std::string::npos ||
          hexUpperCase.find(*tmpBegin) != std::string::npos))
    tmpBegin++;

  if (tmpBegin == end)
    return std::make_pair(libhttp::ChunkDecoder::NO_ENOUGH_DATA, 0);

  // Checking if there is enough data
  // to check wether we reached the CRLF
  if (end - tmpBegin < 2)
    return std::make_pair(libhttp::ChunkDecoder::NO_ENOUGH_DATA, 0);

  // Checking if we reached the CRLF
  if (*tmpBegin != '\r' || *(tmpBegin + 1) != '\n')
    return std::make_pair(libhttp::ChunkDecoder::MALFORMED, 0);

  try {
    std::stringstream stream;
    stream << std::string(begin, tmpBegin);
    stream >> std::hex >> chunkSize;
    vec.erase(vec.begin(), vec.begin() + (tmpBegin - begin + 2));
    return std::make_pair(libhttp::ChunkDecoder::OK, chunkSize);
  } catch (...) {
    return std::make_pair(libhttp::ChunkDecoder::MALFORMED, 0);
  }
}

libhttp::ChunkDecoder::ErrorStatusPair libhttp::ChunkDecoder::decode(libhttp::Request &req) {
  switch (status) {
    case READY: {
      std::cout << "STATUS: READY" << std::endl;
      // Open the file with the appropriate name
      // set status to CHUNK_START

      // Extracint filename
      std::string fileName = req.reqTarget.path;
      if (fileName.length() == 0) {
        // The file name is not provided
        fileName = "/random_file_name";
      }

      filePath = tmpDir + fileName;

      std::cout << "filepath: " << filePath << std::endl;

      // Open file
      file.open(filePath, std::fstream::out | std::fstream::binary | std::fstream::trunc);

      // Check if the is opened
      if (!file.is_open()) {
        reset();
        return std::make_pair(libhttp::ChunkDecoder::CANNOT_OPEN_FILE, READY);
      }

      // Sets the status to CHUNK_START
      status = libhttp::ChunkDecoder::CHUNK_START;

      // More to operate on ?
      // return RERUN
      if (req.body.size())
        return std::make_pair(libhttp::ChunkDecoder::RERUN, status);

      return std::make_pair(libhttp::ChunkDecoder::OK, status);
    }

    case CHUNK_START: {
      std::cout << "STATUS: CHUNK_START" << std::endl;
      std::pair<libhttp::ChunkDecoder::Error, std::vector<char>::size_type> ErrChunkSzPair =
          extractChunkSize(req.body);

      // No enough data to extract the chunk size
      if (ErrChunkSzPair.first == libhttp::ChunkDecoder::NO_ENOUGH_DATA)
        return std::make_pair(libhttp::ChunkDecoder::OK, status);

      // Cannot extract chunkSize
      if (ErrChunkSzPair.first != libhttp::ChunkDecoder::OK) {
        reset();
        status = libhttp::ChunkDecoder::READY;
        return std::make_pair(ErrChunkSzPair.first, status);
      }

      // Check if we hit the zero-chunk aka END
      if (ErrChunkSzPair.second == 0) {
        reset();
        status = libhttp::ChunkDecoder::DONE;
        return std::make_pair(libhttp::ChunkDecoder::OK, status);
      }

      // Set the chunkSize and remainingBytes
      chunkSize = ErrChunkSzPair.second;
      remainingBytes = chunkSize;

      // Set the new status
      status = libhttp::ChunkDecoder::READING_CHUNK;

      // More to operate on ?
      // return RERUN
      if (req.body.size())
        return std::make_pair(libhttp::ChunkDecoder::RERUN, status);

      return std::make_pair(libhttp::ChunkDecoder::OK, status);
    }

    case READING_CHUNK: {
      std::cout << "STATUS: READING_CHUNK" << std::endl;
      // Try to read remainingBytes unitil then subtract readed bytes from remainingBytes
      // if remainingBytes reached zero then this chunk is complete
      // afterwords sets status to CHUNK_START to read the next one

      std::vector<char>::size_type bytesToWrite =
          (remainingBytes < req.body.size() ? remainingBytes : req.body.size());

      // Should check that remainingBytes greater than zero
      // rational:
      //  in some cases, it might be bytes left to check the next thing is CRLF
      //  in which case the remainingBytes is zero
      if (remainingBytes > 0) {
        file.write(&req.body[0], bytesToWrite);
        remainingBytes -= bytesToWrite;
        req.body.erase(req.body.begin(), req.body.begin() + bytesToWrite);
      }

      // If we read the bytes of the current chunk set status CHUNK_START
      // otherwise we need more bytes, aka READING_CHUNK
      if (remainingBytes == 0) {
        // Check if there is enough data
        // to check the existance of the CRLF
        if (req.body.size() < 2)
          return std::make_pair(libhttp::ChunkDecoder::OK, status);

        // If the following is not CRLF
        // its a MALFORMED request body
        if (req.body[0] != '\r' || req.body[1] != '\n') {
          reset();
          status = libhttp::ChunkDecoder::READY;
          return std::make_pair(libhttp::ChunkDecoder::MALFORMED, status);
        }

        // Erase the CRLF
        req.body.erase(req.body.begin(), req.body.begin() + 2);

        // Setting the new status
        status = libhttp::ChunkDecoder::CHUNK_START;

      } else
        status = libhttp::ChunkDecoder::READING_CHUNK;

      // More to operate on ?
      // return RERUN
      if (req.body.size())
        return std::make_pair(libhttp::ChunkDecoder::RERUN, status);

      return std::make_pair(libhttp::ChunkDecoder::OK, status);
    }

    case DONE: {
      std::cout << "STATUS: DONE" << std::endl;
      break;
    }
  }

  return std::make_pair(OK, READY);
}

void libhttp::ChunkDecoder::reset() {
  if (file.is_open())
    file.close();

  // status = READY;
  chunkSize = 0;
  remainingBytes = 0;
}
