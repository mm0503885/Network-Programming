#include <unistd.h>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <fstream>

char* host_ip[5]={NULL};
char* port[5]={NULL};
char* file[5]={NULL};
int cut_length=0;
int start=3;
int h_p_f=0;
int turn=0;
int len;






class session
  : public std::enable_shared_from_this<session>
{
public:
  session(boost::asio::ip::tcp::socket socket,char* file_name,int num)
    : socket_(std::move(socket)),
	  file_name_(file_name),
	  num_(num)
  {
  }

  void start()
  {
	file_.open(file_name_,std::ios::in);
    do_read();
  }

private:
  void do_read()
  {
	memset(data_,'\0',sizeof(data_));
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {  
		        	
		    if (!ec)
            {
			   if(length!=0)
			   {
				 std::cout <<"<script>document.getElementById('s";
				 std::cout <<num_<<"').innerHTML += '";
				 for(int i=0;i<length;i++)
			     {
					 if(data_[i]=='\n')
					   std::cout << "&NewLine;";
				     else
					   std::cout << data_[i];
				     if(data_[i]=='%')
					 {
					   std::cout <<"&NewLine;"; 
			           do_write();

					 }
			      } 
				 std::cout <<"';</script>"<<std::endl; 
			   }
			  
            do_read();
            }
        });
  }

  void do_write()
  {
    memset(write_buffer,'\0',sizeof(write_buffer));
    auto self(shared_from_this());
	file_.getline(write_buffer, sizeof(write_buffer));
	write_buffer[strlen(write_buffer)]='\n';
    boost::asio::async_write(socket_, boost::asio::buffer(write_buffer,strlen(write_buffer)),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
			 
			 if(write_buffer[0]=='e' && write_buffer[1]=='x' && write_buffer[2]=='i' && write_buffer[3]=='t')
			 {
			   socket_.close();
			 }
          }
        });
  }

  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 4096 };
  char data_[max_length];
  char write_buffer[max_length];
  char* file_name_;
  int num_;
  std::fstream file_;
};


class connecter
{
public:
  connecter(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::query q,char* file_name,int num)
    :socket_(io_service),
	 resolver_(io_service),
	 q_(q),
	 file_name_(file_name),
	 num_(num)
  {
	if(file_name_[10]=='t')
      do_resolve();

  }

private:
  void do_resolve()
  {
     resolver_.async_resolve(q_, 
	 [this](const boost::system::error_code &ec,boost::asio::ip::tcp::resolver::iterator it)
	 {
		if (!ec)
	      socket_.async_connect(*it,
	      [this](const boost::system::error_code &ec)
		  {
			 std::make_shared<session>(std::move(socket_),file_name_,num_)->start();
		  });
	 });
  }

  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ip::tcp::resolver::query q_;
  char* file_name_;
  int num_;
};

