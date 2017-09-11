#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<fcntl.h>

#define MAX 256
#define PATH 30
#define chop(str) str[strlen(str) -1] = 0;

void childProcess();
char buf[MAX];
int clnt_sock;
char name[3][10] = {"juneun\0","zzang\0","goodman\0"};

void error_handling(char *message){
	fputs(message, stderr);
	exit(1);
}

int main(int argc, char *argv[]){

	int serv_sock;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	pid_t pid;

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error ");

	printf("접속준비중...\n");

	int proc_status;

	while(1){

		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

		if(clnt_sock==-1)
			continue;

		printf("접속완료!!\n");

		pid=fork();

		if(pid==0){
			close(serv_sock);
			childProcess();

		}
		else{
			close(clnt_sock);
		}	

	}
	return 0;
}

void childProcess(){
	int check=1;
	pid_t pid;
	int proc;
	char id[15]={0};
	char path[30]={0};

	FILE *readPipe,*writePipe;

	strcpy(path,"./\0");

	//로그인 하는부분
	while(check){
		read(clnt_sock,buf,MAX);
		int i;
		for(i=0;i<3;i++)
			if(!strcmp(buf,name[i])){
				printf("%s님 로그인 완료\n",buf);
				strcpy(id,buf);
				strcat(path,buf);
				strcpy(buf,"111");
				write(clnt_sock,buf,MAX);
				check=0;
				continue;
			}
		if(check==1){
			strcpy(buf,"000");
			write(clnt_sock,buf,MAX);
		}
	}

	//루트 디렉토리 없을경우 생성
	memset(buf,0,MAX);
	strcpy(buf,"mkdir ");
	strcat(buf,id);
	writePipe=popen(buf,"r");	
	pclose(writePipe);

	while(1){
		char *token=NULL;
		char tmp[15]={0};
		char bufTmp[MAX]={0};
		memset(buf,0,MAX);

		write(clnt_sock,path,PATH);

		read(clnt_sock,buf,MAX);
		strcpy(bufTmp,buf);

		if(!strcmp(buf,"exit")){
			printf("%s님 로그아웃\n",id);
			exit(0);
		}

		token = strtok(bufTmp," ");
		if(!strcmp(bufTmp,"ls") || !strcmp(bufTmp,"dir")){
			strcat(bufTmp," ");
			strcat(bufTmp,path);
		}
		else if(!strcmp(bufTmp,"cat")|| !strcmp(bufTmp,"mkdir")|| !strcmp(bufTmp,"rmdir")||!strcmp(bufTmp,"rm")){
			strcat(bufTmp," ");
			strcat(bufTmp,path);
			strcat(bufTmp,"/");
			token=strtok(buf," ");
			token=strtok(NULL,"\n");
			strcat(bufTmp,token);
		}
		else if(!strcmp(bufTmp,"cd")){
			token=strtok(buf," ");
			token=strtok(NULL,"\n");

			if(!strcmp(token,"..")){
				int len=0;
				while(path[len] != '\0') len++;
				while(path[len] != '/'){
					path[len]='\0';
					len --;
				}
				if(len!=1){
					path[len] = '\0';
				}
				if(path[2]=='\0'){
					strcat(path,id);
					write(clnt_sock,"루트 디렉토리 입니다\n",MAX);
					continue;
				}
			}
			else{
				if(path[2] != '\0')
					strcat(path,"/");
				strcat(path,token);
			}

			write(clnt_sock,"이동완료\n",MAX);

			continue;
		}
		else if(!strcmp(bufTmp,"cp")){
			strcat(bufTmp," ");
			strcat(bufTmp,path);
			strcat(bufTmp,"/");

			token=strtok(buf," ");
			token=strtok(NULL," ");
			strcat(bufTmp,token);

			strcat(bufTmp," ");
			strcat(bufTmp,path);
			strcat(bufTmp,"/");


			token=strtok(NULL,"\n");
			strcat(bufTmp,token);
		}
		else if(!strcmp(bufTmp,"save")){
			int fd;
			char pathTmp[30] = {0};
			char buf2[MAX] = {0};
			strcpy(pathTmp,path);
			token = strtok(NULL,"\n");
			strcat(pathTmp,"/");
			strcat(pathTmp,token);

			fd = open(pathTmp,O_WRONLY|O_CREAT);

			read(clnt_sock,buf2,MAX);

			write(fd,buf2,sizeof(buf2));
			strcpy(buf2,"저장 되었습니다.\n\0");
			write(clnt_sock,buf2,MAX);

			continue;
		}

		char result[MAX]={0};

		writePipe=popen(bufTmp,"r");	
		fread( (void*)result, sizeof(char), MAX, writePipe );
		write(clnt_sock,result,MAX);
		pclose(writePipe);
	}
}