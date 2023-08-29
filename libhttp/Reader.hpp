#pragma once

#include "libhttp/Request.hpp"
#include "libnet/SessionState.hpp"
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace libhttp {
  struct Reader {
    int               fd;
    Request          &req;
    std::vector<char> raw; // TODO: change the container to deque for batter performance
    unsigned int      readBuffSize;
    unsigned int      reqLineEnd, headerEnd, bodyEnd;

    Reader(int fd, Request &req, unsigned int readBuffSize = 8190);
    enum error {
      OK,
      EMPTY_REQ,
      REQUEST_LINE_EMPTY,
      REQUEST_MISSING_CRLF,
      REQUEST_LINE_WRONG_SP_COUNT,
      HEADER_REPEATED,
      REQUEST_READ_FAILED,
      CANT_DECIDE_BODY_LEN,
      CANT_FIND_BOUNDRY,
      REQUEST_BODY_ERROR
    };

    std::string getRequestLineFromRawData();
    std::string getHeaderstLinesFromRawData();

    // TODO: change these to use indices set buy read
    error build();
    error buildRequestLine();
    error buildRequestHeaders();
    error buildRequestBody();

    std::pair<error, libnet::SessionState> read(libnet::SessionState state);
    std::pair<error, libnet::SessionState> processReadBuffer(libnet::SessionState state);
    std::pair<error, bool>                 readingRequestHeaderHundler();
    std::pair<error, bool>                 readingBodyHundler();

    std::pair<error, bool> processChunkedEncoding();
    std::pair<error, bool> processMultiPartFormData();
    std::pair<error, bool> processContentLength();
  };

  bool TestReaderBuildRequestLine();
  bool TestReaderBuildRequestHeaders();
} // namespace libhttp

// these need to be places in some libutils module
void stdStringTrim(std::string &str, std::string del);
void stdStringTrimRight(std::string &str, std::string del);
void stdStringTrimLeft(std::string &str, std::string del);
