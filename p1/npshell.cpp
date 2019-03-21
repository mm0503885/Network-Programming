#include<iostream>
#include<unistd.h>
#include<string>
#include <sys/types.h>
#include <cstring>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>


using namespace std;

void childHandler(int signo) {
int status;
while (waitpid(-1, &status, WNOHANG) > 0) {
//do nothing
  }
}

void creat_proc(int C,int command_count,char **argv,
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
        if (fd_in != STDIN_FILENO) { dup2(fd_in, STDIN_FILENO); } //ordinary pipes
		else  //numbered pipe 或是兩者皆沒
		{
		  int first_flag=1;
		  int merge_fd[2];
          for(int i=0;i<1500;i++) //merge所有給stdin使用的numbered pipe
		  {
			char buff[10000000];
			 int n;
		    if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1)  
		    {
			  n=read(numbered_pipes_fd[i][0],buff,1500);
			  if(first_flag) //第一個找到的numbered  pipe開啟一個新的pipe來做merge
			  {
				 pipe(merge_fd);
				 first_flag=0;
			  }
			  write(merge_fd[1],buff,n);
			  lseek(merge_fd[1],0,SEEK_END);
		    }
		  }
		  if(!first_flag) //代表此輪有numbered pipe 將merge完的結果給stdin使用
		  {
			 close(merge_fd[1]); 
			 lseek(merge_fd[0],0,SEEK_SET);
		  	 fd_in=merge_fd[0];
		     dup2(fd_in, STDIN_FILENO);
			 close(merge_fd[0]); 
		  }
		 // 無ordinary或numbered pipes stdin即為鍵盤輸入
		}
		
		//OUTPUT
		
		if (fd_out != STDOUT_FILENO)  dup2(fd_out, STDOUT_FILENO); //ordinary pipes
		else if(redirection_flag)  //有redirection 把新開的檔案當成stdout
		{
		  fd_out = open(redirection_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		  dup2(fd_out, STDOUT_FILENO);
		}
		else if(numbered_pipe_start_flag) //有開啟新的numbered pipe
		{
		  fd_out  = numbered_pipes_fd[numbered_pipes_current][1];
		  if(numbered_pipes_error_flag) // !的情況 
		  {
		    dup2(fd_out, STDERR_FILENO);
		    lseek(fd_out,0,SEEK_END);
		  }
		  dup2(fd_out, STDOUT_FILENO);
		}
        else fd_out=fd_out; //螢幕輸出		
    
 
        if(C!=0) close(pipes_fd[C-1][0]); //使用完畢 關閉前一個ordinary pipe的read端
	    if(C!=command_count-1) close(pipes_fd[C][1]); //寫入完畢 關閉ordinary pipe的write端
		for(int i=0;i<1500;i++)  //關閉所有開啟的numbered pipe read端 不管使用與否
		{
		  	if(numbered_pipe_open_flag[i]==1)
			  close(numbered_pipes_fd[i][0]);
		}
		if(numbered_pipe_start_flag) //numbered pipe寫入完成就關閉write端 
		  close(numbered_pipes_fd[numbered_pipes_current][1]);
		
	

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
	  if(C!=0) close(pipes_fd[C-1][0]); //關閉前一個ordinary pipe的read端
	  if(C!=command_count-1) close(pipes_fd[C][1]); //關閉ordinary pipe的write端
	  signal(SIGCHLD, childHandler); //處理large process的情況
	}
}


