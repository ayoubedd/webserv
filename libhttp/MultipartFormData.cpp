#include "libhttp/MultipartFormData.hpp"
#include <cstdlib>
#include <string>
#include <utility>

libhttp::MultipartFormData::MultipartFormData(const std::string &tmpDir)
    : tmpDir(tmpDir) {
  status = libhttp::MultipartFormData::READY;
};

libhttp::MultipartEntity::MultipartEntity() { type = UNKNOWN; };
void libhttp::MultipartEntity::MultipartEntity::clean() {
  type = MultipartEntity::UNKNOWN;
  headers.clear();
  prevBuffSize = 0;
};

libhttp::MultipartEntity::~MultipartEntity(){};

void libhttp::MultipartFormData::cleanup() {
  // Reset state to READY
  status = READY;

  // Clear entities vec
  entities.clear();

  // Clean entity
  entity.clean();
  entity.clean();

  // Clear delimiters
  del.clear();
  afterBodyDel.clear();
  closeDel.clear();
  file.close();
}

static std::string extractHeaderPropKeyValue(const libhttp::HeadersMap &headers,
                                             const std::string &header, const std::string &key) {
  libhttp::HeadersMap::const_iterator contentTypeIter = headers.find(header);

  if (contentTypeIter == headers.end())
    return "";

  std::string::size_type startIdx = contentTypeIter->second.find(key + "=");

  if (startIdx == std::string::npos)
    return "";

  // 9: "boundary=".len()
  startIdx += key.length();

  std::string::size_type endIdx = contentTypeIter->second.length() - 2;

  return contentTypeIter->second.substr(startIdx + 1, endIdx - 1);
}

static bool isStringMatchVec(std::vector<char>::const_iterator begin,
                             std::vector<char>::const_iterator end, const std::string &needle) {

  std::string::size_type i = 0;

  while (begin != end && i < needle.length()) {
    if (*begin != needle[i])
      return false;
    i++;
    begin++;
  }

  // Check the vec match the whole string
  if (i != needle.length())
    return false;

  return true;
}

static bool isVecContainsString(std::vector<char>::const_iterator begin,
                                std::vector<char>::const_iterator end, const std::string &str) {

  std::string::size_type i = 0;

  while (begin != end) {
    if (i == str.length())
      return true;

    if (*begin == str[i])
      i++;
    else
      i = 0;

    begin++;
  }
  return false;
}

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
  std::string                       key;
  std::string                       value;
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

static bool parsePartHeaders(libhttp::MultipartEntity &entity) {
  std::vector<char>::iterator curr = entity.buff.begin();
  std::vector<char>::iterator end = entity.buff.end();

  std::vector<char>::iterator headerBegin;

  while (curr != end) {
    headerBegin = curr;

    while (curr != end && (curr + 1) != end && (*curr != '\r' && *(curr + 1) != '\n'))
      curr++;

    std::pair<std::string, std::string> pair = extractHeaderKeyValue(headerBegin, curr);
    if (!pair.first.length() || !pair.second.length())
      return true;

    entity.headers.insert(pair);

    if (curr != end)
      curr += 2;

    if (isStringMatchVec(curr, end, "\r\n"))
      break;
  }

  entity.buff.erase(entity.buff.begin(), curr + 2);
  return false;
}

static void writeToFileTillDel(libhttp::MultipartEntity &entity, std::fstream &file,
                               const std::string &del) {
  std::vector<char>::iterator begin = entity.buff.begin();
  std::vector<char>::iterator end = entity.buff.end();

  ssize_t i = 0;
  while (begin != end) {
    if (isStringMatchVec(begin, end, del))
      break;
    begin++;
    i++;
  }

  std::cout << "writting " << i << " bytes of data to file" << std::endl;

  file.write(&entity.buff[0], i);
  entity.buff.erase(entity.buff.begin(), entity.buff.begin() + i);
}

