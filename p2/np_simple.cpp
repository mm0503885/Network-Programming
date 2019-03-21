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
#define BACKLOG 10  // �̤j�P??��?�D?
#define MAXDATASIZE 1000
#define ARGSIZE 1000
#define MAXLINE 15000

#include <iostream>
 
using namespace std;

void childHandler(int signo) {
int status;
while (waitpid(-1, &status, WNOHANG) > 0) {
//do nothing
  }
}
char buff[10000];
pid_t pid_table[10000]={0};


void creat_proc(int sockfd,bool socket_in_flag,bool socket_out_flag,int C,int command_count,char** argv,
                int fd_in, int fd_out,
                int pipes_count, int pipes_fd[][2],char* redirection_file,int redirection_flag
				,int numbered_pipes_fd[][2],int numbered_pipes_current,int numbered_pipe_start_flag,int numbered_pipes_counts[]
				,int numbered_pipe_open_flag[],int numbered_pipes_error_flag)
{
	pid_t p;
    while((p = fork())<0)
		usleep(1000);

    if (p == 0)
    {		
        //INPUT
        if (socket_in_flag != true) { dup2(fd_in, STDIN_FILENO); } //ordinary pipes
		else  //numbered pipe �άO��̬ҨS
		{
          for(int i=0;i<1500;i++) //merge�Ҧ���stdin�ϥΪ�numbered pipe
		  {
		    if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1)  
		    {
			   dup2(numbered_pipes_fd[i][0], STDIN_FILENO);
               break;			   
			}
		  }
		
		 // �Lordinary��numbered pipes stdin�Y����L��J
		}
		
		//OUTPUT
		
		if (socket_out_flag != true)  dup2(fd_out, STDOUT_FILENO); //ordinary pipes
		else if(redirection_flag)  //��redirection ��s�}���ɮ׷�stdout
		{
		  dup2(fd_out, STDERR_FILENO);
          fd_out = open(redirection_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		  dup2(fd_out, STDOUT_FILENO);
		}
		else if(numbered_pipe_start_flag) //���}�ҷs��numbered pipe
		{
		  if(!numbered_pipes_error_flag)
          {			  
		  	dup2(fd_out, STDERR_FILENO);
	    	fd_out  = numbered_pipes_fd[numbered_pipes_current][1];
		  }
		  else // !�����p 
		  {
			fd_out  = numbered_pipes_fd[numbered_pipes_current][1];
		    dup2(fd_out, STDERR_FILENO);
		  }
		  dup2(fd_out, STDOUT_FILENO);

		}
        else  //socket��X
		{
			dup2(fd_out, STDERR_FILENO);
			dup2(fd_out, STDOUT_FILENO);
			
		}
    
 
        if(C!=0) close(pipes_fd[C-1][0]); //�ϥΧ��� �����e�@��ordinary pipe��read��
	    if(C!=command_count-1) close(pipes_fd[C][1]); //�g�J���� ����ordinary pipe��write��
		for(int i=0;i<1500;i++)  //�����Ҧ��}�Ҫ�numbered pipe read�� ���ިϥλP�_
		{
		  	if(numbered_pipe_open_flag[i]==1)
			{
			  close(numbered_pipes_fd[i][0]);
			  close(numbered_pipes_fd[i][1]);
			}
		}

        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr,
                    "Unknown command: [%s].\n",
                    argv[0]);

            exit(EXIT_FAILURE);
        }
	

        /* NEVER REACH */
        exit(EXIT_FAILURE);
    }
	else //parent
	{
      pid_table[C]=p;
	  if(C!=0) close(pipes_fd[C-1][0]); //�����e�@��ordinary pipe��read��
	  if(C!=command_count-1) close(pipes_fd[C][1]); //����ordinary pipe��write��
      
	}
}


int readline(int fd,char *ptr,int maxlen)
{
  int n, rc;
  char c;
  for(n=1; n<maxlen; n++)
  {
    rc=read(fd,&c,1);
    if(rc== 1)
    {
	  if(c=='\n') {*ptr++ = '\0';  break;}
	  else if(c=='\r') {*ptr++ = '\0';  break;}
	  else *ptr++ = c;
    }
    else if(rc==0)
    {
      if(n==1)     return 0;
      else         break;
    }
    else return(-1);
  }
  n-=1;

  return n;
}   

