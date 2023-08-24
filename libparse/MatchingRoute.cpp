#include "Config.hpp"
#include <algorithm>

std::string libparse::matching(libparse::Domain domain, std::string location)
{
  std::vector<std::string>v;
  std::pair<std::vector<std::string>, std::vector<int> > vp;
  std::map<std::string, libparse::RouteProps> routes;
  routes = domain.routes;
  std::map<std::string, libparse::RouteProps>::iterator it = routes.begin();
  int count = 0;
  int j = 0;
  while(it != routes.end())
    v.push_back(it->first),it++;

  for(size_t i = 0; i < v.size() ; i++)
  {
    j = 0;
    count = 0;
    if(v[i].length() > location.length())
      continue;
    while(j < v[i].length() && j < location.length())
    {

      if(v[i][j] != location[j])
        break;
      else
        count++;
      j++;
    }
    vp.second.push_back(count);
    vp.first.push_back(v[i]);
  }

  int max = *std::max_element(vp.second.begin(), vp.second.end());

   for (int i = 0; i < vp.first.size(); i++) {  
     if(vp.second[i] == max)
      return vp.first[i];
  }  
  return "";
}
