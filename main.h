/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <rtthread.h>
#include <drivers/pin.h>
#include "dgus.h"	
#include "wt_modbus.h"
#include "userconfig.h"
#include "wt_netclient.h"
#include <sys/socket.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include <netdb.h>
#include <string.h>
#include <finsh.h>

		

/* 串口接收事件标志 */
#define UART_RX1_EVENT (1 << 2)
/*串口发送事件标志*/
#define UART_RX2_EVENT (1 << 1)

extern	rt_uint8_t u1_Rx[40];				//探杆接收用数组
extern  rt_uint8_t u2_Rx[60];				//屏幕接收用数组

extern  rt_sem_t	 usart_sem ;
extern 	struct  rt_event event_u1;
extern 	struct  rt_event event_u2;


rt_err_t uart1_open(void);
rt_err_t uart2_open(void);

void uart1_send(const rt_uint8_t *buf,rt_size_t size);
void uart2_send(const rt_uint8_t *buf,rt_size_t size);

void uart1_receive(void);
void uart2_receive(void);

void WriteFlash(void);	
void ReadFlash(void);

uint16_t CRC16(rt_uint8_t *CrcBuf,rt_uint8_t CrcLen);

extern rt_uint8_t sys_time[6];

void read_time(void);
void Active_cache_deal(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
