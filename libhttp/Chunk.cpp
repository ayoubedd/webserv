#include "libhttp/Chunk.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/Request.hpp"
#include <cstdio>
#include <sstream>
#include <sys/types.h>
#include <utility>
#include <vector>

libhttp::ChunkDecoder::ChunkDecoder(void) {
  status = READY;
  chunkSize = 0;
  remainingBytes = 0;
}

static std::pair<libhttp::ChunkDecoder::Error, std::vector<char>::size_type>
extractChunkSize(std::vector<char> &vec) {
  std::vector<char>::const_iterator begin = vec.begin();
  std::vector<char>::const_iterator end = vec.end();
  std::vector<char>::const_iterator tmpBegin = begin;

  std::vector<char>::size_type chunkSize = 0;
  std::string                  hexLowerCase("abcdef");
  std::string                  hexUpperCase("ABCDEF");

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

libhttp::ChunkDecoder::ErrorStatusPair
libhttp::ChunkDecoder::read(libhttp::Request &req, const std::string &uploadRoot) {
  switch (status) {
    case READY: {
      // Open the file with the appropriate name
      // set status to CHUNK_START

      // Extracint filename
      std::string providedFileName = req.reqTarget.path;

      if (!providedFileName.length() || providedFileName == "/")
        filePath = libhttp::generateFileName(uploadRoot + "/uploaded_file");
      else
        filePath = libhttp::generateFileName(uploadRoot + "/" + providedFileName);

      // Open file
      file.open(filePath, std::fstream::out | std::fstream::binary | std::fstream::trunc);

      // Check if the is opened
      if (!file.is_open()) {
        reset();
        return std::make_pair(libhttp::ChunkDecoder::CANNOT_OPEN_FILE, status);
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
      std::pair<libhttp::ChunkDecoder::Error, std::vector<char>::size_type> ErrChunkSzPair =
          extractChunkSize(req.body);

      // No enough data to extract the chunk size
      if (ErrChunkSzPair.first == libhttp::ChunkDecoder::NO_ENOUGH_DATA)
        return std::make_pair(libhttp::ChunkDecoder::OK, status);

      // Cannot extract chunkSize
      if (ErrChunkSzPair.first != libhttp::ChunkDecoder::OK) {
        reset();
        return std::make_pair(ErrChunkSzPair.first, status);
      }

      // Check if we hit the zero-chunk aka END
      if (ErrChunkSzPair.second == 0) {
        reset(DONE);
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
        // Write bytes
        file.write(&req.body[0], bytesToWrite);

        // Check for file writting failures
        if (file.fail()) {
          reset();
          return std::make_pair(libhttp::ChunkDecoder::ERROR_WRITTING_TO_FILE, status);
        }

        // Subtract written bytes of remainingBytes
        remainingBytes -= bytesToWrite;

        // Erase written bytes from req
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
      break;
    }
  }

  return std::make_pair(OK, READY);
}

void libhttp::ChunkDecoder::reset(libhttp::ChunkDecoder::Status newStatus) {
  if (file.is_open()) {
    file.close();
  }

  if (status != READY && newStatus != DONE) {
    std::remove(filePath.c_str());
  }

  status = newStatus;
  chunkSize = 0;
  remainingBytes = 0;
  filePath = "";
}

libhttp::ChunkEncoder::ChunkEncoder(void) {

}
