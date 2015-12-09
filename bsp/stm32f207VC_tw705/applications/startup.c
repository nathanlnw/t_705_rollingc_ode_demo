/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f2xx.h"
#include "board.h"
#include "App_moduleConfig.h"/**
 * @addtogroup STM32
 */

/*@{*/

//  Rolling  Code   相关
/*Keeloq解码*/	
	
#define KeeLoq_NLF		0x3A5C742E
#define GetBit(x,n)		(((x)>>(n))&1)
#define g5(x,a,b,c,d,e)	(GetBit(x,a)+GetBit(x,b)*2+GetBit(x,c)*4+GetBit(x,d)*8+GetBit(x,e)*16)	



extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN    (&__bss_end)
#endif


#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */





#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_TX_PIN_SOURCE GPIO_PinSource9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_RX_PIN_SOURCE GPIO_PinSource10
#define UART1_GPIO			GPIOA
#define UART1_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART1	RCC_APB2Periph_USART1

typedef struct  _KEELOQ
{
    uint64_t  Encode_64bit_password;   // 64 bit  password
    u16       Sync_Counter;  //  16 bit  syn  counter      +  16  window
    u32       Serial_Number; //  28 bit  serial number
    u16       Identity_code;//   10 bit  indentity
	u16        Update_sync;//  update sync      after  resyncronize
}KEELOQ;


typedef struct _ROLL_LEARN
{
    u8  Learning;  // Learn status    1: learn runing
    u16 Learn_Timer;  //  timer   maxtimer:200s
    u8  Learn_step;  // learn  steps 
    u16 Waiting_enterLearn_timer; // checking  and  waiting enter  into learn mode     
    u16 sync[2];   // rx

}ROLL_LEARN;

 
 
 u32  IC2Value=0;	// 
 u32  IC1Value=0;
 u32  DutyCycle  = 0;
 u16 PrescalerValue=0;
 
 unsigned int count=0;	 
 
 uint32_t rxState=0;
 uint8_t rxPos=0;	//接收计数
 uint8_t rxBuf[66];
 uint8_t rxBuf_bak[66]; // bak _
 uint8_t rxBuf_bak_process;
 
 
 uint8_t rxByte[9];
 
 
 uint8_t  Rx66bit_34bit_FixedPart_repeatBit=0;
 uint8_t  Rx66bit_34bit_FixedPart_VlowBit=0;
 uint8_t  Rx66bit_34bit_FixedPart_ButtonStatus=0;
 uint32_t Rx66bit_34bit_FixedPart_28bit_NO=0; 
 
 unsigned long Rx66bit_32bit_hoppingPart=0;// code_hopping=0;
 unsigned long Rx66bit_32bit_hoppinpPart_decodeTotal=0;
 uint8_t  Rx66bit_32bit_hoppingPart_decode_buttonstatus=0; 
 uint8_t  Rx66bit_32bit_hoppingPart_decode_OVR=0;
 uint16_t Rx66bit_32bit_hoppingPart_decode_DISC=0;
 uint16_t Rx66bit_32bit_hoppingPart_decode_SYNcounter=0;
 
 uint64_t		 EncodeSavePassword_Total=0x0123456789ABCDEF;
 u8  *pointer;
 uint32_t	EncodeSavePassword_LSB=0;
 uint32_t	EncodeSavePassword_MSB=0;
 
 
 uint32_t		 Key_add_Serial=0;	
 u32   TIM3_Timer_Counter=0; //  测试定时器计数器
  
 u16   Decode_key_valueBuf[4];
 u16   Decode_key_valueBuf_counter=0; 
 
 
 
 
 
 uint32_t code_fixed=0;
 
 uint32_t code_decode=0; //  编码密码
 u8  debugTestflag=0;

 u8  Buzz_status=0; 


 ROLL_LEARN  Learn;
 KEELOQ      ROLL_EEPROM;
 KEELOQ      ROLL_REG;
 KEELOQ      ROLL_learn;

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
	rt_kprintf("       file  %s\r\n", file);
	rt_kprintf("       line  %d\r\n", line);

	while (1) ;
}

PUTCHAR_PROTOTYPE
{
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){}
	return ch;
}


/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
	/* init board */
	
