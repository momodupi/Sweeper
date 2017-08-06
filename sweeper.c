#include <stdio.h>  
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

#include "motor.h"
#include "camera.h"
#include "data.h"


extern volatile drive_typedef drive;



int main(int argc, char *argv[])  
{  
	FILE *camera_stream, *poweroff;
	sweeper_typedef swp;
	socket_typedef scktdata;

	char offcmd[1024];

	int server_sockfd;	//服务器端套接字  
	int client_sockfd;	//客户端套接字  
	
	int len = 0, sts = 0, cnt;
	struct sockaddr_in my_addr;  	 //服务器网络地址结构体  
	struct sockaddr_in remote_addr;	 //客户端网络地址结构体  
	
	int sin_size = 0;  
	char rsvbuf[BUFSIZ];  //数据传送的缓冲区  
	char sndbuf[BUFSIZ];

	memset(&my_addr, 0, sizeof(my_addr));		//数据初始化--清零  
	my_addr.sin_family = AF_INET; 				//设置为IP通信  
	my_addr.sin_addr.s_addr = INADDR_ANY;		//服务器IP地址--允许连接到所有本地地址上 
	my_addr.sin_port = htons(8000); 			//服务器端口号  

	/*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/  
	if((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)  
	{    
		printf("Socket error\n");
		close(client_sockfd);
        close(server_sockfd);
		return 1;  	
	}  
	
	/*将套接字绑定到服务器的网络地址上*/  
	if (bind(server_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)  
	{
		printf("Bind error\n"); 
		close(client_sockfd);  
		close(server_sockfd); 
		return 1;  	
	}  
	
	//监听连接请求--监听队列长度为5
	listen(server_sockfd, 5);  					
	sin_size = sizeof(struct sockaddr_in);  

	//等待客户端连接请求到达
	if((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
	{
		printf("Accept error\n");
		close(client_sockfd);
        close(server_sockfd);
		return 1;	
	}  
				
	printf("accept client %s\n", inet_ntoa(remote_addr.sin_addr));  
	len = send(client_sockfd, "Server connection complete\n", 27, 0);		//发送欢迎信息  
	
	if (camera_open(camera_stream) == -1)
	{
		//len = send(client_sockfd, "Camera initation fail\n", 22, 0);
	}
	else
	{
		//len = send(client_sockfd, "Camera initation complete\n", 26, 0);
	}

	snprintf(offcmd, sizeof(offcmd), "home/pi/sweeper/poweroff.sh");

	sts = motor_init();
	if (sts != 0)
	{
		//len = send(client_sockfd, "Motor initation fail\n", 21, 0);
		close(client_sockfd);
        close(server_sockfd);
		return 1;
	}
	else
	{
		//len = send(client_sockfd, "Motor initation complete\n", 25, 0);
	}

	scktdata.type = 0;
	//接收客户端的数据并将其发送给客户端--recv返回接收到的字节数，send返回发送的字节数
	while((len = recv(client_sockfd, rsvbuf, BUFSIZ, 0)) > 0)
	{
		for (cnt = 0; cnt < DATASIZE; cnt++)
		{
			scktdata.data.buf[cnt] = rsvbuf[cnt];
		}
	
		socket_unpack(&scktdata, &swp);

		if (swp.sweeper_mode == 2)
		{
			poweroff = popen(offcmd, "r");
			break;
		}

		motor_ctrl(swp.motor_mode, swp.motor_spd, swp.motor_dirc);
		
		socket_pack(&scktdata, &swp);
		
		for (cnt = 0; cnt < DATASIZE; cnt++)
		{
			sndbuf[cnt] = scktdata.data.buf[cnt];
		}
		
		if (send(client_sockfd, sndbuf, DATASIZE, 0) < 0)  
		{  								
			printf("write error\n");
			break;			
		}
	}  
	
	if (camera_close(camera_stream) == -1)
	{
		//len = send(client_sockfd, "Camera fail\n", 11, 0);
	}

	//len = send(client_sockfd, "Poweroff in 5 sec\n", 18, 0);
	//sleep(5);

	close(client_sockfd);  
	close(server_sockfd);  

	poweroff = popen(offcmd, "r");
	//system("poweroff");

	return 0;  
}

