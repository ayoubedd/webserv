#include "Config.hpp"
#include <algorithm>

std::string libparse::matching(libparse::Domain domain,std::string path)
{
  std::map<std::string, int> pairRoutes;
  std::map<std::string, libparse::RouteProps> routes = domain.routes;
  std::map<std::string, int>::iterator itpair = pairRoutes.begin();
  std::map<std::string, libparse::RouteProps>::iterator it = routes.begin();
  int count = 0;
  size_t j = 0;
  int  maxValue = 0;

  // set all location in vector of string
  while(it != routes.end())
    pairRoutes[it->first] = -1,it++;

  // set matching char between routes and path in vector of pair
  for (itpair = pairRoutes.begin(); itpair != pairRoutes.end(); itpair++) {
    j = 0;
    count = 0;
    // if length routes sup of path you must skip 
    if(itpair->first.length() > path.length())
      continue;
    while(j < itpair->first.length() && j < path.length())
    {
      if(itpair->first[j] != path[j])
        break;
      else
        count++;
      j++;
    }
    if(count != 0)
      itpair->second = count;
  }
  // find max value of maching char 
  for (itpair = pairRoutes.begin(); itpair != pairRoutes.end(); itpair++) {
        if (itpair->second > maxValue) {
            maxValue = itpair->second;
        }
    }
  // find pair of max value of maching char
  for (itpair = pairRoutes.begin(); itpair != pairRoutes.end(); itpair++) {
    if (itpair->second == maxValue) 
        return itpair->first;
  }
  return "";
}