// #ifdef  GSM_UART
		//_gsm_startup();
 //#endif

 // #ifdef APP808
      // Protocol_app_init();   
 // #endif
 
 // #ifdef _485_DEVICE
       //485_startup();
 // #endif

 // #ifdef GPS_UART
	  // gps_init();     
       //mma8451_driver_init();      
 // #endif

    //  printer_driver_init();    


	/* init application_ demo */
	//rt_application_init();
  
#if 0

// -------------------------------------------------------------------------

    /* init timer thread */
      rt_system_timer_thread_init(); 

	/* init idle thread */
	rt_thread_idle_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;
	#endif
}

void  u1_rxAnalysis(void)
{
  if(u1_rxflag ==1)
  {
      if(strncmp(u1_rx_buf,"buzzon",6)==0)
      	{ 
            buzzer_onoff(1);
			printf("on \r\n");
			
      	}
	  else
	  if(strncmp(u1_rx_buf,"buzzoff",7)==0)
      	{ 
            buzzer_onoff(0);
			printf("off \r\n");
      	}
	  else
	  if(strncmp(u1_rx_buf,"enterlearn",10)==0)
	  	{
	  	  Learn.Learning=1;
		    Learn.Learn_Timer=0;
		    Learn.Learn_step=0;
			Learn.Waiting_enterLearn_timer=0;
		   // memset((u8*)ROLL_learn,0,sizeof(ROLL_learn));
		   printf(" enter\r\n");
      	}

	  else
	  if(strncmp(u1_rx_buf,"exitlearn",9)==0)
	  	{
          Learn.Learning=0;
		  Learn.Learn_Timer=0;
		  Learn.Learn_step=0;
		  Learn.Waiting_enterLearn_timer=0;
		  printf("exit\r\n");
      	}
	  
      printf("%s",u1_rx_buf);
	  
      memset(u1_rx_buf,0,sizeof(u1_rx_buf));
      u1_rxflag = 0;
	  u1_rxCounter=0;
  	}
  

}

/*    
     -----------------------------
    2.  应用相关
     ----------------------------- 
*/
//-------------------------------------------------------------------------------------------------

/*采用PA.0 作为外部脉冲计数*/
void pulse_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	TIM_ICInitTypeDef	TIM_ICInitStructure;	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );

	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );

	

	/* TIM5 chennel1 configuration : PB.7 */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/* Connect TIM pin to B7 */
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource7, GPIO_AF_TIM4 ); 


	

	/* Enable the TIM5 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );



    // Prescaler
    #if 1 
      PrescalerValue = (u16) ((SystemCoreClock /2) / 1000000) - 1;

	 
	 TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* Time base configuration */
  //TIM_TimeBaseStructure.TIM_Period = 1000;      // 1Mhz/1000=1000
  //TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
//  TIM_TimeBaseStructure.TIM_Period		= 1000;             /* 1ms */
//  TIM_TimeBaseStructure.TIM_Prescaler		= ( 120 / 2 - 1 );  /* 1M*/
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
 // TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   #endif
    



	TIM_ICInitStructure.TIM_Channel		= TIM_Channel_2; //TIM_Channel_1
	TIM_ICInitStructure.TIM_ICPolarity	= TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter	= 0x0;

	TIM_PWMIConfig( TIM4, &TIM_ICInitStructure );

	/* Select the TIM5 Input Trigger: TI1FP1 */
	TIM_SelectInputTrigger( TIM4, TIM_TS_TI2FP2 );//TIM_TS_TI1FP1

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode( TIM4, TIM_SlaveMode_Reset );
	TIM_SelectMasterSlaveMode( TIM4, TIM_MasterSlaveMode_Enable );

	/* TIM enable counter */
	TIM_Cmd( TIM4, ENABLE );

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig( TIM4, TIM_IT_CC2, ENABLE ); 
}


//    解码
unsigned long KeeLoq_Decrypt(unsigned long x, uint64_t key) 
{
  unsigned long r;
	
  for (r = 0; r < 528; r++)
  {
	      x = 
      (x<<1)^
	  GetBit(x,31)^
	  GetBit(x,15)^
	  (unsigned long)GetBit(key,(15-r)&63)^
	  GetBit(
	    KeeLoq_NLF,
	    g5(x,0,8,19,25,30)
	  );
  }
  return x;
}	


	

