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
	

/*����1-����2���ջ�������*/
rt_uint8_t u1_Rx[40];				//̽�˽���
rt_uint8_t u2_Rx[60];				//��Ļ����
rt_uint8_t u1_num=0;
rt_uint8_t u2_num=0;


/* ����1�¼����ƿ� */
//struct rt_event event_udprec;


/*DGUS�̣߳�������Ļ�봦����ͨѶ*/
extern void DGUS_thread_entry(void* parameter);
/*�����߳�*/
extern void Voice_thread_entry(void* parameter);
/*̽�˲�ѯ*/
void modbus_thread_entry(void* parameter)
{
		rt_thread_delay(1800);
		
    while (1)
    {
			modbus();
			rt_thread_delay(5);   //while�е���ʱ������Ҫ�У�����ȥ��
    }
}

/*����1���ݽ����߳�*/
void receive1_thread_entry(void* parameter)
{
    while (1)
    {
        /* ������ */
			uart1_receive();
			rt_thread_delay(5); 
    }
}
/*����2���ݽ����߳�*/
void receive2_thread_entry(void* parameter)
{
    while (1)
    {
        /* ������ */
			uart2_receive();
			rt_thread_delay(3); 
    }
}
/*���� �߳�*/
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
		//�����ź���
		usart_sem = rt_sem_create("usart_sem",1,RT_IPC_FLAG_FIFO);
    rt_thread_t tid;
	
    /* ���� receive1 �߳� */
    tid = rt_thread_create("receive1",
                    receive1_thread_entry,
                    RT_NULL,
                    256,
                    11,
                    20);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
		/* ���� receive2 �߳� */
    tid = rt_thread_create("receive2",
                    receive2_thread_entry,
                    RT_NULL,
                    360,
                    12,
                    20);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
					  /* ���� ̽�˼�� �߳� */
    tid = rt_thread_create("modbus",
                    modbus_thread_entry,
                    RT_NULL,
                    1024,
                    6,
                    50);
		/* �����ɹ��������߳� */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
    /* ���� DGUS �߳� */
    tid = rt_thread_create("DGUS",
                    DGUS_thread_entry,
                    RT_NULL,
                    360,
                    7,
                    50);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
		
    /* ���� ���� �߳� */
    tid = rt_thread_create("Voice",
                    Voice_thread_entry,
                    RT_NULL,
                    256,
                    9,
                    5);
		/* �����ɹ��������߳� */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);

		/* ���� ���� �߳� */
    tid = rt_thread_create("alert",
                    alert_thread_entry,
                    RT_NULL,
                    512,
                    13,
                    50);
		/* �����ɹ��������߳� */
    if (tid != RT_NULL)
			 rt_thread_startup(tid);
		
		
//		wt_net_open();

    return 0;
}
/**/

