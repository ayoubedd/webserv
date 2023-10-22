#include "libhttp/Post.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/TransferEncoding.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

libhttp::Post::BodyFormat libhttp::Post::extractBodyFormat(const libhttp::HeadersMap &headers) {
  libhttp::HeadersMap::const_iterator transferEncodingIter =
      headers.find(libhttp::Headers::TRANSFER_ENCODING);
  if (transferEncodingIter != headers.end()) {
    if (transferEncodingIter->second.find("chunked") != std::string::npos)
      return libhttp::Post::CHUNKED;
  }

  libhttp::HeadersMap::const_iterator contentTypeIter =
      headers.find(libhttp::Headers::CONTENT_TYPE);
  if (contentTypeIter != headers.end()) {
    if (contentTypeIter->second.find("multipart/form-data") != std::string::npos)
      return libhttp::Post::MULTIPART_FORMDATA;
  }

  return libhttp::Post::NORMAL;
};

static libhttp::Post::HandlerError chunkedPostHandler(libhttp::Request      &req,
                                                      libhttp::ChunkDecoder &chunkDecoder,
                                                      const std::string     &uploadRoot) {
  libhttp::ChunkDecoder::ErrorStatusPair res;

  while (true) {
    res = chunkDecoder.read(req, uploadRoot);
    if (res.first == libhttp::ChunkDecoder::RERUN)
      continue;
    break;
  }

  if (res.first != libhttp::ChunkDecoder::OK) {
    // Something went wrong.
    return libhttp::Post::BAD_REQUEST;
  }

  if (res.second != libhttp::ChunkDecoder::DONE)
    return libhttp::Post::OK;

  // Upload done.

  return libhttp::Post::DONE;
}

static libhttp::Post::HandlerError normalPostHandler(libhttp::Request   &req,
                                                     libhttp::SizedPost *sizedPost,
                                                     const std::string  &uploadRoot) {

  // if SizedPost state is READY. Initialize it.
  if (sizedPost->state == libhttp::SizedPost::READY) {
    std::string path = libhttp::generateFileName(uploadRoot + req.reqTarget.path);

    libhttp::HeadersMap::iterator iter = req.headers.headers.find(libhttp::Headers::CONTENT_LENGTH);

    // Check for Content-Length header existance in headers
    if (iter == req.headers.headers.end())
      return libhttp::Post::BAD_REQUEST;

    ssize_t contentLength;
    try {
      contentLength = atoi(iter->second.c_str());
    } catch (...) {
      return libhttp::Post::BAD_REQUEST;
    }

    libhttp::SizedPost::Error err = sizedPost->init(path, contentLength);

    if (err != libhttp::SizedPost::OK)
      return libhttp::Post::ERROR_OPENING_FILE;
  }

  // Write
  std::pair<libhttp::SizedPost::Error, libhttp::SizedPost::State> ErrStatePair =
      sizedPost->write(req.body);

  // Propagating errors
  switch (ErrStatePair.first) {
    case libhttp::SizedPost::ERROR_OPENING_FILE:
      sizedPost->reset();
      return libhttp::Post::ERROR_OPENING_FILE;
    case libhttp::SizedPost::ERROR_WRITTING_TO_FILE:
      sizedPost->reset();
      return libhttp::Post::ERROR_WRITTING_TO_FILE;
    case libhttp::SizedPost::ERROR_FILE_NOT_OPEN:
      sizedPost->reset();
      return libhttp::Post::ERROR_FILE_NOT_OPEN;
    case libhttp::SizedPost::OK:
      break;
  }

  if (ErrStatePair.second == libhttp::SizedPost::DONE)
    return libhttp::Post::DONE;

  return libhttp::Post::OK;
}

static libhttp::Post::HandlerError
multipartFormDataPostHandler(libhttp::Request &req, libhttp::MultipartFormData &mpFormData,
                             const std::string &uploadRoot) {

  libhttp::MultipartFormData::ErrorStatePair res;

  while (true) {
    res = mpFormData.read(req, uploadRoot);
    if (res.first == libhttp::MultipartFormData::RERUN)
      continue;
    break;
  }

  if (res.first != libhttp::MultipartFormData::OK) {
    // Something went wrong.
    return libhttp::Post::BAD_REQUEST;
  }

  if (res.second != libhttp::MultipartFormData::DONE)
    return libhttp::Post::OK;

  // Upload done.

  return libhttp::Post::DONE;
}

static void removeNoneFileParts(std::vector<libhttp::MultipartEntity> &entities) {
  std::vector<libhttp::MultipartEntity>::iterator begin = entities.begin();
  std::vector<libhttp::MultipartEntity>::iterator end = entities.end();

  while (begin != end) {
    if (begin->type != libhttp::MultipartEntity::FILE) {
      std::remove(begin->filePath.c_str());
    }
    begin++;
  }
}

std::pair<libhttp::Status::Code, libhttp::Response *>
libhttp::Post::post(libhttp::Request &req, libhttp::TransferEncoding *te, libhttp::Multipart *mp,
                    libhttp::SizedPost *sp, const std::string &uploadRoot) {
  libhttp::Post::BodyFormat bodyFormat;
  HandlerError              err;

  bodyFormat = libhttp::Post::extractBodyFormat(req.headers.headers);

  err = libhttp::Post::OK;
  switch (bodyFormat) {
    case NORMAL:
      err = normalPostHandler(req, sp, uploadRoot);
      break;
    case CHUNKED:
      err = chunkedPostHandler(req, te->chunk.decoder, uploadRoot);
      break;
    case MULTIPART_FORMDATA:
      err = multipartFormDataPostHandler(req, mp->formData, uploadRoot);
      break;
  }

  // Error handling
  switch (err) {
    case Post::OK:
      return std::make_pair(libhttp::Status::OK, static_cast<libhttp::Response *>(NULL));

    case Post::ERROR_WRITTING_TO_FILE:
    case Post::ERROR_OPENING_FILE:
    case ERROR_FILE_NOT_OPEN:
      return std::make_pair(libhttp::Status::INTERNAL_SERVER_ERROR,
                            static_cast<libhttp::Response *>(NULL));

    case Post::BAD_REQUEST:
      return std::make_pair(libhttp::Status::BAD_REQUEST, static_cast<libhttp::Response *>(NULL));

    case Post::DONE:
      break;
  }

  // Removing non files parts.
  removeNoneFileParts(mp->formData.entities);
  mp->formData.entities.clear();

  // Success.
  // Start building response.

  libhttp::Response *res = new libhttp::Response();
  std::string        headers = "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";

  res->buffer->insert(res->buffer->begin(), headers.begin(), headers.end());

  return std::make_pair(libhttp::Status::CREATED, res);
}