/*TIM5_CH1*/
void TIM4_IRQHandler( void )
{

  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);

	IC1Value= TIM_GetCapture1(TIM4);
  IC2Value = TIM_GetCapture2(TIM4);
	
  if ((IC2Value != 0)&&(IC1Value!=0))
  {
		
    DutyCycle = TIM_GetCapture1(TIM4) * 100 / IC2Value;
    Frequency = (RCC_Clocks.HCLK_Frequency)/2/60/ IC2Value; 
		if(rxState<30)   //wait for THead 10Te
		{
			//在 Te=400us的情况下  1MHz采样
			if((IC1Value<=450)&&(IC1Value>=350))   //同步头  
			{
				if((IC2Value<=900)&&(IC2Value>=700))   //连续收到多个Preamble    Normal Value=800
				{
					rxState++;
				}
		    else if((IC2Value<=4500)&&(IC2Value>=3500))    // Nromal = 4000
				{
					if(rxState>=5)      //收了5个以上的SYNC
					{	
						rxState=100;
						rxPos=0;
					}
					else
					{
						rxState=0;
					}
				}		
			}
			else
			{
				rxState=0;
			}
		}
		else  //接收数据中
		{
			if(DutyCycle>50)
			{
				rxBuf[rxPos]=0;
			}
			else
			{
				rxBuf[rxPos]=1;
			}
			rxPos++;
			if(rxPos>=66)
			{
				rxState=0;   // finsh and reset the process
				memcpy(rxBuf_bak,rxBuf,sizeof(rxBuf));
				rxBuf_bak_process=1;
               
				
		}
  }
}
  else
  {
    DutyCycle = 0; 
    Frequency = 0;
  }

}

void  Roll_code_keeloq_Init(void)
{
	  u8 i=0;

   
   ROLL_EEPROM.Serial_Number=0x123457;
   ROLL_EEPROM.Sync_Counter=0x0000048d;
   EncodeSavePassword_LSB=KeeLoq_Decrypt((ROLL_EEPROM.Serial_Number+0x20000000),0x0123456789ABCDEF);
   EncodeSavePassword_MSB=KeeLoq_Decrypt((ROLL_EEPROM.Serial_Number+0x60000000),0x0123456789ABCDEF);


				 
				
  EncodeSavePassword_Total=((uint64_t)EncodeSavePassword_MSB<<32)+(uint64_t)EncodeSavePassword_LSB; 
  ROLL_EEPROM.Encode_64bit_password=EncodeSavePassword_Total;

  
  pointer=((u8 *)&ROLL_EEPROM.Encode_64bit_password);

  
  printf("init :		64bit EEprom password :");
		for(i=8;i;i--)
			printf(" %02X",*(pointer+i-1));
		 printf("\r\n");	
  printf(" init:  ROLL_EEPROM.Serial_Number=0x%X    \r\n   init:    ROLL_EEPROM.Sync_Counter=0x%X \r\n",ROLL_EEPROM.Serial_Number,ROLL_EEPROM.Sync_Counter);

}


void  Enter_LearnState_checking(void)
{
   if(Learn.Learning)
  	  return;
  
   if((Rx66bit_34bit_FixedPart_ButtonStatus==0x0F)&&(Rx66bit_32bit_hoppingPart==0x00))
   	{
       Learn.Waiting_enterLearn_timer++;
	   if(Learn.Waiting_enterLearn_timer>=20)
	   	{
           Learn.Learning=1;
		   Learn.Learn_Timer=0;
		   Learn.Learn_step=0;
		   Learn.Waiting_enterLearn_timer=0;
	   	}
   	}   
   else
	 {
		Learn.Learn_Timer=0;
		Learn.Learn_step=0;
		Learn.Waiting_enterLearn_timer=0;
	 }
}


