#include "libnet/SessionState.hpp"

std::ostream &operator<<(std::ostream &os, libnet::SessionState s) {
  switch (s) {
    case libnet::READING_HEADERS: {
      std::cout << "state reading headers";
      break;
    }
    case libnet::READING_BODY: {
      std::cout << "state reading body";
      break;
    }
    case libnet::READING_ERR: {
      std::cout << "state reading err";
      break;
    }
    case libnet::READING_FIN: {
      std::cout << "state reading finished";
      break;
    }
  }
  return os;
}
