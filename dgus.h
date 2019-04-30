/**
  ******************************************************************************
  * File Name          : dgus.h
  *    
  ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __dgus_H
#define __dgus_H
#ifdef __cplusplus
 extern "C" {
#endif


#include "main.h"


	 
extern	 rt_uint8_t equ_type;				//�豸���ͣ���ѹ������ѡһ
extern   rt_uint8_t press_lim;				//ѹ����Χ
extern	 rt_uint8_t keyword[3];				//�û���������
extern   rt_uint8_t addr_changeflag;		//�ĵ�ַ��־λ��������̽����ѯ
extern	 rt_uint8_t voice_flag;				//������־λ
extern   rt_uint8_t gas_name[60];    //����վ��
/**
  *  ̽��״̬��¼�ṹ��
  */
	 
struct	SEN_str
{
	
	rt_uint8_t type;										/*���������ѹ������������Һýʽ*/
	
  rt_uint8_t update_f;									/*ˢ�±�־λ��̽��״̬������1��־λ*/
	
	rt_uint8_t protocols;                /*Э��*/
	
	rt_uint8_t display_f;								/*��ʾ��ǰ̽��״̬��־λ*/
	
  rt_uint8_t int_num;                	/*��ͨ����*/

  rt_uint8_t state[16];                	/*��������ǰ״̬*/

	rt_uint8_t last_sta[16];                	/*��������һ��״̬*/
	
	rt_uint8_t alert[16];										/*����������¼*/

	rt_uint8_t press[8];											/*��¼ѹ�����˲�*/
	
	rt_uint8_t press_d[8];										/*ѹ������*/
	
	rt_uint8_t press_u[8];										/*ѹ������*/	

};    //�ֱ��¼�͹޹����˾�����״̬
/**/

	 
extern struct SEN_str	SEN_state[4];    //�ֱ��¼�͹޹����˾�����״̬
	 
void DGUS_numsend(rt_uint8_t addH,rt_uint8_t addL,rt_uint8_t numH,rt_uint8_t numL);
void DGUS_USART(void);					//dgus���ڷ��ص����ݴ�����dgus�߳�ִ��
void PAGE_send(rt_uint8_t page);		//
void DGUS_DataProce(void);
void alert_refresh(void);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