void  Interrupt_RX_process(void)
{
      	int i;  
		u32   reg_identity=0;		
		u8	  Rx_correct_counter=0; 
			
       if(rxBuf_bak_process==0)
	   	 return;

 


	   
				// 0.  Debug   OUT put  
	#if   0			
			     printf("\r\n Rxdata MSb->LSb:");
			    for(i=66;i;i--)
                    printf(" %d",rxBuf_bak[i-1]);
				 printf("\r\n");			
   #endif           

   
	           //  1. Rx66pit  ==>    34bit   FixPart  
	           
			   Rx66bit_34bit_FixedPart_repeatBit=rxBuf_bak[65];
			   Rx66bit_34bit_FixedPart_VlowBit=rxBuf_bak[64];
			   code_fixed=0;
			   for(i=32;i<64;i++)
			   {
				   if(rxBuf_bak[i]==1)
				   {
					   code_fixed|=(1<<(i-32));
				   }
			   }	               
			   
			   Rx66bit_34bit_FixedPart_ButtonStatus=code_fixed>>28;
			   Rx66bit_34bit_FixedPart_28bit_NO=(code_fixed&0x0FFFFFFF); 
			    
			  // printf("\r\n  Fixedpart ==>                                                RepeatBit=%d     VlowBit=%d    Key_button_=0x%X      SerialNumber=0x%000000X \r\n",
			   	  //    Rx66bit_34bit_FixedPart_repeatBit,
			   	  //    Rx66bit_34bit_FixedPart_VlowBit,
			   	  //    Rx66bit_34bit_FixedPart_ButtonStatus, 
			   	  //    Rx66bit_34bit_FixedPart_28bit_NO); 


			   //  2.   Get  encode key
               EncodeSavePassword_LSB=KeeLoq_Decrypt((Rx66bit_34bit_FixedPart_28bit_NO+0x20000000),0x0123456789ABCDEF);
			   EncodeSavePassword_MSB=KeeLoq_Decrypt((Rx66bit_34bit_FixedPart_28bit_NO+0x60000000),0x0123456789ABCDEF);

               EncodeSavePassword_Total=((uint64_t)EncodeSavePassword_MSB<<32)+(uint64_t)EncodeSavePassword_LSB; 
				
		#if   0		
		        pointer=((u8 *)&EncodeSavePassword_Total);
				printf("RXpassword     2:"); 
			    for(i=8;i;i--)
                    printf(" %02X",*(pointer+i-1));  
				 printf("\r\n");				 
                //  1.0   
                 printf("\r\n          MSB=0x%X    LSB=0x%X           EncodeSavePassword MSB=0x%X       LSB= 0x%X   \r\n",EncodeSavePassword_MSB,EncodeSavePassword_LSB); 
			  
        #endif   
                //  judge  
                //   step 1:     compare  password
                  if(ROLL_EEPROM.Encode_64bit_password!=EncodeSavePassword_Total)
				  	 ;// printf("\r\n                     64bit  Password  Error !");
				  else
				  	 {
				  	   //printf("\r\n                     Check  step 1  64bit   PASSED ");
					   Rx_correct_counter++;
				  	}  
				  
				 //   step 2:     compare  serialnumber
                  if(ROLL_EEPROM.Serial_Number!=Rx66bit_34bit_FixedPart_28bit_NO)
				  	 ;// printf("\r\n                     Serial Number  Error !");
				  else
				  	 {
				  	   //printf("\r\n                     Check  step 2  Serialnumber   PASSED ");
                       Rx_correct_counter++;
				  	}  
				    


			   //  2. Rx66pit  ==>    32bit   encrypt  Part  		
				Rx66bit_32bit_hoppingPart=0;
				for(i=0;i<32;i++)
				{
					if(rxBuf_bak[i]==1)
					{
						Rx66bit_32bit_hoppingPart|=(1<<i); 
					}
				}

				Enter_LearnState_checking(); 
                 
				
				Rx66bit_32bit_hoppinpPart_decodeTotal=KeeLoq_Decrypt(Rx66bit_32bit_hoppingPart,EncodeSavePassword_Total);//0x0123456789ABCDEF); 

				
                Rx66bit_32bit_hoppingPart_decode_buttonstatus=(Rx66bit_32bit_hoppinpPart_decodeTotal&0xF0000000)>>28;
                Rx66bit_32bit_hoppingPart_decode_OVR=(Rx66bit_32bit_hoppinpPart_decodeTotal&0x0C000000)>>26;
                Rx66bit_32bit_hoppingPart_decode_DISC=(Rx66bit_32bit_hoppinpPart_decodeTotal&0x03FF0000)>>16;
                Rx66bit_32bit_hoppingPart_decode_SYNcounter=Rx66bit_32bit_hoppinpPart_decodeTotal&0xFFFF;

        #if   0		
				printf("\r\n EncryptPart ==>  Rx66bit_32bit_hoppingPart=0x%08X ===>Decode= 0x%08X    |    Key=%x  OVR=%x   DISC=%x  sync=%08x \r\n",
				          Rx66bit_32bit_hoppingPart,
				          Rx66bit_32bit_hoppinpPart_decodeTotal,
				          Rx66bit_32bit_hoppingPart_decode_buttonstatus,
				          Rx66bit_32bit_hoppingPart_decode_OVR,
				          Rx66bit_32bit_hoppingPart_decode_DISC,
				          Rx66bit_32bit_hoppingPart_decode_SYNcounter);  
        #endif

               reg_identity=Rx66bit_34bit_FixedPart_28bit_NO&0x0000003FF; 

			  //   step 3:	   compare	10bit
			  if(reg_identity!=Rx66bit_32bit_hoppingPart_decode_DISC)
				  ;//printf("\r\n					   Serial Number  Error !");
			  else
				{
				  ;// printf("\r\n					   Check  step 3  10bit	 PASSED ");
				   Rx_correct_counter++;
			  	}   

			    //   step 4:	   compare	key
			  if(Rx66bit_34bit_FixedPart_ButtonStatus!=Rx66bit_32bit_hoppingPart_decode_buttonstatus)
				 ;// printf("\r\n					   Serial Number  Error !");
			  else
				{
				   //printf("\r\n					   Check  step 4  key	 PASSED ");
				   Rx_correct_counter++;
			  	}

			  //   step 5:	   compare	SYNC  
                 if(Rx66bit_32bit_hoppingPart_decode_SYNcounter< ROLL_EEPROM.Update_sync)
				 	 ;// printf("\r\n					   Sync  Error !");
				 else	  
			     {
			       if(Rx_correct_counter>=4) //前4次校验都通过了
			       	{
					       reg_identity=Rx66bit_32bit_hoppingPart_decode_DISC-ROLL_EEPROM.Update_sync;
		                   if(reg_identity<=16)
		                   	{
		                       //printf("\r\n					   Check  step 5  sync PASSED ");
		                       ROLL_EEPROM.Update_sync=Rx66bit_32bit_hoppingPart_decode_SYNcounter;
							   Rx_correct_counter++;
							   Decode_key_valueBuf_counter=0;// clear
		                   	}
						   else
						   	{
							   	 // printf("\r\n					   Sync  Error >16   !");

								  if(Decode_key_valueBuf_counter)
								  	{
                                       Decode_key_valueBuf[Decode_key_valueBuf_counter]=Decode_key_valueBuf[0];
									   Decode_key_valueBuf[0]=Rx66bit_32bit_hoppingPart_decode_SYNcounter;
									   if((Decode_key_valueBuf[0]-Decode_key_valueBuf[1])<=4)
									   	{
									   	 //  printf("\r\n					   Check  step 5  sync PASSED   updated automatic");
					                       ROLL_EEPROM.Update_sync=Rx66bit_32bit_hoppingPart_decode_SYNcounter;
										   Rx_correct_counter++;
									   	  Decode_key_valueBuf[0]=Rx66bit_32bit_hoppingPart_decode_SYNcounter; // save  
									   	  Decode_key_valueBuf_counter=1;  // clear 

									   	}
									   else  
									   	 {									   	    
									   	    Decode_key_valueBuf[0]=Rx66bit_32bit_hoppingPart_decode_SYNcounter; // save 
									   	    Decode_key_valueBuf_counter=1;  // clear
									    }
								  	}
								    else
	                                 {
	                                    Decode_key_valueBuf[Decode_key_valueBuf_counter++]=Rx66bit_32bit_hoppingPart_decode_SYNcounter; // save 
								    }   

								  
						   	}  

						   
			       	}	
				   else
				   	          ;//   printf("\r\n					  |  Error: Not  check  exactly   | !"); 
 
				 }

			 if(Rx_correct_counter>=5)
			 	{ 
			 	    printf("\r\n-------------------------------------------------");
					printf("\r\n|                        All    PASSED   SerialNumber=0x%X    key=0x%X",Rx66bit_34bit_FixedPart_28bit_NO,Rx66bit_32bit_hoppingPart_decode_buttonstatus);
                    printf("\r\n-------------------------------------------------");
					Buzz_status=1;
					buzzer_onoff(Buzz_status);
			 	}
             else
			 	 printf("\r\n|                        All    Failed!   Learn.Waiting_enterLearn_timer=%d" ,Learn.Waiting_enterLearn_timer); 

				

      //   end
      rxBuf_bak_process=0;  // clear
}

