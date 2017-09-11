#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define MAX 256 
#define PATH 30
#define chop(str) str[strlen(str) -1] = 0;

int main(int argc,char * argv[]){
	int sock;
	struct sockaddr_in saddr;
	int len;

	sock = socket(PF_INET,SOCK_STREAM,0);

	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(9199);
	//	saddr.sin_port = htons(atoi(argv[2]));
	//	saddr.sin_addr.s_addr=inet_addr(argv[1]);
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	connect(sock,(struct sockaddr *)&saddr,sizeof(saddr));

	char buf[MAX]; 
	memset(buf,0,sizeof(buf));

	//id 입력
	//로그인 
	while(1){
		printf("id를 입력하세요 : ");
		fgets(buf,MAX-1,stdin);
		chop(buf);
		write(sock,buf,MAX);

		read(sock,buf,MAX);
		if(buf[0]=='1'){
			printf("로그인 완료\n");
			break;
		}
		else if(buf[0] == '0')
			printf("없는 id입니다!\n");
	}

	//터미널 실행
	while(1){
		char path[30]= {0};
		memset(buf,0,MAX);

		read(sock,path,PATH);
		printf("%s",path);

		printf("$ ");
		fgets(buf,MAX-1,stdin);
		chop(buf);

		if(!strcmp(buf,"exit")){
			write(sock,buf,MAX);
			printf("로그아웃\n");
			break;
		}
		else if(buf[0]=='s' && buf[1]=='a' && buf[2]=='v' && buf[3]=='e'){
			char buf2[MAX] = {0};
			printf("저장할 내용을 입력하세요 : ");
			fgets(buf2,MAX-1,stdin);
			chop(buf2);

			write(sock,buf,MAX);
			write(sock,buf2,MAX);

			read(sock,buf,MAX);
			printf("%s\n",buf);
		}
		else{
			write(sock,buf,MAX);
			printf("send : %s\n",buf);

			read(sock,buf,MAX);
			printf("%s\n",buf);

		}
	}

	close(sock);

	return 0;
}
