/*
 * Copyright (c) 2019
 *
 * Change Logs:
 * Date             Author      Notes
 *  网络部分：
 */

#include "wt_netclient.h"
#include "wizchip_socket.h"
#include <wiz.h>

#define TCPBUFSZ   512
#define UDPBUFSZ   256




#ifdef USE_DEBUG

/*调试用ip*/
rt_uint8_t local_ip[4]={192,168,1,48};       //本地ip    //改了底层，见wiz.c  360行左右
rt_uint8_t local_gw[4]={192,168,1,1};				//网关
rt_uint8_t local_sn[4]={255,255,255,0};				//子关掩码
#else

/*现场用ip*/
rt_uint8_t local_ip[4]={10,196,40,38};       //本地ip    //改了底层，见wiz.c  360行左右
rt_uint8_t local_gw[4]={10,196,40,33};				//网关
rt_uint8_t local_sn[4]={255,255,255,240};				//子关掩码
#endif

//		rt_thread_delay(1000);                //延时  1/RT_TICK_PER_SECOND 秒   RT_TICK_PER_SECOND 为系统周期，现为500

rt_uint8_t client_ip[4]={192,168,1,0};    //记录要建立连接的ip

rt_uint8_t client_flag = 0;        //网络连接标志位，0网络无连接，1建立UDP接收，2建立UDP发送，3建立TCP连接，能力有限，想不到建立标志位外更好的解决办法


rt_uint8_t net_ico[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x11, 0x00, 0x00};           //迪文网络图标

int tcpsock;
int udpresock;
int udpsesock;

rt_thread_t tid_tcpcli=RT_NULL;
rt_thread_t tid_udpsend=RT_NULL;
rt_thread_t tid_udprec=RT_NULL;

void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L); //确认消息
void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID); //单探杆单数据应答消息
void TcpallReply(int sock,rt_uint8_t DB_Ad_H);//单探杆全部数据应答消息


//url  输入IP或者域名  //port 端口  
void udpsend(const rt_uint8_t *addr,const char *url,const char *udpport,rt_tick_t time)
{
    int port;
    struct hostent *host;
    struct sockaddr_in server_addr;
	
		char udpbeat_data[10] = {192,168,1,177,0x0D,0x9E,7,1,1,1}; /* 发送用到的数据 */		
		
		udpbeat_data[0] = addr[0];
		udpbeat_data[1] = addr[1];
		udpbeat_data[2] = addr[2];
		udpbeat_data[3] = addr[3];
	
		time=time*RT_TICK_PER_SECOND;
		
	  port = strtoul(udpport, 0, 10);
	
    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = (struct hostent *) gethostbyname(url);
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((udpsesock = socket(AF_WIZ,SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        return;
    }
    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_WIZ;
    server_addr.sin_port = htons(port);    //9e 0d
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);     //ff01a8c0
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
      /* 发送数据到服务远端 */
//		IINCHIP_WRITE(Sn_KPALVTR(udpsesock),0x01);
		while (1)
    {
//			rt_event_recv(&event_udprec, UDP_REC_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);
			rt_thread_delay(time); 
			if((sendto(udpsesock, udpbeat_data, 10, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr)))==-1)
			{
				return;
			}
		}
}
/**/

/*udp心跳包发送线程*/
void udpsend_thread_entry(void* parameter)
{
	udpsend(local_ip,UDPBEATIP,UDPBEATPORT,UDPBEATTIM);   	 //本机ip，目标IP地址或者域名 端口 延时时间（s）
	
	net_ico[7]=2;
	uart2_send(net_ico,8);    //发送网络连接失败图标
	if((*tid_tcpcli).stat==2) rt_thread_delete(tid_tcpcli);
	if((*tid_udprec).stat==2) rt_thread_delete(tid_udprec);
	wiz_socket_init();
	client_flag=0;
	rt_kprintf("UDP send error\n");
	return;
}
/**/

