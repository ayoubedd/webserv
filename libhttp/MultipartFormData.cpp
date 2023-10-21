#include "libhttp/MultipartFormData.hpp"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

static std::string asStr(size_t i) {
  std::stringstream ss;

  ss << i;
  return ss.str();
}

libhttp::MultipartFormData::MultipartFormData() {
  status = libhttp::MultipartFormData::READY;
  searchedBytes = 0;
};

libhttp::MultipartEntity::MultipartEntity() { type = UNKNOWN; };

void libhttp::MultipartEntity::MultipartEntity::clean() {
  type = MultipartEntity::UNKNOWN;
  headers.clear();
  filePath = "";
};

libhttp::MultipartEntity::~MultipartEntity(){};

libhttp::MultipartFormData::~MultipartFormData() { cleanup(READY); }

void libhttp::MultipartFormData::cleanup(libhttp::MultipartFormData::Status newStatus) {
  if (newStatus != DONE && entities.size()) {
    std::vector<libhttp::MultipartEntity>::iterator entitiesIter = entities.begin();
    while (entitiesIter != entities.end()) {
      std::remove(entitiesIter->filePath.c_str());
      entitiesIter++;
    }
  }

  // Reset searchedBytes
  searchedBytes = 0;

  // Clear entities vec
  entities.clear();

  // Set status to newStatus
  status = newStatus;

  // Clean entity
  if (newStatus != DONE)
    std::remove(entity.filePath.c_str());
  entity.clean();

  // Clear delimiters
  del.clear();
  afterBodyDel.clear();
  closeDel.clear();

  // Close file
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

static bool parsePartHeaders(std::vector<char> &vec, libhttp::MultipartEntity &entity) {
  std::vector<char>::iterator curr = vec.begin();
  std::vector<char>::iterator end = vec.end();

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

  vec.erase(vec.begin(), curr + 2);
  return false;
}

static void writeToFileTillDel(std::vector<char> &vec, std::fstream &file, const std::string &del) {
  std::vector<char>::iterator begin = vec.begin();
  std::vector<char>::iterator end = vec.end();

  ssize_t i = 0;
  while (begin != end) {
    if (isStringMatchVec(begin, end, del))
      break;
    begin++;
    i++;
  }

  file.write(&vec[0], i);
  vec.erase(vec.begin(), vec.begin() + i);
}

libhttp::MultipartFormData::ErrorStatePair
libhttp::MultipartFormData::read(libhttp::Request &req, const std::string &uploadRoot) {
  // Should extract the boundary at the first time.
  if (status == READY) {
    std::string boundary =
        extractHeaderPropKeyValue(req.headers.headers, "Content-Type", "boundary");

    // Check if boundary extracted
    if (!boundary.length()) {
      cleanup(READY);
      return std::make_pair(libhttp::MultipartFormData::CANNOT_EXTRACT_BOUNRAY, status);
    }

    del = "--" + boundary + "\r\n";
    afterBodyDel = "\r\n" + del;
    closeDel = "\r\n--" + boundary + "--\r\n";
    commonDel = "\r\n--" + boundary;

    status = libhttp::MultipartFormData::BEFORE_DEL;
  }

  switch (status) {
    case libhttp::MultipartFormData::BEFORE_DEL: {
      // Checking if the raw size is less than smallest del.
      if (req.body.size() < del.length()) {
        return std::make_pair(libhttp::MultipartFormData::OK, status);
      }

      // Reached the end
      if (isStringMatchVec(req.body.begin(), req.body.end(), closeDel)) {
        cleanup(DONE);
        // Erase closing del from req.body
        req.body.erase(req.body.begin(), req.body.begin() + closeDel.length());
        return std::make_pair(libhttp::MultipartFormData::OK, status);
      }

      // Erase the delmiter if there is one
      // otherwise the request is malformed
      if (entities.size() == 0 && isStringMatchVec(req.body.begin(), req.body.end(), del)) {
        req.body.erase(req.body.begin(), req.body.begin() + del.length());
      } else if (isStringMatchVec(req.body.begin(), req.body.end(), afterBodyDel)) {
        req.body.erase(req.body.begin(), req.body.begin() + afterBodyDel.length());
      } else {
        cleanup(READY);
        return std::make_pair(libhttp::MultipartFormData::MALFORMED_MUTLIPART, status);
      }

      // Since finding the del next State would be READING_HEADERS
      status = libhttp::MultipartFormData::READING_HEADERS;

      // In case there is more bytes in the buffer
      // return RERUN to the caller to recall immediately
      if (req.body.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READING_HEADERS: {
      // Checking if the buffer contains end of headers (TWO CRLFs)
      if (!isVecContainsString(req.body.begin() + searchedBytes, req.body.end(), "\r\n\r\n")) {
        // Always subtract four bytes
        // Rational: to avoid splitting delimter
        if (req.body.size() > 4)
          searchedBytes = req.body.size() - 4;
        return std::make_pair(libhttp::MultipartFormData::OK, status);
      }

      // Resetting searchedBytes
      searchedBytes = 0;

      // Parse headers
      bool error = parsePartHeaders(req.body, entity);
      if (error) {
        cleanup(READY);
        return std::make_pair(libhttp::MultipartFormData::MALFORMED_MUTLIPART, status);
      }

      if (entity.headers.find("Content-Disposition") == entity.headers.end()) {
        cleanup(READY);
        return std::make_pair(libhttp::MultipartFormData::PART_MISSING_CONTENT_DISPOSITION, status);
      }

      // Extracting filename of the part
      std::string providedFileName =
          extractHeaderPropKeyValue(entity.headers, "Content-Disposition", "filename");
      providedFileName = providedFileName.empty()
                             ? "upload_"
                             : providedFileName.substr(1, providedFileName.length() - 2);

      if (!providedFileName.length())
        entity.filePath = libhttp::generateFileName(uploadRoot + "/uploaded_file");
      else
        entity.filePath = libhttp::generateFileName(uploadRoot + "/" + providedFileName);

      // Setting the new status
      status = libhttp::MultipartFormData::READING_BODY;

      // If there is more data to be processed return RERUN to the caller
      if (req.body.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READING_BODY: {
      // Checking if the file of the current part is already open
      // if not so open it.
      if (!file.is_open()) {
        file.open(entity.filePath.c_str(),
                  std::fstream::out | std::fstream::binary | std::fstream::app);

        // Failure Opening the file
        if (!file.is_open()) {
          cleanup(READY);
          return std::make_pair(libhttp::MultipartFormData::ERROR_CREATING_FILE, status);
        }
      }

      // Write all bytes till the commonDel
      writeToFileTillDel(req.body, file, commonDel);

      // if not bytes left (didnt found the commonDel)
      // or remaining bytes less than commonDel.len() (cannot performe comparsion)
      // return OK (file not complete yet)
      if (req.body.size() < commonDel.length())
        return std::make_pair(libhttp::MultipartFormData::OK, status);

      // End of a part or reached the last multipart/form-data part
      if (isStringMatchVec(req.body.begin(), req.body.end(), closeDel) ||
          isStringMatchVec(req.body.begin(), req.body.end(), afterBodyDel)) {
        entities.push_back(entity);
        file.close();
        entity.clean();
        status = libhttp::MultipartFormData::BEFORE_DEL;
      }

      // if still some bytes to process return RERUN
      if (req.body.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::READY: {
      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }

    case libhttp::MultipartFormData::DONE: {
      cleanup(READY);

      // More to process ? return RERUN
      if (req.body.size())
        return std::make_pair(libhttp::MultipartFormData::RERUN, status);

      return std::make_pair(libhttp::MultipartFormData::OK, status);
    }
  }

  return std::make_pair(libhttp::MultipartFormData::OK, status);
}

std::string libhttp::generateFileName(const std::string &prefix) {
  if (access(prefix.c_str(), F_OK) != 0)
    return prefix;

  std::string            filename;
  std::string            ext;
  std::string::size_type idx = prefix.rfind(".");

  if (idx != std::string::npos && idx != 0) {
    filename = prefix.substr(0, idx);
    ext = prefix.substr(idx + 1);
  } else
    filename = prefix;

  std::string            newPath;
  std::string::size_type i = 0;
  while (true) {
    newPath = filename + "_" + asStr(i) + (ext.length() > 0 ? "." : "") + ext;
    if (access((newPath).c_str(), F_OK) != 0)
      break;
    i++;
  };
  return newPath;
}
