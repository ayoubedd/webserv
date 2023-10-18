#pragma once

namespace libhttp {
  struct Status {
    enum Code {
      DONE = -1,
      OK = 200,
      CREATED = 201,
      PARTIAL_CONTENT = 206,
      PERMANENT_REDIRECT = 308,
      BAD_REQUEST = 400,
      UNAUTHORIZED = 401,
      FORBIDDEN = 403,
      NOT_FOUND = 404,
      METHOD_NOT_ALLOWED = 405,
      REQUEST_TIMEOUT = 408,
      LENGTH_REQUIRED = 411,
      PAYLOAD_TOO_LARGE = 413,
      RANGE_NOT_SATISFIABLE = 416,
      INTERNAL_SERVER_ERROR = 500,
      NOT_IMPLEMENTED = 501,
      GATEWAY_TIMEOUT = 504,
      HTTP_VERSION_NOT_SUPPORTED = 505,
    };
  };
} // namespace libhttp
