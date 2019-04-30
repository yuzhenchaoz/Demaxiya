/**
  ******************************************************************************
  * @file           : userconfig.h
  * @brief          : 
  *                   
	*	���ֺ궨��
	*
	*
	**/

#ifndef USERCONFIG_H__
#define USERCONFIG_H__

#define USE_DEBUG      //�������ԣ����ڴ�ӡ������Ϣ�� #ifdef USE_DEBUG   #else   #endif   ��������  ��ʽ�����ע�ʹ˺궨��Ϳ���ȥ�����д�ӡ��Ϣ

/*����1ʹ��*/
#define UART1_RXEN rt_pin_write(41, PIN_LOW);
#define UART1_TXEN rt_pin_write(41, PIN_HIGH);

/*�̵�������*/
#define K_OFF rt_pin_write(10, PIN_LOW);
#define K_ON rt_pin_write(10, PIN_HIGH);

/*������ҳ��ˢͼ��ʼҳ*/
#define  SEN_TYPE_P   23			//������ѹ�����ͣ�23ҳΪ��ʼҳ
#define  SEN_TYPE_G   11			//��������ͨ����, 11ҳΪ��ʼҳ
	 
/*���ù��Ϻ궨�壬dgusͼ�����*/
#define ERR      			 0x02                 //ͨѶ����
#define OIL     			 0x04									//��
#define WATER    			 0x01									//ˮ
#define HIGH    			 0x06									//��Һλ
#define LOW    			   0x07									//��Һλ
#define NORM     			 0x00									//����
#define LOW_PRESS    	 0x02									//ѹ������
#define NORM_PRESS	   0x00									//ѹ������
#define ERR_PRESS	  	 0x03									//ѹ������

/*���������ͱ�־λ�궨��*/
#define SENNORM         0x00						  			//��ͨ������
#define SENYEMEI        0x01										//Һý������
#define SENPRESS        0x02										//ѹ��������

/*������Э���ж�*/
#define QDWT             0x00
#define CPPEI            0x01

/*����Ŀ��IP���˿ں�ʱ��������*/
#define UDPBEATIP     "255.255.255.255"    //�㲥ip
#define UDPBEATPORT    "3486"      //������Ŀ��˿�
#define UDPBEATTIM    10        //�룬���������ͼ��

/* WIZnet network configure */

/*���粿�ֺ궨��*/

#define NET_CACHE  72         //������¼��������

#define NET_SENERR        (1<<0)     //����Э��
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

/*    RAMʹ�������  ROM����ʹ����    RBT6����ʹ���� 
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
