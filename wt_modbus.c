/**
  ******************************************************************************
  * File Name          : modbus.c
  *    ������ͨѶ
  ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "wt_modbus.h"


//		rt_thread_delay(1000);                //��ʱ 
//		uart1_send(text,sizeof(text)); //sizeof()�ɶ�ȡĿ�곤��
//
//		uart1_send(text,6); //sizeof()�ɶ�ȡĿ�곤��

	rt_uint8_t CRCH,CRCL;
/*
���ܣ��Ե�����Ӯ��ͨ̽�˽���һ�β�ѯ������ֵ��������״̬,����ֵ���͹޹����˾����裬��ַ
*/
rt_uint8_t ordinaryquery_wt(rt_uint8_t addr_h , rt_uint8_t addr_l)    //������ʽ   00 03 04 00 01 00 00 CRCH CRCL
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //����������ѯ��������Ӯ
	
	for(i=0;i<40;i++)   //�建��
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	(addr_h<<4)+addr_l;
	modbus_send[1]	=	03;
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc��
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6));//crc��
	
	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//��ȡ�����ź���
	uart1_send(modbus_send,8);    //��ѯ
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//�����ź���	
		return ERR;    //��ʱ����
	}
	if((u1_Rx[0]>0x50)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //�յ����ݣ��ж��Ƿ�Ϊ�����̽�˷��ش�������
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //�յ��������ݣ��ٲ�ѯһ��
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x50)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//�����ź���	
			return ERR;       //�յ��������ݻ��߳�ʱ������err
		}
	}
	rt_sem_release(usart_sem);					//�����ź���	
	
	/*�����ݽ����ж�*/
		CRCH = CRC16(u1_Rx,7)>>8;
		CRCL = CRC16(u1_Rx,7);
		if((u1_Rx[8] == CRCL)&&(u1_Rx[7] == CRCH))     //CRCͨ��
		{
			if(addr_h!=0)   //�����͹޵�̽�˱�־λ�ڵ�7λ
			{
				u1_Rx[4]=u1_Rx[6];
			}
			switch(u1_Rx[4])
			{
				case 1:
					return NORM;
				case 2:
					return WATER;
				case 5:
					return OIL;
				default:
					return ERR;
			}
		}
	return ERR;
}
/**/

/*
���ܣ��Ե�����ӮҺý̽�˽���һ�β�ѯ������ֵ��������״̬,����ֵ���͹޹����˾����裬��ַ
*/
rt_uint8_t liqmediumquery_wt(rt_uint8_t addr_l)    
{
	rt_uint8_t change;
	change=ordinaryquery_wt(0,addr_l);   //������ͨ̽�˲�ѯ��Ȼ��ת��һ��
	switch(change)
	{
		case NORM:
			change=NORM;
		break;
		case WATER:
			change=HIGH;
		break;
		case OIL:
			change=LOW;
		break;
	}
	return change;
}
/**/