void  SensorPlus_caculateSpeed (void)
{
	
	 //if(DispContent==4)
	if((DispContent==4)&&(debugTestflag))  //  disp  显示    
	 {         
	  switch(debugTestflag)
	  	{
	  	  case 1:
		  	      printf("SYNC OK  %d(rxState=%d)>H=%d (H+L)=%d \r\n",count++,rxState,IC1Value,IC2Value);
                  break;
		  case 2:
                  printf("%d>H=%d IC2VALUE=%d ,IC1VALUE=%d duty=%d  rxState=%d  Frequency=%d\r\n",count++,TIM_GetCapture1(TIM4),IC2Value,IC1Value,DutyCycle,rxState,Frequency);
			      break;
		  case 3:
                  printf("Rx>%08x=>%08x sync=%08x DISC=%x OVR=%x Key=%x\r\n",
				          Rx66bit_32bit_hoppingPart,
				          code_decode,
				          code_decode&0xFFFF,
				         (code_decode&0x03FF0000)>>16,
				          (code_decode&0x0C000000)>>26,
				          (code_decode&0xF0000000)>>28);
			      break;
	      case  4:

			      break;
		  default:		
	             printf("\r\n DutyCycle=%d    IC2Value=%d  IC1Value=%d\r\n",DutyCycle,IC2Value,IC1Value);   
	  	}
	    debugTestflag=0;
	  } 


}



