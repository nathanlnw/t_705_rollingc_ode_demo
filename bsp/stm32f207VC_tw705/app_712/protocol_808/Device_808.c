/*
     Device_808.C       ��808   Э����ص� I/O �ܽ�����     
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"


#define ADC1_DR_Address  ((uint32_t)0X4001204C)

u16 ADC_ConValue[3];   //   3  ��ͨ��ID    0 : ��� 1: ����   2:  ����
u16   AD_2through[2]; //  ����2 ·AD ����ֵ


u8  HardWareVerion=0;   //   Ӳ���汾��� 
//-----  WachDog related----
u8    wdg_reset_flag=0;    //  Task Idle Hook ���

//--------  ��ѹ��� ��� ---------------------------------
AD_POWER  Power_AD; 







//------------  AD    ��ѹ���  -------------------- 
void AD_PowerInit(void)
{
   Power_AD.ADC_ConvertedValue=0; //��ص�ѹAD��ֵ    
   Power_AD.AD_Volte=0;      // �ɼ�����ʵ�ʵ�ѹ��ֵ
   Power_AD.Classify_Door=160;   //  ���ִ�С�����ͣ�  >16V  ���ͳ� <16V С�ͳ� 
   Power_AD.LowWarn_Limit_Value=10;  //  Ƿѹ��������ֵ      
}

u8  HardWareGet(void)   
{  //  ��ȡӲ���汾��Ϣ   
   // -----    Ӳ���汾״̬��� init  ----------------------
   /*
	PA13	1	����Ӳ���汾�ж�
	PA14	1	����Ӳ���汾�ж�
	PB3       0	����Ӳ���汾�ж�
    */
   u8   Value=0;

     //-----------------------------------------------------------  
     if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_13))  // bit 2 
	 	   Value|=0x04;
	 else
	 	   Value&=~0x04;  
     //----------------------------------------------------------
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_14))  // bit 1
	 	   Value|=0x02;
	 else
	 	   Value&=~0x02;   
	//------------------------------------------------------------ 
	 if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)) // bit0 
	 	   Value|=0x01; 
	 else
	 	   Value&=~0x01;  
	 //------------------------------------------------------------
      rt_kprintf("\r\n  Ӳ���汾��ȡ: %2X",Value);      
     return Value;
}
//FINSH_FUNCTION_EXPORT(HardWareGet, HardWareGet); 


void WatchDog_Feed(void)
{
    if(wdg_reset_flag==0)
           IWDG_ReloadCounter();   
}

void  reset(void)
{
   IWDG_SetReload(0);
   IWDG->KR = 0x00001;  //not regular
  wdg_reset_flag=1; 
} 
FINSH_FUNCTION_EXPORT(reset, ststem reset);
void WatchDogInit(void)
{    
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 

  /* IWDG counter clock: LSI/32 */
    /*   prescaler            min/ms    max/ms
         4                        0.1             409.6
         8                        0.2             819.2
         16                      0.4             1638.4
         32                      0.8              3276.8
         64                      1.6              6553.5
         128                    3.2              13107.2
         256                    6.4              26214.4   
  */
  IWDG_SetPrescaler(IWDG_Prescaler_16);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(0X4AAA);//(LsiFreq/128);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

void  APP_IOpinInit(void)   //��ʼ�� �͹�����ص�IO �ܽ�
{
  	GPIO_InitTypeDef        gpio_init;

     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);      
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	 

    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz; 
    gpio_init.GPIO_OType = GPIO_OType_PP;  
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	 
 // 		IN
	//------------------- PE8 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_8;	  //��������
	gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
	GPIO_Init(GPIOE, &gpio_init);
	//------------------- PE9 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------ACC  ״̬
	gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
	GPIO_Init(GPIOE, &gpio_init);
	//------------------- PE7 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_7;				//------���ſ���״̬  0 ��Ч  ��̬��Ϊ��   
	gpio_init.GPIO_Mode  = GPIO_Mode_IN;   //���ֻ��ɲ�����Ǿ���PE5��ɲ������ 
	GPIO_Init(GPIOE, &gpio_init); 
 
   //	OUT
   
   //------------------- PB1 -----------------------------
   gpio_init.GPIO_Pin	= GPIO_Pin_1;   //------δ����   ��� ��̬��0  
   gpio_init.GPIO_Mode	= GPIO_Mode_OUT; 
   GPIO_Init(GPIOB, &gpio_init); 

 //==================================================================== 
 //-----------------------д�̵�����̬�µ����------------------
// GPIO_ResetBits(GPIOB,GPIO_Pin_1);	 //�����̬ �� 0 
 GPIO_SetBits(GPIOB,GPIO_Pin_1);	 //�����̬ �� 0     

// GPIO_ResetBits(GPIOA,GPIO_Pin_13);	 // �رշ�����          
 /*
      J1 �ӿ� ��ʼ��
 */
	 //---------- PA0--------------------- 
  // gpio_init.GPIO_Pin  =GPIO_Pin_0; 				//-----  PIN 1   �ٶȴ�����   // ��ʱ����
  // gpio_init.GPIO_Mode = GPIO_Mode_IN;
   //GPIO_Init(GPIOA, &gpio_init);
   //------------- -- --------------
   //gpio_init.GPIO_Pin	 = GPIO_Pin_6;				//------PIN  2   NULL 
  // gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   //GPIO_Init(GPIOE, &gpio_init);
    //--------------- --------------
   //gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------PIN 3    NULL 
   //gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   //GPIO_Init(GPIOD, &gpio_init);
    //------------- PC0 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_0;				//------PIN 4    Զ���
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init); 
    //------------- ----------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 5   Ԥ��  ���ŵ�
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init); 
    //------------- PA1 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 6   ����
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOA, &gpio_init);
    //------------- PC3 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_3;				//------PIN 7   ��ת�� 
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init);
    //------------- PC2 --------------   
   gpio_init.GPIO_Pin	 = GPIO_Pin_2;				//------PIN 8   ��ת��   
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init);  
         
    //------------- PE11 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_11;				//------PIN 9   ɲ����
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOE, &gpio_init);
    //------------- PE10 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_10;				//------PIN 10  ��ˢ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOE, &gpio_init);  
 
   //-----------------------------------------------------------------
   

   //------- �ٶ��ź��� ---------------
    gpio_init.GPIO_Pin = GPIO_Pin_0; 
    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz; 
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL; 
    GPIO_Init(GPIOA, &gpio_init); 
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2); 


#if 1
   // -----    Ӳ���汾״̬��� init  ---------------------- 
   /*
	PA13	1	����Ӳ���汾�ж�
	PA14	1	����Ӳ���汾�ж�
	PB3       0	����Ӳ���汾�ж�
    */
       //------------- PA13 --------------   
   gpio_init.GPIO_Pin	 = GPIO_Pin_13;				
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOA, &gpio_init);  
         
    //------------- PA14 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_14;				
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOA, &gpio_init);
    //------------- PB3 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_3;				
   gpio_init.GPIO_Mode  = GPIO_Mode_IN;  
   GPIO_Init(GPIOB, &gpio_init);  
#endif   

}





/*    
     -----------------------------
    1.    ����ܽ�״̬���
     ----------------------------- 
*/
u8  ACC_StatusGet(void)
{  
    // ACC ״̬����   
    //����  ��0 Ϊ  ACC ��   
    //����  0  Ϊ   ACC ��
   return(!GPIO_ReadInputDataBit(ACC_IO_Group,ACC_Group_NUM)); 
}

