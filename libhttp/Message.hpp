#pragma once

#include "libhttp/Headers.hpp"
#include <string>

namespace libhttp {
  struct RequestLine {
    std::string method;
    std::string reqTarget;
    std::string version;
    RequestLine(){};
    ~RequestLine(){};
  };
  struct StatusLine {
    std::string method;
    std::string status;
    std::string reasonPhrase;
    StatusLine(){};
    ~StatusLine(){};
  };

  struct Message {
    union {
      RequestLine req;
      RequestLine res;
    };
    Headers headers;
    std::string body;
  };
} // namespace libhttp