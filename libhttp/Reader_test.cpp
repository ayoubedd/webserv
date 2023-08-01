#include "libhttp/Reader.hpp"

// bool libhttp::TestReaderBuildRequestLine() {
//   typedef libhttp::Reader::error error;
//   libhttp::Reader reader(1337);

//   std::string msg = "GET /hello.php HTTP/1.1\r\n";
//   reader.raw.assign(msg.begin(), msg.end());
//   error err = reader.build();
//   if (err != 0) {
//     std::cout << err << std::endl;
//     return false;
//   }
//   if (reader.req.method != std::string("GET")) {
//     std::cout << reader.req.method << std::endl;
//     return false;
//   }
//   if (reader.req.path != std::string("/hello.php")) {
//     std::cout << reader.req.path << std::endl;
//     return false;
//   }
//   if (reader.req.version != std::string("HTTP/1.1")) {
//     std::cout << reader.req.version << std::endl;
//     return false;
//   }
//   return true;
// }

// bool libhttp::TestReaderBuildRequestHeaders() {
//   typedef libhttp::Reader::error error;
//   libhttp::Reader reader(1337);

//   std::string msg = "Host: www.google.com\r\n"
//                     "Agent: mozilla/firefox\r\n"
//                     "\r\n";
//   reader.raw.assign(msg.begin(), msg.end());
//   reader.current = 0;
//   reader.buildRequestHeaders();
//   std::cout << reader.req.headers.headers.size() << std::endl;
//   // if (reader.req.headers.headers.size() != 2) {
//   //   std::cout << reader.req.headers.headers.size() << std::endl;
//   //   return false;
//   // }
//   return true;
// }