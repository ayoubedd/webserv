#pragma once
#include "../libparse/Config.hpp"
#include "utilities.hpp"

namespace  libhttp {
struct Get{
    enum t_error{
      OK, 
      FILE_NOT_FOUND,
      FORBIDDEN
    }error;
  };
  std::pair<Get::t_error,int> Get(std::string path);
}
