/**
  ******************************************************************************
  * File Name          : dgus.c
  *    屏幕数据处理
	
	写的有些乱，没啥好的想法，先这样吧
	
  ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "dgus.h"

/*常用函数示例*/
//		rt_thread_delay(1000);                //延时 
//		uart2_send(text,sizeof(text)); //sizeof()可读取目标长度
//
//		uart2_send(text,6); //sizeof()可读取目标长度


/*DGUS屏幕相关变量*/
rt_uint8_t dgus_pic[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x01};            //迪文页面切换数组，最后一位为图片地址
rt_uint8_t gas_name[60]={0x5A,0xA5,0x0B,0x82,0x60,0x20,0xD6,0xD0,0xB9,0xFA,0xCA,0xAF,0xBB,0xAF,0xFF,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};     //中国石化
rt_uint8_t dgus_ico[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};           //迪文图标
rt_uint8_t dgus_num[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};  					 //发送数字


rt_uint8_t dgus_reset[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A,0xA5};  //复位迪文屏

rt_uint8_t dgus_data[8];					//迪文屏数据存储，转存迪文屏发送的数据
rt_uint8_t save_page;			//记录当前页面，被页面发送函数调用，用于输入密码完成后根据记录的当前页面判断进入下一界面
rt_uint8_t pre_page;       //预加载画面，同一界面多个触控选项时，记录的为同一界面，判断会出错，记录下要进入的下一界面，没规划好程序。。
rt_uint8_t press_lim;				//压力范围
rt_uint8_t voice_flag;				//语音标志位，感觉可以改成信号量


rt_uint8_t keyword[3];				//用户储存密码

//改地址相关

