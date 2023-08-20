#include "libhttp/Headers.hpp"
#include "libnet/Session.hpp"
#include <cstring>
#include <libhttp/Reader.hpp>
#include <unistd.h>
#include <utility>

int main(int argc, char *argv[]) {

  char b1[] = "GET / HTTP/1.1\r\n"
              "Host: google.com\r\n";

  char b2[] = "Content-Type: boundary=\"hello\"\r\n\r\n";
  char b3[] = "abcdef--hello--";
  std::pair<libhttp::Reader::error, libnet::SessionState> newState;
  int fd[2];
  pipe(fd);

  libnet::Session s(fd[0]);
  write(fd[1], b1, strlen(b1));

  newState = s.reader.read(s.status);
  write(fd[1], b2, strlen(b2));
  newState = s.reader.read(newState.second);
  write(fd[1], b3, strlen(b3));
  newState = s.reader.read(newState.second);
  std::cout << newState.second << std::endl;
  std::cout << "LAST: " << s.reader.raw[s.reader.bodyEnd] << std::endl;
  ;
  // std::cout << s.request << std::endl;
  return 0;
}