u8  WARN_StatusGet(void)
{
    // �������� ״̬����   
    //����  ��0 Ϊ  ������ť����  
    //����  0  Ϊ   ������ť�Ͽ�
   return (!GPIO_ReadInputDataBit(WARN_IO_Group,WARN_Group_NUM));   
}
u8  MainPower_cut(void)
{ 
    // �ϵ籨�� ״̬����   
    //����  ��0 Ϊ  ����Ͽ�  
    //����  0  Ϊ   ���繤�� 
 // return (GPIO_ReadInputDataBit(POWER_IO_Group,POWER_Group_NUM)&0x01) ; 
   return false;
}

//-------------------------------------------------------------------------------------------------
u8  BreakLight_StatusGet(void)
{ 
	//	--------------J1pin8	PE11	         ɲ����------> ��
	   return(!GPIO_ReadInputDataBit(BREAK_IO_Group,BREAK_Group_NUM));	//PE11
			//		 �Ӹ�  ����
}
u8  LeftLight_StatusGet(void)
{
  //  --------------J1pin10	   PE10		   ��ת��------>��
	   return (!GPIO_ReadInputDataBit(LEFTLIGHT_IO_Group,LEFTLIGHT_Group_NUM));	//	PE10
			//		 �Ӹ�  ����
}	
u8  RightLight_StatusGet(void)
{
//	--------------J1pin8   PC2			  ��ת��------>��
		 return(!GPIO_ReadInputDataBit(RIGHTLIGHT_IO_Group,RIGHTLIGHT_Group_NUM));  //PC2 
		     //	   �Ӹ�  ���� 
}			

u8  FarLight_StatusGet(void)
{
  //  --------------J1pin4		PC0	           Զ���-----> ��
	   return (!GPIO_ReadInputDataBit(FARLIGHT_IO_Group,FARLIGHT_Group_NUM));	// PC0
	//		 �Ӹ�  ����
}	
u8  NEARLight_StatusGet(void)
{
  //  --------------J1pin5		PA6	          �����------>  ��
	   return (!GPIO_ReadInputDataBit(NEARLIGHT_IO_Group,NEARLIGHT_Group_NUM));	// PA6
	//		 �Ӹ�  ����
}

u8 FOGLight_StatusGet(void)
{
// --------------J1pin7    PA7		    ���     ------>   ��
		return(!GPIO_ReadInputDataBit(FOGLIGHT_IO_Group,FOGLIGHT_Group_NUM));  //PA7 
			//		 �Ӹ�  ����
}	
u8  DoorLight_StatusGet(void)
{
 //  --------------J1pin6	   PA1		   ���ŵ�    ------>��
	   return (!GPIO_ReadInputDataBit(DOORLIGHT_IO_Group,DOORLIGHT_Group_NUM));	// PA1
			//		 �Ӹ�  ����
}		


/*    
     -----------------------------
    2.  �������
     ----------------------------- 
*/
void  Enable_Relay(void)
{  // �Ͽ��̵���
 
 GPIO_SetBits(RELAY_IO_Group,RELAY_Group_NUM); // �Ͽ�	
}
void  Disable_Relay(void)
{ // ��ͨ�̵���
   
   GPIO_ResetBits(RELAY_IO_Group,RELAY_Group_NUM); // ͨ	 
}


u8  Get_SensorStatus(void)   
{        // ��ѯ������״̬
   u8  Sensorstatus=0;
   
   /*  
	   -------------------------------------------------------------
				F4	�г���¼�� TW705   �ܽŶ���
	   -------------------------------------------------------------
	   ��ѭ  GB10956 (2012)  Page26  ��A.12  �涨
	  -------------------------------------------------------------
	  | Bit  |		Note	   |  �ر�|   MCUpin  |   PCB pin  |   Colour | ADC
	  ------------------------------------------------------------
		  D7	  ɲ��			 *			  PE11			   9				��
		  D6	  ��ת��	 *			   PE10 		   10				��
		  D5	  ��ת��	 *			   PC2				8				 ��
		  D4	  Զ���	 *			   PC0				4				 ��
		  D3	  �����	 *			   PA6				5				 ��
		  D2	  ���		       	add 		 PA7			  7 			   ��	   *
		  D1	  ����			 add 		 PA1			  6 			   ��	   *
		  D0	  Ԥ��
	 */


   //  1.   D7      ɲ��           *            PE11             J1 pin9                ��
		  if(BreakLight_StatusGet())  //PE11  
		   {   //		�Ӹ�  ����
			   Sensorstatus|=0x80;
			  BD_EXT.FJ_IO_1 |=0x80;  //  bit7 
			  BD_EXT.Extent_IO_status |= 0x10;  // bit4 ---->ɲ��
			   // rt_kprintf("\r\n ����1"); 
		   }
		  else
		   {  //   ��̬
			   Sensorstatus&=~0x80;		
			    BD_EXT.FJ_IO_1 &=~0x80;  //  bit7 
			    BD_EXT.Extent_IO_status &= ~0x10; //bit4 ---->ɲ��
		   } 
	// 2.  D6      ��ת��     *             PE10            J1 pin10              ��
		 if(LeftLight_StatusGet())	//	PE10 
		  {   //	   �Ӹ�  ����
			  Sensorstatus|=0x40;
			   BD_EXT.FJ_IO_1 |=0x40;  //  bit6
			   BD_EXT.Extent_IO_status |= 0x08;//bit3---->  ��ת��
		  }
		 else
		  {  //   ��̬
			 Sensorstatus&=~0x40; 	
			 BD_EXT.FJ_IO_1 &=~0x40;  //  bit6 
			 BD_EXT.Extent_IO_status &= ~0x08; //bit3---->  ��ת��
		  }
   //  3.  D5      ��ת��     *             PC2             J1  pin8                ��
	     if(RightLight_StatusGet())	//PC2 
	     { //		 �Ӹ�  ����
				Sensorstatus|=0x20;
				 BD_EXT.FJ_IO_1 |=0x20; //bit5
				 BD_EXT.Extent_IO_status |= 0x04;// bit2----> ��ת��
		}
            else
		{  //	��̬
			   Sensorstatus&=~0x20;		
		   BD_EXT.FJ_IO_1 &=~0x20; //bit5
		   BD_EXT.Extent_IO_status &= ~0x04;//bit2----> ��ת��
		} 
   
   // 4.  D4      Զ���     *             PC0              J1 pin4                ��
	   if(FarLight_StatusGet())	// PC0 
		{	//		 �Ӹ�  ����
			Sensorstatus|=0x10;
			BD_EXT.Extent_IO_status |= 0x02; //bit 1  ----->  Զ���

		}
	   else
		{  //	��̬
		   Sensorstatus&=~0x10;		
		   BD_EXT.Extent_IO_status&= ~0x02;//bit 1  ----->  Զ���

		}  
    //5.   D3      �����     *             PC1              J1 pin5                ��
   		 if(NEARLight_StatusGet())  // PC1
		  {   //       �Ӹ�  ����
		      Sensorstatus|=0x08;
		       BD_EXT.FJ_IO_1 |=0x10; //bit4	  
		       BD_EXT.Extent_IO_status |= 0x01; //bit 0  ----->  �����
		  }
		 else
		  {  //	  ��̬
			 Sensorstatus&=~0x08;		
			  BD_EXT.FJ_IO_1 &=~0x10; //bit4
			   BD_EXT.Extent_IO_status &=~0x01; //bit 0  ----->  ����� 
			  
		  } 
  //  6.    D2      ���          add          PC3              7                ��      *
          if(FOGLight_StatusGet())  //PC3  
		  {   //	   �Ӹ�  ����
			  Sensorstatus|=0x04;
			  BD_EXT.FJ_IO_1 |=0x08; //bit3	
			  BD_EXT.Extent_IO_status |= 0x40;//  bit6 ----> ���
		  }
		 else
		  {  //   ��̬
			  Sensorstatus&=~0x04;
			  BD_EXT.FJ_IO_1 &=~0x08; //bit3
			  BD_EXT.Extent_IO_status &= ~0x40;//  bit6 ----> ���
		  } 
  // 7.    D1      ����          add          PA1              6                ��      *
	    if(DoorLight_StatusGet())  // PE3     
		{	//		 �Ӹ�  ����
			Sensorstatus|=0x02;
			 BD_EXT.FJ_IO_2 |=0x01; //bit2       
		}
	   else
		{  //	��̬
		       Sensorstatus&=~0x02;		
		       BD_EXT.FJ_IO_2 |=0x01; //bit2 
		}	
 			   
 //    8.  Reserved


   return Sensorstatus;
}

