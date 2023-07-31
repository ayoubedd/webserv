#include "libhttp/Reader.hpp"

bool libhttp::TestReaderBuildRequestLine() {
  typedef libhttp::Reader::error error;
  libhttp::Reader reader(1337);

  std::string msg = "GET /hello.php HTTP/1.1\r\n";
  reader.raw.assign(msg.begin(), msg.end());
  error err = reader.build();
  if (err != 0) {
    std::cout << err << std::endl;
    return false;
  }
  if (reader.msg.req.method != std::string("GET")) {
    std::cout << reader.msg.req.method << std::endl;
    return false;
  }
  if (reader.msg.req.reqTarget != std::string("/hello.php")) {
    std::cout << reader.msg.req.reqTarget << std::endl;
    return false;
  }
  if (reader.msg.req.version != std::string("HTTP/1.1")) {
    std::cout << reader.msg.req.version << std::endl;
    return false;
  }
  return true;
}