int str_echo(int sockfd)
{
  char env[]="PATH=bin:.";
  putenv(env);
  int n;
  char line[MAXLINE];

  
  int numbered_pipes_fd[1500][2];  //1500��numbered pipes
  int numbered_pipe_open_flag[1500]={0}; //�P�_����numbered pipe�ϥΤ�
  int numbered_pipes_counts[1500]={0}; //�C��numbered pipe�Ѿl�^�X
  int numbered_pipes_process_num[1500]={0};
  int is_build_in=0;
  while(1)
  {
	if(!is_build_in){
     if(write(sockfd, "% ", 2) == -1) {
                perror("send error");
            }
    }
	else
	  is_build_in=0;
	  
    memset(line, '\0', sizeof(line));
	n=readline(sockfd,line,MAXLINE);


    if(n==0)
	  return 0;
    else if (n<0)
	  perror("str_echo: read line error");
    
    else 
	{
	  char* argvs[15000][50]={NULL};  //15000��commands �C�ӧt50�� cstrings �w�]NULL

      int pro_arg_name_length=0;  //program��argument name������
      int argv_pos=0;  //program��argument ���argv������m
      int start=0; //program��argument name���_�l�I
	  int command_pos=0; //�ĴX��command
	  int redirection_flag=0; //�P�_�O�_redirection to file;
	  char* redirection_file={NULL}; //redirection file �ɦW
	  int numbered_pipe_start_flag=0; //�P�_�O�_�n��numbered pipe
	  int numbered_pipes_current; //�ϥΪ��O����numbered pipe
	  int numbered_pipes_error_flag=0; //�P�_�O�_�npipe stderror
	  int end_flag=0;

	   
      for(int i=0;line[i]!='\0';i++)
	  {
		if(line[i]=='|' || line[i]=='!') //pipe
        {
		  if(line[i]=='!') numbered_pipes_error_flag=1;  
		  if(line[i+1]!=' ') //numbered pipes
		  {
			int count=0;
			for(int j=i+1;line[j]!='\0';j++)
			{
			  count*=10;
			  count+=(line[j]-'0');
			}
			numbered_pipe_start_flag=1;
			int find_same=0;
			for(int a=0;a<1500;a++) //�M��˼ƬۦP��numbered pipes
			{
			  if(numbered_pipe_open_flag[a]==1 && numbered_pipes_counts[a]==count)
			  {
			     numbered_pipes_current=a;
				 find_same=1;
				 break;
			  }
			}
			if(find_same) break;
			for(int a=0;a<1500;a++) //�M��i�Ϊ�numbered pipes
		    {
			  if(numbered_pipe_open_flag[a]==0)
			  {
				numbered_pipes_counts[a]=count;
			    numbered_pipe_open_flag[a]=1;
				numbered_pipes_current=a;
				if (pipe(numbered_pipes_fd[a]) == -1)
                {
                  fprintf(stderr, "Error: Unable to create pipe.");
                  exit(EXIT_FAILURE);
                }
				break;
			  }
			} 
		   break;
		  }
          argv_pos=0;  //ordinary pipe	
		  command_pos++;
		  i++;
		  start+=2;
		}
		else if(line[i]=='>') //redirection
        {
		  redirection_flag=1;
		  i+=2;
		  start=i;
		  for(int j=i;line[j]!='\0';j++)
		    pro_arg_name_length++;
		  break;
        }			
        else if (line[i]==' ') //�J�ť� ������
        {
	
	       argvs[command_pos][argv_pos]= new char [pro_arg_name_length+1];
		   
		   strncpy(argvs[command_pos][argv_pos],line+start,pro_arg_name_length);	
        
           argv_pos++;
           pro_arg_name_length=0;
           start=i+1;
		   if(line[i+1]==' ' || line[i+1]=='\0')
			   end_flag=1;
        }
		else  pro_arg_name_length++;

      }
	 
	   
	
	  
	 
		   
	  //�̫᪺�ťդ����٦��@��argument �A���@������
	  if((!numbered_pipe_start_flag) && (!end_flag))
	  {
		   
		  if(!redirection_flag)
		  {
			argvs[command_pos][argv_pos]= new char [pro_arg_name_length+1];
			strncpy(argvs[command_pos][argv_pos],line+start,pro_arg_name_length);
		  }
       	  else
		  {
		    redirection_file= new char [pro_arg_name_length+1];  
			strncpy(redirection_file,line+start,pro_arg_name_length);
		  }
			
	  }
	


	  //build-in commands
	    char Printenv[]="printenv";
		char Setenv[]="setenv";
		char Exit[]="exit";
		
		if(strcmp(argvs[0][0],Exit)==0)
		{
			is_build_in=1;
	        close(sockfd);
			exit(0);

		}
	    else if(argvs[0][0][0]=='\0')
		{
		
		  continue;
		}
        else if(strcmp(argvs[0][0],Printenv)==0)
		{
 		
		  char *ptr;
			  if( (ptr = getenv(argvs[0][1])) == (char *) 0)
		    ptr=ptr; 
		  else
		  { 
	        is_build_in=1;
	        char ptr2[1000];
			memset(ptr2,'\0',sizeof(ptr2));
	        strcpy(ptr2, ptr);
			int a;
		    for(a=0;ptr2[a]!='\0';a++) ;
			ptr2[a]='\n';
			ptr2[a+1]='%';
			ptr2[a+2]=' ';
           	if(send(sockfd, ptr2, 1000, 0) == -1) {
                perror("send error");
            } 
          }			
		}
		else if(strcmp(argvs[0][0],Setenv)==0)
		{
		  setenv(argvs[0][1],argvs[0][2],1);
		}
		else  //exec-based commands
		{
		 int C;
         int command_count = command_pos+1;
	     int pipe_count = command_pos;
		 int pipes_fd[pipe_count][2];
	    
		signal(SIGCHLD, childHandler);
        for (C = 0; C < command_count; C++)
        {
		  bool socket_in_flag=false;
		  bool socket_out_flag=false;
	      if(C!=(command_count-1))
		    if (pipe(pipes_fd[C]) == -1)
            {
              fprintf(stderr, "Error: Unable to create pipe. (%d)\n",C);
              exit(EXIT_FAILURE);
            }
			if(C == 0) socket_in_flag=true;
			if(C == command_count - 1) socket_out_flag=true;
            int fd_in = (C == 0) ? (sockfd) : (pipes_fd[C - 1][0]);
            int fd_out = (C == command_count - 1) ? (sockfd) : (pipes_fd[C][1]);
 
        
        // �إ� Child Process
        creat_proc(sockfd,socket_in_flag,socket_out_flag,C,command_count,argvs[C], fd_in, fd_out, pipe_count, pipes_fd,redirection_file,redirection_flag
		,numbered_pipes_fd,numbered_pipes_current,numbered_pipe_start_flag,numbered_pipes_counts,numbered_pipe_open_flag
		,numbered_pipes_error_flag);
        }

		
		for(int i=0;i<1500;i++) //�P�_�����w�N����numbered pipes �����G��stdin�ϥΡA���^��pipe
	   {
	    if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1) 
	    {		
	      numbered_pipe_open_flag[i]=0;
		  close(numbered_pipes_fd[i][0]);
		  close(numbered_pipes_fd[i][1]);
		  break;
	    }
	
	   }
		
		
        /* ���ݩҦ���child process���槹���~�i�i��U�@�� */

		if(!numbered_pipe_start_flag)
	      for (C = 0; C < command_count; C++)
          {
            int status;
            waitpid(pid_table[C], &status, 0);
          }

	
	    
       }  //exec-based commands
	   
	
	  
	   for(int i=0;i<1500;i++) //�Ҧ�numbered pipes�˼ƴ�@
	   
	if(numbered_pipes_counts[i]!=0) { numbered_pipes_counts[i]--; }
      
    }
  }
	
}
 
int main(int argc, char *argv[])
{
    if(argc != 2)
	  perror("Input form error");
  
    int sock_fd,client_fd;  // sock_fd�G?�vsocket�Fclient_fd�G?�u??socket
    int sin_size;
	int childpid;
    struct sockaddr_in my_addr; // ����a�}�H��
    struct sockaddr_in remote_addr; // ��?�ݦa�}�H��
    
	in_port_t server_port = atoi(argv[1]);
	 
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creat error�I");
        exit(1);
    }
 

    bzero((char *) &my_addr ,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(server_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error !");
        exit(1);
    }
    if(listen(sock_fd, BACKLOG) == -1) {
        perror("listen error�I");
        exit(1);
    }
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if((client_fd = accept(sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size)) == -1) {
            perror("accept error !");
            continue;
        }
        if( (childpid=fork()) < 0) 
          perror("fork error !"); 
        else if(childpid ==0)	  
		{  
   	      close(sock_fd);
		  str_echo(client_fd);
          exit(0);
        }
        close(client_fd);
    }
    return 0;
}