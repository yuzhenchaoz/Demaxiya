/*
 * Copyright (c) 2019
 *
 * Change Logs:
 * Date             Author      Notes
 *  ���粿�֣�
 */

#include "wt_netclient.h"
#include "wizchip_socket.h"
#include <wiz.h>

#define TCPBUFSZ   512
#define UDPBUFSZ   256




#ifdef USE_DEBUG

/*������ip*/
rt_uint8_t local_ip[4]={192,168,1,48};       //����ip    //���˵ײ㣬��wiz.c  360������
rt_uint8_t local_gw[4]={192,168,1,1};				//����
rt_uint8_t local_sn[4]={255,255,255,0};				//�ӹ�����
#else

/*�ֳ���ip*/
rt_uint8_t local_ip[4]={10,196,40,38};       //����ip    //���˵ײ㣬��wiz.c  360������
rt_uint8_t local_gw[4]={10,196,40,33};				//����
rt_uint8_t local_sn[4]={255,255,255,240};				//�ӹ�����
#endif

//		rt_thread_delay(1000);                //��ʱ  1/RT_TICK_PER_SECOND ��   RT_TICK_PER_SECOND Ϊϵͳ���ڣ���Ϊ500

rt_uint8_t client_ip[4]={192,168,1,0};    //��¼Ҫ�������ӵ�ip

rt_uint8_t client_flag = 0;        //�������ӱ�־λ��0���������ӣ�1����UDP���գ�2����UDP���ͣ�3����TCP���ӣ��������ޣ��벻��������־λ����õĽ���취


rt_uint8_t net_ico[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x11, 0x00, 0x00};           //��������ͼ��

int tcpsock;
int udpresock;
int udpsesock;

rt_thread_t tid_tcpcli=RT_NULL;
rt_thread_t tid_udpsend=RT_NULL;
rt_thread_t tid_udprec=RT_NULL;

void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L); //ȷ����Ϣ
void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID); //��̽�˵�����Ӧ����Ϣ
void TcpallReply(int sock,rt_uint8_t DB_Ad_H);//��̽��ȫ������Ӧ����Ϣ


