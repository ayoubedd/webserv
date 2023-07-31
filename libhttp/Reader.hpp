#pragma once

#include "libhttp/Message.hpp"
#include <string>
#include <vector>

namespace libhttp {
  // TODO: these need to be on a seprated file
  const char CR = '\r';
  const char LF = '\n';
  const char SP = ' ';
  const char HT = '\t';
  struct Reader {
    int fd;
    std::vector<char> raw;
    std::vector<char>::size_type current;
    Message msg;

    Reader(int);
    enum error {
      OK,
      EMPTY_REQ,
      MISSING_METHOD,
      WRONG_TOK_AFTER_METHOD,
      MISSING_URI,
      WRONG_TOK_AFTER_URI,
      MISSING_HTTP_V,
      WRONG_TOK_AFTER_HTTP_V
    };
    error build();
    error buildRequestLine();
    error buildRequestHeaders();
    error buildRequestBody();
  };

  bool TestReaderBuildRequestLine();
} // namespace libhttp