void  IO_statusCheck(void)
{
      Vehicle_sensor=Get_SensorStatus();
	  //------------ 0.2s    �ٶ�״̬ -----------------------
	  Sensor_buf[save_sensorCounter].DOUBTspeed=Spd_Using/10;     //   �ٶ�  ��λ��km/h ���Գ���10
      Sensor_buf[save_sensorCounter++].DOUBTstatus=Vehicle_sensor;//   ״̬ 
		if(save_sensorCounter>100) 
			{
              save_sensorCounter=0; 
			  sensor_writeOverFlag=1; 
			}  
      //-------------------------------------------------- 
}

void  ACC_status_Check(void)
{
                 //------------��������״ָ̬ʾ ---------------   
			    if(ACC_StatusGet())           //bit 0
				{	
				   Vehicle_RunStatus|=0x01;
		           //   ACC ON		 ��� 
				   StatusReg_ACC_ON();  // ACC  ״̬�Ĵ���   
				   Sleep_Mode_ConfigExit(); // �������
			    }
				else
				{
				   Vehicle_RunStatus&=~0x01;
				   //	  ACC OFF	   �ػ�
				   StatusReg_ACC_OFF();  // ACC  ״̬�Ĵ��� 		  
				   Sleep_Mode_ConfigEnter(); // �������
				}  
}

/*************************************************
Function:    void GPIO_Config(void)       
Description: GPIO���ú���              
Input: ��                              
Output:��                              
Return:��                              
*************************************************/ 
void GPIO_Config_PWM(void)
{
/*������һ��GPIO_InitStructure�Ľṹ�壬����һ��ʹ�� */
GPIO_InitTypeDef GPIO_InitStructure;
/* ʹ��GPIOGʱ�ӣ�ʱ�ӽṹ�μ���stm32ͼ��.pdf����*/
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
/*�����ýṹ���еĲ��ֳ�Ա����������£��û�Ӧ�����ȵ��ú���PPP_SturcInit(..)
����ʼ������PPP_InitStructure��Ȼ�����޸�������Ҫ�޸ĵĳ�Ա���������Ա�֤����
��Ա��ֵ����Ϊȱʡֵ������ȷ���롣
 */
GPIO_StructInit(&GPIO_InitStructure);

/*����GPIOA_Pin_8����ΪTIM1_Channel2 PWM���*/
//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_5; //ָ����������
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //ָ����������
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //ģʽ����Ϊ���ã�
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //Ƶ��Ϊ����
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //��������PWM������Ӱ��
GPIO_Init(GPIOA, &GPIO_InitStructure);

//GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //����GPIOA_Pin1ΪTIM2_Ch2
GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //����GPIOA_Pin5ΪTIM2_Ch1, 
}

/*************************************************
Function:    void TIM_Config(void)  
Description: ��ʱ�����ú���       
Input:       ��
Output:      ��                            
*************************************************/
void TIM_Config_PWM(void)
{
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
TIM_DeInit(TIM2);//��ʼ��TIM2�Ĵ���
/*��Ƶ�����ڼ��㹫ʽ��
  Prescaler = (TIMxCLK / TIMx counter clock) - 1;
  Period = (TIMx counter clock / TIM3 output clock) - 1 
  TIMx counter clockΪ������Ҫ��TXM�Ķ�ʱ��ʱ�� 
  */
TIM_TimeBaseStructure.TIM_Period = 10-1; //�������ֲ��֪��TIM2��TIM5Ϊ32λ�Զ�װ�أ���������
/*��system_stm32f4xx.c�����õ�APB1 Prescaler = 4 ,��֪
  APB1ʱ��Ϊ168M/4*2,��Ϊ���APB1��Ƶ��Ϊ1����ʱʱ��*2 
 */
TIM_TimeBaseStructure.TIM_Prescaler = 2100-1;
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

/*��������Ƚϣ�����ռ�ձ�Ϊ20%��PWM����*/
TIM_OCStructInit(&TIM_OCInitStructure);
TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//PWM1Ϊ����ռ�ձ�ģʽ��PWM2Ϊ������ģʽ
TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//TIM_OCInitStructure.TIM_Pulse = 2;//����CCR��ռ�ձ���ֵ��
TIM_OCInitStructure.TIM_Pulse = 5;//����CCR��ռ�ձ���ֵ��
TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//HighΪռ�ձȸ߼��ԣ���ʱռ�ձ�Ϊ20%��Low��Ϊ�����ԣ�ռ�ձ�Ϊ80%

TIM_OC1Init(TIM2, &TIM_OCInitStructure);
TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);//CCR�Զ�װ��Ĭ��Ҳ�Ǵ򿪵�

TIM_ARRPreloadConfig(TIM2, ENABLE);  //ARR�Զ�װ��Ĭ���Ǵ򿪵ģ����Բ�����

TIM_ClearFlag(TIM2, TIM_FLAG_Update);
TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE);
TIM_Cmd(TIM2, ENABLE); //ʹ��TIM2��ʱ��
}


//---------------------------------------------------------------------------------------------------
void Init_ADC(void)
{
  
  ADC_InitTypeDef   ADC_InitStructure;
  GPIO_InitTypeDef		gpio_init;
  ADC_CommonInitTypeDef  ADC_CommonInitStructure;
  DMA_InitTypeDef DMA_InitStructure;


//  1.  Clock 
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC, ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 
//  2.  GPIO  Config   
//------Configure PC.5 (ADC Channel15) as analog input -------------------------
gpio_init.GPIO_Pin = GPIO_Pin_5;
gpio_init.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOC, &gpio_init);


#ifdef BD_IO_Pin6_7_A1C3
//------Configure PA.1 (ADC Channel1) as analog input -------------------------
gpio_init.GPIO_Pin = GPIO_Pin_1;
gpio_init.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOA, &gpio_init);

//------Configure PC.3 (ADC Channel13) as analog input -------------------------
gpio_init.GPIO_Pin = GPIO_Pin_3;
gpio_init.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOC, &gpio_init);
#endif


//  3. ADC Common Init 
  /* ADC Common configuration *************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; /*�ڶ���ģʽ�� ÿ��ADC�ӿڶ�������*/
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;// ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;  // if used  multi channels set enable
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 3;    // number of   channel 
  ADC_Init(ADC1, &ADC_InitStructure);


