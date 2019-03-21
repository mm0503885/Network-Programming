

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>

namespace http {
namespace server {
	
struct reply;
struct request;

class request_handler
{
public:
  request_handler(const request_handler&) = delete;
  request_handler& operator=(const request_handler&) = delete;


  explicit request_handler();
  void handle_request(const request& req, reply& rep);


};
} 
} 

#endif // HTTP_SERVER_HPP