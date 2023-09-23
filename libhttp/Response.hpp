#pragma once
#include <map>
#include "libhttp/Methods.hpp"

namespace libhttp {

struct Response{
  std::map<std::string, std::string > Headers;
  std::string body;
};

}
