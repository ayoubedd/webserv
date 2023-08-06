#include "Config.hpp"

libparse::Domains parser(std::string filename)
{
  std::vector<tokens> tokens;
  libparse::Domains domains;
  libparse::Domains tmp;
  libparse::Routes routes;
  struct libparse::Domain domain;
  struct libparse::RouteProps routeProps ;
  std::string strDomain;
  std::string strRoute;
  std::string contentFile;
  
  contentFile =  readFile(filename);
  tokens = lexer(contentFile);

  while(tokens[0].type != token::ENDFILE)
  {
    if(tokens[0].type == token::DOMAINS)
    {
      strDomain = consumeDomain(domain,tokens);
      continue;
    }
    if(tokens[0].type == token::ROUTE)
    {
      strRoute = consumeRouteProps(routeProps, tokens);
      domain.routes[strRoute]  = routeProps;
      continue;
    }
    if(tokens[0].type == token::CURLYBARCKETLEFT)
    {
      domains[strDomain] = domain; 
      tokens.erase(tokens.begin());
      continue;
    }
    else 
    {
      consume(domain, tokens);
      continue;
    }
  }
  return domains;
}
