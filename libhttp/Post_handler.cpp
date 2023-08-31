#include "libhttp/Post_handler.hpp"
#include "libhttp/Chunk.hpp"
#include "libhttp/MultipartFormData.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

enum BODY_FORMAT {
  NORMAL,
  CHUNKED,
  MULTIPART,
};

static BODY_FORMAT extractBodyFormat(const libhttp::HeadersMap &headers) {
  auto transferEncodingIter = headers.find("Transfer-Encoding");
  auto contentTypeIter = headers.find("Content-Type");

  if (transferEncodingIter != headers.end()) {
    if (transferEncodingIter->second.find("chunked") != std::string::npos)
      return CHUNKED;
  }

  if (contentTypeIter != headers.end()) {
    if (contentTypeIter->second.find("multipart/form-data") != std::string::npos)
      return MULTIPART;
  }

  return NORMAL;
};

static bool writeBufferToFile(const std::vector<char> &src, const std::string &path) {
  std::fstream file;

  // Open file
  file.open(path, std::fstream::out | std::fstream::trunc | std::fstream::binary);

  // Check if opening the file was a success or not
  if (!file.is_open())
    return true;

  // Write file to fs
  file.write(&src[0], src.size());

  // Close fd
  file.close();

  return false;
}

static std::string extractMultiPartBoundary(const libhttp::HeadersMap &headers) {
  libhttp::HeadersMap::const_iterator contentTypeIter = headers.find("Content-Type");

  // Return emtpy boundary if Content-Type header missing
  if (contentTypeIter == headers.end())
    return "";

  std::string::size_type idx = contentTypeIter->second.find("boundary=");

  // Return emtpy boundary if boundary is missing
  if (idx == std::string::npos)
    return "";

  while (contentTypeIter->second[idx] != '=')
    idx++;
  idx++;

  return std::string(contentTypeIter->second.begin() + idx, contentTypeIter->second.end());
}

static std::string extractFileNameFromPart(const libhttp::HeadersMap &headers) {
  libhttp::HeadersMap::const_iterator iter;

  iter = headers.begin();
  std::string header;
  while (iter != headers.end()) {
    if (iter->first == "Content-Disposition") {
      header = iter->second;
      break;
    }
    iter++;
  }

  std::string::size_type beginIndex = header.find("filename=");
  if (beginIndex == std::string::npos)
    return "";

  while (header[beginIndex] && header[beginIndex] != '"')
    beginIndex++;
  beginIndex++;

  std::string::size_type endIndex = beginIndex;
  while (header[endIndex] && header[endIndex] != '"')
    endIndex++;

  return header.substr(beginIndex, endIndex - beginIndex);
  // Content-Disposition: form-data; name="file"; filename="two"^M$
}

static bool isPartFileUpload(libhttp::MutlipartFormDataEntity &part) {
  libhttp::HeadersMap::const_iterator iter;

  iter = part.headers.find("Content-Disposition");
  if (iter == part.headers.end())
    return false;

  if (iter->second.find("filename=") == std::string::npos)
    return false;

  return true;
}

static void writeMultiPartFiles(std::vector<libhttp::MutlipartFormDataEntity> &parts,
                                const std::string                             &uploadRoot) {
  std::vector<libhttp::MutlipartFormDataEntity>::iterator begin = parts.begin();

  std::string fileName;
  while (begin != parts.end()) {
    // Check if part is a file
    if (!isPartFileUpload(*begin)) {
      std::cout << "its not a file upload" << std::endl;
      begin++;
      continue;
    }

    // Extracting file name
    fileName = extractFileNameFromPart(begin->headers);
    if (!fileName.length())
      fileName = "random_file_name_should_be_generated";

    // Write file to fs
    if (writeBufferToFile(begin->body, uploadRoot + fileName))
      std::cout << "failure to write body" << std::endl;

    begin++;
  }
}

void libhttp::postHandler(libhttp::Request &req, const std::string &uploadRoot) {
  enum BODY_FORMAT reqBodyFormat;

  // tell body format
  reqBodyFormat = extractBodyFormat(req.headers.headers);

  // Chunked encoding
  if (reqBodyFormat == BODY_FORMAT::CHUNKED) {
    // decoding
    std::pair<libhttp::Chunk::error, std::vector<char> > data = libhttp::Chunk::decode(req.body);

    // exit on deocding failure
    if (data.first != libhttp::Chunk::OK)
      return;

    // Write data to fs
    if (writeBufferToFile(data.second, uploadRoot + req.reqTarget.path))
      std::cerr << "POST: failure writting file to fs" << std::endl;
  }

  // Regular file upload
  if (reqBodyFormat == BODY_FORMAT::NORMAL) {
    // Write data to fs
    if (writeBufferToFile(req.body, uploadRoot + req.reqTarget.path))
      std::cerr << "POST: failure writting file to fs" << std::endl;
  }

  // multipart/form-data
  if (reqBodyFormat == BODY_FORMAT::MULTIPART) {
    std::string boundary = extractMultiPartBoundary(req.headers.headers);

    // Check if boundary has been found
    if (!boundary.length())
      return;

    auto formData = libhttp::MutlipartFormDataEntity::decode(req.body, boundary);
    if (formData.first != libhttp::MutlipartFormDataEntity::OK)
      return;

    writeMultiPartFiles(formData.second, uploadRoot);
  }
}
