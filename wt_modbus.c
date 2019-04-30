/**
  ******************************************************************************
  * File Name          : modbus.c
  *    传感器通讯
  ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "wt_modbus.h"


//		rt_thread_delay(1000);                //延时 
//		uart1_send(text,sizeof(text)); //sizeof()可读取目标长度
//
//		uart1_send(text,6); //sizeof()可读取目标长度

	rt_uint8_t CRCH,CRCL;
/*
功能：对单根汇赢普通探杆进行一次查询，返回值，传感器状态,输入值，油罐管线人井油盆，地址
*/
rt_uint8_t ordinaryquery_wt(rt_uint8_t addr_h , rt_uint8_t addr_l)    //传感器式   00 03 04 00 01 00 00 CRCH CRCL
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //主机发送问询函数，汇赢
	
	for(i=0;i<40;i++)   //清缓存
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	(addr_h<<4)+addr_l;
	modbus_send[1]	=	03;
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc高
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6));//crc低
	
	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//获取串口信号量
	uart1_send(modbus_send,8);    //查询
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//放弃信号量	
		return ERR;    //超时返回
	}
	if((u1_Rx[0]>0x50)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //收到数据，判断是否为乱码或探杆返回错误数据
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //收到错误数据，再查询一次
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x50)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//放弃信号量	
			return ERR;       //收到错误数据或者超时，返回err
		}
	}
	rt_sem_release(usart_sem);					//放弃信号量	
	
	/*对数据进行判断*/
		CRCH = CRC16(u1_Rx,7)>>8;
		CRCL = CRC16(u1_Rx,7);
		if((u1_Rx[8] == CRCL)&&(u1_Rx[7] == CRCH))     //CRC通过
		{
			if(addr_h!=0)   //不是油罐的探杆标志位在第7位
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
功能：对单根汇赢液媒探杆进行一次查询，返回值，传感器状态,输入值，油罐管线人井油盆，地址
*/
rt_uint8_t liqmediumquery_wt(rt_uint8_t addr_l)    
{
	rt_uint8_t change;
	change=ordinaryquery_wt(0,addr_l);   //按照普通探杆查询，然后转换一下
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
功能：对单根汇赢压力探杆进行一次查询，返回值，压力,输入值，地址
*/
rt_uint8_t pressurequery_wt(rt_uint8_t addr_l)    //压力式  01 04 04 00 50 00 4F CRCL CRCH   1号地址为1
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //主机发送问询函数，汇赢
	rt_uint16_t pre;
	for(i=0;i<40;i++)   //清缓存
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	addr_l+1;
	modbus_send[1]	=	04;
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc高
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6));//crc低
	
	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//获取串口信号量
	uart1_send(modbus_send,8);    //查询
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//放弃信号量	
		return ERR_PRESS;    //超时返回
	}
	if((u1_Rx[0]>0x20)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //收到数据，判断是否为乱码或探杆返回错误数据
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //收到错误数据，再查询一次
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x20)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//放弃信号量	
			return ERR_PRESS;       //收到错误数据或者超时，返回err
		}
	}
	rt_sem_release(usart_sem);					//放弃信号量	
	
	/*对数据进行判断*/
		CRCH = CRC16(u1_Rx,7)>>8;
		CRCL = CRC16(u1_Rx,7);
		if((u1_Rx[7] == CRCL)&&(u1_Rx[8] == CRCH))     //CRC通过
		{
			u1_Rx[4]=u1_Rx[4]-SEN_state[0].press_d[addr_l]+SEN_state[0].press_u[addr_l];
			if(u1_Rx[4]>99&&SEN_state[0].press_d[addr_l]>0)  //判断为向下溢出
			{
				u1_Rx[4]=0;
			}
			if(u1_Rx[4]>99/*&&SEN_state[i].press_u[j]>0*/)  //判断为向上溢出
			{
				u1_Rx[4]=99;
			}
			pre=(rt_uint16_t)(u1_Rx[4])*10+(rt_uint16_t)(u1_Rx[6])/10;
			DGUS_numsend(0x22,addr_l<<1,(rt_uint8_t)(pre>>8),(rt_uint8_t)pre);
			if(u1_Rx[4]>SEN_state[0].press[addr_l])     //差距为两个压力再处理，滤波
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
			
			if(SEN_state[0].press[addr_l]<=press_lim)					//判断压力报警状态
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
功能：对单根中油普通探杆进行一次查询，返回值，传感器状态,输入值，油罐管线人井油盆，地址
*/
rt_uint8_t ordinaryquery_cppei(rt_uint8_t addr_h , rt_uint8_t addr_l)     
{
	rt_uint8_t i;
	rt_uint8_t modbus_send[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x00,0x00};		//中油查询	
	
	for(i=0;i<40;i++)   //清缓存
	{
		u1_Rx[i]=0;
	}
	
	modbus_send[0]	=	(addr_h*20)+addr_l;
	modbus_send[6] = (rt_uint8_t)(CRC16(modbus_send,6));//crc低
	modbus_send[7] = (rt_uint8_t)(CRC16(modbus_send,6)>>8);//crc高

	rt_thread_delay(60);	
	
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//获取串口信号量
	uart1_send(modbus_send,8);    //查询
	if(rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)  
	{
		rt_sem_release(usart_sem);					//放弃信号量	
		return ERR;    //超时返回
	}
	if((u1_Rx[0]>0x60)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))    //收到数据，判断是否为乱码或探杆返回错误数据
	{
		rt_thread_delay(60);
		uart1_send(modbus_send,8);    //收到错误数据，再查询一次
		if((rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,70, RT_NULL)==-RT_ETIMEOUT)||(u1_Rx[0]>0x60)||(u1_Rx[3]==0x57&&u1_Rx[4]==0x52&&u1_Rx[5]==0x4F))
		{
			rt_sem_release(usart_sem);					//放弃信号量	
			return ERR;       //收到错误数据或者超时，返回err
		}
	}
	rt_sem_release(usart_sem);					//放弃信号量	
	
	/*对数据进行判断*/
		CRCH=CRC16(u1_Rx,5)>>8;
		CRCL=CRC16(u1_Rx,5);
		if((u1_Rx[5] == CRCL )&&(u1_Rx[6] == CRCH))     //CRC通过
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
功能：进行一根探杆的查询，输入：探杆类型，探杆协议，油罐管线人井油盆，地址
*/
void sensor_query(rt_uint8_t type , rt_uint8_t protocols , rt_uint8_t addr_h , rt_uint8_t addr_l)     
{
	rt_uint8_t flag;
	if(protocols==QDWT)     //青岛汇赢协议
	{
		if(type==SENPRESS)     //压力式传感器
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
		if(type==SENNORM)     //正常传感器
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
		if(type==SENYEMEI)     //液媒式传感器
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
	if(protocols==CPPEI)     //中油协议
	{
		if(type==SENPRESS)     //压力式传感器
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
功能：针对不同类型的单根探杆完成查询，保存相应探杆状态
*/
void modbus(void)
{
	rt_uint8_t i,j;
	for(i=0;i<4;i++)					//油罐管线人井油盆循环查询
	{
		if(SEN_state[i].int_num!=0)		//如果该组数量不为0，则开始查询
		{
			for(j=0;j<SEN_state[i].int_num;j++)		//按开通数量查询	
			{
				sensor_query(SEN_state[i].type,SEN_state[i].protocols,i,j);
			/*-----------对标志位处理------------*/		
				if(SEN_state[i].state[j]!=SEN_state[i].last_sta[j])				//状态标志位等
				{
					SEN_state[i].last_sta[j]=SEN_state[i].state[j];		//保存此次状态
					SEN_state[i].update_f=1;													//探杆状态被更新
					Active_cache(i,j,SEN_state[i].last_sta[j]);       //保存报警状态到网络缓存
					if(SEN_state[i].state[j]!=NORM)
					{
						SEN_state[i].alert[j]++;
					}
				}
			/*-----------对一个探杆查询结束------------*/		
			}
			for(;j<16;j++)		//按开通数量查询	
			{
				SEN_state[i].last_sta[j]=SEN_state[i].state[j]=0xFF;   //未开通
			}
		}
	/*-----------对一个类别探杆查询结束------------*/		
	}
	/*-----------对所有探杆查询结束------------*/			
}
/**/