rt_uint8_t send_WIN[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //主机发送问询函数，汇赢

//rt_uint8_t send_WIN[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};  //汇赢查询
rt_uint8_t send_CNPC[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x00,0x00};		//中油查询

rt_uint8_t addr_changeflag=0;
rt_uint8_t save_sen[2];			//中油汇赢，地址
rt_uint8_t sen_unlock[4]={0x40,0x55,0xF0,0x4F};			//解锁	2A 40 55 44 4F 57 4E 23 AC DF   *@UDOWN# AC DF
rt_uint8_t sen_lock[4] = {0x40,0x4C,0x31,0x85};					//加锁

rt_uint8_t read_addr[7]= {0x23,0x3F,0x41,0x4E,0x3B,0xC9,0xB4};		//读地址
rt_uint8_t send_add[9] = {0x23,0x21,0x41,0x4E,0x00,0x00,0x3B,0x00,0x00};  //写地址


rt_uint8_t dgus_voi1[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, 0x01, 0x90, 0x80};    //dgus_voi[6]=0;   
rt_uint8_t dgus_voi2[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, 0x01, 0x90, 0x80};    //dgus_voi[6]=0;  

//rt_uint8_t dgus_timesend[14]={0x5A, 0xA5, 0x0B, 0x82, 0x00, 0x9C, 0x5A, 0xA5, 19, 3,14, 9,51,0};   //改屏幕时间发送数组

rt_uint8_t equ_type=1;				//设备类型，单压力、二选一、压力四合一，因为4.3寸，改成默认二选一
	
struct SEN_str	SEN_state[4];
/*
	函数声明，具体作用看函数定义前说明

*/
void SEN_page_set(rt_uint8_t sen);
void SEN_page_title(rt_uint8_t sen);
void sen_refresh(rt_uint8_t sen);
void press_refresh(rt_uint8_t sen);
void Change_addr(rt_uint8_t type1,rt_uint8_t type2);			//改地址
void Addr_ico(void);
/**
***串口数据返回
***功能范围：保存串口返回数据
***
**/
void read_time(void)
{
	rt_uint8_t read_time[7] = {0x5A, 0xA5, 0x04, 0x83, 0x00, 0x10, 0x04};
	uart2_send(read_time,7);
}

void DGUS_thread_entry(void* parameter)
{
	rt_uint8_t i;
	rt_thread_delay(1500); 			
	DGUS_numsend(0x41,0x16,0,press_lim);
	
	rt_thread_delay(50);
	uart2_send(gas_name,60);
	rt_thread_delay(50);
//	uart2_send(dgus_timesend,14);
	rt_thread_delay(50);
	
	PAGE_send(1);
	rt_thread_delay(20);
	read_time();
	while(1)
	{
		rt_event_recv(&event_u2, UART_RX2_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);    //屏幕事件接收，等待时间 RT_WAITING_FOREVER
		if(u2_Rx[2]!=3)	
		{
			DGUS_USART();
			for(i=0;i<60;i++)             //清理一下缓存，事件接收时不时同时收到两次，对一组接收处理两次会导致一些bug
			u2_Rx[i]=0;
		}
	}
}
/**/

/*语音线程*/
void Voice_thread_entry(void* parameter)
{
rt_uint8_t i,j;
	rt_thread_delay(4000); 
	while(1)
	{
		for(i=0;i<4;i++)
		{
			for(j=0;j<SEN_state[i].int_num;j++)
			{
				if(voice_flag==0)          //语音标志位，考虑实时性判断放在了循环内。
				{
					if(j<8)                //1-8号地址的
					{
						if(SEN_state[i].type==2&&SEN_state[i].last_sta[j]!=NORM_PRESS)    //压力式
						{
							dgus_voi1[6]=j*5+i+2; 
							uart2_send(dgus_voi1,10); 
							if(SEN_state[i].last_sta[j]==ERR_PRESS)					//不提前保存状态，延时后状态可能会被刷掉，提前保存状态，中断发送用同一个数组会还没发送完dgus_voi2[6]就被改掉的情况
							{
								dgus_voi2[6]=52;
							}
							if(SEN_state[i].last_sta[j]==LOW_PRESS)
							{
								dgus_voi2[6]=51;
							}
							rt_thread_delay(1200); 					
							uart2_send(dgus_voi2,10);		
							rt_thread_delay(3);
							rt_thread_delay(2000); 					
						}
						if(SEN_state[i].type!=2&&SEN_state[i].last_sta[j]!=NORM)    //普通
						{
							dgus_voi1[6]=j*5+i+2; 		
							uart2_send(dgus_voi1,10); 
							switch(SEN_state[i].last_sta[j])
							{
								case ERR :
									dgus_voi2[6]=52;
								break;
								case OIL :
									dgus_voi2[6]=47;
								break;
								case WATER :
									dgus_voi2[6]=48;
								break;
								case HIGH :
									dgus_voi2[6]=50;
								break;
								case LOW :
									dgus_voi2[6]=49;
								break;
							}
							rt_thread_delay(1200);				
							uart2_send(dgus_voi2,10);
							rt_thread_delay(3);
							rt_thread_delay(2000);
						}
					}
					else    //大于八号，目前只有人井
					{
						if(SEN_state[i].last_sta[j]!=NORM)    //普通
						{
							dgus_voi1[6]=34+j; 		
							uart2_send(dgus_voi1,10); 
							switch(SEN_state[i].last_sta[j])
							{
								case ERR :
									dgus_voi2[6]=52;
								break;
								case OIL :
									dgus_voi2[6]=47;
								break;
								case WATER :
									dgus_voi2[6]=48;
								break;
								case HIGH :
									dgus_voi2[6]=50;
								break;
								case LOW :
									dgus_voi2[6]=49;
								break;
							}
							rt_thread_delay(1200);				
							uart2_send(dgus_voi2,10);
							rt_thread_delay(3);
							rt_thread_delay(2000);
						}
					}
				}
			}
			rt_thread_delay(100);	
		}
		rt_thread_delay(100);		
	}
}
/**/

void DGUS_USART(void)
{
	rt_uint8_t i=0;
	rt_uint8_t j=0;
	if(u2_Rx[4]==0x60&&u2_Rx[5]==0x20)
	{
		for(i=0;i<u2_Rx[2]+3;i++)
		{
			gas_name[i]=u2_Rx[i];
		}
		for(;i<60;i++)
		{
			gas_name[i]=0;
		}
		gas_name[3]=0x82;
		WriteFlash();
	}
	else
	{
		if(u2_Rx[2]==6)					//触摸按键返回
		{
			dgus_data[0]=u2_Rx[4]>>4;
			dgus_data[1]=u2_Rx[4]-(dgus_data[0]<<4);
			dgus_data[2]=u2_Rx[5]>>4;
			dgus_data[3]=u2_Rx[5]-(dgus_data[2]<<4);
			dgus_data[4]=u2_Rx[7]>>4;
			dgus_data[5]=u2_Rx[7]-(dgus_data[4]<<4);
			dgus_data[6]=u2_Rx[8]>>4;
			dgus_data[7]=u2_Rx[8]-(dgus_data[6]<<4);
			DGUS_DataProce();   //屏幕处理
		}
		if(u2_Rx[2]==8)					//收到参数 u2_Rx[7]=0  u2_Rx[8]=值  
		{
			switch(save_page)
			{
				case 32 :
					if(u2_Rx[8]==0)
					{
						SEN_state[0].type=1; 
					}
					if(u2_Rx[8]==1)
					{
						SEN_state[0].type=0; 
					}
	//					if(u2_Rx[8]==2)
	//					{
	//						SEN_state[0].type=2; 
	//					}
					PAGE_send(33);
					WriteFlash();
					break;
					
			}
		}
		
		if(u2_Rx[2]==16)			//密码
		{
			switch(pre_page)
			{
				case 1 :
					if(u2_Rx[8]==keyword[0]&&u2_Rx[9]==keyword[1]&&u2_Rx[10]==keyword[2])
					{
						if(equ_type==0)
						{
							SEN_state[0].type=2;
							PAGE_send(34);			//单油罐
						}
						if(equ_type==1)
						{	
							PAGE_send(32);		//二合一
						}
						if(equ_type==2)
						{
							SEN_state[0].type=2;
							PAGE_send(33);		//四合一
						}
					}
					else
					{
						DGUS_numsend(0x43,0,0,2);   //密码错误
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					break;
				case 2:
					if(u2_Rx[8]==0x05&&u2_Rx[9]==0x17&&u2_Rx[10]==0x62)
					{
						PAGE_send(38);
					}
					else
					{
						DGUS_numsend(0x43,0,0,2);   //密码错误
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					break;
				case 3:
					if(u2_Rx[8]==0x02&&u2_Rx[9]==0x27&&u2_Rx[10]==0x9B)				//重置
					{
						uart2_send(dgus_reset,10);
						for(i=0;i<4;i++)
						{
							SEN_state[i].update_f=1;
							SEN_state[i].display_f=0;
							SEN_state[i].int_num=0;
							SEN_state[i].protocols=QDWT;
							for(j=0;j<16;j++)
							{
								SEN_state[i].state[j]=0;
								SEN_state[i].last_sta[j]=0;
								SEN_state[i].alert[j]=0;
							}
							for(j=0;j<8;j++)
							{
								SEN_state[i].press_d[j]=0;
								SEN_state[i].press_u[j]=0;
							}
						}
						for(i=0;i<60;i++)
						{
							gas_name[i]=0;
						}
						gas_name[0]=0x5A;gas_name[1]=0xA5;gas_name[2]=0x0E;gas_name[3]=0x82;gas_name[4]=0x60;gas_name[5]=0x20;
						gas_name[6]=0xD6;gas_name[7]=0xD0;gas_name[8]=0xB9;gas_name[9]=0xFA;gas_name[10]=0xCA;
						gas_name[11]=0xAF;gas_name[12]=0xBB;gas_name[13]=0xAF;gas_name[14]=0xFF;gas_name[15]=0xFF;
						keyword[0]=0;
						keyword[1]=0;
						keyword[2]=0;
						press_lim=0;
						voice_flag=0;
						
						WriteFlash();
						rt_thread_delay(800); 
						uart2_send(gas_name,60);
						rt_thread_delay(200);
						PAGE_send(31);
					}
					else
					{
						DGUS_numsend(0x43,0,0,2);   //密码错误
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					break;
					case 4:				//修改密码
					if(u2_Rx[8]==keyword[0]&&u2_Rx[9]==keyword[1]&&u2_Rx[10]==keyword[2]&&u2_Rx[12]==u2_Rx[16]&&u2_Rx[13]==u2_Rx[17]&&u2_Rx[14]==u2_Rx[18])
					{
						DGUS_numsend(0x43,0,0,1);   //密码修改成功
						rt_thread_delay(20);
						PAGE_send(6);
						keyword[0]=u2_Rx[12];
						keyword[1]=u2_Rx[13];
						keyword[2]=u2_Rx[14];
						WriteFlash();
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					else
					{
						DGUS_numsend(0x43,0,0,3);   //输入不一致
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(5);
					}
					break;
			}
				
		}
		
		if(u2_Rx[2]==12)			//保存数量
		{
			switch(save_page)
			{
				case 33 :
					u2_Rx[19]=u2_Rx[8]+u2_Rx[10]+u2_Rx[12]+u2_Rx[14];
					if(u2_Rx[19]==0)
					{
						DGUS_numsend(0x43,0,0,4);   //传感器全为0
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(33);
					}
					else
					{
						SEN_state[0].int_num=u2_Rx[8];
						SEN_state[1].int_num=u2_Rx[10];
						SEN_state[2].int_num=u2_Rx[12];
						SEN_state[3].int_num=u2_Rx[14];
						WriteFlash();
						if(SEN_state[0].type==2)   //压力，到35页
						{
							PAGE_send(35);
						}
						else
						{
							DGUS_numsend(0x43,0,0,0);   //成功
							rt_thread_delay(20);
							PAGE_send(6);
							rt_thread_delay(1000); 
							PAGE_send(31);
						}
					}
					break;
				case 34:
					SEN_state[0].type=2;
					if(u2_Rx[8]==0)
					{
						DGUS_numsend(0x43,0,0,4);   //全为0
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(34);
					}
					else
					{
						PAGE_send(35);
						SEN_state[0].int_num=u2_Rx[8];
						SEN_state[1].int_num=0;
						SEN_state[2].int_num=0;
						SEN_state[3].int_num=0;
						WriteFlash();
					}
					break;
				case 35:
					/*保存压力值*/
					DGUS_numsend(0x43,0,0,0);   //成功
					rt_thread_delay(20);
					press_lim=u2_Rx[8];
					PAGE_send(6);
					WriteFlash();
					rt_thread_delay(1000); 
					PAGE_send(31);
					break;
				case 40:						//设置压力校准
					if(u2_Rx[8]!=0)
					{
						if(!(u2_Rx[10]!=0&&u2_Rx[12]!=0))				//其中一个不等于0
						{
							SEN_state[0].press_d[u2_Rx[8]-1]=u2_Rx[12];
							SEN_state[0].press_u[u2_Rx[8]-1]=u2_Rx[10];
							DGUS_numsend(0x43,0,0,0);   //成功
							rt_thread_delay(20);
							PAGE_send(6);
							WriteFlash();
							rt_thread_delay(1000); 
							PAGE_send(38);
						}
						else
						{
							DGUS_numsend(0x43,0,0,5);   //不能全为0
							rt_thread_delay(20);
							PAGE_send(6);
							rt_thread_delay(1000); 
							PAGE_send(40);					//报错
						}
					}
					else
					{
						DGUS_numsend(0x43,0,0,5);   //不能全为0
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(40);					//报错，
					}
					break;
					case 41:						//修改地址   Rx8类型   Rx10地址
					if(u2_Rx[10]==0)				//都不为0，报错
					{
						u2_Rx[10]=1;
					}
					u2_Rx[10]=u2_Rx[10]-1;
					Change_addr(u2_Rx[8],u2_Rx[10]);			//改地址
					break;
					case 42:						//更改协议
						SEN_state[0].protocols=u2_Rx[8];
						SEN_state[1].protocols=u2_Rx[10];
						SEN_state[2].protocols=u2_Rx[12];
						SEN_state[3].protocols=u2_Rx[14];
						WriteFlash();
						DGUS_numsend(0x43,0,0,0);   //成功
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(38);
					break;
			}
			
			
		}
	/*	*/
	}


}
/**/

void DGUS_DataProce(void)
{
	rt_uint8_t clear=0;
	switch(dgus_data[0])
	{
		/*一级*/
		case 1 :
			switch(dgus_data[1])
			{
				case 1 :
					switch(dgus_data[2])
					{
						case 0 :
							switch(dgus_data[3])
							{
								case 1 :
									voice_flag=1-voice_flag;
									DGUS_numsend(0x13,1,0,voice_flag);
								break;
							}
						break;
					}
				break;
			}
		break;
		/*一级*/
		case 2 :							//返回探杆数据
			switch(dgus_data[1])
			{
				/*二级*/
				case 1 :
					switch(dgus_data[2])
					{
						case 0 : 		  //进入查看界面，进入开通显示的第一个界面，如果都未开通，则进入设置
							if(SEN_state[0].int_num!=0)
							{
								SEN_page_title(0x0A);
							}
							else
							{
								if(SEN_state[1].int_num!=0)
								{
									SEN_page_title(0x0B);
								}
								else
								{
									if(SEN_state[2].int_num!=0)
									{
										SEN_page_title(0x0C);
									}
									else
									{
										if(SEN_state[3].int_num!=0)
										{
											SEN_page_title(0x0D);
										}
										else
										{
											if(equ_type==1)
											SEN_state[0].type=0;				//根据类型重置
											else
												SEN_state[0].type=2;
											WriteFlash();
											PAGE_send(31);
										}
									}
								}
							}
							/**/
							
							
						break;
						
						default:
							if(SEN_state[dgus_data[2]-10].int_num!=0)    //查看油罐管线等
							{
								SEN_page_title(dgus_data[2]);
							}
						break;
					}
				break;
				/*二级*/
				default:

				break;
				
			}
		break;
			
		/*一级*/
		/*设置，两部分处理，有变量地址的进入此处，
			无变量地址的按压同步返回在DGUS_DataProce处理，根据返回长度和返回的页面判断
			*/
		case 4 :							
			switch(dgus_data[1])
			{
				/*二级*/
				case 1 :
					switch(dgus_data[2])
					{
						/*三级*/
						case 1 :   //基本设置
							//进入密码，设置预加载界面为基本设置
							pre_page=1;
							PAGE_send(4);

						break;
						/*三级*/
						case 2 :
							switch(dgus_data[3])
							{
								/*四级*/
								case 0 :
									pre_page=2;
									PAGE_send(4);
								break;
								/*四级，单压力和三合一选择*/
								case 1 :
									equ_type=dgus_data[7];
									if(equ_type==0)		//单压力
									{
										SEN_state[0].type=2;
										SEN_state[1].int_num=0;
										SEN_state[2].int_num=0;
										SEN_state[3].int_num=0;

										DGUS_numsend(0x43,0,0,0);   //成功
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(38);
									}
									if(equ_type==1)		//正常二合一
									{
										SEN_state[0].type=0;
										
										DGUS_numsend(0x43,0,0,0);   //成功
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(38);
									}
									if(equ_type==2)		//压力四合一
									{
										SEN_state[0].type=2;
										
										DGUS_numsend(0x43,0,0,0);   //成功
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(38);
									}
									if(equ_type==3)
									{
										equ_type=1;
										SEN_state[0].type=0;
										SEN_state[2].int_num=0;
										SEN_state[3].int_num=0;
										
										DGUS_numsend(0x43,0,0,0);   //成功
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(1);
									}
								break;
									/*四级，进校准*/
								case 2 :
									PAGE_send(40);		//40页
									break;
								case 3 :
									PAGE_send(41);		//41页
									break;
								case 4 :
									PAGE_send(42);		//42页
									break;
							}
						
						break;
						/*三级*/
						case 3 :				//恢复出厂
							switch(dgus_data[3])
							{
								case 0:
									pre_page=3;
									PAGE_send(4);
								break;
								case 9:
									Addr_ico();
								break;
							}
							
							break;
						/*三级*/
						/*三级*/
						case 4 :				//修改密码
							pre_page=4;
							PAGE_send(5);
							break;
						case 0x0F :
							switch(pre_page)
							{
								case 1 :
									PAGE_send(31);
									break;
								case 2 :
									PAGE_send(31);
									break;
								case 3 :
									PAGE_send(31);
									break;
								case 4 :
									PAGE_send(31);
									break;
							}
							break;
						/*三级*/
						default:
							
						break;
					}
				break;
				/*二级*/
				default:

				break;
				
			}			
		break;
		/*一级*/
		default:							
				
		break;
	}
	for(clear=0;clear<8;clear++)  //清数据
	{
		dgus_data[clear]=0;
	}
}
/**/


/* 报警刷新，刷新报警记录和显示探杆状态刷新
*/
void alert_refresh(void)   
{
	rt_uint8_t i,j;
	rt_uint8_t K_flag=0;
	for(i=0;i<4;i++)
	{
		if(SEN_state[i].update_f==1)		//轮询有没有组别探杆状态改变
		{
			SEN_state[i].update_f=0;    //已被处理
			if(SEN_state[i].display_f==1)//显示该页，则刷新探杆状态过去
			{
				if(SEN_state[i].type==SENNORM)
				{
					sen_refresh(i);
				}
				if(SEN_state[i].type==SENYEMEI)
				{
					sen_refresh(i);
				}
				if(SEN_state[i].type==SENPRESS)
				{
					press_refresh(i);
				}
			}
			//刷新报警记录
			for(j=0;j<SEN_state[i].int_num;j++)
			{
				DGUS_numsend(0x22,((10+i)<<4)+j,0,SEN_state[i].alert[j]);
			}
		}
	}
	K_flag=0;
	for(i=0;i<4;i++)
	{
		for(j=0;j<SEN_state[i].int_num;j++)
		{
			if(SEN_state[i].state[j]!=0)
			{
				K_flag=1;
			}
		}
	}
	if(K_flag==1)
	{
		K_ON;
	}
	else
	{
		K_OFF;
	}
}



/*
	*
	*油罐管线人井油盆页面切换函数，输入变量，0xABCD，油罐-油盆,
	*功能范围：根据输入变量刷新页面
	*
*/

void SEN_page_title(rt_uint8_t sen)  //刷新探杆页面探杆选择部分  display_f
{
	rt_uint8_t for_i;
		SEN_page_set(sen);
		for(for_i=0;for_i<4;for_i++)
		{
			if(SEN_state[for_i].int_num==0)			//未开通
			{
				dgus_ico[4]=0x23;
				dgus_ico[5]=((10+for_i)<<4)+0x0F;
				dgus_ico[7]=1;
				SEN_state[for_i].display_f=0;
				rt_thread_delay(2);
				uart2_send(dgus_ico,8);
				rt_thread_delay(2);				
			}
			if(SEN_state[for_i].int_num!=0)			//开通
			{
				if(sen==(for_i+10))
				{
					dgus_ico[4]=0x23;
					dgus_ico[5]=((10+for_i)<<4)+0x0F;
					dgus_ico[7]=3;
					SEN_state[for_i].display_f=1;
					rt_thread_delay(2);
					uart2_send(dgus_ico,8);	
					rt_thread_delay(2);					
				}
				else
				{
					dgus_ico[4]=0x23;
					dgus_ico[5]=((10+for_i)<<4)+0x0F;
					dgus_ico[7]=0;
					SEN_state[for_i].display_f=0;
					rt_thread_delay(2);
					uart2_send(dgus_ico,8);
					rt_thread_delay(2);					
				}
			}
	}
}

/*
	*
	*油罐管线人井油盆页面切换函数，输入变量，0xA、B、C、D，油罐-油盆,
	*功能范围：根据输入变量跳转页面
	*
*/

void SEN_page_set(rt_uint8_t sen)
{
rt_uint8_t page;
	sen=sen-0x0A;
	if(SEN_state[sen].int_num!=0)				//开通不为0
	{
		if(SEN_state[sen].type==SENPRESS)
		{
			DGUS_numsend(0x13,0x10,0,0);
			page=SEN_TYPE_P+SEN_state[sen].int_num-1;
			press_refresh(sen);
		}
		if(SEN_state[sen].type==SENNORM)
		{
			DGUS_numsend(0x13,0x10,0,2);
			page=SEN_TYPE_G+SEN_state[sen].int_num-1;
			sen_refresh(sen);		//非压力探杆状态更新函数   
		}
		if(SEN_state[sen].type==SENYEMEI)
		{
			DGUS_numsend(0x13,0x10,0,1);
			page=SEN_TYPE_G+SEN_state[sen].int_num-1;
			sen_refresh(sen);		//非压力探杆状态更新函数   
		}
		PAGE_send(page);		//发送界面地址
	}
}
/**/

#define ICOTIM 2

/*非压力探杆图标刷新,被page_set和alert_refresh调用
*/
void sen_refresh(rt_uint8_t sen)
{
	rt_uint8_t i;
	for(i=0;i<(SEN_state[sen].int_num);i++)
	{
		dgus_ico[4]=0x23;
		dgus_ico[5]=0x30+i;
		dgus_ico[7]=SEN_state[sen].state[i];
		rt_thread_delay(ICOTIM);
		uart2_send(dgus_ico,8);
		rt_thread_delay(ICOTIM);
		uart2_send(dgus_ico,8);   //屏有BUG？发一遍有些状态刷不过去
		if(SEN_state[sen].state[i]==NORM)
		{
			dgus_ico[5]=0x10+(i<<1);
			dgus_ico[7]=0;   //关动图
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
			dgus_ico[5]=i;
			dgus_ico[7]=sen;
			rt_thread_delay(ICOTIM);		//开静态图
			uart2_send(dgus_ico,8);
		}
		else
		{
			dgus_ico[5]=0x10+(i<<1);
			dgus_ico[7]=1;   //开动图
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
			dgus_ico[5]=i;
			dgus_ico[7]=5;		//关静态图
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
		}

	}
//	for(i=(SEN_state[sen].int_num);i<12;i++)
//	{
//		dgus_ico[5]=0x10+(i<<1);
//		dgus_ico[7]=0;   //关动图
//		rt_thread_delay(ICOTIM);
//		uart2_send(dgus_ico,8);
//		dgus_ico[5]=i;
//		dgus_ico[7]=5;		//关静态图
//		rt_thread_delay(ICOTIM);
//		uart2_send(dgus_ico,8);			
//	}
	rt_thread_delay(ICOTIM);
}
/**/
/*压力探杆图标刷新,被page_set和alert_refresh调用
*/
void press_refresh(rt_uint8_t sen)
{
	rt_uint8_t i;
	for(i=0;i<(SEN_state[sen].int_num);i++)
	{
		dgus_ico[4]=0x23;
		dgus_ico[5]=0x40+i;
		dgus_ico[7]=SEN_state[sen].state[i];
		rt_thread_delay(ICOTIM);
		uart2_send(dgus_ico,8);
		dgus_ico[4]=0x23;
		dgus_ico[5]=0x50+i+i;
		if(SEN_state[sen].state[i]==NORM_PRESS)
		{
			dgus_ico[7]=0;
		}
		else
		{
			dgus_ico[7]=1;
		}
		rt_thread_delay(ICOTIM);
		uart2_send(dgus_ico,8);
		DGUS_numsend(0x24,i,0,100-SEN_state[sen].press[i]);
	}
	rt_thread_delay(ICOTIM);
}
/*给地址发送数据*/
void DGUS_numsend(rt_uint8_t addH,rt_uint8_t addL,rt_uint8_t numH,rt_uint8_t numL)
{
	rt_thread_delay(ICOTIM);
	dgus_num[4]=addH;
	dgus_num[5]=addL;	

	dgus_num[6]=numH;	
	dgus_num[7]=numL;
	uart2_send(dgus_num,8);
	rt_thread_delay(ICOTIM);	
}

/*发送界面并将界面保存*/
void PAGE_send(rt_uint8_t page)
{
	rt_uint8_t dgus_Key[10] = {0x5A, 0xA5, 0x07, 0x82, 0x41, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF};
	if(page!=4&&page!=5&&page!=6)			//密码页和提示页面不保存
	{
		dgus_pic[9]=page;
		save_page=page;
		uart2_send(dgus_pic,10); 
	}
	else
	{
		uart2_send(dgus_Key,10);
		rt_thread_delay(ICOTIM);
		dgus_pic[9]=page;
		uart2_send(dgus_pic,10); 
	}
}
/**/



/*
*		修改地址相关
*/

void serial0_send_buff(rt_uint8_t buf[],rt_uint8_t len)				
{
	rt_uint8_t i;
	for(i=0;i<20;i++)
	{
		u1_Rx[i]=0;
	}
	uart1_send(buf,len);
}


void Read_addr(void)			//读地址和协议
{
	rt_uint8_t CRCH,CRCL;
	save_sen[0]=0;
	save_sen[1]=0;
	rt_thread_delay(500);    //一次查询结束最多0.2s，等待主循环查询完成
	serial0_send_buff(sen_unlock,4);			//解锁
	rt_thread_delay(500);				//从机处理0.25s
	serial0_send_buff(read_addr,7);				//读地址
	rt_thread_delay(500);				//从机处理0.25s	
	save_sen[1]=((u1_Rx[1]-0x30)*10+(u1_Rx[2]-0x30));    //保存地址
	if(save_sen[1]==0xAD)
	{
		save_sen[1]=0x00;
		serial0_send_buff(sen_unlock,4);			//解锁
		rt_thread_delay(500);				//从机处理0.25s
		send_add[4]=0x30;
		send_add[5]=0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);

		rt_thread_delay(500);				//从机处理0.25s
	}
	serial0_send_buff(sen_lock,4);														//加锁
	rt_thread_delay(500);				//从机处理0.25s	
	
	send_CNPC[0]=save_sen[1];  						 //发送地址
	CRCH = CRC16(send_CNPC,6)>>8;					//crc高
  CRCL = CRC16(send_CNPC,6);						//crc低
	send_CNPC[6]=CRCL;
	send_CNPC[7]=CRCH;
	serial0_send_buff(send_CNPC,8);
	rt_thread_delay(110); 							
	if((u1_Rx[6] == (rt_uint8_t)(CRC16(u1_Rx,5)>>8))&&(u1_Rx[5] == (rt_uint8_t) CRC16(u1_Rx,5)))		//有返回，说明为中油
	{
		save_sen[0]=1;
	}
	else
	{
		send_WIN[0]=save_sen[1];
		send_WIN[1]	=	03;
		CRCH = CRC16(send_WIN,6)>>8;
		CRCL = CRC16(send_WIN,6);
		send_WIN[6]=CRCH;
		send_WIN[7]=CRCL;    
		serial0_send_buff(send_WIN,8);
		rt_thread_delay(110);		
		if((u1_Rx[7] == (rt_uint8_t)(CRC16(u1_Rx,7)>>8))&&(u1_Rx[8] == (rt_uint8_t)CRC16(u1_Rx,7)))		//有返回，说明为中油
		{
			save_sen[0]=2;
		}
	}
	if(save_sen[0]==0)		//无返回，报错
	{
		DGUS_numsend(0x43,0,0,7);
		rt_thread_delay(20);
		PAGE_send(6);
		rt_thread_delay(1000); 
		PAGE_send(41);					//报错
	}
	rt_thread_delay(20);
}

void Change_addr(rt_uint8_t type1,rt_uint8_t type2)			//改地址
{
	rt_uint8_t CRCH,CRCL;
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//获取串口信号量
	DGUS_numsend(0x43,0,0,8);   //执行中
	rt_thread_delay(20);
	PAGE_send(6);
	Read_addr();
	if(save_sen[0]==1)  //中油
	{
		serial0_send_buff(sen_unlock,4);			//解锁
		rt_thread_delay(500);				//从机处理0.25s
		send_add[4]=(20*type1+type2)/10+0x30;
		send_add[5]=(20*type1+type2)%10+0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);
		rt_thread_delay(500);				//从机处理0.25s	
		serial0_send_buff(sen_lock,4);														//加锁
		rt_thread_delay(20);		
		DGUS_numsend(0x43,0,0,6);   //执行成功
		rt_thread_delay(1000);
		PAGE_send(41);
	}
	if(save_sen[0]==2)	//汇赢
	{
		serial0_send_buff(sen_unlock,4);			//解锁
		rt_thread_delay(500);				//从机处理0.25s
		send_add[4]=(16*type1+type2)/10+0x30;
		send_add[5]=(16*type1+type2)%10+0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);
		rt_thread_delay(500);				//从机处理0.25s	
		serial0_send_buff(sen_lock,4);														//加锁

		rt_thread_delay(20);
		DGUS_numsend(0x43,0,0,6);   //执行成功
		rt_thread_delay(1000); 
		PAGE_send(41);
	}
	rt_sem_release(usart_sem);					//放弃信号量	
}

void Addr_ico(void)						//读到的地址刷图到屏上
{
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//获取串口信号量
	DGUS_numsend(0x43,0,0,8);   //执行中
	rt_thread_delay(20);
	PAGE_send(6);
	
	
	Read_addr();
	if(save_sen[0]==1)
	{
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x11,0,save_sen[1]/20+1);   
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x10,0,save_sen[1]%20+1);
		rt_thread_delay(20);
		PAGE_send(41);
	}
	if(save_sen[0]==2)   //汇赢
	{
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x11,0,(save_sen[1]>>4)+1);
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x10,0,save_sen[1]%16+1);
		rt_thread_delay(20);
		PAGE_send(41);
	}
	rt_sem_release(usart_sem);					//放弃信号量
}


/**/



