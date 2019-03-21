
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http {
namespace server {

class server
{
public:
  explicit server(const  std::string& address, const  std::string& port);
  server& operator=(const server&) = delete;
  server(const server&) = delete;
  void run();

private:

  void do_accept();
   boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  connection_manager connection_manager_;
  boost::asio::ip::tcp::socket socket_;
  request_handler request_handler_;


};

} 
} 

#endif // HTTP_SERVER_HPP