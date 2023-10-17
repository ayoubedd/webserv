#include "TestParser.hpp"
#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::string getTypeFromInt(int type) {
  switch (type) {
    case 0:
      return "CURLYBARCKETLEF";
    case 1:
      return "CURLYBARCKETRIGTH";
    case 2:
      return "KEYWORD";
    case 3:
      return "ROOT";
    case 4:
      return "ROUTE";
    case 5:
      return "METHODS";
    case 6:
      return "REDIR";
    case 7:
      return "INDEX";
    case 8:
      return "ERROR";
    case 9:
      return "MAXBODYSIZE";
    case 10:
      return "DIRLISTENING";
    case 11:
      return "UPLOAD";
    case 12:
      return "CGI";
    case 13:
      return "DOMAINS";
    case 14:
      return "PORT";
    case 15:
      return "ENDFILE";
    case 16:
      return "PATH";
    default:
      return "NONO";
  }
}
void printVector(std::vector<std::string> v) {
  std::vector<std::string>::iterator it;
  it = v.begin();
  while (it != v.end()) {
    std::cout << *it << std::endl;
    it++;
  }
}

void printVectorToken(std::vector<libparse::tokens> v) {

  std::vector<libparse::tokens>::iterator it = v.begin();

  while (it != v.end()) {
    std::cout << getTypeFromInt(it->type) << " | \'" << it->lexeme << "\'" << std::endl;
    if (it->type == libparse::token::NONO)
      std::cout << "====> " << it->type << "  " << it->lexeme << std::endl;
    if (it->type == libparse::token::ROUTE)
      std::cout << "\n";
    if (it->type == libparse::token::DOMAINS)
      std::cout << "\n\n\n";
    else
      std::cout << "\t\t\t";
    it++;
  }
}

void printMap(std::map<std::string, std::string> map)
{
  std::map<std::string, std::string>::iterator it = map.begin();
  for(; it != map.end(); it++)
    std::cout <<it->first << "| " << it->second << std::endl;
  std::cout<<std::endl;
}

void printVect(std::vector<std::string> vec)
{
  for(size_t i = 0;i <vec.size(); i++)
    std::cout << vec[i] << " ";
  std::cout <<std::endl;
}
void printConfig(libparse::Config config) {
  libparse::Domains           d = config.domains;
  libparse::Domains::iterator itD;
  libparse::Routes::iterator  itR;

  std::cout << "Nbr default Sever is : " << config.defaultServer->size() << std::endl;
  itD = d.begin();
  while (itD != d.end()) {
    std::cout << "Domain: " << itD->first << std::endl;
    std::cout << "\t\t\t\terror: " << itD->second.error << std::endl;
    std::cout << "\t\t\t\tport: " << itD->second.port << std::endl;
    std::cout << "\t\t\t\tMaxSizeBody: " << itD->second.maxBodySize << std::endl;
        std::cout << "\t\t\t\tmaxHeaserSize: " << itD->second.maxHeaserSize << std::endl;
    itR = itD->second.routes.begin();
    while (itR != itD->second.routes.end()) {
      std::cout << "\t\t\t\troute: " << itR->first << std::endl;
      std::cout << "\t\t\t\troot: " << itD->second.routes[itR->first].root << std::endl;
      std::cout << "\t\t\t\tindex: " << itD->second.routes[itR->first].index << std::endl;
      std::cout << "\t\t\t\tmethods: ";
      printVect( itD->second.routes[itR->first].methods);
      std::cout << "\t\t\t\tredir: " << itD->second.routes[itR->first].redir << std::endl;
      std::cout << "\t\t\t\tdirListing: " << itD->second.routes[itR->first].dirListening
                << std::endl;
      std::cout << "\t\t\t\tupload: " << itD->second.routes[itR->first].upload << std::endl;
      std::cout << "\t\t\t\tcgi: " ;
      printMap(itD->second.routes[itR->first].cgi);
      itR++;
    }
    itD++;
  }

  std::cout << "<========================== Defautl Sever ========================>" <<std::endl;
  itD = config.defaultServer->begin();

  while (itD != config.defaultServer->end()) {
    std::cout << "Domain: " << itD->first << std::endl;
    std::cout << "\t\t\t\terror: " << itD->second.error << std::endl;
    std::cout << "\t\t\t\tport: " << itD->second.port << std::endl;
    std::cout << "\t\t\t\tMaxSizeBody: " << itD->second.maxBodySize << std::endl;
        std::cout << "\t\t\t\tmaxHeaserSize: " << itD->second.maxHeaserSize << std::endl;
    itR = itD->second.routes.begin();
    while (itR != itD->second.routes.end()) {
      std::cout << "\t\t\t\troute: " << itR->first << std::endl;
      std::cout << "\t\t\t\troot: " << itD->second.routes[itR->first].root << std::endl;
      std::cout << "\t\t\t\tindex: " << itD->second.routes[itR->first].index << std::endl;
      std::cout << "\t\t\t\tmethods: ";
      printVect( itD->second.routes[itR->first].methods);
      std::cout << "\t\t\t\tredir: " << itD->second.routes[itR->first].redir << std::endl;
      std::cout << "\t\t\t\tdirListing: " << itD->second.routes[itR->first].dirListening
                << std::endl;
      std::cout << "\t\t\t\tupload: " << itD->second.routes[itR->first].upload << std::endl;
      std::cout << "\t\t\t\tcgi: " ;
      printMap(itD->second.routes[itR->first].cgi);
      itR++;
    }
    itD++;
  }


}
