/*
     App_808.C
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>
//#include "usbh_usr.h"P


/* 定时器的控制块 */
 static rt_timer_t timer_app; 


//------- change  on  2013 -7-24  --------
rt_thread_t app_tid = RT_NULL; // app 线程 pid



//----- app_thread   rx     gsm_thread  data  related ----- 	
ALIGN(RT_ALIGN_SIZE)
//static  struct rt_semaphore app_rx_gsmdata_sem;  //  app 提供数据 给gsm发送信号量


// Dataflash  Operate   Mutex 







//----- app_thread   rx     gps_thread  data  related ----- 	
//ALIGN(RT_ALIGN_SIZE)
//static  MSG_Q_TYPE  app_rx_gps_infoStruct;  // app   接收从gsm  来的数据结构
//static  struct rt_semaphore app_rx_gps_sem;  //  app 提供数据 给gps发送信号量

//----- app_thread   rx    485 _thread  data  related ----- 	
//ALIGN(RT_ALIGN_SIZE)
//static  MSG_Q_TYPE  app_rx_485_infoStruct;  // app   接收从gsm  来的数据结构
//static  struct rt_semaphore app_rx_485_sem;  //  app 提供数据 给gps发送信号量 


u8  Udisk_Test_workState=0;  //  Udisk 工作状态
u8  TF_test_workState=0;

rt_device_t   Udisk_dev= RT_NULL;
u8 Udisk_filename[30]; 
int  udisk_fd=0;

u32   sec_num=2;
u32       WarnTimer=0; 

u8   OneSec_CounterApp=0;
u32  app_thread_runCounter=0;  
u32  gps_thread_runCounter=0;



 //  1. MsgQueue Rx
void  App_thread_timer(void)
{
   app_thread_runCounter++;
   if(app_thread_runCounter>300)   // 400* app_thread_delay(dur)
    {
       reset();  
   	}   
}

void  gps_thread_timer(void)
{
   gps_thread_runCounter++;
   if((gps_thread_runCounter>300)&&(BD_ISP.ISP_running==0))   // 400* app_thread_delay(dur)   
    {
       reset();  
   	}   
} 

void Device_RegisterTimer(void)
{
      if(0==JT808Conf_struct.Regsiter_Status)    //注册   
          {
             DEV_regist.Sd_counter++;
			 if(DEV_regist.Sd_counter>32)
			 	{
                   DEV_regist.Sd_counter=0;
				   DEV_regist.Enable_sd=1;  
			 	}
          }
}

void Device_LoginTimer(void)
{  
  if(1==DEV_Login.Operate_enable)
  {
     DEV_Login.Sd_counter++;
	 if(DEV_Login.Sd_counter>32)    
	 {
          DEV_Login.Sd_counter=0;
	      DEV_Login.Enable_sd=1;

		  DEV_Login.Sd_times++;
		  if(DEV_Login.Sd_times>4)
		  	{
		  	   DEV_Login.Sd_times=0;
			   rt_kprintf("\r\n  鉴权次数Max，重新注册!\r\n"); 
               DEV_regist.Enable_sd=1;   			   
		  	}
	 }
  }
}

//          System  reset  related  
#if 0
void  system_reset(void)
{
      Systerm_Reset_counter=Max_SystemCounter-3;
	  rt_kprintf("\r\n handle reset \r\n");
}
FINSH_FUNCTION_EXPORT(system_reset, system_reset);  

void query_reset(void)
{
		  rt_kprintf("\r\n Systerm_Reset_counter=%d     app_thread_runCounter=%d  \r\n",Systerm_Reset_counter,app_thread_runCounter); 
}
FINSH_FUNCTION_EXPORT(query_reset, query_reset);	
#endif
void   Reset_Saveconfig(void)
{

}
	
void  App808_tick_counter(void) 
{
    Systerm_Reset_counter++;
    if(Systerm_Reset_counter>Max_SystemCounter)
     {   	Systerm_Reset_counter=0;	
	        rt_kprintf("\r\n Sysem  Control   Reset \r\n"); 
               reset(); 
     }  
    //-------------------------------------------
    if((Systerm_Reset_counter&0x1FF)==0x1FF)  //0x1ff   
    {
        DistanceWT_Flag=1;  
    }  
}

