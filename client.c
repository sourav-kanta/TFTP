#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

#define PORT 69
#define LISTEN_PORT 4444
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

static const char* MODE="octet";

void error_exit(char *err)
{
	perror(err);
	exit(0);
}

void printf_help()
{
	printf("\n\t\t\t\tTFTP CLIENT\n");
	printf("\nUsage\n\n");
	printf("put <filename> : Upload <filename> to server.\n");
	printf("get <filename> : Download <filename> to server.\n");
	printf("help : Get help for command\n");
	printf("quit : Exit client\n\n");  	
}

void read_file(int fd,char *fnm,const char *mode,struct sockaddr_in serv_addr)
{
	struct sockaddr_in my_addr,inaddr;
	unsigned char buff[516],recv[516];
	memset((void *)buff,0,516);
	int addrlen=sizeof(my_addr);
	if(!(getsockname(fd,(struct sockaddr *)&my_addr,&addrlen)==0 && my_addr.sin_family ==AF_INET && addrlen==sizeof(my_addr)))
		error_exit("Error getting my address");
	
	buff[0]=0x0;
	buff[1]=RRQ;
	strcpy(&buff[2],fnm);
	strcpy(&buff[2+strlen(fnm)+1],mode);
	int err=sendto(fd,buff,516,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in));
	if(err==-1)
		error_exit("Error send read request");
	printf("Sent read request\n");
	int fd_out=open(fnm,O_WRONLY|O_CREAT,0666);	
	if(fd_out==-1)
		error_exit("Error creating file");
	int block=0;
	while(1)
	{
		socklen_t len=sizeof(inaddr);
		err=recvfrom(fd,recv,516,0,(struct sockaddr *)&inaddr,&len);
		if(err!=-1)
			printf("Received from server %d opcode :  block number %d of %d bytes\n",recv[1],recv[2]<<8|recv[3],err-4);
		else
			perror("Error receiveing from socket");
		int code=recv[1];
		if(code==ERROR)
		{
			printf("Server error %d : %s\n",recv[1],&recv[4]);
			break;
		}
		else if(code==DATA)
		{
			write(fd_out,&recv[4],err-4);
			block++;
			memset(buff,0,516);
			buff[0]=0;
			buff[1]=ACK;
			buff[2]=block>>8;
			buff[3]=block%(0xff+1);
			int serr=sendto(fd,buff,4,0,(struct sockaddr *)&inaddr,sizeof(struct sockaddr_in));
			if(serr==-1)
				perror("Error sending ACK");
			else
				printf("Sent ACK for block %d\n",block);
		 	if(err-4!=512)
				break;	
		}
		
	}
	close(fd_out);
			
}

void write_file(int fd,char *fnm,const char *mode,struct sockaddr_in serv_addr)
{
	struct sockaddr_in my_addr,in_addr;
        unsigned char buff[516],recv[516],op[100];
        memset((void *)buff,0,516);
        int addrlen=sizeof(my_addr);
        if(!(getsockname(fd,(struct sockaddr *)&my_addr,&addrlen)==0 && my_addr.sin_family ==AF_INET && addrlen==sizeof(my_addr)))
                error_exit("Error getting my address");

        buff[0]=0x0;
        buff[1]=WRQ;
        strcpy(&buff[2],fnm);
        strcpy(&buff[2+strlen(fnm)+1],mode);
        int err=sendto(fd,buff,516,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in));
        if(err==-1)
                error_exit("Error send read request");
        printf("Sent write request\n");
        int fd_in=open(fnm,O_RDONLY);
        if(fd_in==-1)
                error_exit("Error creating file");
	int block=0,check=0;
	while(1)
	{
		int recv_block;
		//int cnt=read(fd_in,recv,512);
		socklen_t size=sizeof(struct sockaddr_in);
		err=recvfrom(fd,op,100,0,(struct sockaddr *)&in_addr,&size);
		if(err==-1)
			error_exit("Error reading from socket");
		if(op[1]==ERROR)
                {
                        printf("Server error %d : %s\n",op[1],&op[4]);
                        break;
                }
		if(op[1]==ACK)
		{
			block++;
			recv_block=op[2]<<8|op[3];
			printf("Acknowledgement received for block %d\n",recv_block);
			if(check==1)
				break;
			char send[516];
			memset(send,0,516);
			send[0]=0x0;
			send[1]=DATA;
			send[2]=block>>8;
			send[3]=block%(0xff+1);
			int cnt=read(fd_in,&send[4],512);
			err=sendto(fd,send,cnt+4,0,(struct sockaddr *)&in_addr,sizeof(struct sockaddr_in));
			printf("Sending block %d to server of %d bytes\n",block,cnt);
			if(cnt<512)
				check=1;
		}
	}
		

}

void communicate(int fd,struct sockaddr_in addr)
{
	char command[512];
	while(1)
	{
		printf("tftp>");
		fgets(command,512,stdin);
		char file[410],operation[100];
		sscanf(command,"%s %s",operation,file);
		if(strcmp(operation,"quit")==0)
			break;
		else if(strcmp(operation,"get")==0)
			read_file(fd,file,MODE,addr);
		else if(strcmp(operation,"put")==0)
			write_file(fd,file,MODE,addr);
		else if(strcmp(operation,"help")==0)
			printf_help();
	}
	close(fd);
}
	
int main(int argc,char *argv[])
{
	int fd;
	char *addr;
	if(argc==1)
		addr="127.0.0.1";
	else
		addr=argv[1];
	struct sockaddr_in serv_addr;
	if((fd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
		error_exit("Error initializing socket");
	memset((void *)&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(69);
	serv_addr.sin_addr.s_addr=inet_addr(addr);
	communicate(fd,serv_addr);
}		
	
		