/*
���ܣ��Ե�����Ӯѹ��̽�˽���һ�β�ѯ������ֵ��ѹ��,����ֵ����ַ
*/
rt_uint8_t pressurequery_wt(rt_uint8_t addr_l)    //ѹ��ʽ  01 04 04 00 50 00 4F CRCL CRCH   1�ŵ�ַΪ1
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //����������ѯ��������Ӯ
	rt_uint16_t pre;
	for(i=0;i<40;i++)   //�建��
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	addr_l+1;
	modbus_send[1]	=	04;
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc��
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6));//crc��
	
	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//��ȡ�����ź���
	uart1_send(modbus_send,8);    //��ѯ
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//�����ź���	
		return ERR_PRESS;    //��ʱ����
	}
	if((u1_Rx[0]>0x20)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //�յ����ݣ��ж��Ƿ�Ϊ�����̽�˷��ش�������
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //�յ��������ݣ��ٲ�ѯһ��
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x20)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//�����ź���	
			return ERR_PRESS;       //�յ��������ݻ��߳�ʱ������err
		}
	}
	rt_sem_release(usart_sem);					//�����ź���	
	
	/*�����ݽ����ж�*/
		CRCH = CRC16(u1_Rx,7)>>8;
		CRCL = CRC16(u1_Rx,7);
		if((u1_Rx[7] == CRCL)&&(u1_Rx[8] == CRCH))     //CRCͨ��
		{
			u1_Rx[4]=u1_Rx[4]-SEN_state[0].press_d[addr_l]+SEN_state[0].press_u[addr_l];
			if(u1_Rx[4]>99&&SEN_state[0].press_d[addr_l]>0)  //�ж�Ϊ�������
			{
				u1_Rx[4]=0;
			}
			if(u1_Rx[4]>99/*&&SEN_state[i].press_u[j]>0*/)  //�ж�Ϊ�������
			{
				u1_Rx[4]=99;
			}
			pre=(rt_uint16_t)(u1_Rx[4])*10+(rt_uint16_t)(u1_Rx[6])/10;
			DGUS_numsend(0x22,addr_l<<1,(rt_uint8_t)(pre>>8),(rt_uint8_t)pre);
			if(u1_Rx[4]>SEN_state[0].press[addr_l])     //���Ϊ����ѹ���ٴ����˲�
			{
				if(u1_Rx[4]-SEN_state[0].press[addr_l]>1)
				{
					SEN_state[0].press[addr_l]=u1_Rx[4];
					DGUS_numsend(0x24,addr_l,0,100-u1_Rx[4]);
					DGUS_numsend(0x24,addr_l,0,100-u1_Rx[4]);
				}
			}
			else
			{
				if(SEN_state[0].press[addr_l]-u1_Rx[4]>1)
				{
					SEN_state[0].press[addr_l]=u1_Rx[4];
					DGUS_numsend(0x24,addr_l,0,100-u1_Rx[4]);
					DGUS_numsend(0x24,addr_l,0,100-u1_Rx[4]);
				}
			}
			/* */
			
			if(SEN_state[0].press[addr_l]<=press_lim)					//�ж�ѹ������״̬
			{
				return LOW_PRESS;
			}
			else
			{
				return NORM_PRESS;
			}
		}
	return ERR_PRESS;
}
/**/

/*
���ܣ��Ե���������ͨ̽�˽���һ�β�ѯ������ֵ��������״̬,����ֵ���͹޹����˾����裬��ַ
*/
rt_uint8_t ordinaryquery_cppei(rt_uint8_t addr_h , rt_uint8_t addr_l)     
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x00,0x00};		//���Ͳ�ѯ	
	
	for(i=0;i<40;i++)   //�建��
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	(addr_h*20)+addr_l;
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6));//crc��
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc��

	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//��ȡ�����ź���
	uart1_send(modbus_send,8);    //��ѯ
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//�����ź���	
		return ERR;    //��ʱ����
	}
	if((u1_Rx[0]>0x60)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //�յ����ݣ��ж��Ƿ�Ϊ�����̽�˷��ش�������
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //�յ��������ݣ��ٲ�ѯһ��
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x60)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//�����ź���	
			return ERR;       //�յ��������ݻ��߳�ʱ������err
		}
	}
	rt_sem_release(usart_sem);					//�����ź���	
	
	/*�����ݽ����ж�*/
		CRCH=CRC16(u1_Rx,5)>>8;
		CRCL=CRC16(u1_Rx,5);
		if((u1_Rx[5] == CRCL )&&(u1_Rx[6] == CRCH))     //CRCͨ��
		{
			switch(u1_Rx[4])
			{
				case 0:
					return NORM;
				case 1:
					return WATER;
				case 2:
					return OIL;
				case 3:
					return HIGH;
				case 4:
					return LOW;
				default:
					return ERR;
			}
		}
	return ERR;
}
/**/

