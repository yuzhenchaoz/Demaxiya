/**
  ******************************************************************************
  * @file           : userconfig.h
  * @brief          : 
  *                   
	*	各种宏定义
	*
	*
	**/

#ifndef USERCONFIG_H__
#define USERCONFIG_H__

#define USE_DEBUG      //开启调试，串口打印调试信息用 #ifdef USE_DEBUG   #else   #endif   包含起来  正式版程序注释此宏定义就可以去掉所有打印信息

/*串口1使能*/
#define UART1_RXEN rt_pin_write(41, PIN_LOW);
#define UART1_TXEN rt_pin_write(41, PIN_HIGH);

/*继电器开关*/
#define K_OFF rt_pin_write(10, PIN_LOW);
#define K_ON rt_pin_write(10, PIN_HIGH);

/*设置主页面刷图起始页*/
#define  SEN_TYPE_P   23			//传感器压力类型，23页为起始页
#define  SEN_TYPE_G   11			//传感器普通类型, 11页为起始页
	 
/*设置故障宏定义，dgus图标序号*/
#define ERR      			 0x02                 //通讯故障
#define OIL     			 0x04									//油
#define WATER    			 0x01									//水
#define HIGH    			 0x06									//高液位
#define LOW    			   0x07									//低液位
#define NORM     			 0x00									//正常
#define LOW_PRESS    	 0x02									//压力报警
#define NORM_PRESS	   0x00									//压力正常
#define ERR_PRESS	  	 0x03									//压力故障

/*传感器类型标志位宏定义*/
#define SENNORM         0x00						  			//普通传感器
#define SENYEMEI        0x01										//液媒传感器
#define SENPRESS        0x02										//压力传感器

/*传感器协议判断*/
#define QDWT             0x00
#define CPPEI            0x01

/*心跳目标IP、端口和时间间隔设置*/
#define UDPBEATIP     "255.255.255.255"    //广播ip
#define UDPBEATPORT    "3486"      //心跳包目标端口
#define UDPBEATTIM    10        //秒，心跳包发送间隔

/* WIZnet network configure */

/*网络部分宏定义*/

#define NET_CACHE  72         //报警记录缓存条数

#define NET_SENERR        (1<<0)     //网络协议
#define NET_SHORT         (1<<1)
#define NET_OPEN          (1<<2)
#define NET_SENOIL        (1<<3)
#define NET_SENWATER      (1<<4)
#define NET_LIQHIGH       (1<<5)
#define NET_LIQLOW        (1<<6)
#define NET_LEAK          (1<<7)
#define NET_PRESSPREWARN  (1<<8)
#define NET_PRESSWARN     (1<<9)
#define NET_OILGASPREW    (1<<10)
#define NET_OILGASWARN    (1<<11)
#define NET_NOTOPEN       (1<<12)
#define NET_COMMERR       (1<<13)



//#define LOCALIP "192.168.1.177"
//#define LOCALGW "192.168.1.137"
//#define LOCALMSK "255.255.255.0"

/*    RAM使用情况：  ROM基本使用满    RBT6基本使用满 
thread   pri  status      sp     stack size max used left tick  error
-------- ---  ------- ---------- ----------  ------  ---------- ---
tcpcli    16  suspend 0x00000138 0x00000600    53%   0x00000006 000
udpsend   14  suspend 0x000000e4 0x00000600    53%   0x0000000f 000
udprec    15  suspend 0x00000138 0x00000400    42%   0x00000013 000
alert     13  suspend 0x00000070 0x00000200    40%   0x00000029 000
Voice      9  suspend 0x00000070 0x00000100    53%   0x00000002 000
DGUS       7  suspend 0x00000098 0x00000168    62%   0x00000024 000
sendeal    6  suspend 0x000000b0 0x00000400    42%   0x00000004 000
receive2  12  suspend 0x00000070 0x00000168    55%   0x00000004 000
receive1  11  suspend 0x00000070 0x00000100    62%   0x0000000c 000
tshell    20  ready   0x00000104 0x00000200    67%   0x00000008 000
wiz        5  suspend 0x00000090 0x00000200    34%   0x00000002 000
tidle0    31  ready   0x00000040 0x00000100    34%   0x00000017 000
timer      4  suspend 0x00000058 0x00000100    34%   0x0000000a 000
*/



#endif
/*****END OF FILE****/