/*定时器配置*/
void TIM3_Config( void )  
{
	NVIC_InitTypeDef		NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );

	/* Enable the TIM3 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period		= 10000;             /* 1ms */
	TIM_TimeBaseStructure.TIM_Prescaler		= ( 168 / 2 - 1 );  /* 1M*/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_ARRPreloadConfig( TIM3, ENABLE );

	TIM_TimeBaseInit( TIM3, &TIM_TimeBaseStructure );

	TIM_ClearFlag( TIM3, TIM_FLAG_Update );
/* TIM Interrupts enable */
	TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );

/* TIM3 enable counter */
	TIM_Cmd( TIM3, ENABLE );
}


void TIM3_IRQHandler(void)
{

   if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET )
  {
                    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update); 

	        //---------------------Timer  Counter -------------------------- 
            TIM3_Timer_Counter++;

            if(TIM3_Timer_Counter==50)  //  100ms  ----100        
            {
               //if(debugTestflag==0) 
               // debugTestflag=2; 
               if(Buzz_status)
			   {
			      Buzz_status=0;
				  buzzer_onoff(Buzz_status);
               }
		       TIM3_Timer_Counter=0; 		  
            }

			if(Learn.Learning==1)  //  learn  buzzer  sound
			{
                if(TIM3_Timer_Counter%3)
					 buzzer_onoff(1);
				else
					 buzzer_onoff(0); 
			}
 //------------------------------------------------------------
   }
}



