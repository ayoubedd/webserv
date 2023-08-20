#pragma once

#include <iostream>

namespace libnet {
  enum SessionState {
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

std::ostream &operator<<(std::ostream &os, const libnet::SessionState s);
