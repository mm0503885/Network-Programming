
#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http {
namespace server {
	
class connection_manager;

class connection
  : public std::enable_shared_from_this<connection>
{
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;

  explicit connection(boost::asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler);

  void start();
  void stop();

private:
  void do_read();
  void do_write();

  boost::asio::ip::tcp::socket socket_;

  connection_manager& connection_manager_;

  request_handler& request_handler_;

   std::array<char, 8192> buffer_;

  request request_;

  request_parser request_parser_;

  reply reply_;
  

};
  typedef std::shared_ptr<connection> connection_ptr;

} 
} 

#endif // HTTP_SERVER_HPP