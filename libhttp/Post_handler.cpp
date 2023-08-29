#include "libhttp/Post_handler.hpp"
#include "libhttp/Chunk.hpp"
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

  file.open(path, std::fstream::out | std::fstream::trunc | std::fstream::binary);

  if (!file.is_open())
    return false;

  file.write(&src[0], src.size());
  file.close();

  return true;
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

void libhttp::postHandler(libhttp::Request &req) {
  enum BODY_FORMAT reqBodyFormat;
  std::string      uploadRoot = "./upload";

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
    if (!writeBufferToFile(data.second, uploadRoot + req.reqTarget.path))
      std::cerr << "POST: failure writting file to fs" << std::endl;
  }

  // Regular file upload
  if (reqBodyFormat == BODY_FORMAT::NORMAL) {
    // Write data to fs
    if (!writeBufferToFile(req.body, uploadRoot + req.reqTarget.path))
      std::cerr << "POST: failure writting file to fs" << std::endl;
  }

  // multipart/form-data
  if (reqBodyFormat == BODY_FORMAT::MULTIPART) {
    std::string boundary = extractMultiPartBoundary(req.headers.headers);

    // Check if boundary has been found
    if (!boundary.length())
      return;

    // implementation goes here
  }
}
