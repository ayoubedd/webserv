#pragma once

#include "libhttp/Request.hpp"
#include <cstdlib>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace libhttp {
  struct Reader {
    std::vector<char>     raw;
    int                   fd;
    Request              *req;
    unsigned int          readBuffSize;
    unsigned int          reqLineEnd, headerEnd, bodyEnd;
    std::queue<Request *> requests;
    sockaddr_in           clientAddr;

    Reader(int fd, sockaddr_in clientAddr, unsigned int readBuffSize = 8190);
    ~Reader(void);
    enum error {
      OK,
      CONN_CLOSED,
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

    error                            read();
    std::pair<error, Request::State> processReadBuffer(Request::State);
    std::pair<error, bool>           readingRequestHeaderHundler();
    std::pair<error, bool>           readingBodyHundler();

    std::pair<error, bool> processChunkedEncoding();
    std::pair<error, bool> processMultiPartFormData();
    std::pair<error, bool> processContentLength();

    void moveRawDataToRequestBody(std::vector<char>::iterator first,
                                  std::vector<char>::iterator last);
    void clearRawDataIndices();
  };

  bool TestReaderBuildRequestLine();
  bool TestReaderBuildRequestHeaders();
} // namespace libhttp

// these need to be places in some libutils module
void stdStringTrim(std::string &str, std::string del);
void stdStringTrimRight(std::string &str, std::string del);
void stdStringTrimLeft(std::string &str, std::string del);