void  Emergence_Warn_Process(void)
{
  //----------- 紧急报警下拍照相关的处理  ------------------
  if(WARN_StatusGet()) 
  {
     //  拍照过程中和传输过程中不予处理                        
	 if((CameraState.status==other)&&(Photo_sdState.photo_sending==0)&&(0==MultiTake.Taking)&&(0==MultiTake.Transfering))
	  {
		  WarnTimer++;
		  if(WarnTimer>=4)
			  {
			      WarnTimer=0;    	
				       //----------  多路摄像头拍照 -------------
					 // MultiTake_Start();
					 // Start_Camera(1);  //先拍一路满足演示   
			  }   
	  }   
     //-------------------------------------------------------
     
	 fTimer3s_warncount++;
	 if(fTimer3s_warncount>=4)			 
	{
		 // fTimer3s_warncount=0;
		 if ( ( warn_flag == 0 ) && ( f_Exigent_warning == 0 )&&(fTimer3s_warncount==4) )
		 {
			    warn_flag = 1; 					// 报警标志位
			    Send_warn_times = 0;		  //  发送次数 
		        //-----------------------------------
				#if  0
					 rt_kprintf( "紧急报警 ");				 
					 StatusReg_WARN_Enable(); // 修改报警状态位
					 PositionSD_Enable();  
					 Current_UDP_sd=1;   	
			   #endif  
			   //---------------------
			   
			   if((Key_MaskWord[3]&0x01)==0x00)
				 {
					if((Warn_MaskWord[3]&0x01)==0x01)
					 {
						   rt_kprintf( "紧急报警 Fail，MaskEnable !");		 
			   
					 }
					 else
					 {
						  rt_kprintf( "紧急报警 "); 			  
						  StatusReg_WARN_Enable(); // 修改报警状态位
						  PositionSD_Enable();	
						  Current_UDP_sd=1;  
						 // warn_msg_sd(); // 短息报警
					  }
				  }
				  else
					 {
						  /* if((Warn_MaskWord[3]&0x01)==0x01)
							 {
								   rt_kprintf( "\r\n紧急报警 触发上报，屏蔽字使能 ,关键字使能就上报! ");	 
			   
							 }
							else
								rt_kprintf( "\r\n紧急报警 触发上报，关键字使能就上报! ");		
						 */	 
								  StatusReg_WARN_Enable(); // 修改报警状态位
								  PositionSD_Enable();	
								  Current_UDP_sd=1; 
								//  warn_msg_sd();// 短息报警
			   
			   
					 }
		 }
	}

	 //------------------------------------------------- 
  } 
   else
  { 	   
	   WarnTimer=0;
	   fTimer3s_warncount=0;  
	   //------------ 检查是否需要报警拍照 ------------
	 /*  if(CameraState.status==enable)
	   {		   
		 if((CameraState.camera_running==0)||(0==Photoing_statement))  // 在不传输报警图片的情况下执行
		 {
			CameraState.status=disable;
		 }
	   }
	   else*/
	   if(CameraState.camera_running==0)
			CameraState.status=other;
  }   


}

void SIMID_Convert_SIMCODE( void ) 
{
		SIM_code[0] = SimID_12D[0] - 0X30;
		SIM_code[0] <<= 4;
		SIM_code[0] |= SimID_12D[1] - 0X30;	  

		SIM_code[1] = SimID_12D[2] - 0X30;
		SIM_code[1] <<= 4;
		SIM_code[1] |= SimID_12D[3] - 0X30;	

		SIM_code[2] = SimID_12D[4] - 0X30;
		SIM_code[2] <<= 4;
		SIM_code[2] |= SimID_12D[5] - 0X30;	

		SIM_code[3] = SimID_12D[6] - 0X30;
		SIM_code[3] <<= 4;
		SIM_code[3] |= SimID_12D[7] - 0X30;	

		SIM_code[4] = SimID_12D[8] - 0X30;
		SIM_code[4] <<= 4;
		SIM_code[4] |= SimID_12D[9] - 0X30;	

		SIM_code[5] = SimID_12D[10] - 0X30;
		SIM_code[5] <<= 4;
		SIM_code[5] |= SimID_12D[11] - 0X30; 
}


