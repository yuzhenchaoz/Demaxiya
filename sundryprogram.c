 /**
  ******************************************************************************
  * @file           : userconfig.c
  * @brief          : 各种比较零散的功能
  ******************************************************************************
	*/
#include "main.h"
#include "stm32f1xx_hal.h"



/* 串口1事件控制块 */
struct rt_event event_u1;
/* 串口1设备句柄 */
static rt_device_t uart1_device = RT_NULL;
/* 串口2事件控制块 */
struct rt_event event_u2;
/* 串口2设备句柄 */
static rt_device_t uart2_device = RT_NULL;

/* 串口1回调函数 */
static rt_err_t uart1_intput(rt_device_t dev, rt_size_t size)
{
    /* 发送事件 */
//    rt_event_send(&event_u1, UART_RX_EVENT);

    return RT_EOK;
}

/* 串口2回调函数 */
static rt_err_t uart2_intput(rt_device_t dev, rt_size_t size)
{
    /* 发送事件 */
//    rt_event_send(&event_u2, UART_RX_EVENT);

    return RT_EOK;
}
/**/

/*开启串口1*/
rt_err_t uart1_open(void)
{
    rt_err_t res;

    /* 查找系统中的串口设备 */
    uart1_device = rt_device_find("uart1");   
    /* 查找到设备后将其打开 */
    if (uart1_device != RT_NULL)
    {

        res = rt_device_set_rx_indicate(uart1_device, uart1_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n","uart3",res);
            return -RT_ERROR;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart1_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n","uart3",res);
            return -RT_ERROR;
        }

    }
    else
    {
        rt_kprintf("can't find %s device.\n","uart3");
        return -RT_ERROR;
    }

    /* 初始化事件对象 */
    rt_event_init(&event_u1, "u1event", RT_IPC_FLAG_FIFO); 
		
		
		
    return RT_EOK;
}
/**/

/*开启串口2*/
rt_err_t uart2_open(void)
{
    rt_err_t res;

    /* 查找系统中的串口设备 */
    uart2_device = rt_device_find("uart2");   
    /* 查找到设备后将其打开 */
    if (uart2_device != RT_NULL)
    {

        res = rt_device_set_rx_indicate(uart2_device, uart2_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n","uart3",res);
            return -RT_ERROR;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart2_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n","uart3",res);
            return -RT_ERROR;
        }

    }
    else
    {
        rt_kprintf("can't find %s device.\n","uart3");
        return -RT_ERROR;
    }

    /* 初始化事件对象 */
    rt_event_init(&event_u2, "u2event", RT_IPC_FLAG_FIFO); 

    return RT_EOK;
}
/**/


/*串口发送
*
* uart_Transmit(uart1_device, buf, size);
*
*/
void uart1_send(const rt_uint8_t *buf,rt_size_t size)
{
	UART1_TXEN;
	rt_device_write(uart1_device, 0, buf, size);
	UART1_RXEN;
}

void uart2_send(const rt_uint8_t *buf,rt_size_t size)
{
	rt_device_write(uart2_device, 0, buf, size);
}
/**/



/*串口1接收*/
void uart1_receive(void)
{
	rt_uint8_t Rx[20];
	rt_uint8_t i,ch1=0,ch2=0;
	if((ch1=rt_device_read(uart1_device, 0, &Rx, 20)) != 0)
	{
		for(i=0;i<ch1;i++)
		{
			if(i<20)
			{
				u1_Rx[i]=Rx[i];
			}
		}
		ch2=ch1;
		rt_thread_delay(5);     //9600,5ms收5左右
		while((ch1=rt_device_read(uart1_device, 0, &Rx, 20)) != 0)
		{
			for(i=0;i<ch1;i++)
			{
				if((ch2+i)<40)
				{
					u1_Rx[ch2+i]=Rx[i];
				}
			}
			ch2=ch1+ch2;
			u1_Rx[39]=ch2;
			rt_thread_delay(5);
		}
		rt_event_send(&event_u1, UART_RX1_EVENT);
//		uart1_send(u1_Rx,20);
			 /* 接收事件 */
//		rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);
	}
}

