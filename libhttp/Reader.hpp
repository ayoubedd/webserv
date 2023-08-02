#pragma once

#include "libhttp/Request.hpp"
#include <string>
#include <vector>

namespace libhttp {
  // TODO: these need to be on a seprated file
  const char CR = '\r';
  const char LF = '\n';
  const char SP = ' ';
  const char HT = '\t';
  const char COLON = ':';
  struct Reader {
    int fd;
    std::vector<char> raw;
    std::vector<char>::size_type current;
    Request req;

    Reader(int);
    enum error {
      OK,
      EMPTY_REQ,
      REQUEST_LINE_EMPTY,
      REQUEST_MISSING_CRLF,
      REQUEST_LINE_WRONG_SP_COUNT,
      HEADER_REPEATED
    };

    std::string getRequestLineFromRawData();
    std::string getHeaderstLinesFromRawData();

    error build();
    error buildRequestLine();
    error buildRequestHeaders();
    error buildRequestBody();
  };

  bool TestReaderBuildRequestLine();
  bool TestReaderBuildRequestHeaders();
} // namespace libhttp

// these need to be places in some libutils module
void stdStringTrim(std::string &str, std::string del);
void stdStringTrimRight(std::string &str, std::string del);
void stdStringTrimLeft(std::string &str, std::string del);