//static const char tcpsend_data[] = "This is TCP Client from RT-Thread."; /* 发送用到的数据 */
void tcpclient(const char *url, const char *tcpport)
{
//    int i;
    static char *recv_data;
    struct hostent *host;      //主机地址和域名信息
    int bytes_received;
    struct sockaddr_in server_addr;   //IP地址和端口信息
    int port;
    port = strtoul(tcpport, 0, 10);
    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = gethostbyname(url);
    /* 分配用于存放接收数据的缓冲 */
		if (recv_data == RT_NULL)
		{
				recv_data = rt_malloc(TCPBUFSZ);
		}
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
    if ((tcpsock = socket(AF_WIZ, SOCK_STREAM, 0)) == -1)
    {
        /* 创建socket失败 */
        rt_kprintf("Socket error\n");
        /* 释放接收缓冲 */
//        rt_free(recv_data);
        return;
    }
    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_WIZ;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* 连接到服务端 */
		
    if (connect(tcpsock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("Connect fail!\n");
        /*释放接收缓冲 */
//        rt_free(recv_data);
        return;
    }
//		IINCHIP_WRITE(Sn_KPALVTR(tcpsock),0x01);
		client_flag=3;
		net_ico[7]=1;
		uart2_send(net_ico,8);
    while (1)
    {
        /* 从sock连接中接收最大BUFSZ - 1字节数据 */
        bytes_received = recv(tcpsock, recv_data, TCPBUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            /* 接收失败，关闭这个连接 */
            rt_kprintf("\n tcp received error.\r\n");
            /* 释放接收缓冲 */
//            rt_free(recv_data);
            break;
        }
        else if (bytes_received == 0)
        {
            /* 默认 recv 为阻塞模式，此时收到0认为连接出错，关闭这个连接 */
            rt_kprintf("\n tcp received error.\r\n");
            /* 释放接收缓冲 */
//            rt_free(recv_data);
            break;
        }
				if(recv_data[0]==7&&recv_data[1]==1&&recv_data[2]==1&&recv_data[3]==0)
				{
					if(recv_data[5]==1)
					{
						TcpallReply(tcpsock,recv_data[9]);
						TcpConfirm(tcpsock,recv_data[9],recv_data[10]);
					}
					if(recv_data[5]==0)
					{
						TcponlyReply(tcpsock,recv_data[9],recv_data[10],recv_data[11]);
						TcpConfirm(tcpsock,recv_data[10],recv_data[11]);
					}
				}
    }
    return;
}
/**/

/*tcp线程*/
void tcpcli_thread_entry(void* parameter)
{
	rt_uint32_t ip;
	ip=client_ip[3];
	ip=client_ip[2]+(ip<<8);
	ip=client_ip[1]+(ip<<8);
	ip=client_ip[0]+(ip<<8);
	rt_thread_delay(50); 
	tcpclient(inet_ntoa(ip),"8080");   	 //目标IP地址或者域名 端口
	net_ico[7]=2;
	uart2_send(net_ico,8);
	client_flag=0;
	if((*tid_udpsend).stat==2) rt_thread_delete(tid_udpsend);
	if((*tid_udprec).stat==2) rt_thread_delete(tid_udprec);
	wiz_socket_init();
	return;
}



/**/

/**/
void udpreceive(void)
{
	  int bytes_read;
    static char *recv_data;
    socklen_t addr_len;
    struct sockaddr_in server_addr, client_addr;

//		rt_thread_delay(10); 
    /* 分配接收用的数据缓冲 */

		if (recv_data == RT_NULL)
		{
				recv_data = rt_malloc(UDPBUFSZ);
		}
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");        /* 分配内存失败，返回 */
        return;
    }
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((udpresock = socket(AF_WIZ, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        /* 释放接收用的数据缓冲 */
//        rt_free(recv_data);
        return;
    }
    /* 初始化服务端地址 */
    server_addr.sin_family = AF_WIZ;
//    server_addr.sin_port = htons(3486);     //UDP接收端口不能用这个配置
		wizchip_socket(udpresock, Sn_MR_UDP, 3486, 0);				//配置UDP接收端口
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 绑定socket到服务端地址 */
    if (bind(udpresock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        /* 绑定地址失败 */
        rt_kprintf("Bind error\n");

        /* 释放接收用的数据缓冲 */
//        rt_free(recv_data);
        return;
    }

    addr_len = sizeof(struct sockaddr);
		/* 初始化事件对象 */
//    rt_event_init(&event_udprec, "udpevent", RT_IPC_FLAG_FIFO); 
    while (1)
    {
        /* 从sock中收取最大BUFSZ - 1字节数据 */
        bytes_read = recvfrom(udpresock, recv_data, UDPBUFSZ - 1, 0,
                              (struct sockaddr *)&client_addr, &addr_len);
        /* UDP不同于TCP，它基本不会出现收取的数据失败的情况，除非设置了超时等待 */
			if(bytes_read==10)       //c0 a8 01 7b 0d 9e 01 00 01 01
			{
        /* 收到心跳包 */
#ifdef USE_DEBUG
				rt_kprintf("udp ");
#endif
        if (recv_data[6]==01&&recv_data[7]==0&&recv_data[8]==1&&recv_data[9]==1)
        {
//					rt_event_send(&event_udprec, UDP_REC_EVENT);
					if(client_flag==1)
					{
							client_flag=2;
							client_ip[0]=recv_data[0];
							client_ip[1]=recv_data[1];
							client_ip[2]=recv_data[2];
							client_ip[3]=recv_data[3];

						/* 创建 tcp 线程 */
						tid_tcpcli = rt_thread_create("tcpcli",
														tcpcli_thread_entry,
														RT_NULL,
														1536,
														16,
														20);
						/* 创建成功则启动线程 */
						if (tid_tcpcli != RT_NULL)
							rt_thread_startup(tid_tcpcli);
					}
        }
			}
    }
}
/**/

/*w5500udp接收调用了底层配置端口*/
void udpreceive_thread_entry(void* parameter)
{
	udpreceive();
	net_ico[7]=2;
	uart2_send(net_ico,8);
	if((*tid_udpsend).stat==2) rt_thread_delete(tid_udpsend);
	if((*tid_tcpcli).stat==2) rt_thread_delete(tid_tcpcli);
	wiz_socket_init();
	client_flag=0;
	return;
}
/**/

rt_uint32_t count; 
/*开网络线程，先开UDP接收再开UDP发送*/
void wt_net_open(void)
{
	rt_uint8_t flag; 
	if(wiz_init_ok == RT_FALSE)     //初始化失败，可能为没连接上网络
	{
		wiz_init();
	}
	else                            //初始化成功
	{
		if(client_flag==0)            //网络连接上但没有和软件连接
		{
			closesocket(0);
			closesocket(1);
			closesocket(2);
			closesocket(3);
			closesocket(4);
			closesocket(5);
			closesocket(6);
			closesocket(7);
			wiz_socket_init();
			
			net_ico[7]=2;
			uart2_send(net_ico,8);
		
			client_flag=1;
		/* 创建 网络 线程 */
			tid_udprec = rt_thread_create("udpreceive",
											udpreceive_thread_entry,
											RT_NULL,
											1024,
											15,
											20);
			/* 创建成功则启动线程 */
			if (tid_udprec != RT_NULL)
				 rt_thread_startup(tid_udprec);
			
			/* 创建 UDP心跳包发送 线程 */
				tid_udpsend = rt_thread_create("udpsend",
												udpsend_thread_entry,
												RT_NULL,
												1536,
												14,
												20);
				/* 创建成功则启动线程 */
			if (tid_udpsend != RT_NULL)
				rt_thread_startup(tid_udpsend);
			
		}
		if(client_flag==3)         //与软件建立起连接
		{
			flag=0;
			flag=send(tcpsock, &flag, 1, 0);    //发送0，防止TCP套接字卡死，同时判断TCP连接是否断开
#ifdef USE_DEBUG
			rt_kprintf(" %d \n",count++);
#endif
			if(flag!=1)                         //发送失败，说明连接出问题，重新连接
			{
				if((*tid_tcpcli).stat==2)   rt_thread_delete(tid_tcpcli);
				if((*tid_udpsend).stat==2)  rt_thread_delete(tid_udpsend);
				if((*tid_udprec).stat==2) 	rt_thread_delete(tid_udprec);
				closesocket(tcpsock);
				closesocket(udpresock);
				closesocket(udpsesock);
				rt_kprintf("\n tcp break.\r\n");
				client_flag=0;
			}
		}
	}
}
/**/


/*
网络协议部分：
*/


void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L) //确认消息
{
	 char send_buf[12]={1,0,7,1,0,0xE0,0,4,2,0,0,0};
	 send_buf[9]=DB_Ad_H;
	 send_buf[10]=DB_Ad_L;
   send(sock, send_buf, 12, 0);
}
/**/

void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID) //单探杆单数据应答消息
{
	rt_uint8_t type,addr;
	rt_uint8_t j=0;
	char temp;
	char send_buf[20];

	send_buf[j++]=(char)0x01;
	send_buf[j++]=(char)0x00;
	send_buf[j++]=(char)0x07;
	send_buf[j++]=(char)0x01;
	send_buf[j++]=(char)0x00;
	send_buf[j++]=(char)0x20;
	send_buf[j++]=(char)0x00;


	switch(DB_Ad_H)
	{
		case 0x02: //人井
			addr=DB_Ad_L-0x11;
			type=2;
		break;
		case 0x03://管线
			addr=DB_Ad_L-0x11;
			type=1;
		break;
		case 0x04://油罐
			addr=DB_Ad_L-0x11;
			type=0;
		break;
		case 0x05://油盆
			addr=DB_Ad_L-0x11;
			type=3;
		break;
		default:
		return;
	}
	switch(DataID)
	{
		case 0x02:
			send_buf[j++]=(char)0x07;      //[7]
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;    

			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)0x02;

			if( SEN_state[type].type!=SENPRESS)    //不是压力式
			{
				switch (SEN_state[type].last_sta[addr])   
				{
					case NORM://正常
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
					case WATER://水
						send_buf[j++]=(char)(NET_SENWATER);
						send_buf[j++]=(char)(NET_SENWATER>>8);
					break;
					case OIL://油
						send_buf[j++]=(char)(NET_SENOIL);
						send_buf[j++]=(char)(NET_SENOIL>>8);
					break;
					case HIGH://高液位
						send_buf[j++]=(char)(NET_LIQHIGH);
						send_buf[j++]=(char)(NET_LIQHIGH>>8);
					break;
					case LOW://低液位
						send_buf[j++]=(char)(NET_LIQLOW);
						send_buf[j++]=(char)(NET_LIQLOW>>8);
					break;
					case ERR://通讯故障
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://未开通
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
				}
			}
			else    //压力式
			{
				switch (SEN_state[type].last_sta[addr])   
				{
					case NORM_PRESS://正常
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://压力预警
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://压力报警
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://油气预警
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://油气报警
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://通讯故障
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://未开通
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;

				}
			}
		send(sock, send_buf, 15, 0);
		break;
		case 0x03:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x03;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0x04;
		send(sock, send_buf, 14, 0);
		break;
		case 0x04:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x04;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)DB_Ad_L-0x10;
		send(sock, send_buf, 14, 0);
		break;
		    case 0x05:
        send_buf[j++]=((char)0x09);
        send_buf[j++]=((char)0x02);
        send_buf[j++]=((char)DB_Ad_H);
        send_buf[j++]=((char)DB_Ad_L);
        send_buf[j++]=((char)0x05);
        send_buf[j++]=((char)0x04);
        send_buf[j++]=((char)0x01);
        send_buf[j++]=((char)0x00);
        temp=(char)(SEN_state[type].press[addr]);
        temp=(temp/10*16 + temp%10);
        send_buf[j++]=((char)temp);
        send_buf[j++]=((char)0x00);
        break;
		case 0x10:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x10;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0xaa;
		send(sock, send_buf, 14, 0);
		break;
		case 0x11:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x11;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0xff;
		send(sock, send_buf, 14, 0);
		break;
		case 0x12:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x12;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0x55;
		send(sock, send_buf, 14, 0);
		break;
		case 0x13:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x13;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0x11;
		send(sock, send_buf, 14, 0);
		break;
		case 0x14:
			send_buf[j++]=(char)0x06;
			send_buf[j++]=(char)0x02;
			send_buf[j++]=(char)DB_Ad_H;
			send_buf[j++]=(char)DB_Ad_L;

			send_buf[j++]=(char)0x14;
			send_buf[j++]=(char)0x01;
			send_buf[j++]=(char)0xff;
		send(sock, send_buf, 14, 0);
		break;
		default:
		break;
	}

}
/**/

