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


	 
extern	 rt_uint8_t equ_type;				//设备类型，单压力或三选一
extern   rt_uint8_t press_lim;				//压力范围
extern	 rt_uint8_t keyword[3];				//用户储存密码
extern   rt_uint8_t addr_changeflag;		//改地址标志位，用来关探杆轮询
extern	 rt_uint8_t voice_flag;				//语音标志位
extern   rt_uint8_t gas_name[60];    //加油站名
/**
  *  探杆状态记录结构体
  */
	 
struct	SEN_str
{
	
	rt_uint8_t type;										/*传感器类别，压力，传感器，液媒式*/
	
  rt_uint8_t update_f;									/*刷新标志位，探杆状态更新置1标志位*/
	
	rt_uint8_t protocols;                /*协议*/
	
	rt_uint8_t display_f;								/*显示当前探杆状态标志位*/
	
  rt_uint8_t int_num;                	/*开通个数*/

  rt_uint8_t state[16];                	/*传感器当前状态*/

	rt_uint8_t last_sta[16];                	/*传感器上一次状态*/
	
	rt_uint8_t alert[16];										/*报警次数记录*/

	rt_uint8_t press[8];											/*记录压力，滤波*/
	
	rt_uint8_t press_d[8];										/*压力调节*/
	
	rt_uint8_t press_u[8];										/*压力调节*/	

};    //分别记录油罐管线人井油盆状态
/**/

	 
extern struct SEN_str	SEN_state[4];    //分别记录油罐管线人井油盆状态
	 
void DGUS_numsend(rt_uint8_t addH,rt_uint8_t addL,rt_uint8_t numH,rt_uint8_t numL);
void DGUS_USART(void);					//dgus串口返回的数据处理，在dgus线程执行
void PAGE_send(rt_uint8_t page);		//
void DGUS_DataProce(void);
void alert_refresh(void);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

