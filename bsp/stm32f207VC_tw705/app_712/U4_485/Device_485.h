/*
     Device_485_uart4.h
*/
#ifndef  _RT_485_U4
#define _RT_485_U4

#include <rthw.h>
#include <rtthread.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>


//#define   LCD_5inch   


#define  RX_485const         GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define  TX_485const         GPIO_SetBits(GPIOC,GPIO_Pin_4)  
#define  Power_485CH1_ON     GPIO_SetBits(GPIOB,GPIO_Pin_8)  // 第一路485的电	       上电工作
#define  Power_485CH1_OFF    GPIO_ResetBits(GPIOB,GPIO_Pin_8)

#define _485_dev_SIZE   600
#define  normal                 2

typedef  struct _485_REC
{
    u8     _485_receiveflag;
    u16   _485_RxLen;  	

}_485REC_Struct;   


#ifdef LCD_5inch

typedef struct  _LARGE_LCD
{
   u8  Enable_Flag;   //  使能发送标�
   u16 CounterTimer;  // 计数器
   u8   Txinfo[300];   //  寄存器内容
   u16  TxInfolen;    //   发送 信息长度
   u8   TXT_content[256]; 
   u16  TXT_contentLen;
   u8    RxInfo[100];    // 接收内容 
   u16  RxInfolen;     // 接收信息长度 
   u8   Type;    //  01  授时 ， 02   速度方向 03 文本信息 
   u8   Process_Enable; //  发送标志位    

}LARGELCD;

//   SD_Type  LCD 
#define   LCD_IDLE               0
#define   LCD_SETTIME        1
#define   LCD_SETSPD          2
#define   LCD_SDTXT            3
#define   LCD_GPSNUM         4
#define   LCD_GSMNUM        5
#define   LCD_VechInfo       6

//  RX_ADD
//#define   LCD_VecNum        0x3000   // 车牌号
//#define   LCD_Print             0x0000   //  打印
//#define   LCD_NetQry         0x3100   //  网络配置





extern   LARGELCD     DwinLCD;   
#endif

//------------------------------变量声明---------------------------------------------------
extern u8   Take_photo[10];   //----  报警拍照命令 
extern u8   Fectch_photo[10];   //----- 报警取图命令  

 extern u8 	 _485_content[600];
 extern u16	 _485_content_wr;
 extern u8    _485_CameraData_Enable;// 有图片数据过来	1: data come  0:   no data 

 extern  _485REC_Struct 	 _485_RXstatus;	  

 extern  struct rt_device  Device_485;



//-----------------------------
extern void  _485_RxHandler(u8 data);  

//-----------------------------
extern void  Photo_TakeCMD_Update(u8 CameraNum);
extern void  Photo_FetchCMD_Update(u8 CameraNum);

extern void  OpenDoor_TakePhoto(void);

//----------------------------
extern void _485_delay_us(u16 j);
extern void _485_delay_ms(u16 j);
extern void _485_startup(void);
extern  void rt_hw_485_putc(const char c);
extern  void rt_hw_485_output(const char *str);
extern void rt_hw_485_Output_Data(const char *Instr, unsigned int len) ;


//  Dwinlcd  
#ifdef	LCD_5inch   
extern void  DwinLCD_work_Enable(void);  
extern void  DwinLCD_work_Disable(void); 
extern void  DwinLCD_Timer(void);
extern void  DwinLCD_DispTrigger(void);
extern void  DwinLCD_Data_Process(void); 
#endif


#endif 