static void timeout_app(void *  parameter)
{     //  100ms  =Dur 
    u8  SensorFlag=0,i=0;


    
	//---------  Step timer
	//Dial_step_Single_10ms_timer();	   	
	//---------- AT Dial upspeed---------
	if((OneSec_CounterApp%2)==0)   	 
	{ 
	   if((CommAT.Total_initial==1)) 
		{	
		    if(CommAT.cmd_run_once==0)
		        CommAT.Execute_enable=1;	 //  enable send   periodic 		
		  /*  if(( CommAT.Initial_step==17)&&(Login_Menu_Flag==0)) 
				CommAT.cmd_run_once=1;
			else
				CommAT.cmd_run_once=0;  
				*/ 
	   	}
	}	
     OneSec_CounterApp++;
     if(OneSec_CounterApp>=5)	 
	{        
	    OneSec_CounterApp=0;
		 
		//RTC_TimeShow();
		 //SensorPlus_caculateSpeed();  
		  //  debugTestflag=1;

			WatchDog_Feed();       

     }	 


}

 void   MainPower_cut_process(void)
 {
    Powercut_Status=0x02;
	//----------断电了  ------------
	//---------------------------------------------------
	//        D4  D3
	//        1   0  主电源掉电 
	StatusReg_POWER_CUT(); 
	//----------------------------------------------------
	Power_485CH1_OFF;  // 第一路485的电			 关电工作
	//-----------------------------------------------------			    
	//------- ACC 不休眠 ----------
	Vehicle_RunStatus|=0x01;
	//   ACC ON		 打火 
	StatusReg_ACC_ON();  // ACC  状态寄存器   
	Sleep_Mode_ConfigExit(); // 休眠相关
	//-------  不欠压--------------
	Warn_Status[3]&=~0x80; //取消欠压报警   
	SleepCounter=0; 

 }


 void  MainPower_Recover_process(void)
 { 
	//----------------------------------------------
	StatusReg_POWER_NORMAL();
	//----------------------------------------------
	Power_485CH1_ON;  // 第一路485的电 		  开电工作
}


 ALIGN(RT_ALIGN_SIZE)
char app808_thread_stack[4096];       
struct rt_thread app808_thread;

static void App808_thread_entry(void* parameter) 
{
	
//    u32  a=0;	

	 // rt_kprintf("\r\n ---> app808 thread start !\r\n");  

	    pulse_init();
		TIM3_Config();

		//TIM3_Config();
       //  step 3:    usb host init	   	    	//  step  4:   TF card Init    
          usbh_init();    
          //Init_ADC(); 
		 // gps_io_init();
		  
		 // CAN_struct_init();   
		  
        /* watch dog init */
	    WatchDogInit();       

	     Init_Camera();  
        // DoorCameraInit();

       //BUZZER
	  // GPIO_Config_PWM();
	  // TIM_Config_PWM();  
	  // buzzer_onoff(0); 
        
	while (1)
	{

	    SensorPlus_caculateSpeed();   
	    rt_thread_delay(15);    	     
      //    485   related  over   	 	   
      //---------------------------------------- 
	   app_thread_runCounter=0; 
	   //--------------------------------------------------------
	}
}

/* init app808  */
void Protocol_app_init(void)
{
        rt_err_t result;

        
       //---------  timer_app ----------
	         // 5.1. create  timer     100ms=Dur
	   timer_app=rt_timer_create("tim_app",timeout_app,RT_NULL,20,RT_TIMER_FLAG_PERIODIC); 
	        //  5.2. start timer
	   if(timer_app!=RT_NULL)
	           rt_timer_start(timer_app);      

      result=rt_thread_init(&app808_thread, 
		"app808", 
		App808_thread_entry, RT_NULL,
		&app808_thread_stack[0], sizeof(app808_thread_stack),   
		Prio_App808,10);          

    if (result == RT_EOK)
    {
         rt_thread_startup(&app808_thread); 
    }
}



