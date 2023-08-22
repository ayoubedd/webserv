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
calcChunkSize(std::vector<char>::const_iterator begin, std::vector<char>::const_iterator end) {
  std::vector<char>::const_iterator tmpBegin = begin;

  ssize_t chunkSize;

  while (tmpBegin != end && *tmpBegin != '\r') {
    if (!std::isdigit(*tmpBegin))
      return std::make_pair(libhttp::Chunk::INVALID_INPUT, -1);
    if ((tmpBegin + 1) != end && *(tmpBegin + 1) == '\n')
      break;
    tmpBegin++;
  }

  try {
    chunkSize = std::stoi(std::string(begin, tmpBegin));
    return std::make_pair(libhttp::Chunk::OK, chunkSize);
  } catch (...) {
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

std::vector<char> libhttp::Chunk::decode(const std::vector<char> &src) {
  std::vector<char> buff;
  std::vector<char>::const_iterator it = src.begin();
  std::pair<libhttp::Chunk::error, ssize_t> chunkSize;

  chunkSize.second = -1;
  while (it != src.end()) {
    // Calculate chunk size
    chunkSize = calcChunkSize(it, src.end());
    if (chunkSize.first != libhttp::Chunk::OK || chunkSize.second == 0)
      break;

    // Check wheter the extracted chunk size is in range
    if (chunkSize.second > src.end() - it) {
      chunkSize.first = libhttp::Chunk::INVALID_INPUT;
      break;
    }

    // Advance iterator over size line
    it = skipChunkSizeLine(it, src.end());

    // Insert data into result vec
    insertIterRangeIntoVec(buff, it, chunkSize.second);

    // Advance iterator over chunk body
    it += chunkSize.second + 2;
  }

  // Clear resulting vec in case of error
  if (chunkSize.first != libhttp::Chunk::OK)
    buff.clear();
  return buff;
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
