#include "libcgi/Cgi.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include <assert.h>
#include <string.h>

#define assertm(exp, msg)                                                                          \
  (exp == false ? ({                                                                               \
    std::cerr << msg << std::endl;                                                                 \
    assert(exp);                                                                                   \
  })                                                                                               \
                : assert(exp))

int main(int argc, char *argv[]) {
  sockaddr_in        client;
  libcgi::Cgi::error e;
  bzero(&client, sizeof client);
  libhttp::Request r(&client);
  libcgi::Cgi      c(&r, "/tmp/main.pl", &client);

  ///////////////////////////////////////////////////
  e = c.init("domain.com", "main.pl", "/tmp/main.pl");
  assertm(e == libcgi::Cgi::OK, "failed init");

  e = c.exec();
  assertm(e == libcgi::Cgi::OK, "failed exec");
  e = c.read();
  assertm(e == libcgi::Cgi::OK, "failed read");
  c.clean();
  return 0;
}