/*串口2接收*/
void uart2_receive(void)
{
	rt_uint8_t Rx[60];
	rt_uint8_t i,ch1=0,ch2=0;
	if((ch1=rt_device_read(uart2_device, 0, &Rx, 60)) != 0)
	{
		for(i=0;i<ch1;i++)
		{
			if(i<60)
			{
				u2_Rx[i]=Rx[i];
			}
		}
		ch2=ch1;
		rt_thread_delay(1);					//115200,1ms收10左右
		while((ch1=rt_device_read(uart2_device, 0, &Rx, 60)) != 0)
		{
			for(i=0;i<ch1;i++)
			{
				if((ch2+i)<60)
				{
					u2_Rx[ch2+i]=Rx[i];
				}
			}
			ch2=ch1+ch2;
			rt_thread_delay(1);
		}
		if(u2_Rx[0]==0x5A&&u2_Rx[1]==0xA5)
		{
			if(u2_Rx[2]!=3)					//屏蔽屏幕5A A5 03 4F 4B 的自动回复
			{
				if(u2_Rx[4]==0x00&&u2_Rx[5]==0x10&&u2_Rx[6]==0x04)
				{
					sys_time[0]=u2_Rx[7];
					sys_time[1]=u2_Rx[8];
					sys_time[2]=u2_Rx[9];
					sys_time[3]=u2_Rx[11];
					sys_time[4]=u2_Rx[12];
					sys_time[5]=u2_Rx[13];
				}
				else
				{
					/* 接收事件 */
					rt_event_send(&event_u2, UART_RX2_EVENT);
				}
			}
		}
	}
}

/*先来波CRC
*
*	CRCH = vCRC16(Pointer,CrcLen)>>8; //crc校验码高字节
*	CRCL = vCRC16(Pointer,CrcLen);    //crc低字节
*	host_SendBuf3[6] = CRCH;
*	host_SendBuf3[7] = CRCL;	
*
*
*/
uint16_t CRC16(rt_uint8_t *CrcBuf,rt_uint8_t CrcLen)
{
	rt_uint8_t i,j;
    uint16_t CrcSumx;
    CrcSumx = 0xFFFF;
    for (i=0; i<CrcLen; i++)
    {
        CrcSumx ^= *(CrcBuf+i);
        for (j=0; j<8; j++)
        {
            if(CrcSumx&0x01)
            {
                CrcSumx>>=1;
                CrcSumx^=0xA001;
            }
            else
            {
                CrcSumx>>=1;
            }
        }
    }
   return CrcSumx;
}
/**/



/***FLASH***/
/*
HAL_FLASH_Unlock(); 
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError); 
HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data); 
HAL_FLASH_Lock(); 
*/

/*最后一个扇区，4K字节*/
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x0801F000) /* Base @ of Page 0, 1 Kbyte */
#define ADDR_FLASH_PAGE_1     ((uint32_t)0x0801F400) /* Base @ of Page 1, 1 Kbyte */
#define ADDR_FLASH_PAGE_2     ((uint32_t)0x0801F800) /* Base @ of Page 2, 1 Kbyte */
#define ADDR_FLASH_PAGE_3     ((uint32_t)0x0801FC00) /* Base @ of Page 3, 1 Kbyte */

void WriteFlash(void)
{
	rt_uint8_t i=0,j=0;
    //1、解锁FLASH
  HAL_FLASH_Unlock();
	//2、擦除FLASH
	//初始化FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = ADDR_FLASH_PAGE_0;
	f.NbPages = 1;
	//设置PageError
	uint32_t PageError = 0;
	//调用擦除函数
	HAL_FLASHEx_Erase(&f, &PageError);
	//3、对FLASH烧写

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, equ_type);		i+=2;

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, press_lim);		i+=2;

	
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, SEN_state[0].type);		i+=2;	

	for(j=0;j<3;j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i,keyword[j]);		i+=2;
	}
	for(j=0;j<4;j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, SEN_state[j].int_num);		i+=2;		
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, SEN_state[j].protocols);		i+=2;	
	}
	for(j=0;j<8;j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, SEN_state[0].press_d[j]);		i+=2;	
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, SEN_state[0].press_u[j]);		i+=2;		
	}
	for(j=0;j<60;j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ADDR_FLASH_PAGE_0+i, gas_name[j]);		i+=2;
	}
	HAL_FLASH_Lock();
}

//FLASH读取数据
void ReadFlash(void)
{
	rt_uint8_t temp,i=0,j=0;
	uint16_t *Address = (uint16_t*)ADDR_FLASH_PAGE_0;     //最后4K的起始地址
//	uint16_t *Address1 = (uint16_t*)ADDR_FLASH_PAGE_1;
	temp=*Address;
	if(temp==0xFF)
	{
		WriteFlash();
	}
	else
	{
		equ_type=*(Address+i);  	i+=1;
		press_lim=*(Address+i);  	i+=1;
		SEN_state[0].type=*(Address+i);		i+=1;
		for(j=0;j<3;j++)
		{
			keyword[j]=*(Address+i);		i+=1;
		}
		for(j=0;j<4;j++)
		{
			SEN_state[j].int_num=*(Address+i);		i+=1;		
			SEN_state[j].protocols=*(Address+i);		i+=1;	
		}
		for(j=0;j<8;j++)
		{
			SEN_state[0].press_d[j]=*(Address+i);		i+=1;
			SEN_state[0].press_u[j]=*(Address+i);		i+=1;		
		}
		for(j=0;j<60;j++)
		{
			gas_name[j]=*(Address+i);		i+=1;
		}
	}
//  uint32_t temp = *(__IO uint32_t*)(FLASH_TYPEERASE_PAGES);

}

