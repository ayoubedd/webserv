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

  // TODO:
  // - should check that the following are CRLFs

  try {
    std::stringstream stream;
    stream << std::string(begin, tmpBegin);
    stream >> std::hex >> chunkSize;
    std::cout << "there was: " << tmpBegin - begin << " characters." << std::endl;
    vec.erase(vec.begin(), vec.begin() + (tmpBegin - begin));
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

      if (req.body.size())
        return std::make_pair(libhttp::ChunkDecoder::RERUN, status);

      return std::make_pair(libhttp::ChunkDecoder::OK, status);
    }

    case CHUNK_START: {
      std::cout << "STATUS: CHUNK_START" << std::endl;
      // extracts chunkSize
      // sets remainingBytes to chunkSize
      // if successful sets status to READING_CHUNK
      // return RERUN if there is bytes to be processed

      // if chunk size is zero then sets status to DONE

      std::pair<libhttp::ChunkDecoder::Error, std::vector<char>::size_type> ErrChunkSzPair =
          extractChunkSize(req.body);

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

      // TODO:
      // - should check that the following is CRLFs

      // Erasing two CRLFs
      req.body.erase(req.body.begin(), req.body.begin() + 2); // add checks


      // Set the chunkSize and remainingBytes
      chunkSize = ErrChunkSzPair.second;
      remainingBytes = chunkSize;

      // Set the new status
      status = libhttp::ChunkDecoder::READING_CHUNK;

      // If there was more bytes to processed return RERUN to the caller
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

      file.write(&req.body[0], bytesToWrite);
      remainingBytes -= bytesToWrite;
      req.body.erase(req.body.begin(), req.body.begin() + bytesToWrite);

      // If we read the bytes of the current chunk set status CHUNK_START
      // otherwise we need more bytes, aka READING_CHUNK
      if (remainingBytes == 0) {
        // what if the req.body stoped at the last byte of the chunk
        // but we need to erase the two CRLFs to set the new status
        // what about this case

        // TODO:
        // - should check that the following are CRLFs

        req.body.erase(req.body.begin(), req.body.begin() + 2); // add checks

        status = libhttp::ChunkDecoder::CHUNK_START;
      } else
        status = libhttp::ChunkDecoder::READING_CHUNK;

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
