#include "libhttp/Post-Handler.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/TransferEncoding.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
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
  ERROR_FILE_NOT_OPEN,
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
    res = chunkDecoder.read(req, uploadRoot);
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

static HANDLER_ERROR normalPostHandler(libhttp::Request &req, libhttp::SizedPost &sizedPost,
                                       const std::string &uploadRoot) {

  // if SizedPost state is READY. Initialize it.
  if (sizedPost.state == libhttp::SizedPost::READY) {
    std::string path = libhttp::generateFileName(uploadRoot + req.reqTarget.path);

    libhttp::HeadersMap::iterator iter = req.headers.headers.find(libhttp::Headers::CONTENT_LENGTH);

    // Check for Content-Length header existance in headers
    if (iter == req.headers.headers.end())
      return HANDLER_ERROR::BAD_REQUEST;

    ssize_t contentLength;
    try {
      contentLength = std::stoi(iter->second);
    } catch (...) {
      return HANDLER_ERROR::BAD_REQUEST;
    }

    libhttp::SizedPost::Error err = sizedPost.init(path, contentLength);

    if (err != libhttp::SizedPost::OK)
      return HANDLER_ERROR::ERROR_OPENING_FILE;
  }

  // Write
  std::pair<libhttp::SizedPost::Error, libhttp::SizedPost::State> ErrStatePair =
      sizedPost.write(req.body);

  // Propagating errors
  switch (ErrStatePair.first) {
    case libhttp::SizedPost::ERROR_OPENING_FILE:
      sizedPost.reset();
      return HANDLER_ERROR::ERROR_OPENING_FILE;
    case libhttp::SizedPost::ERROR_WRITTING_TO_FILE:
      sizedPost.reset();
      return HANDLER_ERROR::ERROR_WRITTING_TO_FILE;
    case libhttp::SizedPost::ERROR_FILE_NOT_OPEN:
      sizedPost.reset();
      return HANDLER_ERROR::ERROR_FILE_NOT_OPEN;
    case libhttp::SizedPost::OK:
      break;
  }

  if (ErrStatePair.second == libhttp::SizedPost::DONE) {
    sizedPost.reset();
    return HANDLER_ERROR::DONE;
  }

  return HANDLER_ERROR::OK;
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

std::pair<libhttp::PostHandlerState, libhttp::Response *>
libhttp::postHandler(libhttp::Request &req, libhttp::TransferEncoding &te, libhttp::Multipart &mp,
                     libhttp::SizedPost &sp, const std::string &uploadRoot) {
  BODY_FORMAT   bodyFormat;
  HANDLER_ERROR err;

  bodyFormat = extractBodyFormat(req.headers.headers);

  switch (bodyFormat) {
    case NORMAL:
      err = normalPostHandler(req, sp, uploadRoot);
      break;
    case CHUNKED:
      err = chunkedPostHandler(req, te.chunk.decoder, uploadRoot);
      break;
    case MULTIPART_FORMDATA:
      err = multipartFormDataPostHandler(req, mp.formData, uploadRoot);
      break;
  }

  // Error handling
  switch (err) {
    case HANDLER_ERROR::OK:
      return std::make_pair(libhttp::PostHandlerState::OK, nullptr);

    case HANDLER_ERROR::ERROR_WRITTING_TO_FILE:
    case HANDLER_ERROR::ERROR_OPENING_FILE:
      return std::make_pair(libhttp::PostHandlerState::ERROR_500, nullptr);

    case HANDLER_ERROR::BAD_REQUEST:
      return std::make_pair(libhttp::PostHandlerState::ERROR_400, nullptr);

    case HANDLER_ERROR::DONE:
      break;
  }

  // Success.
  // Start building response.

  libhttp::Response *res = new libhttp::Response();
  std::string        headers = "HTTP/1.1 201 Created\r\n\r\n";

  res->buffer.insert(res->buffer.begin(), headers.begin(), headers.end());

  return std::make_pair(libhttp::PostHandlerState::DONE, res);
}
