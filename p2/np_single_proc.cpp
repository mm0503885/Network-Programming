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
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <map>
#include <string>
#define BACKLOG 100  // 最大同??接?求?
#define MAXDATASIZE 1000
#define ARGSIZE 1000
#define MAXLINE 15000

#include <iostream>
 
using namespace std;

map<int, string> total_env;
int user_fd[30];
int user_number_used [30]={0};
char user_name[30][1000];
int user_pipes_fd[30][30][2];
int user_pipes_used[30][30]={0};

int numbered_pipes_fd[1500][2];  //1500個numbered pipes
int numbered_pipe_open_flag[1500]={0}; //判斷哪些numbered pipe使用中
int numbered_pipes_counts[1500]={0}; //每個numbered pipe剩餘回合
int numbered_pipes_which_user[1500];  //哪個user在使用此number pipe

char merge_buff[150000];
char line[MAXLINE];

pid_t pid_table[10000]={0};
 

void childHandler(int signo) {
int status;
while (waitpid(-1, &status, WNOHANG) > 0) {
//do nothing
  }
}

void creat_proc(int sockfd,bool socket_in_flag,bool socket_out_flag,int C,int command_count,char** argv,
                int fd_in, int fd_out,
                int pipes_count, int pipes_fd[][2],char* redirection_file,int redirection_flag
				,int numbered_pipes_current,int numbered_pipe_start_flag
				,int numbered_pipes_error_flag,int send_user_pipe_flag,int user_pipe_receiver
				,int receive_user_pipe_flag,int user_pipe_sender)
{
	pid_t p;
    while((p = fork())<0)
		usleep(1000);

    if (p == 0)
    {		
        //INPUT
        if (socket_in_flag != true) { dup2(fd_in, STDIN_FILENO); } //ordinary pipes
		else if (receive_user_pipe_flag)  //user pipe
		{
		  int i;
		  for(i=0;i<30;i++) if(user_fd[i]==sockfd) break;
		  if(user_pipes_used[user_pipe_sender-1][i])
		  {	
		    dup2(user_pipes_fd[user_pipe_sender-1][i][0], STDIN_FILENO);		
		  }
		}
		else  //numbered pipe 或是stdin
		{
		  for(int i=0;i<1500;i++) //merge所有給stdin使用的numbered pipe
		  {
		    if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1 && numbered_pipes_which_user[i]==sockfd)  
		    {
			   dup2(numbered_pipes_fd[i][0], STDIN_FILENO);
               break;			   
			}
		  }
		 // 無ordinary或numbered pipes stdin即為鍵盤輸入
		}
		
		//OUTPUT
		
		if (socket_out_flag != true)  dup2(fd_out, STDOUT_FILENO); //ordinary pipes
		else if(send_user_pipe_flag)  //user pipe
		{ 
		  int i;
		  for(i=0;i<30;i++) if(user_fd[i]==sockfd) break;
		  dup2(user_pipes_fd[i][user_pipe_receiver-1][1], STDERR_FILENO);
		  dup2(user_pipes_fd[i][user_pipe_receiver-1][1], STDOUT_FILENO);
		}
		else if(redirection_flag)  //有redirection 把新開的檔案當成stdout
		{
		  dup2(fd_out, STDERR_FILENO);
          fd_out = open(redirection_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		  dup2(fd_out, STDOUT_FILENO);
		}
		else if(numbered_pipe_start_flag) //有開啟新的numbered pipe
		{
		  if(!numbered_pipes_error_flag)
          {			  
		  	dup2(fd_out, STDERR_FILENO);
	    	fd_out  = numbered_pipes_fd[numbered_pipes_current][1];
		  }
		  else // !的情況 
		  {
			fd_out  = numbered_pipes_fd[numbered_pipes_current][1];
		    dup2(fd_out, STDERR_FILENO);
		  }
		  dup2(fd_out, STDOUT_FILENO);

		}
        else  //socket輸出
		{
			dup2(fd_out, STDERR_FILENO);
			dup2(fd_out, STDOUT_FILENO);
			
		}
    
 
        if(C!=0) close(pipes_fd[C-1][0]); //使用完畢 關閉前一個ordinary pipe的read端
	    if(C!=command_count-1) close(pipes_fd[C][1]); //寫入完畢 關閉ordinary pipe的write端
		for(int i=0;i<1500;i++)  //關閉所有開啟的numbered pipe read端 不管使用與否
		{
		  	if(numbered_pipe_open_flag[i]==1)
			{
			  close(numbered_pipes_fd[i][0]);
			  close(numbered_pipes_fd[i][1]);
			}
		}
		
		for(int i=0;i<30;i++)
		  for(int j=0;j<30;j++)
		  {
			 if(user_pipes_used[i][j]==1)
			 {
			   close(user_pipes_fd[i][j][0]);
			   close(user_pipes_fd[i][j][1]);
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
	  if(C!=0) close(pipes_fd[C-1][0]); //關閉前一個ordinary pipe的read端
	  if(C!=command_count-1) close(pipes_fd[C][1]); //關閉ordinary pipe的write端

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
	
  string s=total_env.find(sockfd)->second;	
  char env[1000];
  strcpy(env, s.c_str());
  putenv(env);

  int n;
  memset(line, '\0', sizeof(line));
  


 

	n=readline(sockfd,line,MAXLINE);
	
	if(line[n-1]==' ') line[n-1]='\0';


    if(n==0)
	{
	  if(write(sockfd, "% ", 2) == -1) {
             perror("send error9");
            }
	  return 1;
	}
    else if (n<0)
	  perror("str_echo: read line error");
    
    else 
	{
	  char* argvs[20][20]={NULL};  //15000個commands 每個含50個 cstrings 預設NULL

      int pro_arg_name_length=0;  //program或argument name的長度
      int argv_pos=0;  //program或argument 放到argv內的位置
      int start=0; //program或argument name的起始點
	  int command_pos=0; //第幾個command
	  int redirection_flag=0; //判斷是否redirection to file;
	  char* redirection_file={NULL}; //redirection file 檔名
	  int numbered_pipe_start_flag=0; //判斷是否要做numbered pipe
	  int numbered_pipes_current; //使用的是哪個numbered pipe
	  int numbered_pipes_error_flag=0; //判斷是否要pipe stderror
	  int end_flag=0;
	  int yell_flag=0;
	  int tell_flag=0;
	  char Yell[]="yell";
	  char Tell[]="tell";
	  int tell_num=0;
	  int user_pipe_sender=0;
	  int user_pipe_receiver=0;
	  int send_user_pipe_flag=0;
	  int receive_user_pipe_flag=0;

		
		   
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
			for(int a=0;a<1500;a++) //尋找倒數相同的numbered pipes
			{
			  if(numbered_pipe_open_flag[a]==1 && numbered_pipes_counts[a]==count && numbered_pipes_which_user[a]==sockfd)
			  {
			     numbered_pipes_current=a;
				 find_same=1;
				 break;
			  }
			}
			if(find_same) break;
			for(int a=0;a<1500;a++) //尋找可用的numbered pipes
			{
			  if(numbered_pipe_open_flag[a]==0)
			  {
				numbered_pipes_counts[a]=count;
			    numbered_pipe_open_flag[a]=1;
				numbered_pipes_current=a;
				numbered_pipes_which_user[a]=sockfd;
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
		else if(line[i]=='>') //redirection or send to user pipe
        {
		  if(line[i+1]!=' ') //send to user pipe
		  {
			 for(int j=i+1;(line[j]!='\0' && line[j]!=' ');j++)
			 {
			   user_pipe_receiver*=10;
			   user_pipe_receiver+=(line[j]-'0');
			 }
			 if(!user_number_used[user_pipe_receiver-1])
			 {
				char error_msg[200];
				sprintf(error_msg,"*** Error: user #%d does not exist yet. ***\n",user_pipe_receiver);
				if(write(sockfd, error_msg,strlen(error_msg)) == -1) {
                  perror("send error10");
                 }
				if(write(sockfd, "% ", 2) == -1) {
                  perror("send error11");
				}
				return 1;
			 }
			 send_user_pipe_flag=1;
			 int j;
		     for(j=0;j<30;j++) if(user_fd[j]==sockfd) break;
		     if(!user_pipes_used[j][user_pipe_receiver-1])
		     {
		       user_pipes_used[j][user_pipe_receiver-1]=1;  
		       pipe(user_pipes_fd[j][user_pipe_receiver-1]);
			   
			   char before[1000];
			   char after[1000];
			   char total[5000];
			   sprintf(before," (#%d) just piped '",j+1);
			   sprintf(after," (#%d) ***\n",user_pipe_receiver);
			   sprintf(total,"*** %s%s%s' to %s%s",user_name[j],before,line,user_name[user_pipe_receiver-1],after);
			   for(int a=0;a<30;a++)
			     if(user_number_used[a])
			       write(user_fd[a],total,strlen(total));
			 }
			 else
			 {
			    char error_msg[200];
				sprintf(error_msg,"*** Error: the pipe #%d->#%d already exists. ***\n",j+1,user_pipe_receiver);
				
				if(write(sockfd, error_msg,strlen(error_msg)) == -1) {
                  perror("send error12");
                 }
				if(write(sockfd, "% ", 2) == -1) {
                  perror("send error13");
				}
				return 1;
			 }
	
			 break;
		  }
		  redirection_flag=1;
		  i+=2;
		  start=i;
		  for(int j=i;line[j]!='\0';j++)
		    pro_arg_name_length++;
		  break;
        }			
		else if(line[i]=='<') //receive from user pipe
		{
	      int x;
		  for(x=i+1;(line[x]!='\0' && line[x]!=' ');x++)
		  {
		    user_pipe_sender*=10;
			user_pipe_sender+=(line[x]-'0');
		  }
		  if(line[x]=='\0') end_flag=1;
		  i=x;
		  start=i+1;
		  if(!user_number_used[user_pipe_sender-1])
	      {
				char error_msg[200];
				sprintf(error_msg,"*** Error: user #%d does not exist yet. ***\n",user_pipe_sender);
				if(write(sockfd, error_msg,strlen(error_msg)) == -1) {
                  perror("send error14");
                 }
				if(write(sockfd, "% ", 2) == -1) {
                  perror("send error15");
				}
				return 1;
		  }
		  int a;
		  for(a=0;a<30;a++) if(user_fd[a]==sockfd) break;
		  if(user_pipes_used[user_pipe_sender-1][a])
		  {
		    receive_user_pipe_flag=1;
			
			char before[1000];
			char after[1000];
			char total[5000];
			sprintf(before," (#%d) just received from ",a+1);
			sprintf(after," (#%d) by '",user_pipe_sender);
			sprintf(total,"*** %s%s%s%s%s' ***\n",user_name[a],before,user_name[user_pipe_sender-1],after,line);
			   for(int b=0;b<30;b++)
			     if(user_number_used[b])
			       write(user_fd[b],total,strlen(total));
		   
		  }
		  else
		  {
		   char error_msg[200];
		   sprintf(error_msg,"*** Error: the pipe #%d->#%d does not exist yet. ***\n",user_pipe_sender,a+1);
		   if(write(sockfd, error_msg,strlen(error_msg)) == -1) {
             perror("send error16");
           }
		   if(write(sockfd, "% ", 2) == -1) {
             perror("send error17");
		   }
		   return 1;
		  }
		}
        else if ((line[i]==' ') && (yell_flag!=1) && (tell_flag!=1)) //遇空白 做切割
        {
	
	       argvs[command_pos][argv_pos]= new char [pro_arg_name_length+1];
		   
		   strncpy(argvs[command_pos][argv_pos],line+start,pro_arg_name_length);
		   argvs[command_pos][argv_pos][pro_arg_name_length]='\0';
           if(strcmp(argvs[command_pos][argv_pos],Yell)==0)
		   {
	     	 yell_flag=1;
		   }		
           if(strcmp(argvs[command_pos][argv_pos],Tell)==0)
		   {
	     	 tell_flag=1;
			 int j;
			 for(j=i+1;line[j]!=' ';j++)
			 {
			   tell_num*=10;
			   tell_num+=(line[j]-'0');
			 }
			 
			 i=j;
		   }		   
           argv_pos++;
           pro_arg_name_length=0;
           start=i+1;
		   if(yell_flag || tell_flag)
		   {
		     for(int t=i+1;line[t]!='\0';t++)
			   pro_arg_name_length++;
		     break;
		   }
		  
		   if((!yell_flag) && (!tell_flag) && (line[i+1]==' ' || line[i+1]=='\0'))
			 end_flag=1;			   
        }
		else  pro_arg_name_length++;

      }
	  
  
	 
		   
	  //最後的空白之後還有一個argument 再做一次切割
	  if((!numbered_pipe_start_flag) && (!end_flag) && (!send_user_pipe_flag))
	  {
		   
		  if(!redirection_flag)
		  {
			argvs[command_pos][argv_pos]= new char [pro_arg_name_length+1];
			strncpy(argvs[command_pos][argv_pos],line+start,pro_arg_name_length);
			argvs[command_pos][argv_pos][pro_arg_name_length]='\0';
		  }
       	  else
		  {
		    redirection_file= new char [pro_arg_name_length+1];  
			strncpy(redirection_file,line+start,pro_arg_name_length);
			redirection_file[pro_arg_name_length]='\0';
		  }
			
	  }
	


	  //build-in commands
	    char Printenv[]="printenv";
		char Setenv[]="setenv";
		char Exit[]="exit";
		char Name[]="name";
		char Who[]="who";
	
		
		if(strcmp(argvs[0][0],Exit)==0)
		{
			char leave_msg[5000];
			for(int i=0;i<30;i++)
			{
			  if(user_fd[i]==sockfd)
			  {

		        sprintf(leave_msg,"*** User '%s' left. ***",user_name[i]);
		        for(int j=0;j<30;j++)
		        {
		    	 if(user_number_used[j])
		           if(write(user_fd[j], leave_msg,strlen(leave_msg)) == -1) {
                     perror("send error18");
                   }
                 if(user_pipes_used[j][i])
                 {
				    close(user_pipes_fd[j][i][0]);
					user_pipes_used[j][i]=0;
				 }					 
		        }
			
			    user_number_used[i]=0;
				memset(user_name[i],'\0', sizeof(user_name[i]));
			    strcpy(user_name[i],"(no name)");
				break;
			  }
			}
	        return 0;

		}
	    else if(argvs[0][0][0]=='\0')
		{
		
		  return 1;
		}
        else if(strcmp(argvs[0][0],Printenv)==0)
		{
 		
		  char *ptr;
			  if( (ptr = getenv(argvs[0][1])) == (char *) 0)
		    ptr=ptr; 
		  else
		  { 
	        char ptr2[1000];
			memset(ptr2,'\0',sizeof(ptr2));
	        strcpy(ptr2, ptr);
			int a;
		    for(a=0;ptr2[a]!='\0';a++) ;
			ptr2[a]='\n';
           	if(send(sockfd, ptr2,a+1, 0) == -1) {
                perror("send error19");
            } 
          }			
		}
		else if(strcmp(argvs[0][0],Setenv)==0)
		{
		  char combine[1000];
		  sprintf(combine,"%s=%s",argvs[0][1],argvs[0][2]);
		  string s1(combine);
		  total_env[sockfd]=combine;
		}
		else if(strcmp(argvs[0][0],Name)==0)
		{
		  int same_name_flag=0;
 		  for(int i=0;i<30;i++)
		    if(strcmp(argvs[0][1],user_name[i])==0)
			  same_name_flag=1;
		  if(same_name_flag==1)
		  {
		    char name_exist_msg[5000];
			sprintf(name_exist_msg,"*** User '%s' already exists. ***\n",argvs[0][1]);
			if(write(sockfd, name_exist_msg,strlen(name_exist_msg)) == -1) {
               perror("send error20");
            }
		  }
		  else
		  {
		    for(int i=0;i<30;i++)
		    {
		      if(user_fd[i]==sockfd)
			  { 
		        memset(user_name[i],'\0', sizeof(user_name[i]));
			    strcpy(user_name[i],argvs[0][1]);
			    char broc_msg[5000];
		        int j;
		        sprintf(broc_msg,"*** User from CGILAB/511 is named '%s'. ***\n",user_name[i]);
		        for(j=0;j<30;j++)
		        {
			      if(user_number_used[j])
		            if(write(user_fd[j], broc_msg,strlen(broc_msg)) == -1) {
                      perror("send error21");
                    } 
		        }
			    break;			  
			  }
		    }
		  }
		}
		else if(strcmp(argvs[0][0],Who)==0)
		{
		  char who_msg[]="<ID>\t<nickname>\t<IP/port>\t<indicate me>\n";
		  char ip_port_msg[]="CGILAB/511";
		  if(write(sockfd, who_msg,strlen(who_msg)) == -1) {
                  perror("send error22");
                 }
		  for(int i=0;i<30;i++)
		  {
			 if(user_number_used[i])
			 {
			   char user_msg[1000];
			   if(user_fd[i]!=sockfd)
			   {
			     sprintf(user_msg,"%d\t%s\t%s\n",i+1,user_name[i],ip_port_msg);
			     if(write(sockfd, user_msg,strlen(user_msg)) == -1) {
                  perror("send error23");
                 }
			   }
			   else
			   {
				 sprintf(user_msg,"%d\t%s\t%s\t<-me\n",i+1,user_name[i],ip_port_msg);
			     if(write(sockfd, user_msg,strlen(user_msg)) == -1) {
                  perror("send error24");
                 }
			   }				   
			     
			   
			 }
		  }
		  
		}
		else if(strcmp(argvs[0][0],Yell)==0)
		{
		   char yell_msg[5000];
		   int i;
		   for(i=0;i<30;i++) if(user_fd[i]==sockfd) break;
		   sprintf(yell_msg,"*** %s yelled ***: %s\n",user_name[i],argvs[0][1]);
		   for(i=0;i<30;i++)
		   {
			  if(user_number_used[i])
		        if(write(user_fd[i], yell_msg,strlen(yell_msg)) == -1) {
                  perror("send error25");
                 } 
		   }
		}
		else if(strcmp(argvs[0][0],Tell)==0)
		{
		   char tell_msg[5000];
		   if(user_number_used[tell_num-1]!=1)
		   {
			 sprintf(tell_msg,"*** Error: user #%d does not exist yet. ***\n",tell_num);
			 if(write(sockfd, tell_msg,strlen(tell_msg)) == -1) {
               perror("send error26");
             }
		   }
		   else
		   {
		     int i;
		     for(i=0;i<30;i++) if(user_fd[i]==sockfd) break;
		     sprintf(tell_msg,"*** %s told you ***: %s",user_name[i],argvs[0][1]);
	  	     if(write(user_fd[tell_num-1], tell_msg,strlen(tell_msg)) == -1) {
               perror("send error27");
             }
		   }			 
	
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
 
        // 建立 Child Process
        creat_proc(sockfd,socket_in_flag,socket_out_flag,C,command_count,argvs[C], fd_in, fd_out, pipe_count, pipes_fd,redirection_file,redirection_flag
		,numbered_pipes_current,numbered_pipe_start_flag,numbered_pipes_error_flag,send_user_pipe_flag,user_pipe_receiver,receive_user_pipe_flag,user_pipe_sender);
        }
		
		for(int i=0;i<1500;i++) //判斷此輪已將哪些numbered pipes 的結果給stdin使用，收回此pipe
	    {
	      if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1 && numbered_pipes_which_user[i]==sockfd) 
	      {		
	        numbered_pipe_open_flag[i]=0;
		    close(numbered_pipes_fd[i][0]);
		    close(numbered_pipes_fd[i][1]);
		    break;
	      }
		}
		
		if(receive_user_pipe_flag)
		{
	      int i;
		  for(i=0;i<30;i++) if(user_fd[i]==sockfd) break;	
		  close(user_pipes_fd[user_pipe_sender-1][i][0]);
		  close(user_pipes_fd[user_pipe_sender-1][i][1]);
          user_pipes_used[user_pipe_sender-1][i]=0;		
		}
	
		

        /* 等待所有的child process執行完畢才可進行下一輪 */
        if(!numbered_pipe_start_flag && !send_user_pipe_flag)
	      for (C = 0; C < command_count; C++)
          {
            int status;
            waitpid(pid_table[C], &status, 0);
          }
	
	
	    
       }  //exec-based commands
	   
	  
	   for(int i=0;i<1500;i++) //所有numbered pipes倒數減一
	     if(numbered_pipes_counts[i]!=0 && numbered_pipes_which_user[i]==sockfd) numbered_pipes_counts[i]--;
      
	
          if(write(sockfd, "% ", 2) == -1) {
             perror("send error28");
            }

    }
	
	return 1;
  
	
}
 
int main(int argc, char *argv[])
{
    if(argc != 2)
	  perror("Input form error");
  
    int server_listen_fd;  
    int client_address_length;
	int childpid;
    struct sockaddr_in my_addr; // host address
    struct sockaddr_in client_addr; // client address
	fd_set rfds;
	fd_set afds;
	int nfds,fd;
	
	for(int i=0;i<30;i++)
      strcpy(user_name[i],"(no name)");
	
	  char welcome_msg[]="****************************************\n"\
	                         "** Welcome to the information server. **\n"\
					         "****************************************\n"\
					         "*** User '(no name)' entered from ";
	  char prompt_msg[]="% ";
	  char dash[]="/";
	  char chang_line[]="\n";				 
	  char enter_msg[]="*** User '(no name)' entered from ";
	  char last[]=". ***";
    
	in_port_t server_port = atoi(argv[1]);
	 
    if((server_listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creat error！");
        exit(1);
    }
 

    bzero((char *) &my_addr ,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(server_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_listen_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error !");
        exit(1);
    }
    if(listen(server_listen_fd, BACKLOG) == -1) {
        perror("listen error！");
        exit(1);
    }
	
	nfds = getdtablesize();
	FD_ZERO(&afds);
	FD_SET(server_listen_fd,&afds);
	
	
	
    while(1) {
		memcpy(&rfds, &afds,sizeof(rfds));
		
		while(select(nfds, &rfds , (fd_set *)0 , (fd_set *)0,(struct timeval *)0)<0) memcpy(&rfds, &afds,sizeof(rfds));
	
	
	    
	   
	    if (FD_ISSET(server_listen_fd, &rfds)) {
          int client_fd;
          client_address_length = sizeof(client_addr);
          client_fd = accept(server_listen_fd, (struct sockaddr *)&client_addr,(socklen_t*)&client_address_length);
		  
		  char* client_addr_msg;
		  client_addr_msg=new char[100];
		  client_addr_msg=inet_ntoa(client_addr.sin_addr);
		  int len1;
		  for(len1=0;client_addr_msg[len1]!='\0';len1++);
		  
		  char* client_port_msg;
		  client_port_msg=new char[100];
		  unsigned int client_port_number=ntohs(client_addr.sin_port);
		  sprintf(client_port_msg,"%u",client_port_number);
		  int len2;
		  for(len2=0;client_port_msg[len2]!='\0';len2++);
		  
          if (client_fd < 0)
            perror("accept error !");
          FD_SET(client_fd, &afds);

		  if(write(client_fd, welcome_msg, 157) == -1) {
                perror("send error1");
            }
		  if(write(client_fd, "CGILAB", 6) == -1) {
                perror("send error2");
            }
		  if(write(client_fd, dash, 1) == -1) {
                perror("send error3");
            }
		  if(write(client_fd, "511", 3) == -1) {
                perror("send error4");
            }
		  if(write(client_fd, last, 5) == -1) {
                perror("send error5");
            }
		  if(write(client_fd, chang_line, 1) == -1) {
                perror("send error6");
            }
		  if(write(client_fd, prompt_msg, 2) == -1) {
                perror("send error7");
            }	
		 
			
			total_env[client_fd]="PATH=bin:.";
			for (fd=0; fd<nfds; ++fd)
			  if (fd != server_listen_fd && fd != client_fd && FD_ISSET(fd, &afds))
			  {
				 char enter_msg2[2000];
				 sprintf(enter_msg2,"%sCGILAB%s511%s",enter_msg,dash,last);
			     if(write(fd, enter_msg2,strlen(enter_msg2)) == -1) {
                   perror("send error8");
				 }
               
			  }
		  	
		  for(int i=0;i<30;i++)
          {
            if(!user_number_used[i])
            {
            	 user_fd[i]=client_fd;
	             user_number_used[i]=1;
				 break;
            }
          }

		}
		
		
		
		for (fd=0; fd<nfds; ++fd)
          if (fd != server_listen_fd && FD_ISSET(fd, &rfds))
            if (str_echo(fd) == 0) {
              (void) close(fd);
              FD_CLR(fd, &afds);
            }
    }
		
  
 
}