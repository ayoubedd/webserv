#pragma once

namespace libnet {
  enum sessionState {
    READING_HEADERS,
    READING_BODY,
    READING_ERR,
    READING_FIN,

    CGI_WAIT,
    CGI_TIMEOUT,
    CGI_FIN,
    CGI_ERR,

    WRITTING_TO_CLIENT,
    WRTTING_ERR,
    FIN
  };

} // namespace libnet
