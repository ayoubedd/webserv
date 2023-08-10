#include "Config.hpp"

libparse::Domains parser(std::vector<tokens> tokens)
{
  libparse::Domains domains;
  libparse::Domains tmp;
  libparse::Routes routes;
  struct libparse::Domain domain;
  struct libparse::RouteProps routeProps;

  std::string strDomain;
  std::string strRoute;
  std::vector<std::string> tmpSplit;

  while (tokens[0].type != token::ENDFILE)
  {
    if (tokens[0].type == token::DOMAINS)
    {
      setDefautValue(routeProps,domain);
      strDomain = consumeDomain(domain, tokens);
      continue;
    }
    if (tokens[0].type == token::ERROR || tokens[0].type == token::INDEX || tokens[0].type == token::ROOT || tokens[0].type == token::MAXBODYSIZE)
    {
      consume(domain, tokens);
      continue;
    }
    if (tokens[0].type == token::ROUTE)
    {
      consumeRoute(routeProps, domain, tokens);
      consumeOutRoute(routeProps.path, routeProps, domain, tokens);
      continue;
    }
    if (tokens[0].type == token::CURLYBARCKETLEFT)
    {
      tokens.erase(tokens.begin());
      if (tokens[0].type == token::ENDDOMAIN)
      {
        domains[strDomain] = domain;
        tokens.erase(tokens.begin());
        continue;
      }
    }
  }
  return domains;
}
