#include "libhttp/Post-Handler.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/TransferEncoding.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>

enum BODY_FORMAT {
  NORMAL,
  CHUNKED,
  MULTIPART_FORMDATA,
};

enum HANDLER_ERROR {
  OK,
  ERROR_WRITTING_TO_FILE,
  ERROR_OPENING_FILE,
  BAD_REQUEST,
  DONE,
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

static HANDLER_ERROR chunkedPostHandler(libhttp::Request &req, libhttp::ChunkDecoder &chunkDecoder,
                                        const std::string &uploadRoot) {
  libhttp::ChunkDecoder::ErrorStatusPair res;

  while (true) {
    libhttp::ChunkDecoder::ErrorStatusPair res = chunkDecoder.read(req, uploadRoot);
    if (res.first == libhttp::ChunkDecoder::RERUN)
      continue;
    break;
  }

  if (res.first != libhttp::ChunkDecoder::OK) {
    // Something went wrong.
    return BAD_REQUEST;
  }

  if (res.second != libhttp::ChunkDecoder::DONE)
    return OK;

  // Upload done.

  return DONE;
}

static HANDLER_ERROR normalPostHandler(libhttp::Request &req, const std::string &uploadRoot) {
  std::fstream file;

  std::string fileName = req.reqTarget.path;
  if (fileName == "/" || fileName.length() == 0)
    fileName = "/uploaded_file";

  std::string fullPath = libhttp::generateFileName(uploadRoot + fileName);
  file.open(fullPath, std::ios::out);

  if (file.is_open() == false) {
    // Error opening the file
    return ERROR_OPENING_FILE;
  }

  file.write(&req.body[0], req.body.size());

  // Error wrtting to fs
  if (file.bad() == true) {
    std::remove(fullPath.c_str());
    file.close();
    return ERROR_WRITTING_TO_FILE;
  }

  // Success
  file.close();

  return DONE;
}

static HANDLER_ERROR multipartFormDataPostHandler(libhttp::Request           &req,
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
    return BAD_REQUEST;
  }

  if (res.second != libhttp::MultipartFormData::DONE)
    return OK;

  // Upload done.

  return DONE;
}

std::pair<libhttp::PostHandlerError, libhttp::Response *>
libhttp::postHandler(libhttp::Request &req, libhttp::TransferEncoding &te, libhttp::Multipart &mp,
                     const std::string &uploadRoot) {
  BODY_FORMAT   bodyFormat;
  HANDLER_ERROR err;

  bodyFormat = extractBodyFormat(req.headers.headers);

  switch (bodyFormat) {
    case NORMAL:
      err = normalPostHandler(req, uploadRoot);
      break;
    case CHUNKED:
      err = chunkedPostHandler(req, te.chunk.decoder, uploadRoot);
      break;
    case MULTIPART_FORMDATA:
      err = multipartFormDataPostHandler(req, mp.formData, uploadRoot);
      break;
  }

  // Uploading still ongoing.
  if (err == HANDLER_ERROR::OK) {
    return std::make_pair(libhttp::PostHandlerError::OK, nullptr);
  }

  // Errors
  if (err != DONE) {
    if (err == BAD_REQUEST)
      return std::make_pair(libhttp::PostHandlerError::ERROR_400, nullptr);
    else
      return std::make_pair(libhttp::PostHandlerError::ERROR_500, nullptr);
  }

  // Success.
  // Start building response.

  libhttp::Response *res = new libhttp::Response();
  std::string        headers = "HTTP/1.1 201 Created\r\n\r\n";

  res->headers.insert(res->headers.begin(), headers.begin(), headers.end());
  res->fd = -1;
  res->inReady = true;

  return std::make_pair(libhttp::PostHandlerError::OK, res);
}
