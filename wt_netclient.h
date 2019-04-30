/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WT_NETCLIENT_H
#define __WT_NETCLIENT_H

#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#include "main.h"
#include "w5500.h"
	
///*udp接收事件标志*/	
//#define UDP_REC_EVENT (1 << 3)

	
extern	rt_uint8_t local_ip[4];
extern	rt_uint8_t local_gw[4];
extern	rt_uint8_t local_sn[4];
//extern  rt_uint8_t client_flag;

extern int tcpsock;	

//extern 	struct  rt_event event_udprec;

void Active_cache(rt_uint8_t type,rt_uint8_t  addr,rt_uint8_t state);

void wt_net_open(void);
//void TcpConfirm(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t DB_Ad_L); //确认消息
//void TcponlyReply(int sock,rt_uint8_t DB_Ad_H,rt_uint8_t  DB_Ad_L,rt_uint8_t DataID); //单探杆单数据应答消息
//void TcpallReply(int sock,rt_uint8_t DB_Ad_H);//单探杆全部数据应答消息

#endif /* __WT_NETCLIENT_H */
	
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
