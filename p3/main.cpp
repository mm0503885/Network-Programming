
#include <iostream>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<netdb.h>
#include <unistd.h>
#include <boost/asio.hpp>

#include "server.hpp"

int main(int argc, char* argv[])
{
  
    // Check command line arguments.
    if (argc != 2)
    {
      std::cerr << "input form error\n";

      return 1;
    }

	//Get host ip
	addrinfo *addr_info = NULL;
    char hostName[100];
    char ip[30] = {0};
    gethostname(hostName, sizeof(hostName));
    int err = getaddrinfo(hostName,NULL,NULL,&addr_info);
    in_addr ipaddr =(((sockaddr_in*)(addr_info->ai_addr))->sin_addr);
    strcpy(ip,inet_ntoa(ipaddr));
  

    http::server::server s(ip, argv[1]);


    s.run();
   

  return 0;
}