libhttp::MultipartFormData::ErrorStatePair libhttp::MultipartFormData::read(libhttp::Request &req) {
  // Should extract the boundary at the first time.
  if (status == READY) {
    std::cout << "==> STATUS: READY" << std::endl;
    std::string boundary =
        extractHeaderPropKeyValue(req.headers.headers, "Content-Type", "boundary");

    // Check if boundary extracted
    if (!boundary.length()) {
      std::cout << "cleaning" << std::endl;
      cleanup();
      return std::make_pair(libhttp::MultipartFormData::CANNOT_EXTRACT_BOUNRAY, status);
    }

    del = "--" + boundary + "\r\n";
    afterBodyDel = "\r\n" + del;
    closeDel = "\r\n--" + boundary + "--\r\n";
    commonDel = "\r\n--" + boundary;

    status = libhttp::MultipartFormData::BEFORE_DEL;
  }

  // TODO:
  // - to avoid a lot of copying of data mabye i should work on the req.body
  // and what ever left then pass to entity.buff to be used later

  // Copy data from request
  entity.buff.insert(entity.buff.end(), req.body.begin(), req.body.end());
  req.body.clear();

  switch (status) {
    case libhttp::MultipartFormData::BEFORE_DEL: {
      std::cout << "==> STATUS: BEFORE_DEL" << std::endl;

      // Checking if the raw size is less than smallest del.
      if (entity.buff.size() < del.length()) {
        return std::make_pair(libhttp::MultipartFormData::OK,
                              libhttp::MultipartFormData::BEFORE_DEL);
      }

      // Reached the end
      if (isStringMatchVec(entity.buff.begin(), entity.buff.end(), closeDel)) {
        entity.clean();
        entity.buff.clear();
        return std::make_pair(libhttp::MultipartFormData::OK, libhttp::MultipartFormData::DONE);
      }

      // Erase the delmiter if there is one
      // otherwise the request is malformed
      if (isStringMatchVec(entity.buff.begin(), entity.buff.end(), del)) {
        entity.buff.erase(entity.buff.begin(), entity.buff.begin() + del.length());
      } else if (isStringMatchVec(entity.buff.begin(), entity.buff.end(), afterBodyDel)) {
        entity.buff.erase(entity.buff.begin(), entity.buff.begin() + afterBodyDel.length());
      } else {
        cleanup();
        return std::make_pair(libhttp::MultipartFormData::MALFORMED_MUTLIPART,
                              libhttp::MultipartFormData::READY);
      }

      // Since finding the del next State would be READING_HEADERS
      status = libhttp::MultipartFormData::READING_HEADERS;

      // In case there is more bytes in the buffer
      // return RERUN to the caller to recall immediately
      if (entity.buff.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READING_HEADERS: {
      std::cout << "==> STATUS READING_HEADERS" << std::endl;

      // Checking if the buffer containers end of headers (TWO CRLFs)
      if (!isVecContainsString(entity.buff.begin(), entity.buff.end(), "\r\n\r\n"))
        return std::make_pair(libhttp::MultipartFormData::OK,
                              libhttp::MultipartFormData::READING_HEADERS);

      // Parse headers
      bool error = parsePartHeaders(entity);
      if (error) {
        std::cout << "- error parsing headres" << std::endl;
        // clear, return
      }
      // std::cout << "- headers complete" << std::endl;

      // TODO:
      // - not all parts are file parts

      // Extracting filename of the part
      std::string fileName =
          extractHeaderPropKeyValue(entity.headers, "Content-Disposition", "filename");
      fileName = fileName.substr(1, fileName.length() - 2);
      entity.filePath = tmpDir + fileName;

      // Setting the new status
      status = libhttp::MultipartFormData::READING_BODY;

      // If there is more data to be processed return RERUN to the caller
      if (entity.buff.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READING_BODY: {
      std::cout << "==> STATUS: READING_BODY" << std::endl;

      // TODO:
      // - if part not a file upload dont bother creating a file for it.

      // Checking if the file of the current part is already open
      // if not so open it.
      if (!file.is_open()) {
        file.open(entity.filePath, std::fstream::out | std::fstream::binary | std::fstream::app);

        // std::cout << "- creating file file" << std::endl;

        if (!file.is_open()) {
          cleanup();
          return std::make_pair(libhttp::MultipartFormData::ERROR_CREATING_FILE,
                                libhttp::MultipartFormData::READY);
        }
      }

      // Write all bytes till the commonDel
      writeToFileTillDel(entity, file, commonDel); // this might be dangerous

      // if not bytes left (didnt found the commonDel) return OK (file not complete yet)
      if (!entity.buff.size())
        return std::make_pair(libhttp::MultipartFormData::OK, status);

      // End of a part or reached the last multipart/form-data part
      if (isStringMatchVec(entity.buff.begin(), entity.buff.end(), closeDel) ||
          isStringMatchVec(entity.buff.begin(), entity.buff.end(), afterBodyDel)) {
        entities.push_back(entity);
        file.close();
        entity.clean();
        status = libhttp::MultipartFormData::BEFORE_DEL;
      }

      // if still some bytes to process return RERUN
      if (entity.buff.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READY: {
      std::cout << "==> STATUS: READY" << std::endl;
      return std::make_pair(libhttp::MultipartFormData::OK, libhttp::MultipartFormData::READY);
    }

    case libhttp::MultipartFormData::DONE: {
      std::cout << "==> STATUS: DONE" << std::endl;
      cleanup();
      // what if the caller called in this state with new data ??
      return std::make_pair(libhttp::MultipartFormData::OK, libhttp::MultipartFormData::READY);
    }
  }

  return std::make_pair(libhttp::MultipartFormData::OK, status);
}
