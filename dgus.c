/**
  ******************************************************************************
  * File Name          : dgus.c
  *    ��Ļ���ݴ���
	
	д����Щ�ң�ûɶ�õ��뷨����������
	
  ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "dgus.h"

/*���ú���ʾ��*/
//		rt_thread_delay(1000);                //��ʱ 
//		uart2_send(text,sizeof(text)); //sizeof()�ɶ�ȡĿ�곤��
//
//		uart2_send(text,6); //sizeof()�ɶ�ȡĿ�곤��


/*DGUS��Ļ��ر���*/
rt_uint8_t dgus_pic[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x01};            //����ҳ���л����飬���һλΪͼƬ��ַ
rt_uint8_t gas_name[60]={0x5A,0xA5,0x0B,0x82,0x60,0x20,0xD6,0xD0,0xB9,0xFA,0xCA,0xAF,0xBB,0xAF,0xFF,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};     //�й�ʯ��
rt_uint8_t dgus_ico[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};           //����ͼ��
rt_uint8_t dgus_num[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};  					 //��������


rt_uint8_t dgus_reset[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A,0xA5};  //��λ������

rt_uint8_t dgus_data[8];					//���������ݴ洢��ת����������͵�����
rt_uint8_t save_page;			//��¼��ǰҳ�棬��ҳ�淢�ͺ������ã���������������ɺ���ݼ�¼�ĵ�ǰҳ���жϽ�����һ����
rt_uint8_t pre_page;       //Ԥ���ػ��棬ͬһ����������ѡ��ʱ����¼��Ϊͬһ���棬�жϻ������¼��Ҫ�������һ���棬û�滮�ó��򡣡�
rt_uint8_t press_lim;				//ѹ����Χ
rt_uint8_t voice_flag;				//������־λ���о����Ըĳ��ź���


rt_uint8_t keyword[3];				//�û���������

//�ĵ�ַ���

rt_uint8_t send_WIN[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};     //����������ѯ��������Ӯ

//rt_uint8_t send_WIN[8]={0x00,0x03,0x00,0x40,0x00,0x02,0x00,0x00};  //��Ӯ��ѯ
rt_uint8_t send_CNPC[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x00,0x00};		//���Ͳ�ѯ

rt_uint8_t addr_changeflag=0;
rt_uint8_t save_sen[2];			//���ͻ�Ӯ����ַ
rt_uint8_t sen_unlock[4]={0x40,0x55,0xF0,0x4F};			//����	2A 40 55 44 4F 57 4E 23 AC DF   *@UDOWN# AC DF
rt_uint8_t sen_lock[4] = {0x40,0x4C,0x31,0x85};					//����

rt_uint8_t read_addr[7]= {0x23,0x3F,0x41,0x4E,0x3B,0xC9,0xB4};		//����ַ
rt_uint8_t send_add[9] = {0x23,0x21,0x41,0x4E,0x00,0x00,0x3B,0x00,0x00};  //д��ַ


rt_uint8_t dgus_voi1[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, 0x01, 0x90, 0x80};    //dgus_voi[6]=0;   
rt_uint8_t dgus_voi2[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, 0x01, 0x90, 0x80};    //dgus_voi[6]=0;  

//rt_uint8_t dgus_timesend[14]={0x5A, 0xA5, 0x0B, 0x82, 0x00, 0x9C, 0x5A, 0xA5, 19, 3,14, 9,51,0};   //����Ļʱ�䷢������

rt_uint8_t equ_type=1;				//�豸���ͣ���ѹ������ѡһ��ѹ���ĺ�һ����Ϊ4.3�磬�ĳ�Ĭ�϶�ѡһ
	
