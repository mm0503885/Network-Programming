#include<iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include<string>
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#define BACKLOG 1000  // 最大同??接?求?
#define MAXDATASIZE 1000
#define ARGSIZE 1000
#define MAXLINE 15000

#include <iostream>
 
using namespace std;



int str_echo(int SOCK_server_fd,char S_IP[],unsigned int S_PORT)
{
  unsigned char buffer[100];
  read(SOCK_server_fd, buffer, sizeof(buffer));
  unsigned char VN = buffer[0] ;
  unsigned char CD = buffer[1] ;
  unsigned int DST_PORT = buffer[2] << 8 | buffer[3] ;
  unsigned int DST_IP = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7] ;


   
  
  unsigned int DST_IP2 = htonl(DST_IP);
  string resultIP=inet_ntoa(*(in_addr*)(&DST_IP2));
  const char *resultIP2 = resultIP.c_str();
  
  string Mode;
  if(CD==0x01) Mode="CONNECT";
  else Mode="BIND";  
  
  char permit[10];
  char permit_mode[5];
  char permit_address[20];
  char no_use[100];
  bool Accept=false;
  
  char* pch;
  fstream firewall;
  firewall.open("socks.conf");
  memset(permit,'\0',10);
  memset(permit_mode,'\0',5);
  memset(permit_address,'\0',20);
  
  while(firewall.getline(permit,10,' '))
  {
	 firewall.getline(permit_mode,5,' ');
	 //cout<<permit_mode<<endl;
	 firewall.getline(permit_address,20,'\n');
	// cout<<permit_address<<endl;
	 if(!strcmp(permit_mode,"c"))
	   if(Mode!="CONNECT")
	     continue;
	 if(!strcmp(permit_mode,"b"))
	   if(Mode!="BIND")
	     continue;
	 pch = strtok(permit_address,".");
	 int i=0;
	 unsigned char a;
	 while (pch != NULL)
    {
	 a=(unsigned char)atoi(pch);
	 if(a!=0 && a!=buffer[i+4])
	   break;
     pch = strtok (NULL,".");
	 i++;
    }      
    if(i==4) 
	{
     Accept=true;
	 break;
	}
	memset(permit,'\0',10);
	memset(permit_mode,'\0',5);
	memset(permit_address,'\0',20);
  }
  


 
  cout<<"<S_IP>	 : "<<S_IP<<endl;
  cout<<"<S_PORT> : "<<S_PORT<<endl;
  printf("<D_IP>   : %u.%u.%u.%u\n",buffer[4],buffer[5],buffer[6],buffer[7]);
  cout<<"<D_PORT> : "<<DST_PORT<<endl;
  cout<<"<Command>: "<<Mode<<endl;
  printf("<Reply>  : %s\n",(Accept)? "Accept":"Reject");

  int SOCK_client_fd;
  unsigned char package[100];
  package[1]=(Accept)? 0x5A : 0x5B;
 if(Mode=="CONNECT") 
 {
    struct sockaddr_in serv_addr;
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr =inet_addr(resultIP2);
  serv_addr.sin_port = htons(DST_PORT);
  
 

  
  if ( (SOCK_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    cerr<<"client: can't open stream socket";
  
  if (connect(SOCK_client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    cerr<<"client: can't connect to server";
  /*else
      cout<<"connect success"<<endl; */


 
  
  package[0] = 0;
  package[2] = DST_PORT / 256;
  package[3] = DST_PORT % 256;
  package[4] = DST_IP >> 24;  
    // ip = ip in SOCKS4_REQUEST for connect mode
	// ip = 0 for bind mode
  package[5] = (DST_IP >> 16) & 0xFF;
  package[6] = (DST_IP >> 8)  & 0xFF;
  package[7] = DST_IP & 0xFF;
  write(SOCK_server_fd, package, 8);
 

  
  
 }
 else
 {

	int bindfd; 
    struct sockaddr_in bind_addr;
	struct sockaddr_in true_bind_addr;
    struct sockaddr_in remote_addr1;     
	int sin_size;
	sin_size = sizeof(struct sockaddr_in);
	 
    if((bindfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creat error！");
        exit(1);
    }
 

    bzero((char *) &bind_addr ,sizeof(bind_addr));
    bind_addr.sin_family=AF_INET;
    bind_addr.sin_port=htons(INADDR_ANY);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	

    if(bind(bindfd, (struct sockaddr *)&bind_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error !");
        exit(1);
    }
	
	getsockname(bindfd,(struct sockaddr *)&true_bind_addr,(socklen_t*)&sin_size);
    if(listen(bindfd, BACKLOG) == -1) {
        perror("listen error！");
        exit(1);
    }
	
	package[0] = 0;
    package[2] = (unsigned char)(ntohs(true_bind_addr.sin_port)/256);
    package[3] = (unsigned char)(ntohs(true_bind_addr.sin_port)%256);
    package[4] = 0;
    // ip = ip in SOCKS4_REQUEST for connect mode
	// ip = 0 for bind mode
    package[5] = 0;
    package[6] = 0;
    package[7] = 0;
    write(SOCK_server_fd, package, 8);

	

      if((SOCK_client_fd = accept(bindfd, (struct sockaddr *)&remote_addr1, (socklen_t*)&sin_size)) == -1) {
        perror("accept error !");
      }

	  
	write(SOCK_server_fd, package, 8);
 }
 
 fd_set rfds;
 fd_set afds;
  int nfds;
  nfds = getdtablesize();
  FD_ZERO(&afds);
  FD_SET(SOCK_server_fd,&afds);
  FD_SET(SOCK_client_fd,&afds);
  char mid_buffer[10000];
  int num;
  int down=0;
  int up=0;
  //int a=0;
  //int b=0;
  
  
  while(1)
  {
     FD_ZERO(&rfds);
	 memcpy(&rfds, &afds,sizeof(rfds));
	 
	 if(select(nfds, &rfds , (fd_set *)0 , (fd_set *)0,(struct timeval *)0) < 0)
		   cerr<<"select error"<<endl;
	 /*else
	   cout<<"select success"<<endl;*/
	 if(FD_ISSET(SOCK_client_fd,&rfds))
	 {
	   memset(mid_buffer,'\0',10000);
	   num=read(SOCK_client_fd,mid_buffer,10000);
	   if(num==0){
		 //cout<<"down complete"<<endl;
	     FD_CLR(SOCK_client_fd,&afds);
         down=1;
		 //cout<<a<<endl;
		// a++;
	   }
	   else if(num <0)
	   {
		 FD_CLR(SOCK_client_fd,&afds);
		 down=1;
	   }
	   else
	    {

		  num=write(SOCK_server_fd,mid_buffer,num);
		}
	 }
	 if(FD_ISSET(SOCK_server_fd,&rfds))
	 {
	   memset(mid_buffer,'\0',10000);
	   num=read(SOCK_server_fd,mid_buffer,10000);
	   if(num==0){
		// cout<<"up complete"<<endl;  
	     FD_CLR(SOCK_server_fd,&afds);
		 up=1;
		 //cout<<b<<endl;
		// b++;

	   }
	   else if(num <0)
	   {
		 FD_CLR(SOCK_server_fd,&afds);
		 up=1;
	   }
	    
	   else
	   {
		 num=write(SOCK_client_fd,mid_buffer,num);

	   }
	 }
     
	 if(down==1 && up==1)
	   exit(0);
     
  }
}
 
int main(int argc, char *argv[])
{
    if(argc != 2)
	  perror("Input form error");
  
    int sock_fd,client_fd;  // sock_fd：?听socket；client_fd：?据??socket
    int sin_size;
	int childpid;
    struct sockaddr_in my_addr; // 本机地址信息
    struct sockaddr_in remote_addr; // 客?端地址信息
    
	in_port_t server_port = atoi(argv[1]);
	 
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creat error！");
        exit(1);
    }
 

    bzero((char *) &my_addr ,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(server_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
	
	int optval=1;
	setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);
    if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error !");
        exit(1);
    }
    if(listen(sock_fd, BACKLOG) == -1) {
        perror("listen error！");
        exit(1);
    }
	
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if((client_fd = accept(sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size)) == -1) {
            perror("accept error !");
            continue;
        }
		
		char S_IP[100];
		unsigned int S_PORT;
	    sprintf(S_IP,"%s",inet_ntoa(remote_addr.sin_addr));
		S_PORT=remote_addr.sin_port;
		setenv("REMOTE_ADDR",S_IP,1);
		setenv("REMOTE_HOST",S_IP,1);
        if( (childpid=fork()) < 0) 
          perror("fork error !"); 
        else if(childpid ==0)	  
		{  
   	      close(sock_fd);
		  str_echo(client_fd,S_IP,S_PORT);
          exit(0);
        }
		else
          close(client_fd);
    }
    return 0;
}