void  Learn_processing(void)
{
   	int i;  
		u32   reg_identity=0;		
		u8	  Rx_correct_counter=0; 
		
  
	  if(rxBuf_bak_process==0)
		return;
  
  
  
  
	  
			   // 0.  Debug   OUT put 
 #if   0		 
				printf(" Rxdata MSb->LSb:");
			   for(i=66;i;i--)
				   printf(" %d",rxBuf_bak[i-1]);
				printf("\r\n"); 		   
#endif			 
  
  
			  //  1. Rx66pit  ==>	 34bit	 FixPart  
			  
			  Rx66bit_34bit_FixedPart_repeatBit=rxBuf_bak[65];
			  Rx66bit_34bit_FixedPart_VlowBit=rxBuf_bak[64];
			  code_fixed=0;
			  for(i=32;i<64;i++)
			  {
				  if(rxBuf_bak[i]==1)
				  {
					  code_fixed|=(1<<(i-32));
				  }
			  } 				  
			  
			  Rx66bit_34bit_FixedPart_ButtonStatus=code_fixed>>28;
			  Rx66bit_34bit_FixedPart_28bit_NO=(code_fixed&0x0FFFFFFF); 
			   
		 //	printf("  Fixedpart ==> 											   RepeatBit=%d 	VlowBit=%d	  Key_button_=0x%X		SerialNumber=0x%000000X \r\n",
			//	 Rx66bit_34bit_FixedPart_repeatBit,
		   	// Rx66bit_34bit_FixedPart_VlowBit,
		   	// Rx66bit_34bit_FixedPart_ButtonStatus, 
		   	// Rx66bit_34bit_FixedPart_28bit_NO); 
  
  
			  //  2.   Get	encode key
			  EncodeSavePassword_LSB=KeeLoq_Decrypt((Rx66bit_34bit_FixedPart_28bit_NO+0x20000000),0x0123456789ABCDEF);
			  EncodeSavePassword_MSB=KeeLoq_Decrypt((Rx66bit_34bit_FixedPart_28bit_NO+0x60000000),0x0123456789ABCDEF);
  
			  EncodeSavePassword_Total=((uint64_t)EncodeSavePassword_MSB<<32)+(uint64_t)EncodeSavePassword_LSB; 
			   
	 #if   0 
			   pointer=((u8 *)&EncodeSavePassword_Total);
			   printf("RXpassword	  2:"); 
			   for(i=8;i;i--)
				   printf(" %02X",*(pointer+i-1));	
				printf("\r\n"); 				
			   //  1.0	 
				printf("\r\n		  MSB=0x%X	  LSB=0x%X			 EncodeSavePassword MSB=0x%X	   LSB= 0x%X   \r\n",EncodeSavePassword_MSB,EncodeSavePassword_LSB); 
			 
	 #endif   
			   //  judge  
	
  
			  //  2. Rx66pit  ==>	 32bit	 encrypt  Part		   
			   Rx66bit_32bit_hoppingPart=0;
			   for(i=0;i<32;i++)
			   {
				   if(rxBuf_bak[i]==1)
				   {
					   Rx66bit_32bit_hoppingPart|=(1<<i); 
				   }
			   }
  
				
			   
			   Rx66bit_32bit_hoppinpPart_decodeTotal=KeeLoq_Decrypt(Rx66bit_32bit_hoppingPart,EncodeSavePassword_Total);//0x0123456789ABCDEF); 
  
			   
			   Rx66bit_32bit_hoppingPart_decode_buttonstatus=(Rx66bit_32bit_hoppinpPart_decodeTotal&0xF0000000)>>28;
			   Rx66bit_32bit_hoppingPart_decode_OVR=(Rx66bit_32bit_hoppinpPart_decodeTotal&0x0C000000)>>26;
			   Rx66bit_32bit_hoppingPart_decode_DISC=(Rx66bit_32bit_hoppinpPart_decodeTotal&0x03FF0000)>>16;
			   Rx66bit_32bit_hoppingPart_decode_SYNcounter=Rx66bit_32bit_hoppinpPart_decodeTotal&0xFFFF;

			   
			   reg_identity=Rx66bit_34bit_FixedPart_28bit_NO&0x0000003FF; 
  
               
#if   0	
					   printf("\r\n EncryptPart ==>  Rx66bit_32bit_hoppingPart=0x%08X ===>Decode= 0x%08X	|	 Key=%x  OVR=%x   DISC=%x  sync=%08x  ident=%X\r\n",
								 Rx66bit_32bit_hoppingPart,
								 Rx66bit_32bit_hoppinpPart_decodeTotal,
								 Rx66bit_32bit_hoppingPart_decode_buttonstatus,
								 Rx66bit_32bit_hoppingPart_decode_OVR,
								 Rx66bit_32bit_hoppingPart_decode_DISC,
								 Rx66bit_32bit_hoppingPart_decode_SYNcounter,
								 reg_identity);	
#endif
  

			  //  Learn  process

			  switch(Learn.Learn_step)
			  	{
			  	  case  0:  
                  case  1: 
				  case  2:  	
								   //   compare  10bit
								   if(reg_identity==Rx66bit_32bit_hoppingPart_decode_DISC)
								   {
									  //printf("\r\n					  Check    10bit	PASSED ");									 
									  Rx_correct_counter++;
								   }   					  
								   // compare  key
								 if(Rx66bit_34bit_FixedPart_ButtonStatus==Rx66bit_32bit_hoppingPart_decode_buttonstatus)
								   {
									  //printf("\r\n					  Check     key	PASSED ");
									  Rx_correct_counter++;
									  if(Rx_correct_counter>=2)
									  {
									    if(Learn.Learn_step==0)    // work when  case  = 0
									      {
									        Learn.Learn_step=1;
											ROLL_learn.Encode_64bit_password=EncodeSavePassword_Total;
											ROLL_learn.Serial_Number=Rx66bit_34bit_FixedPart_28bit_NO;
											ROLL_EEPROM.Update_sync=
											
											printf("\r\n|					 		Learn step   1		");
											break;
									      }	
									  }	 
								   }						       

								 
					  
								 //   step 2:	  compare   64bit  password
								    if(ROLL_learn.Encode_64bit_password==EncodeSavePassword_Total)
								    {
								       Rx_correct_counter++;
									  if(Rx_correct_counter>=3)
									  	{
									  	  if(Learn.Learn_step==1)
                                          {
                                             Learn.Learn_step=2;
											 Learn.sync[0]=Rx66bit_32bit_hoppingPart_decode_SYNcounter;
											 printf("\r\n|					 		Learn step   2		");
										     break;
									  	  }	 

										  if(Learn.Learn_step==2)
										  {
										     if((Rx66bit_32bit_hoppingPart_decode_SYNcounter>Learn.sync[0])&&
											 	((Rx66bit_32bit_hoppingPart_decode_SYNcounter-Learn.sync[0])<=4))
										     	{
										     	   printf("\r\n					  Learn over   exit  learn ");
												   Learn.Learning=0;// clear
												   Learn.Learn_Timer=0;
												   Learn.Learn_step=0;
												   Learn.Waiting_enterLearn_timer=0;
												   
												   ROLL_EEPROM.Encode_64bit_password=ROLL_learn.Encode_64bit_password;
												   ROLL_EEPROM.Serial_Number=ROLL_learn.Serial_Number;
												   ROLL_EEPROM.Sync_Counter=Rx66bit_32bit_hoppingPart_decode_SYNcounter;

 
											 }
										     

										  }
									  	}

								    }
                                break;
				  
								   
			  	}
	 //   end
	 rxBuf_bak_process=0;  // clear


}



