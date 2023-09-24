#include "libhttp/Post-Handler.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/TransferEncoding.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

enum BODY_FORMAT {
  NORMAL,
  CHUNKED,
  MULTIPART_FORMDATA,
};

static BODY_FORMAT extractBodyFormat(const libhttp::HeadersMap &headers) {
  libhttp::HeadersMap::const_iterator transferEncodingIter = headers.find("Transfer-Encoding");
  if (transferEncodingIter != headers.end()) {
    if (transferEncodingIter->second.find("chunked") != std::string::npos)
      return CHUNKED;
  }

  libhttp::HeadersMap::const_iterator contentTypeIter = headers.find("Content-Type");
  if (contentTypeIter != headers.end()) {
    if (contentTypeIter->second.find("multipart/form-data") != std::string::npos)
      return MULTIPART_FORMDATA;
  }

  return NORMAL;
};

static void chunkedPostHandler(libhttp::Request &req, libhttp::ChunkDecoder &chunkDecoder,
                               const std::string &uploadRoot) {
  libhttp::ChunkDecoder::ErrorStatusPair res;

  while (true) {
    libhttp::ChunkDecoder::ErrorStatusPair res = chunkDecoder.read(req, uploadRoot);
    if (res.first == libhttp::ChunkDecoder::RERUN)
      continue;
    break;
  }

  if (res.second == libhttp::ChunkDecoder::DONE) {
    // Upload done.
  }
}

static void normalPostHandler(libhttp::Request &req, const std::string &uploadRoot) {
  std::fstream file;

  std::string fileName = req.reqTarget.path;
  if (fileName == "/" || fileName.length() == 0)
    fileName = "/uploaded_file";

  std::string fullPath = libhttp::generateFileName(uploadRoot + fileName);
  file.open(fullPath, std::ios::out);

  if (file.is_open() == false) {
    // Error opening the file
    std::cerr << "normalPostHandler::Error: faillure opening output file" << std::endl;
    return;
  }

  file.write(&req.body[0], req.body.size());

  // Error wrtting to fs
  if (file.bad() == true) {
    std::remove(fullPath.c_str());
    std::cerr << "normalPostHandler::Error: faillure writting buffer to file" << std::endl;
    file.close();
    return;
  }

  file.close();
  // Success
}

static void multipartFormDataPostHandler(libhttp::Request           &req,
                                         libhttp::MultipartFormData &mpFormData,
                                         const std::string          &uploadRoot) {

  libhttp::MultipartFormData::ErrorStatePair res;

  while (true) {
    res = mpFormData.read(req, uploadRoot);
    if (res.first == libhttp::MultipartFormData::RERUN)
      continue;
    break;
  }

  if (res.first != libhttp::MultipartFormData::OK) {
    // Something went wrong.
  }

  if (res.second == libhttp::MultipartFormData::DONE) {
    // Upload done.
  }
}

void libhttp::postHandler(libhttp::Request &req, libhttp::TransferEncoding &te,
                          libhttp::Multipart &mp, const std::string &uploadRoot) {
  BODY_FORMAT bodyFormat;

  bodyFormat = extractBodyFormat(req.headers.headers);

  if (bodyFormat == NORMAL) {
    normalPostHandler(req, uploadRoot);
  }

  if (bodyFormat == CHUNKED) {
    chunkedPostHandler(req, te.chunk.decoder, uploadRoot);
  }

  if (bodyFormat == MULTIPART_FORMDATA) {
    multipartFormDataPostHandler(req, mp.formData, uploadRoot);
  }
}
