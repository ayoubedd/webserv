#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include "libhttp/Methods.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }

  libparse::Domains domains;
  libnet::Netenv net;

  libparse::parser(argv[1], domains);
  {
    std::string buffer;
    std::string path = "dev/";
    buffer = generateTemplateFiles(path);
    std::cout << buffer <<std::endl;
    exit(1);
  }
    // test simple request get
    {
       std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> res;
        libhttp::Request req;
        req.reqTarget.path = "dev/index.html";
        res = libhttp::Get(req);
        std::cout<< " status " << res.first << " fd " << res.second.fd
                << " start " << res.second.range.first << " end " << res.second.range.second 
                << std::endl;
    }
    
    {
      std::pair<libhttp::Methods::error,libhttp::Methods::GetRes> res;
        libhttp::Request req;
        req.reqTarget.path = "dev/";
        std::string index;
        res = libhttp::GetIfDir(req.reqTarget.path, index);
        std::cout<< " status " << res.first << " fd " << res.second.fd
                << " start " << res.second.range.first << " end " << res.second.range.second 
                << std::endl;
    }
    // test delelet 
    {
      int status;
      std::string path = "dev/home.html";
      status = libhttp::Deletes(path);
      std::cout<< " status " <<  status << std::endl;
    }
  return 0;
}