int main()
{ 
  putenv("PATH=bin:.");

  int numbered_pipes_fd[1500][2];  //1500個numbered pipes
  int numbered_pipe_open_flag[1500]={0}; //判斷哪些numbered pipe使用中
  int numbered_pipes_counts[1500]={0}; //每個numbered pipe剩餘回合

  while(1){
   
    cout<<"% ";
    string s;
    getline(cin,s);
	if(s=="") continue;
	if(s=="exit") return 0;

    char* argvs[15000][50]={NULL};  //15000個commands 每個含50個 cstrings 預設NULL

    int pro_arg_name_length=0;  //program或argument name的長度
    int argv_pos=0;  //program或argument 放到argv內的位置
    int start=0; //program或argument name的起始點
	int command_pos=0; //第幾個command
	int redirection_flag=0; //判斷是否redirection to file;
	char* redirection_file; //redirection file 檔名
	int numbered_pipe_start_flag=0; //判斷是否要做numbered pipe
	int numbered_pipes_current; //使用的是哪個numbered pipe
	int numbered_pipes_error_flag=0; //判斷是否要pipe stderror
    for(int i=0;i<s.size();i++)
    {
		if(s[i]=='|' || s[i]=='!') //pipe
        {
		  if(s[i]=='!') numbered_pipes_error_flag=1;  
		  if(s[i+1]!=' ') //numbered pipes
		  {
			int count=0;
			for(int j=i+1;j<s.size();j++)
			{
			  count*=10;
			  count+=(s[j]-'0');
			}
			numbered_pipe_start_flag=1;
			for(int a=0;a<1500;a++) //尋找可用的numbered pipes
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
		else if(s[i]=='>') //redirection
        {
		  redirection_flag=1;
		  i+=2;
		  start=i;
		  for(int j=i;j<s.size();j++)
		    pro_arg_name_length++;
		  break;
        }			
        else if (s[i]==' ') //遇空白 做切割
        {
           string s1=s.substr(start,pro_arg_name_length);
           char* cstr = new char [s1.length()+1];
           strcpy (cstr, s1.c_str()); //string轉換成cstring
           argvs[command_pos][argv_pos]=cstr;
           argv_pos++;
           pro_arg_name_length=0;
           start=i+1;
        }
		else  pro_arg_name_length++;

    }
    //最後的空白之後還有一個argument 再做一次切割
	if(!numbered_pipe_start_flag)
	{
      string s1=s.substr(start,pro_arg_name_length);
           char* cstr = new char [s1.length()+1];
           strcpy (cstr, s1.c_str());
		   if(!redirection_flag)
             argvs[command_pos][argv_pos]=cstr;
		   else
			 redirection_file=cstr;
	}
	    
		//build-in commands
	    char* Printenv="printenv";
		char* Setenv="setenv";
	    
        if(strcmp(argvs[0][0],Printenv)==0)
		{
		  char *ptr;
		  if( (ptr = getenv(argvs[0][1])) == (char *) 0)
		    ptr=ptr;
		  else
		    cout<<ptr<<endl;
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
	
        for (C = 0; C < command_count; C++)
        {
	      if(C!=(command_count-1))
		    if (pipe(pipes_fd[C]) == -1)
            {
              fprintf(stderr, "Error: Unable to create pipe. (%d)\n",C);
              exit(EXIT_FAILURE);
            }
            int fd_in = (C == 0) ? (STDIN_FILENO) : (pipes_fd[C - 1][0]);
            int fd_out = (C == command_count - 1) ? (STDOUT_FILENO) : (pipes_fd[C][1]);
 
        // 建立 Child Process
        creat_proc(C,command_count,argvs[C], fd_in, fd_out, pipe_count, pipes_fd,redirection_file,redirection_flag
		,numbered_pipes_fd,numbered_pipes_current,numbered_pipe_start_flag,numbered_pipes_counts,numbered_pipe_open_flag
		,numbered_pipes_error_flag);
        }


    
	    if(numbered_pipe_start_flag)
        close(numbered_pipes_fd[numbered_pipes_current][1]);
     

        /* 等待所有的child process執行完畢才可進行下一輪 */
        for (C = 0; C < command_count; C++)
        {
          int status;
          wait(&status);
        }
	
	
	    for(int i=0;i<1500;i++) //判斷此輪已將哪些numbered pipes 的結果給stdin使用，收回此pipe
	    if(numbered_pipes_counts[i]==0 && numbered_pipe_open_flag[i]==1) 
	    {	  
	      numbered_pipe_open_flag[i]=0;
		  close(numbered_pipes_fd[i][0]);
	    }
	
	  
	   for(int i=0;i<1500;i++) //所有numbered pipes倒數減一
	     if(numbered_pipes_counts[i]!=0) numbered_pipes_counts[i]--;
       }  //exec-based commands
   } //while
}  //main
