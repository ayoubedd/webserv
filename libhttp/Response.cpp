#include "libhttp/Response.hpp"

libhttp::Response::Response() {
  fd = -1;
  bytesToServe = -1;
  readBytes = 0;
  doneReading = false;
}