int main() {
  
  char* queryString=getenv("QUERY_STRING");
  
  for(int i=3;i<strlen(queryString);i++)
  {
	 if(queryString[i]=='&')
	 {
		if(cut_length==0)
		{
		  start=i+4;	
		  i+=3;
		}
	    else
		{
		  switch(h_p_f) {
		  case 0:
		    host_ip[turn]= new char [cut_length+1];
			strncpy(host_ip[turn],queryString+start,cut_length);
			host_ip[turn][cut_length]='\0';
			break;
		  case 1:
		    port[turn]= new char [cut_length+1];
			strncpy(port[turn],queryString+start,cut_length);
			port[turn][cut_length]='\0';
			break;
		  case 2:
		    file[turn]= new char [cut_length+1+10];
			strcpy(file[turn],"test_case/");
			strncat(file[turn],queryString+start,cut_length);
			file[turn][cut_length+10]='\0';
			break;
		  }
		  start=i+4;
		  i+=3;
		  cut_length=0;
		}
		if(h_p_f==2) {turn++; h_p_f=0;}
		else h_p_f++;
		
	    
     }
     else
     {
		cut_length++;
	 }	
     	 
   
   
  }
    file[turn]= new char [cut_length+1+10];
	strcpy(file[turn],"test_case/");
	strncat(file[turn],queryString+start,cut_length);
	file[turn][cut_length+10]='\0';
    
    std::cout<< "Content-type: text/html" << std::endl << std::endl;
	
std::cout<<"<!DOCTYPE html>"<< std::endl;
std::cout<<"<html lang=\"en\">"<< std::endl;
std::cout<<"  <head>"<< std::endl;
std::cout<<"    <meta charset=\"UTF-8\" />"<< std::endl;
std::cout<<"    <title>NP Project 3 Console</title>"<< std::endl;
std::cout<<"    <link"<< std::endl;
std::cout<<"      rel=\"stylesheet\""<< std::endl;
std::cout<<"      href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\""<< std::endl;
std::cout<<"      integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\""<< std::endl;
std::cout<<"      crossorigin=\"anonymous\""<< std::endl;
std::cout<<"    />"<< std::endl;
std::cout<<"    <link"<< std::endl;
std::cout<<"      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""<< std::endl;
std::cout<<"      rel=\"stylesheet\""<< std::endl;
std::cout<<"    />"<< std::endl;
std::cout<<"    <link"<< std::endl;
std::cout<<"      rel=\"icon\""<< std::endl;
std::cout<<"      type=\"image/png\""<< std::endl;
std::cout<<"      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\""<< std::endl;
std::cout<<"    />"<< std::endl;
std::cout<<"    <style>"<< std::endl;
std::cout<<"      * {"<< std::endl;
std::cout<<"        font-family: 'Source Code Pro', monospace;"<< std::endl;
std::cout<<"        font-size: 1rem !important;"<< std::endl;
std::cout<<"      }"<< std::endl;
std::cout<<"      body {"<< std::endl;
std::cout<<"        background-color: #212529;"<< std::endl;
std::cout<<"      }"<< std::endl;
std::cout<<"      pre {"<< std::endl;
std::cout<<"        color: #cccccc;"<< std::endl;
std::cout<<"      }"<< std::endl;
std::cout<<"      b {"<< std::endl;
std::cout<<"        color: #ffffff;"<< std::endl;
std::cout<<"      }"<< std::endl;
std::cout<<"    </style>"<< std::endl;
std::cout<<"  </head>"<< std::endl;
std::cout<<"  <body>"<< std::endl;
std::cout<<"    <table class=\"table table-dark table-bordered\">"<< std::endl;
std::cout<<"      <thead>"<< std::endl;
std::cout<<"        <tr>"<< std::endl;
std::cout<<"          <th scope=\"col\">npbsd1.cs.nctu.edu.tw:5487</th>"<< std::endl;
std::cout<<"          <th scope=\"col\">npbsd2.cs.nctu.edu.tw:5487</th>"<< std::endl;
std::cout<<"          <th scope=\"col\">npbsd3.cs.nctu.edu.tw:5487</th>"<< std::endl;
std::cout<<"          <th scope=\"col\">npbsd2.cs.nctu.edu.tw:5487</th>"<< std::endl;
std::cout<<"          <th scope=\"col\">npbsd3.cs.nctu.edu.tw:5487</th>"<< std::endl;
std::cout<<"        </tr>"<< std::endl;
std::cout<<"      </thead>"<< std::endl;
std::cout<<"      <tbody>"<< std::endl;
std::cout<<"        <tr>"<< std::endl;
std::cout<<"          <td><pre id=\"s0\" class=\"mb-0\"></pre></td>"<< std::endl;
std::cout<<"          <td><pre id=\"s1\" class=\"mb-0\"></pre></td>"<< std::endl;
std::cout<<"          <td><pre id=\"s2\" class=\"mb-0\"></pre></td>"<< std::endl;
std::cout<<"          <td><pre id=\"s3\" class=\"mb-0\"></pre></td>"<< std::endl;
std::cout<<"          <td><pre id=\"s4\" class=\"mb-0\"></pre></td>"<< std::endl;
std::cout<<"        </tr>"<< std::endl;
std::cout<<"      </tbody>"<< std::endl;
std::cout<<"    </table>"<< std::endl;
std::cout<<"  </body>"<< std::endl;
std::cout<<"</html>"<< std::endl;
    




	boost::asio::io_service ioservice;
    
    

	    boost::asio::ip::tcp::resolver::query q{host_ip[0], port[0]};
	    connecter c(ioservice, q ,file[0],0);
	    boost::asio::ip::tcp::resolver::query q1{host_ip[1], port[1]};
	    connecter c1(ioservice, q1,file[1],1);
	    boost::asio::ip::tcp::resolver::query q2{host_ip[2], port[2]};
	    connecter c2(ioservice, q2 ,file[2],2);


   
	  
	
    ioservice.run();
	
    return 0;
    
  
}