//  4. DMA  Config  
  /* DMA2 Stream0 channel0 configuration */
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_ConValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 3;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);


 /* ADC1 regular channel15 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_3Cycles);  // ͨ��1  ��ص���
 /* ADC1 regular channel1 configuration *************************************/ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);   //  ͨ��2   ����
  /* ADC1 regular channel13 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_3Cycles);  // ͨ��3   ����

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC1, ENABLE);

  ADC_SoftwareStartConv(ADC1);

  //==================================== 

}



//==========================================================
void  sys_status(void)
{
     rt_kprintf("\r\n ״̬��ѯ: "); 	     
     //-----  ����  ---	 
     if(WARN_StatusGet())
	                 rt_kprintf(" ��������      "); 	      
    //     ACC 	 
     if(ACC_StatusGet())
                        rt_kprintf("ACC ��    "); 
     else
	 	          rt_kprintf("ACC ��    ");  
    //     AD ��ѹ 
    rt_kprintf ("\r\n  ��ȡ���ĵ��AD��ֵΪ:	%d	",Power_AD.ADC_ConvertedValue);	   
    rt_kprintf(" AD ��ѹ: %d.%d  V  ",Power_AD.AD_Volte/10,Power_AD.AD_Volte%10);    	        
    //    �ź���	 
     rt_kprintf("    %s      ",XinhaoStatus);       
     if( Powercut_Status==0x02)
	 	rt_kprintf("      ��Դ״̬ :   ��ع���");
     else
      if( Powercut_Status==0x01)	 	
	       rt_kprintf("      ��Դ״̬ :  ���Դ����");
    //   ��λģʽ	
     rt_kprintf("\r\n ��λģʽ:    ");  
		      switch(GpsStatus.Position_Moule_Status)
		      	{  
		      	         case 1:           rt_kprintf(" BD");  
						 	break;
				  case 2:           rt_kprintf(" GPS ");  
				  	              break;
				  case 3:           rt_kprintf(" BD+GPS");  
				  	              break;
		      	} 
     //   ��λ״״̬
     if(ModuleStatus&Status_GPS)
	 	        rt_kprintf("         ��λ״̬:  ��λ   ���ǿ���: %d �� ",Satelite_num);   
    else	 
                      rt_kprintf("         ��λ״̬: δ��λ");  	 
    //    GPS ����״̬
    if(GpsStatus.Antenna_Flag==1)
         rt_kprintf("        ����:     �Ͽ�");  
   else	
   	  rt_kprintf("        ����:     ����");   
     //   GPRS  ״̬ 
    if(ModuleStatus&Status_GPRS)
	 	        rt_kprintf("      GPRS ״̬:   Online\r\n");  
    else	 
                      rt_kprintf("      GPRS ״̬:   Offline\r\n");  	   

   rt_kprintf("\r\n ��Դ ADV1=%d ,ģ����1(8pin   ������)ADV2=%d, ģ����2( 10pin ���� PC3 )ADV3=%d \r\n",ADC_ConValue[0],ADC_ConValue[1],ADC_ConValue[2]);
   rt_kprintf("\r\nAD1  Voltage=%d.%d V,   Voltage=%d.%d V   \r\n",AD_2through[0]/10,AD_2through[0]%10,AD_2through[1]/10,AD_2through[1]%10);
   
}
FINSH_FUNCTION_EXPORT(sys_status, Status);



void dispdata(char* instr)
{
     if (strlen((const char*)instr)==0)
	{
	     DispContent=1;
	    return ;
	}
	else 
	{         
	       DispContent=(instr[0]-0x30);    
	  rt_kprintf("\r\n		Dispdata =%d \r\n",DispContent); 
	  return;  
	}
}
FINSH_FUNCTION_EXPORT(dispdata, Debug disp set) ; 


void Socket_main_Set(u8* str)
{
  u8 i=0;
  u8 reg_str[80];
  
	if (strlen((const char*)str)==0){
	    rt_kprintf("\r\n input error\r\n"); 
		return ;
	}
	else 
	{      
	  i = str2ipport((char*)str, RemoteIP_main, &RemotePort_main);
	  if (i <= 4) return ;;
	   
	  memset(reg_str,0,sizeof(reg_str));
	  IP_Str((char*)reg_str, *( u32 * ) RemoteIP_main);		   
	  strcat((char*)reg_str, " :"); 	  
	  sprintf((char*)reg_str+strlen((const char*)reg_str), "%u\r\n", RemotePort_main);  
         memcpy((char*)SysConf_struct.IP_Main,RemoteIP_main,4);
	  SysConf_struct.Port_main=RemotePort_main;
	 Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));

        DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,1);
		 DataLink_EndFlag=1; //AT_End();  
			return ;
	}

}
FINSH_FUNCTION_EXPORT(Socket_main_Set,Set Socket main); 

  void  debug_relay(u8 *str)  
{
 if (strlen((const char*)str)==0)
	{
      ; //rt_kprintf("\r\n�̵���(1:�Ͽ�0:�պ�)JT808Conf_struct.relay_flag=%d",JT808Conf_struct.relay_flag);
    }
else 
	{
	       if(str[0]=='1')
		{
		 Car_Status[2]|=0x08;     // ��Ҫ���Ƽ̵���
		JT808Conf_struct.relay_flag=1;
		Enable_Relay();
		//rt_kprintf("\r\n  �Ͽ��̵���,JT808Conf_struct.relay_flag=%d\r\n",JT808Conf_struct.relay_flag); 
		}
	else if(str[0]=='0')
		{
		Car_Status[2]&=~0x08;    // ��Ҫ���Ƽ̵���
		JT808Conf_struct.relay_flag=0;
		Disable_Relay();
		//rt_kprintf("\r\n  ��ͨ�̵���,JT808Conf_struct.relay_flag=%d\r\n",JT808Conf_struct.relay_flag); 
		}
	}
 Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct)); 
 //rt_kprintf("\r\n(debug_relay)״̬��Ϣ,[0]=%X  [1]=%X  [2]=%X  [3]=%X",Car_Status[0],Car_Status[1],Car_Status[2],Car_Status[3]);	
 }
//FINSH_FUNCTION_EXPORT(debug_relay, Debug relay set) ;

//==========================================================
 
/*    
     -----------------------------
    3.  RT �������
     ----------------------------- 
*/



