 /**
  ******************************************************************************
  * @file           : userconfig.c
  * @brief          : ���ֱȽ���ɢ�Ĺ���
  ******************************************************************************
	*/
#include "main.h"
#include "stm32f1xx_hal.h"



/* ����1�¼����ƿ� */
struct rt_event event_u1;
/* ����1�豸��� */
static rt_device_t uart1_device = RT_NULL;
/* ����2�¼����ƿ� */
struct rt_event event_u2;
/* ����2�豸��� */
static rt_device_t uart2_device = RT_NULL;

/* ����1�ص����� */
static rt_err_t uart1_intput(rt_device_t dev, rt_size_t size)
{
    /* �����¼� */
//    rt_event_send(&event_u1, UART_RX_EVENT);

    return RT_EOK;
}

/* ����2�ص����� */
static rt_err_t uart2_intput(rt_device_t dev, rt_size_t size)
{
    /* �����¼� */
//    rt_event_send(&event_u2, UART_RX_EVENT);

    return RT_EOK;
}
/**/

/*��������1*/
rt_err_t uart1_open(void)
{
    rt_err_t res;

    /* ����ϵͳ�еĴ����豸 */
    uart1_device = rt_device_find("uart1");   
    /* ���ҵ��豸����� */
    if (uart1_device != RT_NULL)
    {

        res = rt_device_set_rx_indicate(uart1_device, uart1_intput);
        /* ��鷵��ֵ */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n","uart3",res);
            return -RT_ERROR;
        }

        /* ���豸���Կɶ�д���жϷ�ʽ */
        res = rt_device_open(uart1_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* ��鷵��ֵ */
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

    /* ��ʼ���¼����� */
    rt_event_init(&event_u1, "u1event", RT_IPC_FLAG_FIFO); 
		
		
		
    return RT_EOK;
}
/**/

/*��������2*/
rt_err_t uart2_open(void)
{
    rt_err_t res;

    /* ����ϵͳ�еĴ����豸 */
    uart2_device = rt_device_find("uart2");   
    /* ���ҵ��豸����� */
    if (uart2_device != RT_NULL)
    {

        res = rt_device_set_rx_indicate(uart2_device, uart2_intput);
        /* ��鷵��ֵ */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n","uart3",res);
            return -RT_ERROR;
        }

        /* ���豸���Կɶ�д���жϷ�ʽ */
        res = rt_device_open(uart2_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* ��鷵��ֵ */
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

    /* ��ʼ���¼����� */
    rt_event_init(&event_u2, "u2event", RT_IPC_FLAG_FIFO); 

    return RT_EOK;
}
/**/


/*���ڷ���
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



/*����1����*/
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
		rt_thread_delay(5);     //9600,5ms��5����
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
			 /* �����¼� */
//		rt_event_recv(&event_u1, UART_RX1_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);
	}
}

/*����2����*/
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
		rt_thread_delay(1);					//115200,1ms��10����
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
			if(u2_Rx[2]!=3)					//������Ļ5A A5 03 4F 4B ���Զ��ظ�
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
					/* �����¼� */
					rt_event_send(&event_u2, UART_RX2_EVENT);
				}
			}
		}
	}
}

/*������CRC
*
*	CRCH = vCRC16(Pointer,CrcLen)>>8; //crcУ������ֽ�
*	CRCL = vCRC16(Pointer,CrcLen);    //crc���ֽ�
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

/*���һ��������4K�ֽ�*/
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x0801F000) /* Base @ of Page 0, 1 Kbyte */
#define ADDR_FLASH_PAGE_1     ((uint32_t)0x0801F400) /* Base @ of Page 1, 1 Kbyte */
#define ADDR_FLASH_PAGE_2     ((uint32_t)0x0801F800) /* Base @ of Page 2, 1 Kbyte */
#define ADDR_FLASH_PAGE_3     ((uint32_t)0x0801FC00) /* Base @ of Page 3, 1 Kbyte */

void WriteFlash(void)
{
	rt_uint8_t i=0,j=0;
    //1������FLASH
  HAL_FLASH_Unlock();
	//2������FLASH
	//��ʼ��FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = ADDR_FLASH_PAGE_0;
	f.NbPages = 1;
	//����PageError
	uint32_t PageError = 0;
	//���ò�������
	HAL_FLASHEx_Erase(&f, &PageError);
	//3����FLASH��д

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

//FLASH��ȡ����
void ReadFlash(void)
{
	rt_uint8_t temp,i=0,j=0;
	uint16_t *Address = (uint16_t*)ADDR_FLASH_PAGE_0;     //���4K����ʼ��ַ
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

