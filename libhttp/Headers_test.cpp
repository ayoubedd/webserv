#include "libhttp/Headers.hpp"

bool libhttpHeadersDelHeaderTest() {
  libhttp::Headers h;

  h["host"] = "main";
  h.delHeader("host");
  if (h["host"] == "main") {
    return false;
  }
  return true;
}

bool libhttpHeadersAccessTest() {
  libhttp::Headers h;

  h["host"] = "main";
  if (h["host"] != "main") {
    return false;
  }
  return true;
}
