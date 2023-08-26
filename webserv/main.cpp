// #include "libnet/Net.hpp"
// #include "libparse/Config.hpp"
// #include "libparse/utilities.hpp"

// void sessionsHandler(libnet::Netenv &net) {
//   libnet::Sessions::iterator session;
//   libnet::Sessions &readySessions = net.readyClients;

//   session = readySessions.begin();
//   while (session != readySessions.end()) {
//     session->second->reader.read(session->second->status);
//     std::cout << session->second->request << std::endl;
//     session++;
//   };
// }

// int main(int argc, char *argv[]) {
//   if (argc < 2) {
//     std::cerr << "error: Missing config file \n";
//     return 0;
//   }

//   libparse::Domains domains;
//   libnet::Netenv net;

//   libparse::parser(argv[1], domains);

//   net.setupSockets(domains);

//   while (true) {
//     net.prepFdSets();
//     net.awaitEvents();

//     if (!net.readReadySockets.empty())
//       net.acceptNewClients();

//     sessionsHandler(net);
//   };

//   return 0;
// }
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }

  libparse::Domains domains;

  libparse::parser(argv[1], domains);

  libparse::Domain domain;
  libparse::Domains::iterator it;

  it = domains.begin();
  domain = it->second;
  std::cout << libparse::matching(domain, "/dev") << std::endl;
  // std::cout << libparse::matching(domain, "/test") << std::endl;
  // std::cout << libparse::matching(domain, "/dev/test") << std::endl;
  // std::cout << libparse::matching(domain, "/") << std::endl;
  // std::cout << libparse::matching(domain, "/static/dev") << std::endl;
}
