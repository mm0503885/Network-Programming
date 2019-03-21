
#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "reply.hpp"
#include "request.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace http {
namespace server {

request_handler::request_handler()
{
}

void request_handler::handle_request(const request& req, reply& rep)
{
  rep.method=req.method;
  rep.uri=req.uri;
  int found = req.uri.find_first_of("?");
  if(found>=0)
  {
	rep.querystring=req.uri.substr(found+1);
	rep.cgi="." + req.uri.substr(0,found);
  }
  else
  {
	rep.cgi="." + req.uri;
  }
  char version[4]={'\0'};
  sprintf(version,"%d.%d",req.http_version_major,req.http_version_minor);
  std::string s_version=version;
  const std::string http_protocol_="HTTP/" + s_version;
  const std::string after_=" 200 ok\r\n";
  rep.http_protocol=http_protocol_;
  rep.status_string.append(http_protocol_);
  rep.status_string.append(after_);
  


}

} 
} 