int main(void)
{

	//rt_hw_board_init(); 
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
//	USART_InitTypeDef	USART_InitStructure;

	/* disable interrupt first */
	//rt_hw_interrupt_disable();



		RCC_AHB1PeriphClockCmd(UART1_GPIO_RCC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APBPeriph_UART1, ENABLE);
	
	/*uart1 管脚设置*/
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX | UART1_GPIO_TX;
		GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
		GPIO_PinAFConfig(UART1_GPIO, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);
		GPIO_PinAFConfig(UART1_GPIO, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);
	
	/*NVIC 设置*/
		NVIC_InitStructure.NVIC_IRQChannel						= USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
		NVIC_Init( &NVIC_InitStructure );
	
		uart1_baud(115200);
	
		USART_Cmd( USART1, ENABLE );
		USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );

	  
	    printf("\n\r Rolling code  test\r\n ");		 
		
		 //---------------App Thread	 -----------------------		 
		 pulse_init();
		 TIM3_Config(); 
		  WatchDogInit();     

        // DF_init();	
		 
         
		 //BUZZER
		   GPIO_Config_PWM();
		   TIM_Config_PWM();
		   buzzer_onoff(0);

		   Roll_code_keeloq_Init();
 

		  
		  while(1)
			{
	
				SensorPlus_caculateSpeed();
				WatchDog_Feed();
				
				 //  Learn
				if(Learn.Learning==1)
				 {
				    Learn_processing();
				 }
				else
				    Interrupt_RX_process();
				
				u1_rxAnalysis();
				
			}


	return 0;
}

/*@}*/