void TcpallReply(int sock,rt_uint8_t DB_Ad_H)//单探杆全部数据应答消息
{
	rt_uint8_t addr,type;
	rt_uint8_t  DB_Ad_L;
	rt_uint8_t i,j=0;
	char temp;
	char send_buf[40];
	if(DB_Ad_H==0x02)
	{
		i=0x21;
	}
	else
	{
		i=0x19;
	}

	for(DB_Ad_L=0x11;DB_Ad_L<i;DB_Ad_L++)
	{
		switch(DB_Ad_H)
		{
			case 0x02: //人井
				addr=DB_Ad_L-0x11;
				type=2;
			break;
			case 0x03://管线
				addr=DB_Ad_L-0x11;
				type=1;
			break;
			case 0x04://油罐
				addr=DB_Ad_L-0x11;
				type=0;
			break;
			case 0x05://油盆
				addr=DB_Ad_L-0x11;
				type=3;
			break;
			default:
			return;
		}
    send_buf[j++]= ((char)0X01);
    send_buf[j++]= ((char)0X00);
    send_buf[j++]= ((char)0X07);
    send_buf[j++]= ((char)0X01);
    send_buf[j++]= ((char)0X00);//应用消息
    send_buf[j++]= ((char)0x20);//令牌

    send_buf[j++]= ((char)0x00);//消息长度H
    if(DB_Ad_H==0x03)
    {
			send_buf[j++]= ((char)0x10);//消息长度L
    }
    else
    {
			send_buf[j++]= ((char)0x1F);//消息长度L
    }
    //数据库地址/////////////////////////
    send_buf[j++]= ((char)0x02);
    send_buf[j++]= ((char)DB_Ad_H);
    send_buf[j++]= ((char)DB_Ad_L);
    //02H数据//////////////////////////
    send_buf[j++]= ((char)0x02);
    send_buf[j++]= ((char)0x02);
    if( SEN_state[type].type!=SENPRESS)    //不是压力式
		{
			switch (SEN_state[type].last_sta[addr])   
			{
				case NORM://正常
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
				case WATER://水
					send_buf[j++]=(char)(NET_SENWATER);
					send_buf[j++]=(char)(NET_SENWATER>>8);
				break;
				case OIL://油
					send_buf[j++]=(char)(NET_SENOIL);
					send_buf[j++]=(char)(NET_SENOIL>>8);
				break;
				case HIGH://高液位
					send_buf[j++]=(char)(NET_LIQHIGH);
					send_buf[j++]=(char)(NET_LIQHIGH>>8);
				break;
				case LOW://低液位
					send_buf[j++]=(char)(NET_LIQLOW);
					send_buf[j++]=(char)(NET_LIQLOW>>8);
				break;
				case ERR://通讯故障
					send_buf[j++]=(char)(NET_COMMERR);
					send_buf[j++]=(char)(NET_COMMERR>>8);
				break;
				case 0xff://未开通
					send_buf[j++]=(char)(NET_NOTOPEN);
					send_buf[j++]=(char)(NET_NOTOPEN>>8);
				break;
				default:
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
			}
		}
		else    //压力式
		{
			switch (SEN_state[type].last_sta[addr])   
			{
				case NORM_PRESS://正常
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://压力预警
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://压力报警
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://油气预警
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://油气报警
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://通讯故障
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://未开通
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;

			}
		}
    //03H数据//////////////////////////
    send_buf[j++]= ((char)0x03);
    send_buf[j++]= ((char)0x01);
    send_buf[j++]= ((char)0x04);
    //04H数据//////////////////////////
    send_buf[j++]= ((char)0x04);
    send_buf[j++]= ((char)0x01);
    send_buf[j++]= ((char)DB_Ad_L-0x10); //13个
    if(DB_Ad_H==0x03)
    {
        //14H数据//////////////////////////
        send_buf[j++]= ((char)0x14);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0xff);
    }
    else
    {
        
        send_buf[j++]=((char)0x05);
        send_buf[j++]=((char)0x04);
        send_buf[j++]=((char)0x01);
        send_buf[j++]=((char)0x00);
        temp=(char)(SEN_state[type].press[addr]);
        temp=(temp/10*16 + temp%10);
        send_buf[j++]=((char)temp);
        send_buf[j++]=((char)0x00);
			//10H数据//////////////////////////
        send_buf[j++]= ((char)0x10);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0xaa);
        //11H数据//////////////////////////
        send_buf[j++]= ((char)0x11);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0xff);
        //12H数据//////////////////////////
        send_buf[j++]= ((char)0x12);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0x55);
        //13H数据//////////////////////////
        send_buf[j++]= ((char)0x13);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0x11);

    }
    send(sock, send_buf, j, 0);		
		j=0;
    rt_thread_delay(20);
    }
}

