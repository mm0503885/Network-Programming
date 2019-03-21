
#include "connection.hpp"
#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>



namespace http {
namespace server {

connection::connection(boost::asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler)
{
}

void connection::start()
{
  do_read();
}

void connection::stop()
{
  socket_.close();
}

void connection::do_read()
{
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
          request_parser::result_type result;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.data(), buffer_.data() + bytes_transferred);

          if (result == request_parser::good)
          {
            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else
          {
            do_read();
          }
        }
        else if (ec != boost::asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

void connection::do_write()
{
  auto self(shared_from_this());

  
  boost::asio::async_write(socket_,boost::asio::buffer(reply_.status_string),
      [this, self](boost::system::error_code ec, std::size_t)
      {
        if (!ec)
        {
		  
		  pid_t p=fork();
		  if(p<0)
		    std::cerr<<"fork error"<<std::endl;
		  else if(p==0)
		  {
			 std::cerr<<reply_.status_string<<std::endl;
			 const char *method = reply_.method.c_str();
             setenv("REQUEST_METHOD",method,1);
             const char *url = reply_.uri.c_str();
             setenv("REQUEST_URI",url,1);
	         const char *Querystring = reply_.querystring.c_str();
	         setenv("QUERY_STRING",Querystring,1);
	         const char *Pro = reply_.http_protocol.c_str(); 
             setenv("SERVER_PROTOCOL",Pro,1);
			 std::string Host=boost::asio::ip::host_name();
             const char *HOST = Host.c_str();
             setenv("HTTP_HOST",HOST,1);
			 
			 boost::asio::ip::tcp::endpoint l_endpoint = socket_.local_endpoint();
             boost::asio::ip::address l_ad = l_endpoint.address();
             std::string L_ad = l_ad.to_string();
             const char *SERVER_ADDR = L_ad.c_str();
             setenv("SERVER_ADDR",SERVER_ADDR,1);
             boost::asio::ip::tcp::endpoint r_endpoint = socket_.remote_endpoint();
             boost::asio::ip::address r_ad = r_endpoint.address();
             std::string R_ad = r_ad.to_string();
             const char *REMOTE_ADDR = R_ad.c_str();
             setenv("REMOTE_ADDR",REMOTE_ADDR,1);
  
             char sport[10]={'\0'};
             sprintf(sport,"%u",l_endpoint.port());
             std::string s_port=sport;
             const char *SERVER_PORT =s_port.c_str(); 
             setenv("SERVER_PORT",SERVER_PORT,1);
  
             char rport[10]={'\0'};
             sprintf(rport,"%u",r_endpoint.port());
             std::string r_port=rport;
             const char *REMOTE_PORT =r_port.c_str(); 
             setenv("REMOTE_PORT",REMOTE_PORT,1);
  
            std::cerr<<getenv("REQUEST_METHOD")<<std::endl;
			std::cerr<<getenv("REQUEST_URI")<<std::endl;
            std::cerr<<getenv("QUERY_STRING")<<std::endl;
            std::cerr<<getenv("HTTP_HOST")<<std::endl;
			std::cerr<<getenv("SERVER_ADDR")<<std::endl;
			std::cerr<<getenv("REMOTE_ADDR")<<std::endl;
			std::cerr<<getenv("SERVER_PORT")<<std::endl;
			std::cerr<<getenv("REMOTE_PORT")<<std::endl;
       
			 char* arg[2] = {NULL};
			 arg[0]=new char[reply_.cgi.length()+1];
	         for(int i=0;i<reply_.cgi.length();i++)	
		       arg[0][i]=reply_.cgi[i]; 
			 arg[0][reply_.cgi.length()]='\0';
			  std::cerr<<arg[0]<<std::endl;
			 int fd=socket_.native_handle();
			 dup2(fd,STDIN_FILENO);	
			 dup2(fd,STDOUT_FILENO);		 
			 if(execlp(arg[0],arg[0],(char*)NULL)<0)
				 perror("error");
			 exit(0);
		  }
        }
          connection_manager_.stop(shared_from_this());
      });
}

}
}