#pragma once
#include "../libparse/Config.hpp"
#include "utilities.hpp"


namespace libhttp {

   struct Delete{
    enum t_error{
      OK, 
      FILE_NOT_FOUND,
      FORBIDDEN
    }error;
   };

  Delete::t_error Deletes(std::string &path);

}
