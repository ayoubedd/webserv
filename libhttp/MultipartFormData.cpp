#include "libhttp/MultipartFormData.hpp"
#include "libhttp/Reader.hpp"
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <regex>
#include <string>
#include <utility>
#include <vector>

typedef std::pair< libhttp::MutlipartFormDataEntity::error,
                   std::pair<std::vector<char>::const_iterator, std::vector<char>::const_iterator> >
    PartRange;
typedef std::pair<std::vector<char>::const_iterator, std::vector<char>::const_iterator> HeaderRange;

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

  if (isStringMatchVec(begin, end, del + "\r\n")) {
    startDel = del;
  } else if (isStringMatchVec(begin, end, "\r\n" + del + "\r\n")) {
    startDel = del + "\r\n";
  } else if (isStringMatchVec(begin, end, del + "--" + "\r\n")) {
    return std::make_pair(libhttp::MutlipartFormDataEntity::END, std::make_pair(end, end));
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

static std::pair<std::vector<char>::const_iterator, std::vector<char>::const_iterator>
findHeaderRange(std::vector<char>::const_iterator begin, std::vector<char>::const_iterator end) {
  bool foundColumn = false;
  std::vector<char>::const_iterator headerStart;

  // Skip CRLFs
  while (*begin == '\r' || *begin == '\n')
    begin++;

  headerStart = begin;
  while (begin != end) {
    if (*begin == ':')
      foundColumn = true;

    if (isStringMatchVec(begin, end, "\r\n"))
      break;
    begin++;
  };

  if (!foundColumn)
    return std::make_pair(end, end);

  return (std::make_pair(headerStart, begin));
};

static std::string trimString(const std::string &str, const std::string &set) {
  std::string::const_iterator trimedBegin = str.end();
  std::string::const_iterator trimedEnd = str.end();

  std::string::const_iterator begin = str.begin();
  while (begin != str.end()) {
    if (trimedBegin == str.end() && set.find(*begin) == std::string::npos)
      trimedBegin = begin;

    if (set.find(*begin) == std::string::npos)
      trimedEnd = begin;

    begin++;
  };

  return std::string(trimedBegin, trimedEnd + 1);
}

static std::pair<std::string, std::string>
extractHeaderKeyValue(std::vector<char>::const_iterator begin,
                      std::vector<char>::const_iterator end) {
  std::string key;
  std::string value;
  std::vector<char>::const_iterator beginnig;

  beginnig = begin;
  while (begin != end) {
    if (*begin != ':') {
      begin++;
      continue;
    }

    key = trimString(std::string(beginnig, begin), " ");
    value = trimString(std::string(begin + 1, end), " ");

    break;
  };

  return std::make_pair(key, value);
}

static std::vector<char> extractBody(std::vector<char>::const_iterator begin,
                                     std::vector<char>::const_iterator end) {
  std::vector<char> body;

  while (begin != end && !isStringMatchVec(begin, end, "\r\n\r\n"))
    begin++;

  if (begin >= end)
    return body;

  // Skip two CRLFs
  begin += 4;

  return std::vector(begin, end);
}

static libhttp::HeadersMap extractHeaders(std::vector<char>::const_iterator begin,
                                          std::vector<char>::const_iterator end) {
  libhttp::HeadersMap headers;
  HeaderRange headerRange;

  while (begin != end) {
    if (isStringMatchVec(begin, end, "\r\n\r\n"))
      break;

    headerRange = findHeaderRange(begin, end);

    if (headerRange.first == end || headerRange.second == end) {
      // headers.clear();
      break;
    }

    std::pair<std::string, std::string> headrPair =
        extractHeaderKeyValue(headerRange.first, headerRange.second);
    headers.insert(headrPair);

    // add two for CRLFs
    begin = headerRange.second + 2;
  }
  return headers;
}

std::vector<libhttp::MutlipartFormDataEntity>
libhttp::MutlipartFormDataEntity::decode(const std::vector<char> &src, const std::string &del) {
  std::vector<char>::const_iterator begin = src.begin();
  std::vector<char>::const_iterator end = src.end();
  std::vector<libhttp::MutlipartFormDataEntity> entities;
  libhttp::MutlipartFormDataEntity entity;
  PartRange partRange;

  while (begin != end) {
    // Find part range
    partRange = findPartRange(begin, end, del);

    // Found end boundary
    if (partRange.first == libhttp::MutlipartFormDataEntity::END)
      break;

    // in case of a malformed part clean and break
    if (partRange.first != libhttp::MutlipartFormDataEntity::OK) {
      entities.clear();
      break;
    };

    // Extracting headers out of range
    entity.headers = extractHeaders(partRange.second.first, partRange.second.second);
    // Extracting body out of range
    entity.body = extractBody(partRange.second.first, partRange.second.second);

    // Adding entity to entities vector
    entities.push_back(entity);

    // Jump by part Size
    begin = partRange.second.second;
  };

  return entities;
}
