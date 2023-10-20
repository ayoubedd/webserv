#pragma once

#include <iostream>

namespace libnet {
  enum SessionState {
    READING_HEADERS,
    READING_BODY,
    READING_FIN,
    READING_ERR,

    CGI_INIT,
    CGI_WAIT,
    CGI_TIMEOUT,
    CGI_READING_HEADERS,
    CGI_READING_BODY,
    CGI_FIN,
    CGI_ERR,

    WRITTING_TO_CLIENT,
    WRTTING_ERR,
    FIN
  };

} // namespace libnet

// std::ostream &operator<<(std::ostream &os, const libnet::SessionState s);
