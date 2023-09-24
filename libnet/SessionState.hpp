#pragma once

#include <iostream>

namespace libnet {
  enum SessionState {
    READING_HEADERS,
    READING_BODY,
    READING_FIN,
    READING_ERR,

    CGI_WAIT,
    CGI_TIMEOUT,
    CGI_FIN,
    CGI_ERR,

    WRITING_HEADERS,
    WRITING_BODY,
    WRTTING_ERR,
    FIN
  };

} // namespace libnet

std::ostream &operator<<(std::ostream &os, const libnet::SessionState s);