struct SEN_str	SEN_state[4];
/*
	�����������������ÿ���������ǰ˵��

*/
void SEN_page_set(rt_uint8_t sen);
void SEN_page_title(rt_uint8_t sen);
void sen_refresh(rt_uint8_t sen);
void press_refresh(rt_uint8_t sen);
void Change_addr(rt_uint8_t type1,rt_uint8_t type2);			//�ĵ�ַ
void Addr_ico(void);
/**
***�������ݷ���
***���ܷ�Χ�����洮�ڷ�������
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
		rt_event_recv(&event_u2, UART_RX2_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);    //��Ļ�¼����գ��ȴ�ʱ�� RT_WAITING_FOREVER
		if(u2_Rx[2]!=3)	
		{
			DGUS_USART();
			for(i=0;i<60;i++)             //����һ�»��棬�¼�����ʱ��ʱͬʱ�յ����Σ���һ����մ������λᵼ��һЩbug
			u2_Rx[i]=0;
		}
	}
}
/**/

/*�����߳�*/
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
				if(voice_flag==0)          //������־λ������ʵʱ���жϷ�����ѭ���ڡ�
				{
					if(j<8)                //1-8�ŵ�ַ��
					{
						if(SEN_state[i].type==2&&SEN_state[i].last_sta[j]!=NORM_PRESS)    //ѹ��ʽ
						{
							dgus_voi1[6]=j*5+i+2; 
							uart2_send(dgus_voi1,10); 
							if(SEN_state[i].last_sta[j]==ERR_PRESS)					//����ǰ����״̬����ʱ��״̬���ܻᱻˢ������ǰ����״̬���жϷ�����ͬһ������ỹû������dgus_voi2[6]�ͱ��ĵ������
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
						if(SEN_state[i].type!=2&&SEN_state[i].last_sta[j]!=NORM)    //��ͨ
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
					else    //���ڰ˺ţ�Ŀǰֻ���˾�
					{
						if(SEN_state[i].last_sta[j]!=NORM)    //��ͨ
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
		if(u2_Rx[2]==6)					//������������
		{
			dgus_data[0]=u2_Rx[4]>>4;
			dgus_data[1]=u2_Rx[4]-(dgus_data[0]<<4);
			dgus_data[2]=u2_Rx[5]>>4;
			dgus_data[3]=u2_Rx[5]-(dgus_data[2]<<4);
			dgus_data[4]=u2_Rx[7]>>4;
			dgus_data[5]=u2_Rx[7]-(dgus_data[4]<<4);
			dgus_data[6]=u2_Rx[8]>>4;
			dgus_data[7]=u2_Rx[8]-(dgus_data[6]<<4);
			DGUS_DataProce();   //��Ļ����
		}
		if(u2_Rx[2]==8)					//�յ����� u2_Rx[7]=0  u2_Rx[8]=ֵ  
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
		
		if(u2_Rx[2]==16)			//����
		{
			switch(pre_page)
			{
				case 1 :
					if(u2_Rx[8]==keyword[0]&&u2_Rx[9]==keyword[1]&&u2_Rx[10]==keyword[2])
					{
						if(equ_type==0)
						{
							SEN_state[0].type=2;
							PAGE_send(34);			//���͹�
						}
						if(equ_type==1)
						{	
							PAGE_send(32);		//����һ
						}
						if(equ_type==2)
						{
							SEN_state[0].type=2;
							PAGE_send(33);		//�ĺ�һ
						}
					}
					else
					{
						DGUS_numsend(0x43,0,0,2);   //�������
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
						DGUS_numsend(0x43,0,0,2);   //�������
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					break;
				case 3:
					if(u2_Rx[8]==0x02&&u2_Rx[9]==0x27&&u2_Rx[10]==0x9B)				//����
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
						DGUS_numsend(0x43,0,0,2);   //�������
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(31);
					}
					break;
					case 4:				//�޸�����
					if(u2_Rx[8]==keyword[0]&&u2_Rx[9]==keyword[1]&&u2_Rx[10]==keyword[2]&&u2_Rx[12]==u2_Rx[16]&&u2_Rx[13]==u2_Rx[17]&&u2_Rx[14]==u2_Rx[18])
					{
						DGUS_numsend(0x43,0,0,1);   //�����޸ĳɹ�
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
						DGUS_numsend(0x43,0,0,3);   //���벻һ��
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(5);
					}
					break;
			}
				
		}
		
		if(u2_Rx[2]==12)			//��������
		{
			switch(save_page)
			{
				case 33 :
					u2_Rx[19]=u2_Rx[8]+u2_Rx[10]+u2_Rx[12]+u2_Rx[14];
					if(u2_Rx[19]==0)
					{
						DGUS_numsend(0x43,0,0,4);   //������ȫΪ0
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
						if(SEN_state[0].type==2)   //ѹ������35ҳ
						{
							PAGE_send(35);
						}
						else
						{
							DGUS_numsend(0x43,0,0,0);   //�ɹ�
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
						DGUS_numsend(0x43,0,0,4);   //ȫΪ0
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
					/*����ѹ��ֵ*/
					DGUS_numsend(0x43,0,0,0);   //�ɹ�
					rt_thread_delay(20);
					press_lim=u2_Rx[8];
					PAGE_send(6);
					WriteFlash();
					rt_thread_delay(1000); 
					PAGE_send(31);
					break;
				case 40:						//����ѹ��У׼
					if(u2_Rx[8]!=0)
					{
						if(!(u2_Rx[10]!=0&&u2_Rx[12]!=0))				//����һ��������0
						{
							SEN_state[0].press_d[u2_Rx[8]-1]=u2_Rx[12];
							SEN_state[0].press_u[u2_Rx[8]-1]=u2_Rx[10];
							DGUS_numsend(0x43,0,0,0);   //�ɹ�
							rt_thread_delay(20);
							PAGE_send(6);
							WriteFlash();
							rt_thread_delay(1000); 
							PAGE_send(38);
						}
						else
						{
							DGUS_numsend(0x43,0,0,5);   //����ȫΪ0
							rt_thread_delay(20);
							PAGE_send(6);
							rt_thread_delay(1000); 
							PAGE_send(40);					//����
						}
					}
					else
					{
						DGUS_numsend(0x43,0,0,5);   //����ȫΪ0
						rt_thread_delay(20);
						PAGE_send(6);
						rt_thread_delay(1000); 
						PAGE_send(40);					//����
					}
					break;
					case 41:						//�޸ĵ�ַ   Rx8����   Rx10��ַ
					if(u2_Rx[10]==0)				//����Ϊ0������
					{
						u2_Rx[10]=1;
					}
					u2_Rx[10]=u2_Rx[10]-1;
					Change_addr(u2_Rx[8],u2_Rx[10]);			//�ĵ�ַ
					break;
					case 42:						//����Э��
						SEN_state[0].protocols=u2_Rx[8];
						SEN_state[1].protocols=u2_Rx[10];
						SEN_state[2].protocols=u2_Rx[12];
						SEN_state[3].protocols=u2_Rx[14];
						WriteFlash();
						DGUS_numsend(0x43,0,0,0);   //�ɹ�
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
		/*һ��*/
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
		/*һ��*/
		case 2 :							//����̽������
			switch(dgus_data[1])
			{
				/*����*/
				case 1 :
					switch(dgus_data[2])
					{
						case 0 : 		  //����鿴���棬���뿪ͨ��ʾ�ĵ�һ�����棬�����δ��ͨ�����������
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
											SEN_state[0].type=0;				//������������
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
							if(SEN_state[dgus_data[2]-10].int_num!=0)    //�鿴�͹޹��ߵ�
							{
								SEN_page_title(dgus_data[2]);
							}
						break;
					}
				break;
				/*����*/
				default:

				break;
				
			}
		break;
			
		/*һ��*/
		/*���ã������ִ����б�����ַ�Ľ���˴���
			�ޱ�����ַ�İ�ѹͬ��������DGUS_DataProce�������ݷ��س��Ⱥͷ��ص�ҳ���ж�
			*/
		case 4 :							
			switch(dgus_data[1])
			{
				/*����*/
				case 1 :
					switch(dgus_data[2])
					{
						/*����*/
						case 1 :   //��������
							//�������룬����Ԥ���ؽ���Ϊ��������
							pre_page=1;
							PAGE_send(4);

						break;
						/*����*/
						case 2 :
							switch(dgus_data[3])
							{
								/*�ļ�*/
								case 0 :
									pre_page=2;
									PAGE_send(4);
								break;
								/*�ļ�����ѹ��������һѡ��*/
								case 1 :
									equ_type=dgus_data[7];
									if(equ_type==0)		//��ѹ��
									{
										SEN_state[0].type=2;
										SEN_state[1].int_num=0;
										SEN_state[2].int_num=0;
										SEN_state[3].int_num=0;

										DGUS_numsend(0x43,0,0,0);   //�ɹ�
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(38);
									}
									if(equ_type==1)		//��������һ
									{
										SEN_state[0].type=0;
										
										DGUS_numsend(0x43,0,0,0);   //�ɹ�
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(38);
									}
									if(equ_type==2)		//ѹ���ĺ�һ
									{
										SEN_state[0].type=2;
										
										DGUS_numsend(0x43,0,0,0);   //�ɹ�
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
										
										DGUS_numsend(0x43,0,0,0);   //�ɹ�
										rt_thread_delay(10);
										WriteFlash();
										rt_thread_delay(10);
										PAGE_send(6);
										rt_thread_delay(10);
										rt_thread_delay(1000); 
										PAGE_send(1);
									}
								break;
									/*�ļ�����У׼*/
								case 2 :
									PAGE_send(40);		//40ҳ
									break;
								case 3 :
									PAGE_send(41);		//41ҳ
									break;
								case 4 :
									PAGE_send(42);		//42ҳ
									break;
							}
						
						break;
						/*����*/
						case 3 :				//�ָ�����
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
						/*����*/
						/*����*/
						case 4 :				//�޸�����
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
						/*����*/
						default:
							
						break;
					}
				break;
				/*����*/
				default:

				break;
				
			}			
		break;
		/*һ��*/
		default:							
				
		break;
	}
	for(clear=0;clear<8;clear++)  //������
	{
		dgus_data[clear]=0;
	}
}
/**/


/* ����ˢ�£�ˢ�±�����¼����ʾ̽��״̬ˢ��
*/
void alert_refresh(void)   
{
	rt_uint8_t i,j;
	rt_uint8_t K_flag=0;
	for(i=0;i<4;i++)
	{
		if(SEN_state[i].update_f==1)		//��ѯ��û�����̽��״̬�ı�
		{
			SEN_state[i].update_f=0;    //�ѱ�����
			if(SEN_state[i].display_f==1)//��ʾ��ҳ����ˢ��̽��״̬��ȥ
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
			//ˢ�±�����¼
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
	*�͹޹����˾�����ҳ���л����������������0xABCD���͹�-����,
	*���ܷ�Χ�������������ˢ��ҳ��
	*
*/

void SEN_page_title(rt_uint8_t sen)  //ˢ��̽��ҳ��̽��ѡ�񲿷�  display_f
{
	rt_uint8_t for_i;
		SEN_page_set(sen);
		for(for_i=0;for_i<4;for_i++)
		{
			if(SEN_state[for_i].int_num==0)			//δ��ͨ
			{
				dgus_ico[4]=0x23;
				dgus_ico[5]=((10+for_i)<<4)+0x0F;
				dgus_ico[7]=1;
				SEN_state[for_i].display_f=0;
				rt_thread_delay(2);
				uart2_send(dgus_ico,8);
				rt_thread_delay(2);				
			}
			if(SEN_state[for_i].int_num!=0)			//��ͨ
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
	*�͹޹����˾�����ҳ���л����������������0xA��B��C��D���͹�-����,
	*���ܷ�Χ���������������תҳ��
	*
*/

void SEN_page_set(rt_uint8_t sen)
{
rt_uint8_t page;
	sen=sen-0x0A;
	if(SEN_state[sen].int_num!=0)				//��ͨ��Ϊ0
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
			sen_refresh(sen);		//��ѹ��̽��״̬���º���   
		}
		if(SEN_state[sen].type==SENYEMEI)
		{
			DGUS_numsend(0x13,0x10,0,1);
			page=SEN_TYPE_G+SEN_state[sen].int_num-1;
			sen_refresh(sen);		//��ѹ��̽��״̬���º���   
		}
		PAGE_send(page);		//���ͽ����ַ
	}
}
/**/

#define ICOTIM 2

/*��ѹ��̽��ͼ��ˢ��,��page_set��alert_refresh����
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
		uart2_send(dgus_ico,8);   //����BUG����һ����Щ״̬ˢ����ȥ
		if(SEN_state[sen].state[i]==NORM)
		{
			dgus_ico[5]=0x10+(i<<1);
			dgus_ico[7]=0;   //�ض�ͼ
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
			dgus_ico[5]=i;
			dgus_ico[7]=sen;
			rt_thread_delay(ICOTIM);		//����̬ͼ
			uart2_send(dgus_ico,8);
		}
		else
		{
			dgus_ico[5]=0x10+(i<<1);
			dgus_ico[7]=1;   //����ͼ
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
			dgus_ico[5]=i;
			dgus_ico[7]=5;		//�ؾ�̬ͼ
			rt_thread_delay(ICOTIM);
			uart2_send(dgus_ico,8);
		}

	}
//	for(i=(SEN_state[sen].int_num);i<12;i++)
//	{
//		dgus_ico[5]=0x10+(i<<1);
//		dgus_ico[7]=0;   //�ض�ͼ
//		rt_thread_delay(ICOTIM);
//		uart2_send(dgus_ico,8);
//		dgus_ico[5]=i;
//		dgus_ico[7]=5;		//�ؾ�̬ͼ
//		rt_thread_delay(ICOTIM);
//		uart2_send(dgus_ico,8);			
//	}
	rt_thread_delay(ICOTIM);
}
/**/
/*ѹ��̽��ͼ��ˢ��,��page_set��alert_refresh����
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
/*����ַ��������*/
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

/*���ͽ��沢�����汣��*/
void PAGE_send(rt_uint8_t page)
{
	rt_uint8_t dgus_Key[10] = {0x5A, 0xA5, 0x07, 0x82, 0x41, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF};
	if(page!=4&&page!=5&&page!=6)			//����ҳ����ʾҳ�治����
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
*		�޸ĵ�ַ���
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


void Read_addr(void)			//����ַ��Э��
{
	rt_uint8_t CRCH,CRCL;
	save_sen[0]=0;
	save_sen[1]=0;
	rt_thread_delay(500);    //һ�β�ѯ�������0.2s���ȴ���ѭ����ѯ���
	serial0_send_buff(sen_unlock,4);			//����
	rt_thread_delay(500);				//�ӻ�����0.25s
	serial0_send_buff(read_addr,7);				//����ַ
	rt_thread_delay(500);				//�ӻ�����0.25s	
	save_sen[1]=((u1_Rx[1]-0x30)*10+(u1_Rx[2]-0x30));    //�����ַ
	if(save_sen[1]==0xAD)
	{
		save_sen[1]=0x00;
		serial0_send_buff(sen_unlock,4);			//����
		rt_thread_delay(500);				//�ӻ�����0.25s
		send_add[4]=0x30;
		send_add[5]=0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);

		rt_thread_delay(500);				//�ӻ�����0.25s
	}
	serial0_send_buff(sen_lock,4);														//����
	rt_thread_delay(500);				//�ӻ�����0.25s	
	
	send_CNPC[0]=save_sen[1];  						 //���͵�ַ
	CRCH = CRC16(send_CNPC,6)>>8;					//crc��
  CRCL = CRC16(send_CNPC,6);						//crc��
	send_CNPC[6]=CRCL;
	send_CNPC[7]=CRCH;
	serial0_send_buff(send_CNPC,8);
	rt_thread_delay(110); 							
	if((u1_Rx[6] == (rt_uint8_t)(CRC16(u1_Rx,5)>>8))&&(u1_Rx[5] == (rt_uint8_t) CRC16(u1_Rx,5)))		//�з��أ�˵��Ϊ����
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
		if((u1_Rx[7] == (rt_uint8_t)(CRC16(u1_Rx,7)>>8))&&(u1_Rx[8] == (rt_uint8_t)CRC16(u1_Rx,7)))		//�з��أ�˵��Ϊ����
		{
			save_sen[0]=2;
		}
	}
	if(save_sen[0]==0)		//�޷��أ�����
	{
		DGUS_numsend(0x43,0,0,7);
		rt_thread_delay(20);
		PAGE_send(6);
		rt_thread_delay(1000); 
		PAGE_send(41);					//����
	}
	rt_thread_delay(20);
}

void Change_addr(rt_uint8_t type1,rt_uint8_t type2)			//�ĵ�ַ
{
	rt_uint8_t CRCH,CRCL;
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//��ȡ�����ź���
	DGUS_numsend(0x43,0,0,8);   //ִ����
	rt_thread_delay(20);
	PAGE_send(6);
	Read_addr();
	if(save_sen[0]==1)  //����
	{
		serial0_send_buff(sen_unlock,4);			//����
		rt_thread_delay(500);				//�ӻ�����0.25s
		send_add[4]=(20*type1+type2)/10+0x30;
		send_add[5]=(20*type1+type2)%10+0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);
		rt_thread_delay(500);				//�ӻ�����0.25s	
		serial0_send_buff(sen_lock,4);														//����
		rt_thread_delay(20);		
		DGUS_numsend(0x43,0,0,6);   //ִ�гɹ�
		rt_thread_delay(1000);
		PAGE_send(41);
	}
	if(save_sen[0]==2)	//��Ӯ
	{
		serial0_send_buff(sen_unlock,4);			//����
		rt_thread_delay(500);				//�ӻ�����0.25s
		send_add[4]=(16*type1+type2)/10+0x30;
		send_add[5]=(16*type1+type2)%10+0x30;
		CRCH = CRC16(send_add,7)>>8;
		CRCL = CRC16(send_add,7);
		send_add[8]=CRCH;
		send_add[7]=CRCL;
		serial0_send_buff(send_add,9);
		rt_thread_delay(500);				//�ӻ�����0.25s	
		serial0_send_buff(sen_lock,4);														//����

		rt_thread_delay(20);
		DGUS_numsend(0x43,0,0,6);   //ִ�гɹ�
		rt_thread_delay(1000); 
		PAGE_send(41);
	}
	rt_sem_release(usart_sem);					//�����ź���	
}

void Addr_ico(void)						//�����ĵ�ַˢͼ������
{
	rt_sem_take(usart_sem,RT_WAITING_FOREVER);			//��ȡ�����ź���
	DGUS_numsend(0x43,0,0,8);   //ִ����
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
	if(save_sen[0]==2)   //��Ӯ
	{
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x11,0,(save_sen[1]>>4)+1);
		rt_thread_delay(20);
		DGUS_numsend(0x42,0x10,0,save_sen[1]%16+1);
		rt_thread_delay(20);
		PAGE_send(41);
	}
	rt_sem_release(usart_sem);					//�����ź���
}


/**/



