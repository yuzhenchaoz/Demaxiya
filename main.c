/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
 /**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 
  ******************************************************************************
	*/
	
#include "main.h"

rt_sem_t usart_sem = RT_NULL;

rt_uint8_t sys_time[6];
	

/*串口1-串口2接收缓存数组*/
rt_uint8_t u1_Rx[40];				//探杆接收
rt_uint8_t u2_Rx[60];				//屏幕接收
rt_uint8_t u1_num=0;
rt_uint8_t u2_num=0;


/* 串口1事件控制块 */
//struct rt_event event_udprec;


/*DGUS线程，处理屏幕与处理器通讯*/
extern void DGUS_thread_entry(void* parameter);
/*语音线程*/
extern void Voice_thread_entry(void* parameter);
/*探杆查询*/
void modbus_thread_entry(void* parameter)
{
		rt_thread_delay(1800);
		
    while (1)
    {
			modbus();
			rt_thread_delay(5);   //while中的延时都必须要有，不能去掉
    }
}

/*串口1数据接收线程*/
void receive1_thread_entry(void* parameter)
{
    while (1)
    {
        /* 读数据 */
			uart1_receive();
			rt_thread_delay(5); 
    }
}
/*串口2数据接收线程*/
void receive2_thread_entry(void* parameter)
{
    while (1)
    {
        /* 读数据 */
			uart2_receive();
			rt_thread_delay(3); 
    }
}
/*报警 线程*/
void alert_thread_entry(void* parameter)
{
	rt_uint8_t i;
	rt_thread_delay(3000); 
	while (1)
    {
			for(i=0;i<30;i++)
			{
				alert_refresh();
				rt_thread_delay(300); 
				read_time();
				rt_thread_delay(200);
				Active_cache_deal();
			}
			wt_net_open();
			rt_thread_delay(2);
		}
}


int main(void) 
{
	  rt_pin_mode(41,PIN_MODE_OUTPUT); 
		rt_pin_mode(10,PIN_MODE_OUTPUT); 
		ReadFlash();
	
		uart1_open();
		uart2_open();	
		//创建信号量
		usart_sem = rt_sem_create("usart_sem",1,RT_IPC_FLAG_FIFO);
    rt_thread_t tid;
	
    /* 创建 receive1 线程 */
    tid = rt_thread_create("receive1",
                    receive1_thread_entry,
                    RT_NULL,
                    256,
                    11,
                    20);
    /* 创建成功则启动线程 */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
		/* 创建 receive2 线程 */
    tid = rt_thread_create("receive2",
                    receive2_thread_entry,
                    RT_NULL,
                    360,
                    12,
                    20);
    /* 创建成功则启动线程 */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
					  /* 创建 探杆检测 线程 */
    tid = rt_thread_create("modbus",
                    modbus_thread_entry,
                    RT_NULL,
                    1024,
                    6,
                    50);
		/* 创建成功则启动线程 */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
    /* 创建 DGUS 线程 */
    tid = rt_thread_create("DGUS",
                    DGUS_thread_entry,
                    RT_NULL,
                    360,
                    7,
                    50);
    /* 创建成功则启动线程 */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
		
    /* 创建 语音 线程 */
    tid = rt_thread_create("Voice",
                    Voice_thread_entry,
                    RT_NULL,
                    256,
                    9,
                    5);
		/* 创建成功则启动线程 */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);

		/* 创建 报警 线程 */
    tid = rt_thread_create("alert",
                    alert_thread_entry,
                    RT_NULL,
                    512,
                    13,
                    50);
		/* 创建成功则启动线程 */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
		
//		wt_net_open();

    return 0;
}
/**/

