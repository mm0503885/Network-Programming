

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <string>

namespace http {
namespace server {

struct reply
{ 
  std::string method;
  std::string uri;
  std::string querystring;
  std::string status_string; 
  std::string cgi;
  std::string http_protocol;
};
} 
} 

#endif // HTTP_SERVER_HPP