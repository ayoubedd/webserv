#include "libhttp/Post_handler.hpp"

enum BODY_FORMAT {
  NORMAL,
  CHUNKED,
  MULTIPART,
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
      return MULTIPART;
  }

  return NORMAL;
};

void libhttp::postHandler(libhttp::Request &req, const std::string &uploadRoot) {

}