/**/

int TcpActive(int sock,rt_uint8_t *buff)  //无确认主动消息
{
	rt_uint8_t DB_Ad_H,DB_Ad_L;
	rt_uint8_t j=0;
	char send_buf[30];
	switch(buff[6])
	{
	case 0x02: //人井
		DB_Ad_H=0x02;
	break;
	case 0x01://管线
		DB_Ad_H=0x03;
	break;
	case 0x00://油罐
		DB_Ad_H=0x04;
	break;
	case 0x03://油盆
		DB_Ad_H=0x05;
	break;
	default:
			return -1;
	}
	DB_Ad_L=buff[7]+0x11;
  send_buf[j++]= ((char)0X02);
  send_buf[j++]= ((char)0X01);
  send_buf[j++]= ((char)0X07);
  send_buf[j++]= ((char)0X01);
  send_buf[j++]= ((char)0X00);
  send_buf[j++]= ((char)0x80);
  send_buf[j++]= ((char)0x00);
  send_buf[j++]= ((char)0x0D);
  send_buf[j++]= ((char)0x02);
  send_buf[j++]= ((char)DB_Ad_H);
  send_buf[j++]= ((char)DB_Ad_L);
  send_buf[j++]= ((char)0x64);
  send_buf[j++]= ((char)0x09);
  if( SEN_state[buff[6]].type!=SENPRESS)    //不是压力式
	{
			switch (buff[8])   
			{
				case NORM://正常
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
				case WATER://水
					send_buf[j++]=(char)(NET_SENWATER);
					send_buf[j++]=(char)(NET_SENWATER>>8);
				break;
				case OIL://油
					send_buf[j++]=(char)(NET_SENOIL);
					send_buf[j++]=(char)(NET_SENOIL>>8);
				break;
				case HIGH://高液位
					send_buf[j++]=(char)(NET_LIQHIGH);
					send_buf[j++]=(char)(NET_LIQHIGH>>8);
				break;
				case LOW://低液位
					send_buf[j++]=(char)(NET_LIQLOW);
					send_buf[j++]=(char)(NET_LIQLOW>>8);
				break;
				case ERR://通讯故障
					send_buf[j++]=(char)(NET_COMMERR);
					send_buf[j++]=(char)(NET_COMMERR>>8);
				break;
				case 0xff://未开通
					send_buf[j++]=(char)(NET_NOTOPEN);
					send_buf[j++]=(char)(NET_NOTOPEN>>8);
				break;
				default:
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
			}
		}
		else    //压力式
		{
			switch (buff[8])   
			{
				case NORM_PRESS://正常
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://压力预警
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://压力报警
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://油气预警
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://油气报警
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://通讯故障
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://未开通
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;

			}
		}
  send_buf[j++]= ((char)0x20);
	send_buf[j++]= ((char)((buff[0]/10)*16+buff[0]%10));
  send_buf[j++]= ((char)((buff[1]/10)*16+buff[1]%10));
  send_buf[j++]= ((char)((buff[2]/10)*16+buff[2]%10));
  send_buf[j++]= ((char)((buff[3]/10)*16+buff[3]%10));
  send_buf[j++]= ((char)((buff[4]/10)*16+buff[4]%10));
  send_buf[j++]= ((char)((buff[5]/10)*16+buff[5]%10));
  return send(sock, send_buf, j, 0);

}
/**/