/*
���ܣ�����һ��̽�˵Ĳ�ѯ�����룺̽�����ͣ�̽��Э�飬�͹޹����˾����裬��ַ
*/
void sensor_query(rt_uint8_t type , rt_uint8_t protocols , rt_uint8_t addr_h , rt_uint8_t addr_l)     
{
	rt_uint8_t flag;
	if(protocols==QDWT)     //�ൺ��ӮЭ��
	{
		if(type==SENPRESS)     //ѹ��ʽ������
		{
			flag=pressurequery_wt(addr_l);   //1
			if(flag==ERR_PRESS)
			{
				flag=pressurequery_wt(addr_l);   //2
				if(flag==ERR_PRESS)
				{
					flag=pressurequery_wt(addr_l);   //3
				}
			}
			SEN_state[0].state[addr_l]=flag;     
			
			if(flag==ERR_PRESS)
			{
				SEN_state[0].press[addr_l]=0;
				DGUS_numsend(0x22,addr_l<<1,0,0);
				DGUS_numsend(0x24,addr_l,0,100);
			}
		}
		if(type==SENNORM)     //����������
		{
			flag=ordinaryquery_wt(addr_h,addr_l);   //1
			if(flag==ERR)
			{
				flag=ordinaryquery_wt(addr_h,addr_l);   //2
				if(flag==ERR)
				{
					flag=ordinaryquery_wt(addr_h,addr_l);   //3
				}
			}
			SEN_state[addr_h].state[addr_l]=flag;
		}
		if(type==SENYEMEI)     //Һýʽ������
		{
			flag=liqmediumquery_wt(addr_l);   //1
			if(flag==ERR)
			{
				flag=liqmediumquery_wt(addr_l);   //2
				if(flag==ERR)
				{
					flag=liqmediumquery_wt(addr_l);   //3
				}
			}
			SEN_state[0].state[addr_l]=flag;
		}
	}
	if(protocols==CPPEI)     //����Э��
	{
		if(type==SENPRESS)     //ѹ��ʽ������
		{
			flag=pressurequery_wt(addr_l);   //1
			if(flag==ERR_PRESS)
			{
				flag=pressurequery_wt(addr_l);   //2
				if(flag==ERR_PRESS)
				{
					flag=pressurequery_wt(addr_l);   //3
				}
			}

			SEN_state[0].state[addr_l]=flag;
			if(flag==ERR_PRESS)
			{
				SEN_state[0].press[addr_l]=0;
				DGUS_numsend(0x22,addr_l<<1,0,0);
				DGUS_numsend(0x24,addr_l,0,100);
			}
		}
		else
		{
			flag=ordinaryquery_cppei(addr_h,addr_l);   //1
			if(flag==ERR)
			{
				flag=ordinaryquery_cppei(addr_h,addr_l);   //2
				if(flag==ERR)
				{
					flag=ordinaryquery_cppei(addr_h,addr_l);   //3
				}
			}
			SEN_state[addr_h].state[addr_l]=flag;
		}
	}
}
/**/

/*
���ܣ���Բ�ͬ���͵ĵ���̽����ɲ�ѯ��������Ӧ̽��״̬
*/
void modbus(void)
{
	rt_uint8_t i,j;
	for(i=0;i<4;i++)					//�͹޹����˾�����ѭ����ѯ
	{
		if(SEN_state[i].int_num!=0)		//�������������Ϊ0����ʼ��ѯ
		{
			for(j=0;j<SEN_state[i].int_num;j++)		//����ͨ������ѯ	
			{
				sensor_query(SEN_state[i].type,SEN_state[i].protocols,i,j);
			/*-----------�Ա�־λ����------------*/		
				if(SEN_state[i].state[j]!=SEN_state[i].last_sta[j])				//״̬��־λ��
				{
					SEN_state[i].last_sta[j]=SEN_state[i].state[j];		//����˴�״̬
					SEN_state[i].update_f=1;													//̽��״̬������
					Active_cache(i,j,SEN_state[i].last_sta[j]);       //���汨��״̬�����绺��
					if(SEN_state[i].state[j]!=NORM)
					{
						SEN_state[i].alert[j]++;
					}
				}
			/*-----------��һ��̽�˲�ѯ����------------*/		
			}
			for(;j<16;j++)		//����ͨ������ѯ	
			{
				SEN_state[i].last_sta[j]=SEN_state[i].state[j]=0xFF;   //δ��ͨ
			}
		}
	/*-----------��һ�����̽�˲�ѯ����------------*/		
	}
	/*-----------������̽�˲�ѯ����------------*/			
}
/**/