//url  ����IP��������  //port �˿�  
void udpsend(const rt_uint8_t *addr,const char *url,const char *udpport,rt_tick_t time)
{
    int port;
    struct hostent *host;
    struct sockaddr_in server_addr;
	
		char udpbeat_data[10] = {192,168,1,177,0x0D,0x9E,7,1,1,1}; /* �����õ������� */		
		
		udpbeat_data[0] = addr[0];
		udpbeat_data[1] = addr[1];
		udpbeat_data[2] = addr[2];
		udpbeat_data[3] = addr[3];
	
		time=time*RT_TICK_PER_SECOND;
		
	  port = strtoul(udpport, 0, 10);
	
    /* ͨ��������ڲ���url���host��ַ��������������������������� */
    host = (struct hostent *) gethostbyname(url);
    /* ����һ��socket��������SOCK_DGRAM��UDP���� */
    if ((udpsesock = socket(AF_WIZ,SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        return;
    }
    /* ��ʼ��Ԥ���ӵķ���˵�ַ */
    server_addr.sin_family = AF_WIZ;
    server_addr.sin_port = htons(port);    //9e 0d
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);     //ff01a8c0
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
      /* �������ݵ�����Զ�� */
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

/*udp�����������߳�*/
void udpsend_thread_entry(void* parameter)
{
	udpsend(local_ip,UDPBEATIP,UDPBEATPORT,UDPBEATTIM);   	 //����ip��Ŀ��IP��ַ�������� �˿� ��ʱʱ�䣨s��
	
	net_ico[7]=2;
	uart2_send(net_ico,8);    //������������ʧ��ͼ��
	if((*tid_tcpcli).stat==2) rt_thread_delete(tid_tcpcli);
	if((*tid_udprec).stat==2) rt_thread_delete(tid_udprec);
	wiz_socket_init();
	client_flag=0;
	rt_kprintf("UDP send error\n");
	return;
}
/**/

//static const char tcpsend_data[] = "This is TCP Client from RT-Thread."; /* �����õ������� */
void tcpclient(const char *url, const char *tcpport)
{
//    int i;
    static char *recv_data;
    struct hostent *host;      //������ַ��������Ϣ
    int bytes_received;
    struct sockaddr_in server_addr;   //IP��ַ�Ͷ˿���Ϣ
    int port;
    port = strtoul(tcpport, 0, 10);
    /* ͨ��������ڲ���url���host��ַ��������������������������� */
    host = gethostbyname(url);
    /* �������ڴ�Ž������ݵĻ��� */
		if (recv_data == RT_NULL)
		{
				recv_data = rt_malloc(TCPBUFSZ);
		}
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ����һ��socket��������SOCKET_STREAM��TCP���� */
    if ((tcpsock = socket(AF_WIZ, SOCK_STREAM, 0)) == -1)
    {
        /* ����socketʧ�� */
        rt_kprintf("Socket error\n");
        /* �ͷŽ��ջ��� */
//        rt_free(recv_data);
        return;
    }
    /* ��ʼ��Ԥ���ӵķ���˵�ַ */
    server_addr.sin_family = AF_WIZ;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ���ӵ������ */
		
    if (connect(tcpsock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ����ʧ�� */
        rt_kprintf("Connect fail!\n");
        /*�ͷŽ��ջ��� */
//        rt_free(recv_data);
        return;
    }
//		IINCHIP_WRITE(Sn_KPALVTR(tcpsock),0x01);
		client_flag=3;
		net_ico[7]=1;
		uart2_send(net_ico,8);
    while (1)
    {
        /* ��sock�����н������BUFSZ - 1�ֽ����� */
        bytes_received = recv(tcpsock, recv_data, TCPBUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            /* ����ʧ�ܣ��ر�������� */
            rt_kprintf("\n tcp received error.\r\n");
            /* �ͷŽ��ջ��� */
//            rt_free(recv_data);
            break;
        }
        else if (bytes_received == 0)
        {
            /* Ĭ�� recv Ϊ����ģʽ����ʱ�յ�0��Ϊ���ӳ����ر�������� */
            rt_kprintf("\n tcp received error.\r\n");
            /* �ͷŽ��ջ��� */
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

/*tcp�߳�*/
void tcpcli_thread_entry(void* parameter)
{
	rt_uint32_t ip;
	ip=client_ip[3];
	ip=client_ip[2]+(ip<<8);
	ip=client_ip[1]+(ip<<8);
	ip=client_ip[0]+(ip<<8);
	rt_thread_delay(50); 
	tcpclient(inet_ntoa(ip),"8080");   	 //Ŀ��IP��ַ�������� �˿�
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
    /* ��������õ����ݻ��� */

		if (recv_data == RT_NULL)
		{
				recv_data = rt_malloc(UDPBUFSZ);
		}
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");        /* �����ڴ�ʧ�ܣ����� */
        return;
    }
    /* ����һ��socket��������SOCK_DGRAM��UDP���� */
    if ((udpresock = socket(AF_WIZ, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        /* �ͷŽ����õ����ݻ��� */
//        rt_free(recv_data);
        return;
    }
    /* ��ʼ������˵�ַ */
    server_addr.sin_family = AF_WIZ;
//    server_addr.sin_port = htons(3486);     //UDP���ն˿ڲ������������
		wizchip_socket(udpresock, Sn_MR_UDP, 3486, 0);				//����UDP���ն˿�
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* ��socket������˵�ַ */
    if (bind(udpresock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        /* �󶨵�ַʧ�� */
        rt_kprintf("Bind error\n");

        /* �ͷŽ����õ����ݻ��� */
//        rt_free(recv_data);
        return;
    }

    addr_len = sizeof(struct sockaddr);
		/* ��ʼ���¼����� */
//    rt_event_init(&event_udprec, "udpevent", RT_IPC_FLAG_FIFO); 
    while (1)
    {
        /* ��sock����ȡ���BUFSZ - 1�ֽ����� */
        bytes_read = recvfrom(udpresock, recv_data, UDPBUFSZ - 1, 0,
                              (struct sockaddr *)&client_addr, &addr_len);
        /* UDP��ͬ��TCP�����������������ȡ������ʧ�ܵ���������������˳�ʱ�ȴ� */
			if(bytes_read==10)       //c0 a8 01 7b 0d 9e 01 00 01 01
			{
        /* �յ������� */
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

						/* ���� tcp �߳� */
						tid_tcpcli = rt_thread_create("tcpcli",
														tcpcli_thread_entry,
														RT_NULL,
														1536,
														16,
														20);
						/* �����ɹ��������߳� */
						if (tid_tcpcli != RT_NULL)
							rt_thread_startup(tid_tcpcli);
					}
        }
			}
    }
}
/**/

/*w5500udp���յ����˵ײ����ö˿�*/
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
/*�������̣߳��ȿ�UDP�����ٿ�UDP����*/
void wt_net_open(void)
{
	rt_uint8_t flag; 
	if(wiz_init_ok == RT_FALSE)     //��ʼ��ʧ�ܣ�����Ϊû����������
	{
		wiz_init();
	}
	else                            //��ʼ���ɹ�
	{
		if(client_flag==0)            //���������ϵ�û�к��������
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
		/* ���� ���� �߳� */
			tid_udprec = rt_thread_create("udpreceive",
											udpreceive_thread_entry,
											RT_NULL,
											1024,
											15,
											20);
			/* �����ɹ��������߳� */
			if (tid_udprec != RT_NULL)
				 rt_thread_startup(tid_udprec);
			
			/* ���� UDP���������� �߳� */
				tid_udpsend = rt_thread_create("udpsend",
												udpsend_thread_entry,
												RT_NULL,
												1536,
												14,
												20);
				/* �����ɹ��������߳� */
			if (tid_udpsend != RT_NULL)
				rt_thread_startup(tid_udpsend);
			
		}
		if(client_flag==3)         //���������������
		{
			flag=0;
			flag=send(tcpsock, &flag, 1, 0);    //����0����ֹTCP�׽��ֿ�����ͬʱ�ж�TCP�����Ƿ�Ͽ�
#ifdef USE_DEBUG
			rt_kprintf(" %d \n",count++);
#endif
			if(flag!=1)                         //����ʧ�ܣ�˵�����ӳ����⣬��������
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
����Э�鲿�֣�
*/


void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L) //ȷ����Ϣ
{
	 char send_buf[12]={1,0,7,1,0,0xE0,0,4,2,0,0,0};
	 send_buf[9]=DB_Ad_H;
	 send_buf[10]=DB_Ad_L;
   send(sock, send_buf, 12, 0);
}
/**/

void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID) //��̽�˵�����Ӧ����Ϣ
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
		case 0x02: //�˾�
			addr=DB_Ad_L-0x11;
			type=2;
		break;
		case 0x03://����
			addr=DB_Ad_L-0x11;
			type=1;
		break;
		case 0x04://�͹�
			addr=DB_Ad_L-0x11;
			type=0;
		break;
		case 0x05://����
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

			if( SEN_state[type].type!=SENPRESS)    //����ѹ��ʽ
			{
				switch (SEN_state[type].last_sta[addr])   
				{
					case NORM://����
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
					case WATER://ˮ
						send_buf[j++]=(char)(NET_SENWATER);
						send_buf[j++]=(char)(NET_SENWATER>>8);
					break;
					case OIL://��
						send_buf[j++]=(char)(NET_SENOIL);
						send_buf[j++]=(char)(NET_SENOIL>>8);
					break;
					case HIGH://��Һλ
						send_buf[j++]=(char)(NET_LIQHIGH);
						send_buf[j++]=(char)(NET_LIQHIGH>>8);
					break;
					case LOW://��Һλ
						send_buf[j++]=(char)(NET_LIQLOW);
						send_buf[j++]=(char)(NET_LIQLOW>>8);
					break;
					case ERR://ͨѶ����
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://δ��ͨ
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
				}
			}
			else    //ѹ��ʽ
			{
				switch (SEN_state[type].last_sta[addr])   
				{
					case NORM_PRESS://����
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://ѹ��Ԥ��
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://ѹ������
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://����Ԥ��
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://��������
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://ͨѶ����
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://δ��ͨ
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

void TcpallReply(int sock,rt_uint8_t DB_Ad_H)//��̽��ȫ������Ӧ����Ϣ
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
			case 0x02: //�˾�
				addr=DB_Ad_L-0x11;
				type=2;
			break;
			case 0x03://����
				addr=DB_Ad_L-0x11;
				type=1;
			break;
			case 0x04://�͹�
				addr=DB_Ad_L-0x11;
				type=0;
			break;
			case 0x05://����
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
    send_buf[j++]= ((char)0X00);//Ӧ����Ϣ
    send_buf[j++]= ((char)0x20);//����

    send_buf[j++]= ((char)0x00);//��Ϣ����H
    if(DB_Ad_H==0x03)
    {
			send_buf[j++]= ((char)0x10);//��Ϣ����L
    }
    else
    {
			send_buf[j++]= ((char)0x1F);//��Ϣ����L
    }
    //���ݿ��ַ/////////////////////////
    send_buf[j++]= ((char)0x02);
    send_buf[j++]= ((char)DB_Ad_H);
    send_buf[j++]= ((char)DB_Ad_L);
    //02H����//////////////////////////
    send_buf[j++]= ((char)0x02);
    send_buf[j++]= ((char)0x02);
    if( SEN_state[type].type!=SENPRESS)    //����ѹ��ʽ
		{
			switch (SEN_state[type].last_sta[addr])   
			{
				case NORM://����
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
				case WATER://ˮ
					send_buf[j++]=(char)(NET_SENWATER);
					send_buf[j++]=(char)(NET_SENWATER>>8);
				break;
				case OIL://��
					send_buf[j++]=(char)(NET_SENOIL);
					send_buf[j++]=(char)(NET_SENOIL>>8);
				break;
				case HIGH://��Һλ
					send_buf[j++]=(char)(NET_LIQHIGH);
					send_buf[j++]=(char)(NET_LIQHIGH>>8);
				break;
				case LOW://��Һλ
					send_buf[j++]=(char)(NET_LIQLOW);
					send_buf[j++]=(char)(NET_LIQLOW>>8);
				break;
				case ERR://ͨѶ����
					send_buf[j++]=(char)(NET_COMMERR);
					send_buf[j++]=(char)(NET_COMMERR>>8);
				break;
				case 0xff://δ��ͨ
					send_buf[j++]=(char)(NET_NOTOPEN);
					send_buf[j++]=(char)(NET_NOTOPEN>>8);
				break;
				default:
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
			}
		}
		else    //ѹ��ʽ
		{
			switch (SEN_state[type].last_sta[addr])   
			{
				case NORM_PRESS://����
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://ѹ��Ԥ��
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://ѹ������
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://����Ԥ��
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://��������
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://ͨѶ����
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://δ��ͨ
						send_buf[j++]=(char)(NET_NOTOPEN);
						send_buf[j++]=(char)(NET_NOTOPEN>>8);
					break;
					default:
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;

			}
		}
    //03H����//////////////////////////
    send_buf[j++]= ((char)0x03);
    send_buf[j++]= ((char)0x01);
    send_buf[j++]= ((char)0x04);
    //04H����//////////////////////////
    send_buf[j++]= ((char)0x04);
    send_buf[j++]= ((char)0x01);
    send_buf[j++]= ((char)DB_Ad_L-0x10); //13��
    if(DB_Ad_H==0x03)
    {
        //14H����//////////////////////////
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
			//10H����//////////////////////////
        send_buf[j++]= ((char)0x10);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0xaa);
        //11H����//////////////////////////
        send_buf[j++]= ((char)0x11);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0xff);
        //12H����//////////////////////////
        send_buf[j++]= ((char)0x12);
        send_buf[j++]= ((char)0x01);
        send_buf[j++]= ((char)0x55);
        //13H����//////////////////////////
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

int TcpActive(int sock,rt_uint8_t *buff)  //��ȷ��������Ϣ
{
	rt_uint8_t DB_Ad_H,DB_Ad_L;
	rt_uint8_t j=0;
	char send_buf[30];
	switch(buff[6])
	{
	case 0x02: //�˾�
		DB_Ad_H=0x02;
	break;
	case 0x01://����
		DB_Ad_H=0x03;
	break;
	case 0x00://�͹�
		DB_Ad_H=0x04;
	break;
	case 0x03://����
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
  if( SEN_state[buff[6]].type!=SENPRESS)    //����ѹ��ʽ
	{
			switch (buff[8])   
			{
				case NORM://����
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
				case WATER://ˮ
					send_buf[j++]=(char)(NET_SENWATER);
					send_buf[j++]=(char)(NET_SENWATER>>8);
				break;
				case OIL://��
					send_buf[j++]=(char)(NET_SENOIL);
					send_buf[j++]=(char)(NET_SENOIL>>8);
				break;
				case HIGH://��Һλ
					send_buf[j++]=(char)(NET_LIQHIGH);
					send_buf[j++]=(char)(NET_LIQHIGH>>8);
				break;
				case LOW://��Һλ
					send_buf[j++]=(char)(NET_LIQLOW);
					send_buf[j++]=(char)(NET_LIQLOW>>8);
				break;
				case ERR://ͨѶ����
					send_buf[j++]=(char)(NET_COMMERR);
					send_buf[j++]=(char)(NET_COMMERR>>8);
				break;
				case 0xff://δ��ͨ
					send_buf[j++]=(char)(NET_NOTOPEN);
					send_buf[j++]=(char)(NET_NOTOPEN>>8);
				break;
				default:
					send_buf[j++]=(char)0x00;
					send_buf[j++]=(char)0x00;
				break;
			}
		}
		else    //ѹ��ʽ
		{
			switch (buff[8])   
			{
				case NORM_PRESS://����
						send_buf[j++]=(char)0x00;
						send_buf[j++]=(char)0x00;
					break;
//					case 6://ѹ��Ԥ��
//							send_buf[11]=(char)0x01);
//							send_buf[11]=(char)0x00);
//									break;
					case LOW_PRESS://ѹ������
						send_buf[j++]=(char)(NET_PRESSWARN);
						send_buf[j++]=(char)(NET_PRESSWARN>>8);
					break;
//					case 8://����Ԥ��
//							send_buf[11]=(char)0x04);
//							send_buf[11]=(char)0x00);
//									break;
//					case 9://��������
//							send_buf[11]=(char)0x08);
//							send_buf[11]=(char)0x00);
//									break;
					case ERR_PRESS://ͨѶ����
						send_buf[j++]=(char)(NET_COMMERR);
						send_buf[j++]=(char)(NET_COMMERR>>8);
					break;
					case 0xff://δ��ͨ
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


/*����������Ϣ��������Ϣ���沿��*/

rt_uint8_t net_cache[NET_CACHE][9];   //���棬ѭ�����Ǽ�¼����¼���µ�NET_CACHE��������Ϣ
rt_uint8_t p_netca_h=0;   //ָ�룬ָ�򱨾���������һ��
rt_uint8_t p_netca_n=0;   //ָ�룬ָ��Ҫ�ϴ��ı�������
rt_uint8_t p_flowflag=0;  //�����־
/**/

/*������Ϣ���溯����̽��״̬�ı����ú������뻺����*/
void Active_cache(rt_uint8_t type,rt_uint8_t  addr,rt_uint8_t state)   //̽�����ࣨ�͹޹����˾����裩����ַ����־λ
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
	if(p_netca_h==p_netca_n)p_flowflag=1;    //���
}
/**/


/*������Ϣ���淢�ͺ��������ú��⵽������������������Ϣ*/
void Active_cache_deal(void)
{
	if(client_flag==3)       //���ӱ�־λ
	{
		while(1)
		{
			if(p_netca_h!=p_netca_n||p_flowflag==1)    //�����������ָ��λ�ò�ͬʱ������
			{
				if(p_flowflag==1)    //���������ָ��λ�á�
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
