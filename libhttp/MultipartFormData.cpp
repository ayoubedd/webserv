#include "libhttp/MultipartFormData.hpp"
#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

typedef std::pair< libhttp::MutlipartFormDataEntity::error,
                   std::pair<std::vector<char>::const_iterator, std::vector<char>::const_iterator> >
    PartRange;

static bool isStringMatchVec(std::vector<char>::const_iterator begin,
                             std::vector<char>::const_iterator end, const std::string &str) {
  ssize_t i = 0;

  while (str[i] && begin != end) {
    if (str[i] != *begin)
      return false;
    begin++;
    i++;
  }
  return true;
}

static PartRange findPartRange(std::vector<char>::const_iterator begin,
                               std::vector<char>::const_iterator end, const std::string &del) {
  std::vector<char>::const_iterator partBegin = end;
  std::vector<char>::const_iterator partEnd = end;
  std::string startDel;

  // If the start dosn't match (CRLF del) | (del)
  // its a malformed multipart/form-data
  if (isStringMatchVec(begin, end, del + "\r\n")) {
    startDel = del + "\r\n";
  } else if (isStringMatchVec(begin, end, "\r\n" + del + "\r\n")) {
    startDel = "\r\n" + del + "\r\n";
  } else {
    return std::make_pair(libhttp::MutlipartFormDataEntity::MALFORMED, std::make_pair(end, end));
  }

  // Start should always match del
  partBegin = begin + startDel.length();

  // Advance iter by delimiter length
  begin = partBegin;

  // Searching for ending
  while (begin != end) {
    if (isStringMatchVec(begin, end, del + "\r\n") ||
        isStringMatchVec(begin, end, del + "--" + "\r\n")) {
      // Found end
      partEnd = begin;
      break;
    }
    begin++;
  };

  // in case of not finding end del
  // its a malformed multipart/form-data
  if (partEnd == end)
    return std::make_pair(libhttp::MutlipartFormDataEntity::MALFORMED, std::make_pair(end, end));

  // Resulting range
  return std::make_pair(libhttp::MutlipartFormDataEntity::OK, std::make_pair(partBegin, partEnd));
}

std::vector<libhttp::MutlipartFormDataEntity>
libhttp::MutlipartFormDataEntity::decode(const std::vector<char> &src, const std::string &del) {
  std::vector<char>::const_iterator begin = src.begin();
  std::vector<char>::const_iterator end = src.end();
  std::vector<libhttp::MutlipartFormDataEntity> entities;
  PartRange partRange;

  while (begin != end) {
    // Find part range
    partRange = findPartRange(begin, end, del);

    // in case of error clear and break
    // why: malformed multipart/form-data body
    if (partRange.first != libhttp::MutlipartFormDataEntity::OK) {
      entities.clear();
      break;
    };

    // TODO:
    // - part each entity header and body & build entity
    // - push entity into entities

    // Jump by part Size
    begin = partRange.second.second;
  };

  return entities;
}
