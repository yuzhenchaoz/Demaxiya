/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WT_NETCLIENT_H
#define __WT_NETCLIENT_H

#include <rtthread.h>
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#include "main.h"
#include "w5500.h"
	
///*udp�����¼���־*/	
//#define UDP_REC_EVENT (1 << 3)

	
extern	rt_uint8_t local_ip[4];
extern	rt_uint8_t local_gw[4];
extern	rt_uint8_t local_sn[4];
//extern  rt_uint8_t client_flag;

extern int tcpsock;	

//extern 	struct  rt_event event_udprec;

void Active_cache(rt_uint8_t type,rt_uint8_t  addr,rt_uint8_t state);

void wt_net_open(void);
//void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L); //ȷ����Ϣ
//void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID); //��̽�˵�����Ӧ����Ϣ
//void TcpallReply(int sock,rt_uint8_t DB_Ad_H);//��̽��ȫ������Ӧ����Ϣ

#endif /* __WT_NETCLIENT_H */
	
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