/*

       ������Ӧ��
 
*/



  //  1 .  ѭ���洢 
      u8       Api_cycle_write(u8 *buffer, u16 len) 
      {
	          WatchDog_Feed();
			  DF_TAKE;
	          if( SaveCycleGPS(cycle_write,buffer,len))
		     { //---- updata pointer   -------------		
				cycle_write++;  	
			    if(cycle_write>=Max_CycleNum)
			  	               cycle_write=0;  
				DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd);   
				DF_delay_us(20);  
		        //-------------------------------	
		        DF_RELEASE;
		        return true;
	            }  
		    else
		    {		         
				 //-------------------------------	 
				 DF_RELEASE;
			     return  false;  
		    }	 
  	}

      u8      Api_cycle_read(u8 *buffer, u16 len) 
      {
                return( ReadCycleGPS(cycle_read, buffer, len));
  	}
       u8     Api_cycle_Update(void)
       {
           	 if( ReadCycle_status==RdCycle_SdOver)	 // ���ͳɹ��ж� 
				 {	
				     // process   0704
				     if(delta_0704_rd==0)   
					     cycle_read++;	 //  �յ�Ӧ��ŵ���
					 if(cycle_read>=Max_CycleNum)
							cycle_read=0;
					 ReadCycle_status=RdCycle_Idle; 			
				 }	
				 return 1;
      }
	u8   Api_CHK_ReadCycle_status(void) 
	{
          CHK_ReadCycle_status();    // ����״̬�ļ��
		  return true;
	}

 // 2. Config 
      u8    Api_Config_write(u8 *name,u16 ID,u8* buffer, u16 wr_len)  
 	{
 	     DF_TAKE;
                if(strcmp((const char*)name,config)==0)
                {
                     DF_WriteFlashSector(ConfigStart_offset, 0, buffer, wr_len);
						DF_delay_ms(5);
						DF_RELEASE;
						return true;		 
                }  
		   if(strcmp((const char*)name,tired_config)==0)
                {
                     DF_WriteFlashSector(TiredCondifg_offset, 0, buffer, wr_len);  
					 DF_RELEASE;
			         return true;		 
                } 	
		 DF_RELEASE;  
		 return false;		
 	}
       u8      Api_Config_read(u8 *name,u16 ID,u8* buffer, u16 Rd_len)    //  ��ȡMedia area ID �Ǳ���
       {
            if(strcmp((const char*)name,config)==0)
           	{
                     DF_ReadFlash(ConfigStart_offset, 0, buffer, Rd_len); 
					 DF_delay_ms(80); 	// large content delay	
					 return true;		
           	}
              if(strcmp((const char*)name,jt808)==0) 
             	{   
             	     WatchDog_Feed(); 
                     DF_ReadFlash(JT808Start_offset, 0, buffer, Rd_len); 
			         DF_delay_ms(100); // large content delay
			         return true;	
             	}
		   if(strcmp((const char*)name,tired_config)==0)
                {
                     DF_ReadFlash(TiredCondifg_offset, 0, buffer, Rd_len);
			DF_delay_ms(10); 		 
			return true;		 
                } 	  
     		   if(strcmp((const char*)name,BD_ext_config)==0)
     		   {
                     DF_ReadFlash(DF_BD_Extend_Page, 0, buffer, Rd_len); 
			DF_delay_ms(10); 		 
			return true;	
     		   }
         return false;
			
       }
 
    u8    Api_Config_Recwrite_Large(u8 *name,u16 ID,u8* buffer, u16 wr_len)  // �������¼�¼  
 	{
 	       DF_TAKE;
           if(strcmp((const char*)name,jt808)==0)
                {
                     WatchDog_Feed();
                     DF_WriteFlashSector(JT808Start_offset, 0, buffer, wr_len);  // formal  use
                     WatchDog_Feed(); 
					 DF_WriteFlashSector(JT808_BakSetting_offset,0,buffer,wr_len); // bak  setting   					 
					 WatchDog_Feed(); 
					 DF_WriteFlashSector(JT808_Bak2Setting_offset,0,buffer,wr_len); // bak  setting    	
					 DF_RELEASE;
			         return true;		 
                }
	      if(strcmp((const char*)name,BD_ext_config)==0)
	    	{
                    DF_WriteFlashSector(DF_BD_Extend_Page,0, buffer, wr_len); 
					DF_RELEASE;
                    return true;
	    	}	
		 DF_RELEASE;
		 return false;	
 	}
	  
 //  3.  ���� 
     u8    Api_DFdirectory_Create(u8* name, u16 sectorNum)  // 
 	{ 
              return true ;  // NO  action here
 	}
       void   Api_MediaIndex_Init(void)
       {
          
		  u8 i=0;
		  
		  memset((u8*)&MediaIndex,0,sizeof(MediaIndex)); 
		  
		  for(i=0;i<8;i++)
		  {
			MediaIndex.ID=i+1;
		   if(i==0)
			{
			  MediaIndex.Type=0;
			  memset(MediaIndex.FileName,0,sizeof(MediaIndex.FileName));
			  memcpy(MediaIndex.FileName,"pic.jpg",7);
			  DF_WriteFlashSector(DF_PicIndex_Page,0,(u8*)&MediaIndex,sizeof(MediaIndex));	
			  DF_delay_ms(50);
			  MediaIndex.Type=1;
			  memset(MediaIndex.FileName,0,sizeof(MediaIndex.FileName));
			  memcpy(MediaIndex.FileName,"sound.wav",9);
			  DF_WriteFlashSector(DF_SoundIndex_Page,0,(u8*)&MediaIndex,sizeof(MediaIndex));  
			  DF_delay_ms(50);
			}  
		   else 
			{
			  MediaIndex.Type=0;
			  memset(MediaIndex.FileName,0,sizeof(MediaIndex.FileName));
			  memcpy(MediaIndex.FileName,"pic.jpg",7);
			  DF_WriteFlashDirect(DF_PicIndex_Page+i, 0,(u8*)&MediaIndex, sizeof(MediaIndex));	  
			  DF_delay_ms(10);	  
			  MediaIndex.Type=1;
			  memset(MediaIndex.FileName,0,sizeof(MediaIndex.FileName));
			  memcpy(MediaIndex.FileName,"sound.wav",9);
			  DF_WriteFlashDirect(DF_SoundIndex_Page+i,0,(u8*)&MediaIndex,sizeof(MediaIndex));		
			  DF_delay_ms(10);
			}  
		  }
       }

       u32  Api_DFdirectory_Query(u8 *name, u8  returnType)
       {         //  returnType=0  ���ؼ�¼��Ŀ     returnType=1 ʱ���ظ�Ŀ¼�ļ���С
             u8   flag=0;
	      u32  pic_current_page=0;		 
			 
		 if(strcmp((const char*)name,spdpermin)==0)
		 	{return AvrgSpdPerMin_write;}
		  if(strcmp((const char*)name,tired_warn)==0)
		     {return TiredDrv_write ;}
		  if(strcmp((const char*)name,camera_1)==0)
		  {      pic_current_page=PicStart_offset;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_2)==0)
		     {      pic_current_page=PicStart_offset2;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_3)==0)
		   {      pic_current_page=PicStart_offset3;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_4)==0)
			  {      pic_current_page=PicStart_offset4;
		          flag=1;
		   }
		   if(strcmp((const char*)name,voice)==0)
		   {      DF_ReadFlash(SoundStart_offdet,0,(u8*)&SrcFileSize,4);  
		           return SrcFileSize;
		   }
		   if(flag)
		   	{
		   	    DF_delay_ms(5);
		        DF_ReadFlash(pic_current_page, 0,PictureName, 23);
                      memcpy((u8*)&PicFileSize,PictureName+19,4);   
			 return   PicFileSize	;
		   }
		    return  0;
       }
	   
         u8 Api_DFdirectory_Write(u8 *name,u8 *buffer, u16 len) 
         {
		 
          if(strcmp((const char*)name,spdpermin)==0)
		  {
		            Save_PerMinContent(AvrgSpdPerMin_write,buffer, len);
			     //-----  Record update----		
			  AvrgSpdPerMin_write++;
			  if(AvrgSpdPerMin_write>=Max_SPDSperMin)
			     AvrgSpdPerMin_write=0;
			  DF_Write_RecordAdd(AvrgSpdPerMin_write, AvrgSpdPerMin_write, TYPE_AvrgSpdAdd);   
			     //----------------------	 
			return true;	 
               }
		 if(strcmp((const char*)name,spd_warn)==0) 
		{
			   Common_WriteContent( ExpSpdRec_write, buffer, len, TYPE_ExpSpdAdd);    
			   //-----  Record update----	
			  ExpSpdRec_write++;
			  if(ExpSpdRec_write>=Max_SPDSperMin)
			      ExpSpdRec_write=0;
	                DF_Write_RecordAdd(ExpSpdRec_write, ExpSpdRec_write, TYPE_ExpSpdAdd); 	  
			  //-----  Record update----	
			   return true;
               }
                if(strcmp((const char*)name,tired_warn)==0) 
		 {
			   Common_WriteContent( TiredDrv_write, buffer, len,  TYPE_TiredDrvAdd);      
			   //-----  Record update----	
			  TiredDrv_write++;
			  if(TiredDrv_write>=Max_CommonNum)  
			  	TiredDrv_write=0;			  
			  DF_delay_us(10);
			  DF_Write_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd);   
			  //-------------------------
			  return  true;
                }	
		
		 //------- MultiMedia   RAW  data  ---------
		 if(strcmp((const char*)name,voice)==0) 
		{
	            DF_WriteFlashDirect(SoundStart_offdet+Dev_Voice.Voice_PageCounter,0,Dev_Voice.Voice_Reg,500);  
                   return true;
		 }
		if(strcmp((const char*)name,camera_1)==0)
		{
                     DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
                     DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
                      DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		  
               }
	       if(strcmp((const char*)name,camera_4)==0)
		{
                      DF_WriteFlashDirect(pic_current_page,0,buffer, len); 
			return true;		  
               }	 
		
              return false;
         }
          u8  Api_DFdirectory_Read(u8 *name,u8 *buffer, u16 len, u8  style ,u16 numPacket)  // style  1. old-->new   0 : new-->old 
         {   /*  �������ü��� �����ΰ�style ��ʽ����*/
               // style  1. old-->new   0 : new-->old 
               //   numPacket    : ��װ style  ��ʽ��ȡ��ʼ �ڼ������ݰ�  from: 0
               u16   read_addr=0;
			   
			 DF_delay_ms(1); 
              if(strcmp((const char*)name,spdpermin)==0)
		    {
		        if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(AvrgSpdPerMin_write==0)
				  	    return   false; 
				else  
			      if(AvrgSpdPerMin_write>=(numPacket+1))		
			            read_addr=AvrgSpdPerMin_write-1-numPacket;
				else
					 return false;
			 }  
		            Read_PerMinContent(read_addr,buffer, len);
			     //----------------------	 
			return true;	 
               }
		 if(strcmp((const char*)name,spd_warn)==0) 
		{
			 if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(ExpSpdRec_write==0)
				  	    return   false; 
				else  
			      if(ExpSpdRec_write>=(numPacket+1))		
			            read_addr=ExpSpdRec_write-1-numPacket;
				else
					 return false;
			 }  	
			   Common_ReadContent( read_addr, buffer, len, TYPE_ExpSpdAdd);    
			   return true;
               }
                if(strcmp((const char*)name,tired_warn)==0) 
		 {
			   if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(TiredDrv_write==0)
				  	    return   false; 
				else  
			      if(TiredDrv_write>=(numPacket+1))		
			            read_addr=TiredDrv_write-1-numPacket;
				else
					 return false;
			 } 
			   Common_ReadContent( read_addr, buffer, len,  TYPE_TiredDrvAdd);      

			  //-------------------------
			  return  true;
                }		
	
		 //------- MultiMedia   RAW  data  ---------
		 if(strcmp((const char*)name,voice)==0)
		{
		     if(style==0)
					return  false;  //  ֻ�����old  -> new
	            DF_ReadFlash(SoundStart_offdet+numPacket,0, buffer, len);  
                   return true;
		 }
		if(strcmp((const char*)name,camera_1)==0)
		{
                     DF_ReadFlash(PicStart_offset+numPacket,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
                     DF_ReadFlash(PicStart_offset2+numPacket,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
                      DF_ReadFlash(PicStart_offset3+numPacket,0,buffer, len);
			return true;		  
               }
	       if(strcmp((const char*)name,camera_4)==0)
		{
                      DF_ReadFlash(PicStart_offset4+numPacket,0,buffer, len); 
			return true;		  
               }	 
		
              return false;
         }

	u8    Api_DFdirectory_Delete(u8* name)
 	{ 
	   
              if(strcmp((const char*)name,voice)==0)
		{
		                 WatchDog_Feed();    
				   SST25V_BlockErase_32KByte((SoundStart_offdet<<9));
				   DF_delay_ms(300);  
				   WatchDog_Feed(); 
			    return true;
               }
		if(strcmp((const char*)name,camera_1)==0)
		{
		       WatchDog_Feed(); 
                     SST25V_BlockErase_64KByte((PicStart_offset<<9));      
			DF_delay_ms(100);   	
			WatchDog_Feed(); 
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
		       WatchDog_Feed(); 
                     SST25V_BlockErase_64KByte((PicStart_offset2<<9));   
			DF_delay_ms(100); 	
			WatchDog_Feed(); 
			return true;			 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
		         WatchDog_Feed(); 
                       SST25V_BlockErase_64KByte((PicStart_offset3<<9));    
			  DF_delay_ms(100); 
			  WatchDog_Feed(); 
			  return true;	 		   
               }
	     if(strcmp((const char*)name,camera_4)==0)
		{
		        WatchDog_Feed(); 
                       SST25V_BlockErase_64KByte((PicStart_offset4<<9));    
		      DF_delay_ms(100); 
			  WatchDog_Feed(); 	 
			  return true;			   
               }  
		  return false; 
 	}
	  
//-------  �̶�λ�� ��ż�¼  -----------
  u8   Api_RecordNum_Write( u8 *name,u8 Rec_Num,u8 *buffer, u16 len)    //  Rec_Num<128  Len<128
{
          WatchDog_Feed();
           if(strcmp((const char*)name,event_808)==0)
           	{
                   DF_WriteFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
           if(strcmp((const char*)name,msg_broadcast)==0)
		{
                   DF_WriteFlash(DF_Broadcast_offset+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}   	
	    if(strcmp((const char*)name,phonebook)==0)
		{
                   DF_WriteFlash(DF_PhoneBook_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
	    if(strcmp((const char*)name,Rail_cycle)==0)
		{
                   DF_WriteFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
           if(strcmp((const char*)name,Rail_rect)==0)
	        {
                   DF_WriteFlash(DF_RectangleRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		   	
	    if(strcmp((const char*)name,Rail_polygen)==0)	
	     {
                   DF_WriteFlash(DF_PolygenRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}			
	    if(strcmp((const char*)name,turn_point)==0)
		{
                   DF_WriteFlash(DF_turnPoint_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
           if(strcmp((const char*)name,route_line)==0)
		{
                   DF_WriteFlash(DF_Route_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	   	
	    if(strcmp((const char*)name,ask_quesstion)==0)	
		{
                   DF_WriteFlash(DF_AskQuestion_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
	    if(strcmp((const char*)name,text_msg)==0)
	      {
                   DF_WriteFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
  //-------------------------------
                if(strcmp((const char*)name,pic_index)==0)
                {
                            DF_WriteFlash(DF_PicIndex_Page+Rec_Num, 0,buffer, len); 
				return true;			
                }
		 if(strcmp((const char*)name,voice_index)==0)
		{
                            DF_WriteFlash(DF_SoundIndex_Page+Rec_Num, 0,buffer, len);
				return  true;			
              }
		 
		 return false;
}

  u8   Api_RecordNum_Read( u8 *name,u8 Rec_Num,u8 *buffer, u16 len)    //  Rec_Num<128  Len<128
  	{

             if(strcmp((const char*)name,event_808)==0)
           	{
                   DF_ReadFlash(DF_Event_Page+Rec_Num, 0,buffer, len);    
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}
           if(strcmp((const char*)name,msg_broadcast)==0)
		{
                   DF_ReadFlash(DF_Broadcast_offset+Rec_Num, 0,buffer, len);    
                   //DF_delay_ms(10);   				   
		     return true;		   
           	}   	
	    if(strcmp((const char*)name,phonebook)==0)
		{
                 DF_ReadFlash(DF_PhoneBook_Page+Rec_Num, 0,buffer, len);   
		   //DF_delay_ms(10);   		   
		     return true;		   
           	}	
	    if(strcmp((const char*)name,Rail_cycle)==0)
		{
                   DF_ReadFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}	
           if(strcmp((const char*)name,Rail_rect)==0)
	        {
                   DF_ReadFlash(DF_RectangleRail_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}		   	
	    if(strcmp((const char*)name,Rail_polygen)==0)	
	     {
                   DF_ReadFlash(DF_PolygenRail_Page+Rec_Num, 0,buffer, len);   
		     //DF_delay_ms(10);   		   
		     return true;		   
           	}			
	    if(strcmp((const char*)name,turn_point)==0)
		{
                   DF_ReadFlash(DF_turnPoint_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}		
           if(strcmp((const char*)name,route_line)==0)
		{
                   DF_ReadFlash(DF_Route_Page+Rec_Num, 0,buffer, len);   
		    DF_delay_ms(10); 		   
		     return true;		   
           	}	   	
	    if(strcmp((const char*)name,ask_quesstion)==0)	
		{
                   DF_ReadFlash(DF_AskQuestion_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
	    if(strcmp((const char*)name,text_msg)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     //DF_delay_ms(10); 		   
		     return true;		   
           	}
  	      if(strcmp((const char*)name,pic_index)==0)
                {
                            DF_ReadFlash(DF_PicIndex_Page+Rec_Num, 0,buffer, len);  
				return true;			
                }
		 if(strcmp((const char*)name,voice_index)==0)
		{
                            DF_ReadFlash(DF_SoundIndex_Page+Rec_Num, 0,buffer, len);
				//DF_delay_ms(10); 			
				return  true;			
              }
		 return false;

  	}

    void  Api_WriteInit_var_rd_wr(void)    //   д��ʼ���������Ͷ�д��¼��ַ
    	{
		 DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd); 
		 DF_delay_ms(50);   
		 DF_Write_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);
		 DF_delay_ms(50);
		// DF_Write_RecordAdd(AvrgSpdPerSec_write,AvrgSpdPerSec_Read,TYPE_AvrgSpdSecAdd);
		// DF_delay_ms(50);  
		 //DF_Write_RecordAdd(Login_write,Login_Read,TYPE_LogInAdd);  
		// DF_delay_ms(50);  
		 DF_Write_RecordAdd(Settingchg_write,Settingchg_read,TYPE_SettingChgAdd);
		 DF_delay_ms(50);  						 
		 DF_Write_RecordAdd(AvrgMintPosit_write,AvrgMintPosit_Read,TYPE_MintPosAdd); 
		 DF_delay_ms(50);  
		 
               DF_Write_RecordAdd(Distance_m_u32,DayStartDistance_32,TYPE_DayDistancAdd); 
	 	 DF_Write_RecordAdd(ExpSpdRec_write,ExpSpdRec_read,TYPE_ExpSpdAdd);  
	 	 DF_delay_ms(50); 
		 
	 	 DF_Write_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd); 
	 	 DF_delay_ms(50); 
	 	 DF_Write_RecordAdd(AvrgSpdPerMin_write,AvrgSpdPerMin_Read,TYPE_AvrgSpdAdd); 
		
    	}
      void  Api_Read_var_rd_wr(void)    //   ����ʼ���������Ͷ�д��¼��ַ
    	{
             DF_Read_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd); 
	      DF_delay_ms(50); 
		DF_Read_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);    
		 DF_delay_ms(50); 
		//DF_Read_RecordAdd(AvrgSpdPerSec_write,AvrgSpdPerSec_Read,TYPE_AvrgSpdSecAdd);
		//DF_Read_RecordAdd(Login_write,Login_Read,TYPE_LogInAdd);  
		DF_Read_RecordAdd(Settingchg_write,Settingchg_read,TYPE_SettingChgAdd);
		DF_Read_RecordAdd(AvrgMintPosit_write,AvrgMintPosit_Read,TYPE_MintPosAdd); 

               DF_Read_RecordAdd(Distance_m_u32,DayStartDistance_32,TYPE_DayDistancAdd); 
	 	 DF_Read_RecordAdd(ExpSpdRec_write,ExpSpdRec_read,TYPE_ExpSpdAdd);  
	 	 DF_delay_ms(50); 
	 	
		 
	 	 DF_Read_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd); 
	 	 DF_delay_ms(50); 
	 	 DF_Read_RecordAdd(AvrgSpdPerMin_write,AvrgSpdPerMin_Read,TYPE_AvrgSpdAdd); 
 

    	}


//----------   TF �����״̬
u8     TF_Card_Status(void)
{                    //    1:  succed             0:  fail
           return 0 ;

}
//===============================================================
u8  DF_Write_RecordAdd(u32 Wr_Address,u32 Rd_Address, u8 Type) 
{
  u8     head[448];
  u16    offset=0;  
  u16    Add_offset=0;  //  page offset  
  u16    Savepage=0;      // �洢page ҳ
  u16    InpageOffset=0;  // ҳ��ƫ�� 
  u8     reg[9];  
  u8     Flag_used=0x01;
  
  //  1.   Classify
    switch(Type)
    {
		case TYPE_CycleAdd:
							 Add_offset=DF_CycleAdd_Page;
							 break;
		case TYPE_PhotoAdd:
							 Add_offset=DF_PhotoAdd_Page;
							 break;
		case TYPE_TiredDrvAdd:
							 Add_offset=DF_TiredDrvAdd_Page;
							 break;
		case TYPE_ExpSpdAdd:
							 Add_offset=DF_ExpSpdAdd_Page;
							 break; 					 
		case TYPE_AvrgSpdAdd:
							 Add_offset=DF_AvrgSpdPerMinAdd_Page;
							 break;
		case TYPE_ErrorLogAdd:
							 Add_offset=DF_AbnormalLogAdd_Page;
							 break;
		case TYPE_VechRecordAdd:
							 Add_offset=DF_RecordAdd_Page;	
							 break;
		case TYPE_AvrgSpdSecAdd:
                             Add_offset=DF_AvrgSpdSec_Page;
			                 break;
		case TYPE_LogInAdd:
                             Add_offset=DF_Login_Page;
			                 break;
		case TYPE_SettingChgAdd:
                             Add_offset=DF_Settingchg_Page;
			                 break;		
		case TYPE_MintPosAdd:
			                 Add_offset=DF_Minpos_Page;   
			                 break; 
		case TYPE_DayDistancAdd:
			                 Add_offset=DF_DayDistance_Page;
							 break;
		case TYPE_ACCONFFcounterAdd:
			                 Add_offset=DF_ACCONFFcounter_Page;
							 break;
							 
		default :
							 return false;							 
    }
  //  2 .  Excute 
    
     DF_ReadFlash(Add_offset,0,(unsigned char*)head,448);   
     DF_delay_us(100);

	/*
	
      ͨ����ѯBlock ��1��Page��ǰ448�ֽ��Ƿ�Ϊ0xFF �жϣ������Ҫд�����ݵ�ƫ�Ƶ�ַ����448����ʶʹ����󣬲�����Block��
      Ȼ���ͷ��ʼ��
	 ����ÿ��page�ܴ�64�����ݣ�����Ҫ�ȼ���洢��PageȻ���ټ���ƫ�Ƶ�ַ
	  �洢page�ļ��㷽��Ϊ �� 
	        Savepage=Startpage+n/64;
	 �洢ҳ�ڵ�ƫ�Ƶ�ַ���㷽��Ϊ��
		   InpageOffset=��n%64��*8��

	*/ 		
     for(offset=0;offset<448;offset++)
     {
       if(0xFF==head[offset])
	   	  break;
     }

	 if(offset==448)
	 	{     
		   SST25V_SectorErase_4KByte((8*((u32)Add_offset/8))*PageSIZE); // Erase block
		   offset=0; 		   
		   DF_delay_ms(50);
	    }   
	 Savepage=Add_offset+1+(offset>>6);   //Add_offset+offset/64  
	 InpageOffset=((offset%64)<<3);      //(offset%64��*8;    

     
	 memcpy(reg,&Wr_Address,4);  
	 memcpy(reg+4,&Rd_Address,4);   
	 
	 DF_WriteFlashDirect(Add_offset,offset,&Flag_used,1); //  ����״̬λ
	 DF_delay_us(100); 
	 DF_WriteFlashDirect(Savepage,InpageOffset,reg,8);    //  ���¼�¼����
	 
	 return true;
   //                 The  End     	     
}


u8  DF_Read_RecordAdd(u32 Wr_Address,u32 Rd_Address, u8 Type)
{
  u8	   head[448];
  u16    offset=0;  
  u32   Add_offset=0;  //  page offset  
  u32   Reg_wrAdd=0,Reg_rdAdd=0;  
  u16    Savepage=0;      // �洢page ҳ
  u16    InpageOffset=0;  // ҳ��ƫ�� 

   //Wr_Address, Rd_Address  , û��ʲô��ֻ�Ǳ�ʾ���룬����д�۲����    
  //  1.   Classify
    switch(Type)
    {
       case TYPE_CycleAdd:
                            Add_offset=DF_CycleAdd_Page;
	                        break;
	   case TYPE_PhotoAdd:
	   	                    Add_offset=DF_PhotoAdd_Page;
	                        break;
	   case TYPE_TiredDrvAdd:
	   	                    Add_offset=DF_TiredDrvAdd_Page;
	                        break;
	   case TYPE_ExpSpdAdd:
	   	                    Add_offset=DF_ExpSpdAdd_Page;
	                        break;						
	   case TYPE_AvrgSpdAdd:
	   	                    Add_offset=DF_AvrgSpdPerMinAdd_Page;
	                        break;
       case TYPE_ErrorLogAdd:
	   	                    Add_offset=DF_AbnormalLogAdd_Page;
	                        break;
       case TYPE_VechRecordAdd:
	   	                    Add_offset=DF_RecordAdd_Page;  
	                        break;
		case TYPE_AvrgSpdSecAdd:
                             Add_offset=DF_AvrgSpdSec_Page;
			                 break;
		case TYPE_LogInAdd:
                             Add_offset=DF_Login_Page;
			                 break;
		case TYPE_SettingChgAdd:
                             Add_offset=DF_Settingchg_Page;
			                 break;	 	
		case TYPE_MintPosAdd:
		 				     Add_offset=DF_Minpos_Page;  
							 break;
		case TYPE_DayDistancAdd:
			                 Add_offset=DF_DayDistance_Page;
							 break;
		case TYPE_ACCONFFcounterAdd:
			                 Add_offset=DF_ACCONFFcounter_Page;
							 break;	  	 			 
						 
							 
	   default :
	   	                    return false;							

    }
  //  2 .  Excute 
    
     DF_ReadFlash(Add_offset,0,(unsigned char*)head,448); //   ������Ϣ  
		
		/*
		
		  ͨ����ѯBlock ��1��Page��ǰ448�ֽ��Ƿ�Ϊ0xFF �жϣ������Ҫд�����ݵ�ƫ�Ƶ�ַ����448����ʶʹ����󣬲�����Block��
		  Ȼ���ͷ��ʼ��
		 ����ÿ��page�ܴ�64�����ݣ�����Ҫ�ȼ���洢��PageȻ���ټ���ƫ�Ƶ�ַ
		  �洢page�ļ��㷽��Ϊ �� 
				Savepage=Startpage+n/64;
		 �洢ҳ�ڵ�ƫ�Ƶ�ַ���㷽��Ϊ��
			   InpageOffset=��n%64��*8��
		
		*/

   
		for(offset=0;offset<448;offset++)
		{
		  if(0xFF==head[offset])
			 break;
		}

		offset--;  // ��һ������Ϊ0xFF

		
		Savepage=Add_offset+1+(offset>>6);	 //Add_offset+offset/64  
		InpageOffset=((offset%64)<<3); 	 //(offset%64��*8;	  
		//rt_kprintf("\r\n Read	offset=%d\r\n", offset);
		DF_ReadFlash(Savepage,InpageOffset,(u8*)&Reg_wrAdd,4);
		DF_ReadFlash(Savepage,InpageOffset+4,(u8*)&Reg_rdAdd,4);    

	
	    // rt_kprintf("\r\n  RecordAddress  READ-1   write=%d , read=%d \r\n",Reg_wrAdd,Reg_rdAdd);    

	
  //  3. Get reasult 
    switch(Type)
    {
       case TYPE_CycleAdd:
                            cycle_write=Reg_wrAdd;
							cycle_read=Reg_rdAdd;
	                        break;
	   case TYPE_PhotoAdd:
	   	                    pic_write=Reg_wrAdd;
							pic_read=Reg_rdAdd;
	                        break;
	   case TYPE_TiredDrvAdd:
	   	                    TiredDrv_write=Reg_wrAdd;
							TiredDrv_read=Reg_rdAdd;
	                        break;
	   case TYPE_ExpSpdAdd:
	   	                    ExpSpdRec_write=Reg_wrAdd;
							ExpSpdRec_read=Reg_rdAdd;
	                        break;						
	   case TYPE_AvrgSpdAdd:
	   	                    AvrgSpdPerMin_write=Reg_wrAdd;
							AvrgSpdPerMin_Read=Reg_rdAdd;
	                        break;
       case TYPE_ErrorLogAdd:
	   	                    ErrorLog_write=Reg_wrAdd;
							ErrorLog_Read=Reg_rdAdd;
	                        break;
   	 	 	case TYPE_LogInAdd:
			                 Login_write=Reg_wrAdd;
	 						 Login_Read=Reg_rdAdd;
	 						 break;
	 	case TYPE_SettingChgAdd:
	 						 Settingchg_write=Reg_wrAdd;
	 						 Settingchg_read=Reg_rdAdd;
	 						 break; 	   						
	    case TYPE_MintPosAdd: 
						    AvrgMintPosit_write=Reg_wrAdd;
						    AvrgMintPosit_Read=Reg_rdAdd;   
						    break;
		case TYPE_DayDistancAdd:
			                Distance_m_u32=Reg_wrAdd;
						    DayStartDistance_32=Reg_rdAdd;         
							break;
		case TYPE_ACCONFFcounterAdd:
			                //Tired_drive.ACC_ONstate_counter=Reg_wrAdd;
						//	Tired_drive.ACC_Offstate_counter=Reg_rdAdd;      
							break;					
						
	   						
	   default :
	   	                    return false;							

    } 
   //                 The  End     	     
    return true;
}



		 