/*网络主动消息及主动消息缓存部分*/

rt_uint8_t net_cache[NET_CACHE][9];   //缓存，循环覆盖记录，记录最新的NET_CACHE条报警信息
rt_uint8_t p_netca_h=0;   //指针，指向报警缓存最新一条
rt_uint8_t p_netca_n=0;   //指针，指向要上传的报警缓存
rt_uint8_t p_flowflag=0;  //溢出标志
/**/

/*主动信息缓存函数，探杆状态改变后调用函数存入缓存区*/
void Active_cache(rt_uint8_t type,rt_uint8_t  addr,rt_uint8_t state)   //探杆种类（油罐管线人井油盆），地址，标志位
{
	net_cache[p_netca_h][0]=sys_time[0];
	net_cache[p_netca_h][1]=sys_time[1];
	net_cache[p_netca_h][2]=sys_time[2];
	net_cache[p_netca_h][3]=sys_time[3];
	net_cache[p_netca_h][4]=sys_time[4];
	net_cache[p_netca_h][5]=sys_time[5];
	net_cache[p_netca_h][6]=type;
	net_cache[p_netca_h][7]=addr;
	net_cache[p_netca_h][8]=state;
	p_netca_h++;
	if(p_netca_h>NET_CACHE-1)p_netca_h=0;
	if(p_netca_h==p_netca_n)p_flowflag=1;    //溢出
}
/**/


/*主动信息缓存发送函数，调用后检测到网络正常发送主动信息*/
void Active_cache_deal(void)
{
	if(client_flag==3)       //连接标志位
	{
		while(1)
		{
			if(p_netca_h!=p_netca_n||p_flowflag==1)    //溢出或者两个指针位置不同时，发送
			{
				if(p_flowflag==1)    //溢出，调整指针位置。
				{
					p_flowflag=0;
					p_netca_n=p_netca_h;
				}
				TcpActive(tcpsock,net_cache[p_netca_n]);
				rt_thread_delay(2); 
//				rt_kprintf("time:%d:%d:%d.  type:%d addr:%d sta:%d\n",net_cache[p_netca_n][3],net_cache[p_netca_n][4],net_cache[p_netca_n][5], net_cache[p_netca_n][6], net_cache[p_netca_n][7],net_cache[p_netca_n][8]);
				p_netca_n++;
				if(p_netca_n>NET_CACHE-1)p_netca_n=0;
			}
			else
			{
				break;
			}
		}
	}
}
