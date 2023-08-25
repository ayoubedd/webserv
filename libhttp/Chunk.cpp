#include "libhttp/Chunk.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

static std::pair<libhttp::Chunk::error, ssize_t>
extractChunkSize(std::vector<char>::const_iterator begin, std::vector<char>::const_iterator end) {
  std::vector<char>::const_iterator tmpBegin = begin;

  ssize_t chunkSize;

  while (tmpBegin != end && std::isdigit(*tmpBegin))
    tmpBegin++;

  try {
    chunkSize = std::stoi(std::string(begin, tmpBegin));
    return std::make_pair(libhttp::Chunk::OK, chunkSize);
  } catch (...) {
    std::cout << "failure" << std::endl;
    return std::make_pair(libhttp::Chunk::INVALID_INPUT, -1);
  }
}

static std::vector<char>::const_iterator skipChunkSizeLine(std::vector<char>::const_iterator begin,
                                                           std::vector<char>::const_iterator end) {
  while (begin != end) {
    if (*begin == '\r' && (begin + 1) != end) {
      if (*(begin + 1) == '\n')
        return (begin + 2);
    }
    begin++;
  }
  return begin;
}

static void insertIterRangeIntoVec(std::vector<char> &dst,
                                   std::vector<char>::const_iterator srcIter, int size) {
  int i = 0;

  while (i < size) {
    dst.push_back(*srcIter);
    srcIter++;
    i++;
  }
}

static void insertStringIntoVec(std::vector<char> &vec, const std::string &str) {
  vec.insert(vec.end(), str.begin(), str.end());
}

std::pair<libhttp::Chunk::error, std::vector<char> >
libhttp::Chunk::decode(const std::vector<char> &src) {
  std::vector<char> buff;
  std::vector<char>::const_iterator it = src.begin();
  std::pair<libhttp::Chunk::error, ssize_t> chunkSizeErrPair;

  chunkSizeErrPair.second = -1;
  while (it != src.end()) {
    // Calculate chunk size
    chunkSizeErrPair = extractChunkSize(it, src.end());
    if (chunkSizeErrPair.first != libhttp::Chunk::OK) {
      buff.clear();
      return std::make_pair(libhttp::Chunk::INVALID_INPUT, buff);
    }

    if (chunkSizeErrPair.second == 0)
      break;

    // Check wheter the extracted chunk size is in range
    if (chunkSizeErrPair.second > src.end() - it) {
      buff.clear();
      return std::make_pair(libhttp::Chunk::INVALID_INPUT, buff);
    }

    // Advance iterator over size line
    it = skipChunkSizeLine(it, src.end());

    // Insert data into result vec
    insertIterRangeIntoVec(buff, it, chunkSizeErrPair.second);

    // Advance iterator over chunk body
    it += chunkSizeErrPair.second + 2;
  }

  return std::make_pair(libhttp::Chunk::OK, buff);
}

std::vector<char> libhttp::Chunk::encode(const std::vector<char> &src, ssize_t chunkSize) {
  std::vector<char>::const_iterator it = src.begin();
  std::vector<char> buff;

  while (it != src.end()) {
    // Falling back to remainning bytes instead of chunkSize
    // in case of left bytes less than chunkSize
    chunkSize = chunkSize < (src.end() - it) ? chunkSize : (src.end() - it);

    // Inserting chunk size line
    insertStringIntoVec(buff, std::to_string(chunkSize) + "\r\n");

    // Inserting chunk body
    insertIterRangeIntoVec(buff, it, chunkSize);
    insertStringIntoVec(buff, "\r\n");

    // Advance iterator by chunkSize
    it += chunkSize;
  };

  // Zero-Sized chunk
  insertStringIntoVec(buff, "0\r\n\r\n");

  return buff;
}
