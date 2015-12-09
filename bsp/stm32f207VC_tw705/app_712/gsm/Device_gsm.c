/*
    App Gsm uart
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include "App_moduleConfig.h" 
#include "App_gsm.h"
#include "SMS_PDU.h"
#include "SMS.h"

#define GSM_GPIO			GPIOC
#define GSM_TX_PIN			GPIO_Pin_10
#define GSM_TX_PIN_SOURCE	GPIO_PinSource10

#define GSM_RX_PIN			GPIO_Pin_11
#define GSM_RX_PIN_SOURCE	GPIO_PinSource11



#ifdef   M66_GSM



//----------  EM310 �Դ�Э��ջ���  -----------------------------
//   1.  ͨ�������״̬ ��ʼ������   
/*
flash char  SIM_Check_Str[]="AT%TSIM\r\n";   //  ���SIM���Ĵ���
flash char  IMSI_Check_str[]="AT+CIMI\r\n";   
flash char  Signal_Intensity_str[]="AT+CSQ\r\n";  // �ź�ǿ�� ������
flash char  CommAT_Str1[]="ATV1\r\n";
flash char  CommAT_Str2[]="ATE0\r\n";
flash char  CommAT_Str3[]="AT+COPS?\r\n";  
flash char  CommAT_Str4[]="AT%SNFS=1\r\n";           // ������Ƶ���ͨ��ѡ�� �ڶ�·
flash char  CommAT_Str5[] ="AT%NFI=1,10,0,0\r\n";  // ������Ƶ����ͨ�� ѡ��� ��·      
flash char  CommAT_Str6[] ="AT+CLVL=6\r\n";  
flash char  CommAT_Str7[]="AT%NFV=5\r\n";  // ����������  --û����Ƶ���� û��
flash char  CommAT_Str8[]="AT%NFO=1,6,0\r\n";//�����������
flash char  CommAT_Str9[]="AT%VLB=1\r\n";//ATI
flash char  CommAT_Str10[]="AT%NFW=1\r\n";    //   ������Ƶ����
flash char  CommAT_Str11[]="AT+CGMR\r\n";//"AT%RECFDEL\r\n";//"AT\r\n"; //    
flash char  CommAT_Str12[]="AT+CMGF=0\r\n"; 		///PDUģʽ
flash char  CommAT_Str13[]="AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n";   
flash char  CommAT_Str14[]="AT+CNMI=1,2\r\n"; 		///ֱ�ӽ��������
flash char  CommAT_Str15[]="AT+CMGD=1,4\r\n"; 
flash char  CommAT_Str16[]="AT+CSCA?\r\n"; 
flash char  CommAT_Str17[]="AT+CCID\r\n"; 
flash char  CommAT_Str18[]="AT+VGR=9\r\n";  // ͨ���������  
*/
#endif

#ifdef MC8332_CDMA 
#define   RX_IDLE              0
#define   RX_HEADright         1
#define   RX_HEADerror         2   
//   2.  CDMA   ͨ�������״̬ ��ʼ������   
flash char  SIM_Check_Str[]="AT%TSIM\r\n";   //  ���SIM���Ĵ���
flash char  IMSI_Check_str[]="AT+CIMI\r\n";   
flash char  Signal_Intensity_str[]="AT+CSQ\r\n";  // �ź�ǿ�� ������
flash char  CommAT_Str1[]="ATV1\r\n";
flash char  CommAT_Str2[]="ATE0\r\n";
flash char  CommAT_Str3[]="AT+CGMR\r\n";    // ����汾��Ϣ
flash char  CommAT_Str4[]="AT+SPEAKER=1\r\n";           // ������Ƶ���ͨ��ѡ�� �ڶ�·
flash char  CommAT_Str5[] ="AT+VGT=0\r\n";  // ������Ƶ����ͨ�� ѡ���һ·      
flash char  CommAT_Str6[] ="AT\r\n";  
flash char  CommAT_Str7[]="AT+VGR=1\r\n";  // ����������  --û����Ƶ���� û��
flash char  CommAT_Str8[]="AT+ZCVF=2\r\n";//ͨ������
flash char  CommAT_Str9[]="ATI\r\n";//ATI
flash char  CommAT_Str10[]="AT+CLIP=1\r\n";    //   ������Ƶ����
flash char  CommAT_Str11[]="AT+ECHO=2\r\n";//"AT%RECFDEL\r\n";//"AT\r\n"; //   
flash char  CommAT_Str12[]="AT+CMGF=0\r\n"; 		///PDUģʽ
flash char  CommAT_Str13[]="AT+CPMS=\"MT\",\"MT\",\"MT\"\r\n";   
flash char  CommAT_Str14[]="AT+CNMI=2,1,0,0,0\r\n"; 		///ֱ�ӽ��������
flash char  CommAT_Str15[]="AT+CMGD=1,4\r\n"; 
flash char  CommAT_Str16[]="AT\r\n"; // ��ȡ��Ϣ���ĺ���
flash char  CommAT_Str17[]="AT+GETICCID\r\n"; 
flash char  CommAT_Str18[]="ATI=9\r\n";  // ͨ���������  
#endif

#ifdef M66_GSM
//    2.   ͨ��ģ���½������·�������
flash char       DialInit1[]                          =    "AT+CREG?\r\n"; // "AT+CGACT=1,1\r\n"; S
flash char	DialInit2[]				=	"AT%IOMODE=1,2,1\r\n";	
flash char	DialInit3[]				=	"AT%ETCPIP=\"user\",\"gprs\"\r\n";	  
flash char	DialInit4[] 			       =	"AT%ETCPIP?\r\n";  
char	Dialinit_DNSR[50] 			=	"AT%DNSR=\"";     //  ���� up.gps960.com
char	Dialinit_DNSR2[50] 			=	"AT%DNSR=\"";     //  ����

flash char	DialInit6[] 			       =	"AT\r\n";  
flash char	DialInit7[] 			       =	"AT\r\n";	 
flash char	DialInit8[] 			       =	"AT\r\n";//"AT+CGPADDR\r\n";     

flash char	Open_TCP[]="AT%ETCPIP=\"user\",\"gprs\"\r\n";	

static u8	Send_TCP[18]="AT%IPSENDX=1,\"";
static u8	Send_ISP[18]="AT%IPSENDX=2,\"";	
static u8	DialStr_Link1[90]="AT%IPOPENX=1,\"TCP\",\"";//"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8   DialStr_LinkAux[50]="AT%IPOPENX=1,\"TCP\",\"";//"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8	Dialinit_APN[40]="AT+CGDCONT=1,\"IP\",\"";		//CMNET\"\r\n"; 	 // oranges Access Point Name
static u8   DialStr_IC_card[50]="AT%IPOPENX=2,\"TCP\",\""; //  Link2    




flash char  CutDataLnk_str1[]="AT%IPCLOSE=3\r\n";
flash char  CutDataLnk_str2[]="AT%IPCLOSE=2\r\n";
flash char  CutDataLnk_str3[]="AT%IPCLOSE=1\r\n";
flash char  CutDataLnk_str4[]="AT%IPCLOSE=5\r\n";
flash char  CutDataLnk_str5[]="ATH\r\n";     

#endif

//  M66  Rec voice
flash char VoiceRec_config[]="AT%RECCFG=\"AMR\",0\r\n";  // ����¼����ʽ��¼������
flash char VoiceRec_start[]="AT%RECSTART=\"";  //at%recstart="test1.amr",10 
flash char VoiceRec_stop[]="AT%RECSTOP\r\n"; 	 // %RECSTOP: 10,6824
flash char  VoiceRec_getdata[]="AT%RECFGET=\"";          //at%recfget="test1.amr",100,100
flash char  VoiceRec_delFile[]="AT%RECFDEL\r\n";   //  AT%RECFDEL="test1.amr" 


#ifdef  MC8332_CDMA
//    2.   ͨ��ģ���½������·�������
flash char       DialInit1[]                          =    "AT+ZPPPCLOSE\r\n"; // "AT+CGACT=1,1\r\n"; S  
flash char	DialInit2[]				=	"AT+ZPNUM=#777\r\n";	
flash char	DialInit3[]				=	"AT+ZPIDPWD=card,card\r\n";	  
flash char	DialInit4[] 			       =	"AT+ZPPPSTATUS\r\n";  

flash char	DialInit6[] 			       =	"AT+ZPPPOPEN\r\n";  
flash char	DialInit7[] 			       =	"AT\r\n";	 
flash char	DialInit8[] 			       =	"AT+ZIPGETIP\r\n";//"AT+CGPADDR\r\n";     

char	Dialinit_DNSR[50] 			=	"AT+ZDNSGETIP=";     //  ���� up.gps960.com
char	Dialinit_DNSR2[50] 			=	"AT+ZDNSGETIP=";     //  ���� 



static u8	Send_TCP[18]="AT+ZIPSEND=0,";
static u8	Send_ISP[18]="AT+ZIPSEND=1,";	
static u8	DialStr_Link1[90]="AT+ZIPSETUP=0,";//"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8   DialStr_LinkAux[50]="AT+ZIPSETUP=1,";//"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8	Dialinit_APN[40]="AT+CREG\r\n";		//CMNET\"\r\n"; 	 // oranges Access Point Name 
static u8   DialStr_IC_card[50]="AT%IPOPENX=2,\"TCP\",\""; //  Link2    




flash char  CutDataLnk_str1[]="AT+ZIPCLOSE=2\r\n"; 
flash char  CutDataLnk_str2[]="AT+ZIPCLOSE=1\r\n";
flash char  CutDataLnk_str3[]="AT+ZIPCLOSE=0\r\n";
flash char  CutDataLnk_str4[]="AT+ZPPPCLOSE\r\n";    // �ر���������
flash char  CutDataLnk_str5[]="ATH\r\n";  

u8  GSM_Rx_RawFlag=RX_IDLE; // 0: idle  1: headRight  2: headerror     GSM ģʽ�½���ģ��ԭʼ������Ϣ ״̬λ  


#endif

//-------  struct  variables -------------
GSM_POWER   GSM_PWR;  
static IMSI_GET       IMSIGet; 
COMM_AT               CommAT; 
DATA_DIAL             DataDial;  



ALIGN(RT_ALIGN_SIZE)
 u8     GSM_rx[GSMRX_SIZE];       
u16     GSM_rx_Wr=0;
u16     info_len=0;  
static u8  former_byte=0; 

static GSM_typeBUF GSM_INT_BUFF;
static GSM_typeBUF GSM_RX_BUFF;

u8  GSM_AsciiTx[GSM_AsciiTX_SIZE];
u16   GSM_AsciiTx_len=0; 

//-----  1 s timer  related below   ------
u16   one_second_couner=0;  
u8     Enable_UDP_sdFlag=0;      
 

u8    Dial_jump_State=0;  // ���Ź�����  ״̬��ת��־�� DialInit5  DialInit6 �� DialInit7 ����ת���
u8    Dnsr_state=0;  //  DNSR ״̬   1: ��ʾ�����������ɹ���ǰ����


//------- GPPRS ������� ----------------
//u8 Datalink_close=0;  //�ҶϺ��ٵ�½

//--------   �绰����ָʾ -------------------
u8  Ring_counter=0;  // ring   ���绰������ָʾ
u8  Calling_ATA_flag=0; //     �����绰����

//---------------------------
u8  Delete_all_sms_flag=0;  // ɾ�����ж��ű�־λ  


//-------- TCP2 send ---------
u8     TCP2_ready_dial=0;
u16    Ready_dial_counter2=0;
u16    TCP2_notconnect_counter=0;
u8     TCP2_Connect=0;
u8	  TCP2_sdFlag=0;		//��ʱ����GPSλ����Ϣ��־
u16    TCP2_sdDuration=50; 
u8      TCP2_Coutner=0;  // ��ʱ������
u8      TCP2_login=0;       // TCP ���������Ӻ�ı�־λ
u8       Online_error_counter=0;   // ���������������,�������ִ��������     


//    TTS   ���
 TTS              TTS_Var;  //  TTS ���ͱ���
 ALIGN(RT_ALIGN_SIZE)
 u8                AT_TTS[1024];
 u16               TTS_Len=0;	 


//  Voice  Record 

 

static void GSM_Process(u8 *instr, u16 len);
u32 GSM_HextoAscii_Convert(u8*SourceHex,u16 SouceHexlen,u8 *Dest);   

//=========================================================================
//    TTS  realated
u8    TTS_Get_Data(u8 *Instr,u16 LEN)     //  return   0   : OK     return   1 : busy
{
     // 1. check status 
   if(TTS_Var.Playing)
   	{
           memset(TTS_Var.HEX_BUF,0,sizeof((const char*)TTS_Var.HEX_BUF));
          TTS_Var.HEX_len=LEN;
	    memcpy(TTS_Var.HEX_BUF,Instr,LEN);		
           TTS_Var.Save=1;  
   	    return  TTS_BUSY;
   	}  	
    //  2.   HEX to  ASCII convert 
     memset(TTS_Var.ASCII_BUF,0,sizeof((const char*)TTS_Var.ASCII_BUF));
     TTS_Var.ASCII_Len=GSM_HextoAscii_Convert(Instr,LEN,TTS_Var.ASCII_BUF);
     TTS_Var.ASCII_Len=(LEN<<1);// ���ȳ��� 2	 
     //  3. cacu  timeout value
     TTS_Var.TimeOut_limt=LEN/3+3;  // 3����ÿ��+ 3  �뱣��ʱ��
     TTS_Var.TimeCounter=0;
     //  4.  ready to play
	TTS_Var.NeedtoPlay=1;	 
    return  TTS_OK;
}
u8    TTS_Data_Play(void) 
{
       u16   i=0;
	//  1.  check status  
      if(TTS_Var.Playing)
              return  TTS_BUSY;
      else
	if(TTS_Var.Save)  //  2.  check  save  status
	{      
	      TTS_Var.Save=0;
	     //  3. HEX to  ASCII	  
	     memset(TTS_Var.ASCII_BUF,0,sizeof((const char*)TTS_Var.ASCII_BUF));
	     TTS_Var.ASCII_Len=GSM_HextoAscii_Convert(TTS_Var.HEX_BUF,TTS_Var.HEX_len,TTS_Var.ASCII_BUF);
           TTS_Var.ASCII_Len=(TTS_Var.HEX_len<<1);// ���ȳ��� 2	 
	     //  4. cacu  timeout value
	     TTS_Var.TimeOut_limt=TTS_Var.HEX_len/3+3;  // 3����ÿ��+ 3  �뱣��ʱ��
	     TTS_Var.TimeCounter=0;
	     //  5.  ready to play
		TTS_Var.NeedtoPlay=1;        
	}
       	
      //   6.   play process
      if( TTS_Var.NeedtoPlay	==1)
      	{
      	  Speak_ON;   // ������
      	   TTS_Var.Playing=1;
           //  head
            memset(AT_TTS,0,sizeof(AT_TTS));  
		 #ifdef MC8332_CDMA
             strcat(AT_TTS,"AT+ZTTS=2,\"");   // 2  GBK ����
		 #endif   
		    TTS_Len=strlen((const char*)AT_TTS)	;	      
	   //  info		 
	     memcpy(AT_TTS+TTS_Len,TTS_Var.ASCII_BUF,TTS_Var.ASCII_Len);	
	     TTS_Len+=TTS_Var.ASCII_Len; 
	   //  tail	 
	      memcpy((char*)AT_TTS+TTS_Len,"\"\r\n",3);    // tail 	 
            TTS_Len+=3;     	 
	        
	    for(i=0;i<TTS_Len;i++)
			 rt_kprintf("%c",AT_TTS[i]);   

	   rt_hw_gsm_output_Data(AT_TTS,TTS_Len);	

	  WatchDog_Feed();
         delay_ms(30); // rt_thread_delay(RT_TICK_PER_SECOND/8);	
 
           //---------------------
           TTS_Var.NeedtoPlay=0; 
      	}
	  return TTS_OK;
}

void   TTS_Play_End(void)
{
             TTS_Var.Playing=0;
	      TTS_Var.TimeCounter=0;
		 
	   Speak_OFF;	   // �رչ���
}

void TTS_Exception_TimeLimt(void)     //  ��λ: s 
{    //  TTS  �����쳣 ��ʱ������
      if(TTS_Var.Playing)
      	{
	         TTS_Var.TimeCounter++;
		  if(TTS_Var.TimeCounter>TTS_Var.TimeOut_limt)
		  	{
		           TTS_Var.Playing=0;
		           TTS_Var.TimeCounter=0;
		  	}
      	}
}

u8  TTS_ACK_Error_Process(void)
{   // ����TTS  ������������󷵻�Error �쳣
      if(TTS_Var.Playing)
        {      
	           TTS_Var.Playing=0;
	           TTS_Var.TimeCounter=0; 
		     return true;	   
	 }	
      else
	  	   return false;
}

void TTS_play(u8 * instr)
{
      TTS_Get_Data(instr,strlen((const char*)instr));
     // rt_kprintf("\r\n    �ֶ���������: %s\r\n",instr);
}
FINSH_FUNCTION_EXPORT(TTS_play, TTS play);


void GSM_CSQ_timeout(void)
{
            CSQ_counter++;
          if(CSQ_counter>=CSQ_Duration)
          {
              CSQ_counter=0;  
              CSQ_flag=1;    
          }	 

}

u8 GSM_CSQ_Query(void)
{    
       if((CSQ_flag==1)&&(MediaObj.Media_transmittingFlag==0)&&(Dev_Voice.CMD_Type!='1'))  
	   { 
		  CSQ_flag=0; 
		  delay_ms(100);  
		  rt_hw_gsm_output("AT+CSQ\r\n");    //����ź�ǿ��
		    if(DispContent)	
		        rt_kprintf("AT+CSQ\r\n");  
		  return true;	
	   } 

	   return false;   
}




//-------------------------------------------------------------------------------
void  DataLink_MainSocket_set(u8 *IP, u16  PORT, u8 DebugOUT)
{
   #ifdef MC8332_CDMA
       memset((char *)DialStr_Link1+14,0,sizeof(DialStr_Link1)-14);
	   IP_Str((char *)DialStr_Link1+14, *( u32 * ) IP);		 

      strcat((char *) DialStr_Link1,"," );
	  sprintf((char*)DialStr_Link1+strlen((char const*)DialStr_Link1), "%u\r\n", PORT);	
   #endif
	     if(DebugOUT)
	     	{
		  rt_kprintf("		   Main Initial  Str:");  
		  rt_kprintf((char*)DialStr_Link1);
	     	}  
}

void  DataLink_AuxSocket_set(u8 *IP, u16  PORT,u8 DebugOUT) 
{
  

   #ifdef MC8332_CDMA
	  memset((char *)DialStr_LinkAux+14,0,sizeof(DialStr_LinkAux)-14);
	  IP_Str((char *)DialStr_LinkAux+14, *( u32 * ) IP);		
  
	 strcat((char *) DialStr_LinkAux,"," );
	 sprintf((char*)DialStr_LinkAux+strlen((char const*)DialStr_LinkAux), "%u\r\n", PORT); 
  
  
     #endif
         if(DebugOUT)
	 {
	   rt_kprintf("\r\n  ��IP   DialString : "); 
	   rt_kprintf((char*)DialStr_LinkAux);          
	 } 
}
void  DataLink_IC_Socket_set(u8 *IP, u16  PORT,u8 DebugOUT) 
{

   	  memset((char *)DialStr_IC_card+14,0,sizeof(DialStr_IC_card)-14);
	  IP_Str((char *)DialStr_IC_card+14, *( u32 * ) IP);   	  
	   
	  strcat((char *) DialStr_IC_card,"\"," );
	  sprintf((char*)DialStr_IC_card+strlen((char const*)DialStr_IC_card), "%u\r\n", PORT); 

         if(DebugOUT)
	 {
	     rt_kprintf("\r\n IC  DialString : "); 
	     rt_kprintf((char*)DialStr_IC_card);         
	 } 	 
		 
} 

void  DataLink_IspSocket_set(u8 *IP, u16  PORT,u8 DebugOUT) 
{
         u8 * regstr[30];
		 
               memset((char *)regstr,0,sizeof(regstr));
		  IP_Str((char *)regstr, *( u32 * ) IP);		 
		  
		  sprintf(( char*)regstr, IP);	 

		if(DebugOUT)
		    rt_kprintf("		  Aux Socket : %s\r\n",regstr);       
}

void  DataLink_APN_Set(u8* apn_str,u8 DebugOUT)
{
         memset(Dialinit_APN+APN_initSTR_LEN,0,sizeof(Dialinit_APN)-APN_initSTR_LEN);
	  memcpy(Dialinit_APN+APN_initSTR_LEN,apn_str,strlen((char const*)apn_str));
	  strcat( (char *)Dialinit_APN,"\"\r\n" );

	  if(DebugOUT)
	  {
	     rt_kprintf("\r\n APN ���� :  ");    
	     rt_kprintf((const char*)Dialinit_APN);      
	 }
}


void  DataLink_DNSR_Set(u8* Dns_str,u8 DebugOUT)
{
   #ifdef  MC8332_CDMA
      memset(Dialinit_DNSR+13,0,sizeof(Dialinit_DNSR)-13); 
	  memcpy(Dialinit_DNSR+13,Dns_str,strlen((char const*)Dns_str)); 
	  strcat( Dialinit_DNSR,"\r\n" );  
   #endif
	  	
	if(DebugOUT)
	{
		  rt_kprintf("\r\n		 ����1 ���� :	 "); 
		  rt_kprintf(Dialinit_DNSR);  
	}
}


void  DataLink_DNSR2_Set(u8* Dns_str,u8 DebugOUT)
{

	 #ifdef  MC8332_CDMA
      memset(Dialinit_DNSR2+13,0,sizeof(Dialinit_DNSR2)-13);  
	  memcpy(Dialinit_DNSR2+13,Dns_str,strlen((char const*)Dns_str)); 
	  strcat( Dialinit_DNSR2,"\r\n" );  
   #endif  
	  	
	if(DebugOUT)
	{
		  rt_kprintf("\r\n		Aux  �������� :	 "); 
		  rt_kprintf(Dialinit_DNSR2); 
	}

}

 void Gsm_RegisterInit(void)
{  
     //--------   Power  Related   ---------
     GSM_PWR.GSM_PowerEnable=1;  // ��ʼʹ��
     GSM_PWR.GSM_powerCounter=0;
     GSM_PWR.GSM_power_over=0;  

    // --------  IMSI  -------------------
    memset((u8*)&IMSIGet,0,sizeof(IMSIGet));       

    //---------- COMM AT ----------------
    memset((u8*)&CommAT,0,sizeof(CommAT)); 
   

   //----------- Data Dial ----------------
   memset((u8*)&DataDial,0,sizeof(DataDial)); 
   DataDial.start_dial_stateFLAG=1; //important 

  //---  Network Setting  Default ----
  DataLink_APN_Set(APN_String,1);   // apn 
  DataLink_DNSR_Set(DomainNameStr,0);    // DNSR  MG323  û�� 
  DataLink_DNSR2_Set(DomainNameStr_aux,0);  
  DataLink_MainSocket_set(RemoteIP_main, RemotePort_main,1); 
  
//  DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_aux,1);

}


/* write one character to serial, must not trigger interrupt */
 void rt_hw_gsm_putc(const char c)
{
	/*
		to be polite with serial console add a line feed
		to the carriage return character
	*/
	//if (c=='\n')rt_hw_gps_putc('\r');	  
	//USART_SendData(UART4,  c);  
	//while (!(UART4->SR & USART_FLAG_TXE));   
	//UART4->DR = (c & 0x1FF);   
       	USART_SendData( UART4, c );
		while( USART_GetFlagStatus( UART4, USART_FLAG_TC ) == RESET )  
		{
		}
	
}

void rt_hw_gsm_output(const char *str)
{
  //   u16  len=0;
	/* empty console output */
	//--------  add by  nathanlnw ---------
	
     while (*str) 
	{
		rt_hw_gsm_putc (*str++);
	}
	
	/* len=strlen(str);
       while( len )
	{
		USART_SendData( UART4, *str++ );	
		while (!(UART4->SR & USART_FLAG_TXE));  
		UART4->DR = (*str++ & 0x1FF);   
		len--;
	}  */
       //--------  add by  nathanlnw  --------	
}

void rt_hw_gsm_output_Data(u8 *Instr,u16 len)  
{
      unsigned int  infolen=0;

	  infolen=len;

	  //--------  add by  nathanlnw ---------
       while (infolen)
	{
		rt_hw_gsm_putc (*Instr++);
		infolen--;
	}
       //--------  add by  nathanlnw  --------	

}
void Dial_Stage(T_Dial_Stage  Stage)
{	// set the AT modem stage
	//if (DataDial.Dial_step == Stage) return;	// no change    
									//
	DataDial.Dial_step = Stage;				//
	DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;
	DataDial.Dial_step_Retry=0;
}
void  GSM_RxHandler(u8 data)
{
  rt_interrupt_enter( );   
   
#ifdef MC8332_CDMA
	   if((GSM_INT_BUFF.gsm_wr>9)&&(GSM_Rx_RawFlag==RX_IDLE))
	   {
			if(strncmp(GSM_INT_BUFF.gsm_content,"+ZIPRECV:",9)==0)
			{
				 GSM_Rx_RawFlag=RX_HEADright;
			}
			else
				 GSM_Rx_RawFlag=RX_HEADerror;
   
	   }
   
   
   if((GSM_Rx_RawFlag==RX_HEADright)&&( data==0x0D)&&(former_byte==0x7E ))
   {
	   GSM_Rx_RawFlag=RX_IDLE;
	  //-----------  7E 0D	  ����-----------------
	  GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;
	   if( GSM_INT_BUFF.gsm_wr < 1400 )
	   {				
		   rt_mq_send( &mq_GSM, (void*)&GSM_INT_BUFF, GSM_INT_BUFF.gsm_wr+ 2 ); 			   
	   }		   
	   
	   GSM_INT_BUFF.gsm_wr = 0; 
   }
   else
	  if( (data==0x0a)&&(former_byte==0x0d)&&(GSM_Rx_RawFlag!=RX_HEADright) ) /*����0d 0a ��û��RECV ��������*/
	   {   GSM_Rx_RawFlag=RX_IDLE;// clear
#endif	
	   GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;
	   if( GSM_INT_BUFF.gsm_wr < 1400 )
	   {				
		   rt_mq_send( &mq_GSM, (void*)&GSM_INT_BUFF, GSM_INT_BUFF.gsm_wr+ 2 ); 			   
	   }		   
	   
	   GSM_INT_BUFF.gsm_wr = 0;  
   }
   else
   {
	   GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;	  
	   if( GSM_INT_BUFF.gsm_wr == GSM_TYPEBUF_SIZE )
	   {
		   GSM_INT_BUFF.gsm_wr = 0;
	   }
	   GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr]=0;  
   }
   //---------------------------------
   former_byte = data;	 
  rt_interrupt_leave( );	 
        		  
}


void  GSM_Buffer_Read_Process(void)
{
     // char ch;  
//-----------------------------------------------------
	rt_err_t	res;

	{
		res = rt_mq_recv( &mq_GSM,(void*)&GSM_RX_BUFF, 1400, 3 ); //�ȴ�100ms,ʵ���Ͼ��Ǳ䳤����ʱ,�100ms
		if( res == RT_EOK )                                                     //�յ�һ������
		{
				GSM_Process(GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr); 
		}
	}

}
//------------------------  ASCII    HEX  convert   ---------------------
u8  HexValue (u8 inchar) 
{
     switch(inchar)
     	{     	    
		   case '0':
                       return  0;
		   case '1': 
		   	           return  1;
		   case '2':
		   	           return  2;
	         case '3':
		   	           return  3;
		   case '4':
		   	           return  4;
		   case '5':    
		   	           return 5;
		   case '6':   
		   	           return 6;
		   case '7':
		   	            return 7;
		   case '8':     
		   	            return 8;
		   case '9':   
		   	           return 9;
		   case 'A':
		   	            return 0x0A;
		   case 'B':   

		   	            return 0x0B;
		   case 'C':
		   	            return 0x0C;
		   case 'D':    
		   	             return 0x0D;
		   case 'E': 
		   	            return 0x0E;
		   case 'F':    
		   	             return 0x0F;
		   default :
		   	           rt_kprintf("\r\n ת���д���:%c \r\n",inchar);    
				    return  0xFF;
     	}
}
//------------------------------- HEX to ASCII --------------------------------------
u32 GSM_HextoAscii_Convert(u8*SourceHex,u16 SouceHexlen,u8 *Dest)   
{
  u16 len_counter=0;
  u16 dest_counter=0;  
  u8  c=0;

  for(len_counter=0;len_counter<SouceHexlen;len_counter++)   
  {
     c=SourceHex[len_counter];

	//---------------- High -------------------------------- 
    if((c>>4)>=10)
       Dest[dest_counter++]='A'+(c>>4)-10;
	else
	   Dest[dest_counter++]='0'+(c>>4);
	
    Dest[dest_counter]=0x00;

	//----------------- Low --------------------------------
	c=c&0x0F;
    if(c>=10)  
       Dest[dest_counter++]='A'+c-10;
	else 
	   Dest[dest_counter++]='0'+c; 
	Dest[dest_counter]=0x00;
  }
  
 return dest_counter; 
}
 u16  GSM_AsciitoHEX_Convert(u8 *Src_str,u16 Src_infolen,u8* Out_Str) 
 {
   u16 Counter=0,Out_Str_Len=0;
   u8  C=0;
	 
   Out_Str_Len=0;

     if((u8)Src_infolen&0x01)
   	{
   	     rt_kprintf("\r\n      ����ASCII��Ϣ����ȷ!   %u  \r\n",Src_infolen);     
   	}
   	
   for(Counter=0;Counter<Src_infolen;Counter++) 
   {   
        //--------------------------------------------------
           if((Counter&0x01)==1)
           	{
	           	 C=HexValue(*(Src_str+Counter));
			 if(C!=0xff)		 
	              {  
	                 Out_Str[Out_Str_Len]+=C;
	                 Out_Str_Len++; 
			  }	
			  else 
			  	  rt_kprintf("\r\n    Convert Error at:   %u  \r\n",Counter);    
           	}
	   else
		{
		     C=((HexValue(*(Src_str+Counter)))<<4);
                      Out_Str[Out_Str_Len]=(HexValue(*(Src_str+Counter)))<<4;	
		      if(C!=0xff)		 
	               {  
	                 Out_Str[Out_Str_Len]=C;
			  }	
			  else 
			  	  rt_kprintf("\r\n    Convert Error at2:   %u  \r\n",Counter);    			  
	   	}

   } 
	return Out_Str_Len;

 } 



void  Data_Send(u8* DataStr, u16  Datalen,u8  Link_Num)
{
    u16  i=0,packet_len=0;
     //  4. Send  


    //  4.1  ����Ҫ���͵���Ϣ����  
	  memset(GSM_AsciiTx,0,sizeof(GSM_AsciiTx));	  
	 if(Link_Num==0)
	 {
	     strcat((char*)GSM_AsciiTx,Send_TCP);        // head
	     packet_len=strlen((const char*)Send_TCP);  
	 }
	 else
	 {
	     strcat((char*)GSM_AsciiTx,Send_ISP);        // head
	     packet_len=strlen((const char*)Send_ISP);   
	 }

      #ifdef MC8332_CDMA
          sprintf(GSM_AsciiTx+packet_len,"%d\r",Datalen);
          packet_len=strlen((const char*)GSM_AsciiTx);  
		  
		  if((Photo_sdState.photo_sending==0)&&(DispContent))   // ����ʱ����������Ϣ 
		  {
		     rt_kprintf("%s",GSM_AsciiTx);
		  }

          // stuff  orignal  infor
          for(i=0;i<Datalen;i++)
          	{
               GSM_AsciiTx[packet_len+i]=DataStr[i]; 
          	}
		  
		  if((Photo_sdState.photo_sending==0)&&(DispContent))  // ����ʱ����������Ϣ 
		  {
			 for(i=0;i<Datalen;i++)
				  rt_kprintf("%02X",GSM_AsciiTx[i+packet_len]);  	   
		  } 
		     packet_len+=Datalen;
			 rt_hw_gsm_output_Data(GSM_AsciiTx,packet_len);  
		 #endif 
        WatchDog_Feed();
        rt_thread_delay(RT_TICK_PER_SECOND/10);//DF_delay_ms(100);    

}

void End_Datalink(void)
{
       if(1==DataLink_EndFlag)          
	{
	   TCP2_Connect=0;
	   DataLink_Online=disable;  // off line

	  DataLink_EndCounter++;
	  if(DataLink_EndCounter==2) 
	  { 	//rt_kprintf(" Ready to escape gprs \r\n");	 
		rt_hw_gsm_output(CutDataLnk_str2);
		rt_kprintf(CutDataLnk_str2);
		DataDial.Dial_step_Retry=0;
		DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;
	  }
	  else
	  if(DataLink_EndCounter==3)
	  { 		
		rt_hw_gsm_output(CutDataLnk_str3);	// �رյ�Internet
	       rt_kprintf(CutDataLnk_str3);	// �رյ�Internet
	       DataDial.Dial_step_Retry=0;
		DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;

	  }    
	  else
	  if(DataLink_EndCounter==5)
	  { 		
		rt_hw_gsm_output(CutDataLnk_str4);	 
		rt_kprintf(CutDataLnk_str4);	 
		DataDial.Dial_step_Retry=0;
		DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;
	  }
	    if(DataLink_EndCounter==8)
	  { 		
		rt_hw_gsm_output(CutDataLnk_str5);	 
		rt_kprintf(CutDataLnk_str5);   
		DataDial.Dial_step_Retry=0;
		DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;
	  }
	  else
	  if(DataLink_EndCounter>10)   
	  {
		DataLink_EndCounter=0; 
		DataLink_EndFlag=0;
		//-----------------------------
               CommAT.Initial_step=0;  
		 CommAT.Total_initial=0;  
               Redial_Init(); 
		 ModuleStatus &=~Status_GPRS;
	        rt_kprintf(" Redial Start\r\n");   
	      //-----------------------------
	  }
    }     

}

void  ISP_Timer(void)
{
    if((BD_ISP.ISP_running==1)&&DataLink_Status())     
    {
       BD_ISP.ISP_runTimer++; 
	   if(BD_ISP.ISP_runTimer>350)  
	   	{
            BD_ISP.ISP_runTimer=0;
			BD_ISP.ISP_running=0;//  clear
			SD_ACKflag.f_BD_ISPResualt_0108H=2;   
		    rt_kprintf("\r\n �����·���ʱ������ʧ��\r\n");    

		}
    } 

}
 
u8  GPRS_GSM_PowerON(void)
{	
	/*
              EM310���ػ�����
              powerkey ���� 50ms ����  Ȼ��������50ms�ػ�
	*/
      if(GSM_PWR.GSM_power_over) 
			  return 1;
	
         GSM_PWR.GSM_powerCounter+=50;   

  #ifdef   MC8332_CDMA
						if((GSM_PWR.GSM_powerCounter>=10)&&(GSM_PWR.GSM_powerCounter<300))
					 {
						GPIO_ResetBits(GPIOD,GPRS_GSM_Power);	 // �ص�
						GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY); //  �õͿظ�
						GPIO_SetBits(GPIOD,GPRS_GSM_RST);	// ��̬�ø߿ص�  
						rt_kprintf(" cstep 1\r\n");   
					 }	
					 if((GSM_PWR.GSM_powerCounter>=300)&&(GSM_PWR.GSM_powerCounter<400))
					 {	 /* ����Դ��rst    �� ����һ��ʱ���ȶ�*/
						  GPIO_SetBits(GPIOD,GPRS_GSM_Power);	 //  ����
						  GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY); //	�õͿظ�
						  rt_kprintf(" cstep 2\r\n");  
						  delay_ms(50);
						  GPIO_ResetBits(GPIOD,GPRS_GSM_RST);	// 20ms ���õͿظ�
						 
					 }	
					 if((GSM_PWR.GSM_powerCounter>=400)&&(GSM_PWR.GSM_powerCounter<600))   
					 {												   // ��GPS �ϵ����		  
						GPIO_SetBits(GPIOD,GPRS_GSM_Power);    //  ����
						GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);	 //  PWK ��
						rt_kprintf(" cstep 3\r\n"); 				
						gps_onoff(1);  // Gps module Power on	GPS ģ�鿪��   
					 }	
					 if((GSM_PWR.GSM_powerCounter>=600)&&(GSM_PWR.GSM_powerCounter<800))   
					 {													 //   ����2 s	 ׼������
						GPIO_SetBits(GPIOD,GPRS_GSM_PWKEY);   //  PWK��
						 rt_kprintf(" cstep 4 \r\n");	  
					 }	 
					 if((GSM_PWR.GSM_powerCounter>=800)&&(GSM_PWR.GSM_powerCounter<900))	  
					 {
						GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);	   //  PWK ��	
						rt_kprintf(" cstep 5\r\n");    
					 }	
					 if(GSM_PWR.GSM_powerCounter>=1000) 		
					 {
						rt_kprintf("		 %s","--GPRS Power over\r\n "); 	
						GSM_PWR.GSM_PowerEnable=0; 
						GSM_PWR.GSM_powerCounter=0;
						GSM_PWR.GSM_power_over=1;  
						IMSIGet.Get_state=1;		 //  cdma  û��AT%TSIM 
						
						CSQ_Duration=200;  // ��ѯ����ӳ�	 
						   //-------add for re g 
					 }	
#endif


#ifdef M66_GSM
		 
               	if((GSM_PWR.GSM_powerCounter>=10)&&(GSM_PWR.GSM_powerCounter<300))
			 {
			    GPIO_ResetBits(GPIOD,GPRS_GSM_Power);    // �ص�
			    GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);      //  PWK �� 
			    rt_kprintf(" step 1\r\n");   
			 }	
			 if((GSM_PWR.GSM_powerCounter>=300)&&(GSM_PWR.GSM_powerCounter<400))
			 {
			      GPIO_SetBits(GPIOD,GPRS_GSM_Power);    //  ����
			      GPIO_SetBits(GPIOD,GPRS_GSM_PWKEY);   //  PWK��  
				  rt_kprintf(" step 2\r\n");   
				 
			 }	
			 if((GSM_PWR.GSM_powerCounter>=400)&&(GSM_PWR.GSM_powerCounter<700)) 
			 {
			    GPIO_SetBits(GPIOD,GPRS_GSM_Power);    //  ����
			    GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);    //  PWK ��
			    rt_kprintf(" step 3\r\n");   				
				gps_onoff(1);  // Gps module Power on	GPS ģ�鿪��   
			 }	
			 if((GSM_PWR.GSM_powerCounter>=700)&&(GSM_PWR.GSM_powerCounter<900))   
			 {
				GPIO_SetBits(GPIOD,GPRS_GSM_PWKEY);   //  PWK��
				 rt_kprintf(" step 4\r\n");     
			 }	 
			 if((GSM_PWR.GSM_powerCounter>=900)&&(GSM_PWR.GSM_powerCounter<1100))      
			 {
			    GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);      //  PWK ��   
				rt_kprintf(" step 5\r\n");    
			 }	
			 if(GSM_PWR.GSM_powerCounter>=1200)        
			 {
				rt_kprintf("         %s","--GPRS Power over\r\n ");     
				GSM_PWR.GSM_PowerEnable=0; 
				GSM_PWR.GSM_powerCounter=0;
				GSM_PWR.GSM_power_over=1;  
				CSQ_Duration=200;  // ��ѯ����ӳ�   
				   //-------add for re g 
			 }	   
  #endif        
  
	
	 return   0;	    
}

void GSM_Module_TotalInitial(void)
{
    
    //----------  Total_initial ------------
   if(CommAT.Total_initial==1)
   {
	 if( CommAT.Execute_enable)
	 {
	   switch(CommAT.Initial_step)
		   {
			 case 0:
					 rt_hw_gsm_output(CommAT_Str1);  
					 if(DispContent)
					    rt_kprintf(CommAT_Str1);  
					 CommAT.Initial_step++;
					 break;
			 case 1:
					 rt_hw_gsm_output(CommAT_Str2);
					 if(DispContent)
					    rt_kprintf(CommAT_Str2); 
					 CommAT.Initial_step++;
					 break;
   
			 case 2:/* Query Operator */
					 rt_hw_gsm_output(CommAT_Str3);  //"AT+COPS?\r\n"
					 if(DispContent)
					 rt_kprintf(CommAT_Str3);  
					 #ifdef MC8332_CDMA
					     CommAT.Initial_step++;  
					 #endif
					 break;
			 case 3:
					 rt_hw_gsm_output(CommAT_Str4);  
					 if(DispContent)
					 rt_kprintf(CommAT_Str4);	
					 CommAT.Initial_step++;
					 break;
			 case 4:
					 rt_hw_gsm_output(CommAT_Str5);
					 if(DispContent)
					 rt_kprintf(CommAT_Str5);   
					 CommAT.Initial_step++;
					 break;
			 case 5:
					 rt_hw_gsm_output(CommAT_Str8);     
					 if(DispContent)
					 rt_kprintf(CommAT_Str8); 
					 CommAT.Initial_step++; 
					 break;
			 case 6:
					 rt_hw_gsm_output(CommAT_Str7);  
					 if(DispContent)
					 rt_kprintf(CommAT_Str7); 
					 CommAT.Initial_step++;
					 break;
			 case 7:	
					 rt_hw_gsm_output(CommAT_Str11); 
					 if(DispContent)
					 rt_kprintf(CommAT_Str11);  
					 CommAT.Initial_step++;
					 break;		
			 case 8:
					 rt_hw_gsm_output(CommAT_Str9);  
					  if(DispContent)
					 rt_kprintf(CommAT_Str9); 
					 CommAT.Initial_step++; 
					 break;	 		 
			 case 9:
					 rt_hw_gsm_output(CommAT_Str10);   
					 if(DispContent)
					 rt_kprintf(CommAT_Str10); 
			 
					 CommAT.Initial_step++;
					 break; 	
			 case 10:
			 	    #ifdef MC8332_CDMA
                     rt_hw_gsm_output("AT+VGR?\r\n");    //  ��������¼������
					 if(DispContent)
					 rt_kprintf("AT+VGR?\r\n");  
					#endif 
					 CommAT.Initial_step++;
					 break;		 
             case 11: 
					 rt_hw_gsm_output(CommAT_Str12);  
					 if(DispContent)
					 rt_kprintf(CommAT_Str12);
					 CommAT.Initial_step++;
					 break; 
			 case 12:
					 rt_hw_gsm_output(CommAT_Str13);
					 if(DispContent)
					 rt_kprintf(CommAT_Str13); 
					 CommAT.Initial_step++;
					 break; 		 
			 case 13:
					 rt_hw_gsm_output(CommAT_Str14);
					 if(DispContent)
					 rt_kprintf(CommAT_Str14);  
					 CommAT.Initial_step++;
					 break; 	
			 case 14:
					 rt_hw_gsm_output(CommAT_Str15); 
					  if(DispContent)
					 rt_kprintf(CommAT_Str15);   
					 CommAT.Initial_step++;
					 break; 	
		        case 15:
					 rt_hw_gsm_output(CommAT_Str16); 
					  if(DispContent)
					 rt_kprintf(CommAT_Str16);   
					 CommAT.Initial_step++;
					 break; 
			 case 16://  CCID /		
			         rt_hw_gsm_output(CommAT_Str17); 
					  if(DispContent)
					 rt_kprintf(CommAT_Str17);
					 
					 if(Login_Menu_Flag==1) 	  // �ȳ�ʼ��Ȼ����ִ�в��š���֤��ϢOK
						CommAT.Initial_step++;  
					 
					 break;		 
			 case 17://  �ź�ǿ�� /		
			         rt_hw_gsm_output(Signal_Intensity_str); 
					  if(DispContent)
					 rt_kprintf(Signal_Intensity_str);    
					 break;					 
	         case 18:/*��ʼ�ܲ���*/ 
					 rt_kprintf("AT_Start\r\n");   
					 CommAT.Initial_step=0; 
					 CommAT.Total_initial=0;   

					 DataDial.Dial_ON=enable;  //  ����  Data   ״̬
					 DataDial.Pre_Dial_flag=1;    // Convert to  DataDial State
					 Dial_Stage(Dial_DialInit0);     		  			 
					 break; 				 
			 default:					  
					break;
	 
		   }  
	    CommAT.Execute_enable=0;
	 }
   } 

}

void Redial_Init(void)
{
        //----------- Data Dial ----------------
   memset((u8*)&DataDial,0,sizeof(DataDial)); 
   DataDial.start_dial_stateFLAG=1; //important 
   DataDial.Dial_ON=enable;  //  ����  Data   ״̬
   DataDial.Pre_Dial_flag=1;    // Convert to  DataDial State
   Dial_Stage(Dial_DialInit0);
}

void  Dial_step_Single_10ms_timer(void)  
{
      //    û������״̬�£��� �ɹ���½�Ϻ󲻽��д���
     	if( (DataDial.Dial_ON==0)||(DataDial.Dial_step==Dial_Idle) )
		   return;				
    
		if (DataDial.Dial_step_RetryTimer>= 4)
			DataDial.Dial_step_RetryTimer -= 4; 
		else
			DataDial.Dial_step_RetryTimer = 0;
}

void  Get_GSM_HexData(u8*  Src_str,u16 Src_infolen,u8 link_num)
{
     u16   i=0;
     //  1.  Check wether   Instr   is  need   to  convert     Exam:  ASCII_2_HEX	 
     #ifdef MC8332_CDMA
              for(i=0;i<Src_infolen;i++)
	              GSM_HEX[i]=Src_str[i]; 
              GSM_HEX_len=Src_infolen;      
	 #endif
     //  2 .  Realse   sem
	 Receive_DataFlag=1;  	
} 

void DataLink_Process(void)
{
   u8  len=0,i=0; 
   
  	// state  filter  1:   û������״̬�£��� �ɹ���½�Ϻ󲻽��д���
     	if( (DataDial.Dial_ON==0)||(DataDial.Dial_step==Dial_Idle) ) 
		    return;				
	// state  filter  2:  	������ʱ��û�е�
       if (DataDial.Dial_step_RetryTimer ) 
	   	    return;						   // not time to retry
	// state  filter  3:      	    
        if (DataDial.Dial_step_Retry>= Dial_Step_MAXRetries)		
        {														  
			   //---------------------------------------			  
			   DataDial.Connect_counter++;
			   if( DataDial.Connect_counter>4)     
				   {
					   DataDial.Pre_Dial_flag=1; 	  //--- ���²���	
					   DataDial.Pre_dial_counter=0;
					   rt_kprintf("\r\n  RetryDialcounter>=4 ���²���\r\n");    
				   }	 
			   Dial_Stage(Dial_DialInit0);    // Clear   from  Dial  start 	
			   DataDial.Dial_step_Retry=0;
			   DataDial.Dial_step_RetryTimer=0; 
			   rt_kprintf("\r\nDataDial.Dial_step_Retry>= Dial_Step_MAXRetries ,redial \r\n");
			   return;													   
	  } 
	  //----------  ����������2�� ʧ���л���������	
        if((DataDial.Dial_step==Dial_MainLnk)&&(DataDial.Dial_step_Retry>2))
         {
              i=0;
              if(Dnsr_state)
              {
                   Dial_Stage(Dial_MainLnk);    // �����DNSR ������ô����mainlink
              }
		else
		{
                    // rt_thread_delay(10);
			rt_kprintf("Dial_MainLnk Retry>2 AT%IPCLOSE=1\r\n");		   
		       rt_hw_gsm_output("AT%IPCLOSE=1\r\n");    	
		       WatchDog_Feed();   
			delay_ms(100);//rt_thread_delay(10);
                     Dial_Stage(Dial_AuxLnk);    
			DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_main,0);	 
		 
		}			 
         }
		
		

	 //  work  on 
       if(DataDial.Dial_ON)
       {
             switch(DataDial.Dial_step)
             	{
                   case Dial_DialInit0: 
				   	 rt_hw_gsm_output(DialInit1);
			               //-----------------------------------------
                                      DataDial.start_dial_stateFLAG=1;   
				         //-----------------------------------------
				         DataDial.Dial_step_RetryTimer=Dial_max_Timeout;    
					  DataDial.Dial_step_Retry++;
					   if(DispContent)
 					     rt_kprintf(DialInit1);   
           
					break;
		      case Dial_DialInit1:   
			  	    #ifdef MC8332_CDMA
                        rt_hw_gsm_output("AT\r\n");  
					#endif
                    DataDial.Dial_step_RetryTimer=Dial_max_Timeout; 
					DataDial.Dial_step_Retry++;

					#ifndef MC8332_CDMA 
					if(DispContent)
					 {
					   len=strlen((const char*)Dialinit_APN); 
					   for(i=0;i<len;i++)										
					         rt_kprintf("%c",Dialinit_APN[i]);     
					 } 
					#endif

				       break;			
		     case Dial_DialInit2:     
			 	    rt_hw_gsm_output(DialInit2);
                    DataDial.Dial_step_RetryTimer=Dial_max_Timeout;  
					DataDial.Dial_step_Retry++;

                     if(DispContent)
					{
					 len=strlen((const char*)DialInit2);
					for(i=0;i<len;i++)										
					         rt_kprintf("%c",DialInit2[i]);   
					}

				       break;
		     case  Dial_DialInit3:  
			 	       rt_hw_gsm_output(DialInit3);
                                   DataDial.Dial_step_RetryTimer=Dial_max_Timeout; 
					DataDial.Dial_step_Retry++;
					 if(DispContent)
					 	{
							 len=strlen((const char*)DialInit3);
							for(i=0;i<len;i++)										
							         rt_kprintf("%c",DialInit3[i]);      
					 	}
				       break;			   
		     case Dial_DialInit4:    rt_hw_gsm_output(DialInit6); // AT
                                   DataDial.Dial_step_RetryTimer=Dial_max_Timeout;
					DataDial.Dial_step_Retry++;
					 if(DispContent)
					 {
						 len=strlen((const char*)DialInit6);
						for(i=0;i<len;i++)										
						         rt_kprintf("%c",DialInit6[i]);  
					  }
					break;
		     
		    case Dial_DialInit5:    rt_hw_gsm_output(DialInit4);
                                   DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time; 
					DataDial.Dial_step_Retry++;
					 if(DispContent)
					{
						len=strlen((const char*)DialInit4);
						for(i=0;i<len;i++)										
						         rt_kprintf("%c",DialInit4[i]);       
					 }
				       break;	
		    case Dial_DialInit6:    rt_hw_gsm_output(DialInit8);  // Query  IP 
                                   DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time; 
					DataDial.Dial_step_Retry++;
					 if(DispContent)
				    {
						 len=strlen((const char*)DialInit8);
						for(i=0;i<len;i++)										
						         rt_kprintf("%c",DialInit8[i]);     
					}

				       break;	 	
		    case Dial_DNSR1:    rt_hw_gsm_output(Dialinit_DNSR);  // main DNSR
                                   DataDial.Dial_step_RetryTimer=Dial_max_Timeout; 
					DataDial.Dial_step_Retry++;
					rt_kprintf("\r\n   ----- Link  main  DNSR ----\r\n");
					if(DispContent)
				   {
					 len=strlen((const char*)Dialinit_DNSR);
					for(i=0;i<len;i++)										
					         rt_kprintf("%c",Dialinit_DNSR[i]);    
				   }
				   else
				   	   rt_kprintf("\r\n        DNSR:%s\r\n",DomainNameStr);
                                   Dial_jump_State=7;
					Dnsr_state=1;//��ʾDNSR ״̬�²��� 				   
				       break;	 
            case Dial_DNSR2:    rt_hw_gsm_output(Dialinit_DNSR2);  // Aux DNSR
                                   DataDial.Dial_step_RetryTimer=Dial_max_Timeout; 
					DataDial.Dial_step_Retry++;
					rt_kprintf("\r\n   ----- Link  Aux  DNSR ----\r\n");
				   if(DispContent)
				   {
					 len=strlen((const char*)Dialinit_DNSR2);
					for(i=0;i<len;i++)										
					         rt_kprintf("%c",Dialinit_DNSR2[i]);  
				   	}
				   else
				       rt_kprintf("\r\n       DNSR:%s\r\n",DomainNameStr_aux);
                                   Dial_jump_State=8; 
					Dnsr_state=1;//��ʾDNSR ״̬�²��� 				   
				       break;	 					   
		    case Dial_DialInit7:    rt_hw_gsm_output(DialInit8);  // Query  IP 
                                   DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time; 
					DataDial.Dial_step_Retry++;
					  if(DispContent)
				      {
					 len=strlen((const char*)DialInit8);
					for(i=0;i<len;i++)										
					         rt_kprintf("%c",DialInit8[i]);    
					 }

				       break;	   
		     case Dial_MainLnk:   // rt_hw_gsm_output(DialStr_Link1); 
		                       if( Dnsr_state==0)//��ʾDNSR ״̬�²��� 	 
		                       	{
		                       	   rt_kprintf("\r\n Dial  orginal   Mainlink\r\n");  
                                            DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,0);
		                       	}
							   
                                   DataDial.Dial_step_RetryTimer=3000; 
					DataDial.Dial_step_Retry++;
					 len=strlen((const char*)DialStr_Link1); 
					 for(i=0;i<len;i++)										
					   {
					        rt_hw_gsm_putc (DialStr_Link1[i]);     
					   }	
					for(i=14;i<len;i++)										
					   {
					        rt_kprintf("%c",DialStr_Link1[i]);     
					   }		
					break; 
		      case Dial_AuxLnk:   // rt_hw_gsm_output(DialStr_IC_card); 
                                   DataDial.Dial_step_RetryTimer=3000;
					DataDial.Dial_step_Retry++;
					 len=strlen((const char*)DialStr_LinkAux); 
					 for(i=0;i<len;i++)										
					   {
					        rt_hw_gsm_putc (DialStr_LinkAux[i]);     
					   }	
					  if(DispContent)
				      { 
						for(i=0;i<len;i++)										
						   {
						        rt_kprintf("%c",DialStr_LinkAux[i]);     
						   }		
					  }
					break; 	
 		     case   Dial_ISP:
 				    DataDial.Dial_step_RetryTimer=Dial_max_Timeout;
					DataDial.Dial_step_Retry++;
					 len=strlen((const char*)DialStr_IC_card); 
					 for(i=0;i<len;i++)										
					   {
					        rt_hw_gsm_putc (DialStr_IC_card[i]);     
					   }	
					 rt_kprintf("\r\n	----- Link	IC ----\r\n");		
					   if(DispContent)
				       {
						  for(i=0;i<len;i++)										
						   {
						        rt_kprintf("%c",DialStr_IC_card[i]);     
						   }	
					   	}
					break;				
		      default:
			  	       break;
             	}

       }  



}

static void GSM_Process(u8 *instr, u16 len)
{
      	u8	ok = false;
		u8	error = false;
		u8	failed = false; 
		u8	connect = false;   		
        u16  i=0,j=0,q=0;//,len=0;//j=0;
         u8 reg_str[80]; 
//----------------------  Debug -------------------------		
  // if(DispContent==2)	 
   memset(GSM_rx,0,sizeof((const char*)GSM_rx));
   memcpy(GSM_rx,instr,len);
   
  if((BD_ISP.ISP_running==0)&&(DispContent))      
   {
       rt_kprintf("\r\n");      
        for(i=0;i<len;i++)  
		{ 
		   rt_kprintf("%c",GSM_rx[i]);  
        }
   }    

   //------------------------------------------------------------------------------------------------------------------- 
  #ifdef MC8332_CDMA
      if(strncmp((char*)GSM_rx, "+ZTTS:0",7)==0)  // MC8332
  #endif 	
	{
                 TTS_Play_End();
		    rt_kprintf("\r\n   TTS  �������\r\n");   
			Speak_OFF;
	}
#ifdef  SMS_ENABLE
	//--------      SMS  service  related Start  -------------------------------------------
	//+CMTI: "SM",1            +CMTI: "SM",1  +CMTI: "MT",0
	if( strncmp( (char*)GSM_rx, "+CMTI: \"SM\",", 12 ) == 0 )
	{
		rt_kprintf( "\r\n�յ�����:" );
		j = sscanf( GSM_rx + 12, "%d", &i );
		if( j )
		{
			SMS_Rx_Notice(i);	
		}
	}
	else if( strncmp( (char*)GSM_rx, "+CMT: ", 6 ) == 0 )
	{
		if(GSM_rx[6]==',')		///PDUģʽ
		{
			if( RT_EOK == rt_mq_recv( &mq_GSM, (void*)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )    //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
			{
				memset( GSM_rx, 0, sizeof( GSM_rx ) );
				memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
				len=GSM_RX_BUFF.gsm_wr;

				SMS_Rx_PDU(GSM_rx,len); 
			}
		}
		else					///TEXTģʽ
		{
			j	= 0;
			q	= 0;
			memset( reg_str, 0, sizeof( reg_str ) );
			for( i = 6; i < len; i++ )
			{
				if( ( j == 1 ) && ( GSM_rx[i] != '"' ) )
				{
					reg_str[q++] = GSM_rx[i];
				}
				if( GSM_rx[i] == '"' )
				{
					j++;
					if(j>1)
						break;
				}
			}
			if( RT_EOK == rt_mq_recv( &mq_GSM, (void*)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )    //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
			{
				memset( GSM_rx, 0, sizeof( GSM_rx ) );
				memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
				len=GSM_RX_BUFF.gsm_wr;
				SMS_Rx_Text(GSM_rx,reg_str);
			}
		}
		
	}
#ifdef SMS_TYPE_PDU
	else if( strncmp( (char*)GSM_rx, "+CMGR:", 6 ) == 0 )
	{
		if( RT_EOK == rt_mq_recv( &mq_GSM, (void*)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )    //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
		{
			memset( GSM_rx, 0, sizeof( GSM_rx ) );
			memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
			len=GSM_RX_BUFF.gsm_wr;

			SMS_Rx_PDU(GSM_rx,len);
		}
	}
#else
	else if( strncmp( (char*)GSM_rx, "+CMGR:", 6 ) == 0 )
	{
		//+CMGR: "REC UNREAD","8613602069191", ,"13/05/16,13:05:29+35"
		// ��ȡҪ���ض�Ϣ��Ŀ�ĺ���
		j	= 0;
		q	= 0;
		memset( reg_str, 0, sizeof( reg_str ) );
		for( i = 6; i < 50; i++ )
		{
			if( ( j == 3 ) && ( GSM_rx[i] != '"' ) )
			{
				reg_str[q++] = GSM_rx[i];
			}
			if( GSM_rx[i] == '"' )
			{
				j++;
			}
		}
    	if( RT_EOK == rt_mq_recv( &mq_GSM, (void*)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )    //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
		{
			memset( GSM_rx, 0, sizeof( GSM_rx ) );
			memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
			len=GSM_RX_BUFF.gsm_wr;
			SMS_Rx_Text(GSM_rx,reg_str);		
		}
	}
#endif

#endif
	
    #ifdef MC8332_CDMA  
     //+CMTI: "MT",0
    //+CMTI: "SM",1            +CMTI: "SM",1  +CMTI: "MT",0
	if( strncmp( (char*)GSM_rx, "+CMTI: \"MT\",", 12 ) == 0 )
	{
		rt_kprintf( "\r\n�յ�cdma����:" );
		j = sscanf( GSM_rx + 12, "%d", &i ); 
		if( j )
		{
			SMS_Rx_Notice(i);	 
		}
	}
	else
    if(strncmp((char*)GSM_rx, "+CIMI: 460",10) == 0)	 // +CIMI: 460036871693080
	{
	      CommAT.Total_initial=1;	// ���в�������
		  rt_kprintf("\r\n��ȡIMSI ����:%s \r\n",GSM_rx+7);       
		  memcpy((char*)IMSI_CODE,(char*)GSM_rx+7,15); 
		  RTC_TimeShow();
		  IMSIGet.imsi_error_count=0;	
		  IMSIGet.Get_state=1;
		  GSM_PWR.GSM_power_over=2;     //  get imsi
		  IMSIGet.Get_state=0;
		  CommAT.Total_initial=1;	// ���в�������  
		 
	}
	else
    if(strncmp((char*)GSM_rx, "+GETICCID:0x",12) == 0)	 // +GETICCID:0x89860312908712564960
	{
	        for(i=0;i<10;i++)
           {
              ProductAttribute._5_Sim_ICCID[ok]=((GSM_rx[12+2*i]-0x30)<<4)+(GSM_rx[12+2*i]-0x30);
              ok++;
           }
		 
	}
	else

    if(strncmp((char*)GSM_rx, "+ZDNSGETIP: failed",18) == 0)     //+ZDNSGETIP: failed
	{
	         rt_kprintf("\r\n   ��������ʧ��\r\n");    
	 		 if((Dial_jump_State==7)||(Dial_jump_State==8))
				Dial_jump_State=0;    
		        Dnsr_state=0;//��ʾDNSR ״̬�²��� 			  

	}
	else      //+ZDNSGETIP:113.31.28.101
    if(strncmp((char*)GSM_rx, "+ZDNSGETIP:",11) == 0)
	{  //%DNSR:113.31.28.100
		//%DNSR:0.0.0.0 
	        {                   
	                    
				i =str2ip((char*)GSM_rx+11, RemoteIP_Dnsr); 
				if (i <= 3) failed = true; 
			    DataLink_MainSocket_set(RemoteIP_Dnsr,RemotePort_main,1);
                rt_kprintf("\r\n   ���������ɹ�\r\n"); 

				Dial_Stage(Dial_MainLnk);  //--   �л�������    
				return ;
		}
    }
	   //+ZNEWIP:42.80.43.113
	   // +ZPPPSTATUS: OPENED
       //============================================================================   
       /*
                +ZTCPESTABLISHED:0
                +ZDORMANT: READY             
	    */   
	   else
	   if(strncmp((char*)GSM_rx, "+ZTCPESTABLISHED:0",18) == 0)    
	   	{
                connect=true;
		}
       //-----------------------------------------------------------------------------------------
        //  +ZIPSEND: 60    �������  
        else
        if(strncmp((char*)GSM_rx, "+ZIPSEND:",9) == 0)    
        {
            if(Send_Rdy4ok==1)
		   	{
		   	    Api_cycle_Update();
			    Send_Rdy4ok=0;	
				Online_error_counter=0; // clear 
		   	}
		    WatchDog_Feed();      
        }
	   //-----------------------------------------
       // +ZIPRECV:0,32,~                  +ZIPRECV:0,
      else 
	  if(strncmp((char*)GSM_rx, "+ZIPRECV:0,",11) == 0)    
	  	{
           for(i=0;i<20;i++)   //  ��ǰ20���ֽ����ҵ�һ��"
			{
				if(GSM_rx[i+11]==',') 
					break;
			} 
		  info_len=len-i-5;  	  
		
			sscanf(GSM_rx+11, "%d", (u32*)&info_len); 	
		   // rt_kprintf("\r\n rxlen=%d\r\n",info_len);      
			WatchDog_Feed();
		  // if(BD_ISP.ISP_running==1)  
			//  OutPrint_HEX("rx",GSM_rx+i+12,info_len);   
			Get_GSM_HexData(GSM_rx+i+12,info_len,0);     
	         goto RXOVER; 	 

	  	}
	  else
	    if(strncmp((char*)GSM_rx, "+CME ERROR: 3",13) == 0)  
	    {
	        if(DataDial.Dial_step!=Dial_DialInit1)
	        {
               if(DataLink_Status())        //   Online  state  Error    , End Link and Redial    
			   {
		              Online_error_counter++;
					  if(Online_error_counter>3)	 
				         {   
				                DataLink_EndFlag=1; //AT_End();    
		                        Online_error_counter=0;
					  }
				      rt_kprintf("\r\n C Online error\r\n"); 	       
			   	}
			   rt_kprintf("\r\n  C ERROR\r\n");  
	         }   
	    }
     else
	    if(strncmp((char*)GSM_rx, "+ZPPPSTATUS: OPENED",19) == 0)   
        {              
			  connect=true;
	    }

		
	    if(strncmp((char*)GSM_rx, "+ZCEND:",7) == 0)    //+ZCEND:28
	    {
             Speak_OFF; //�رչ���
	    }

		//+VGR: 1
		else
		if( strncmp( (char*)GSM_rx, "+VGR:", 5 ) == 0 )
	    {
			
			j = sscanf( GSM_rx + 6, "%d", &i );
			rt_kprintf( "\r\n��ȡ����: %d ",j );
			if( i<=2)
			 {	 
			    Menu_voice_value=0;// ����С
				 rt_kprintf( " �ȼ�  1\r\n");
			  }
			else
			if(i<=4)
			{
				Menu_voice_value=1;// ������
                 rt_kprintf( " �ȼ�  2\r\n");
			}
			else
			if(i<=7)	
			{
				Menu_voice_value=2;// ������
                rt_kprintf( " �ȼ�  3\r\n"); 
			}

	    }
        else     //+CMGS:19 
		if( strncmp( (char*)GSM_rx, "+CMGS:", 6 ) == 0 )  
	    {
			
			j = sscanf( GSM_rx + 6, "%d", &i );
			rt_kprintf( "\r\n +CMGS: %d ",i );
			if( i>=30)  
			 {	 
			   Delete_all_sms_flag=1;  
			 }
	    } 
		if( strncmp( (char*)GSM_rx, "+ZCEND:25", 9 ) == 0 )  
		{   //+ZCEND:25
		              
		     CallState=CallState_Idle;	
			  Speak_OFF;// �رչ��� 
			 failed = true;
			 rt_kprintf("\r\n Callstate=Idle\r\n"); 
		  }
		//+ZCEND
		// +ZPPPSTATUS: CLOSED
	   //============================================================================
	#endif
	else
	if(strncmp((char*)GSM_rx, "OK",2) == 0)  
		{				
		    ok = true;
		  if(DispContent)	
			   rt_kprintf(" OK\r\n");     			

		  //   Online  state  OK  ,clear Error Counter
                  if(DataLink_Status())       
	          {
	                Online_error_counter=0; // clear 
                 }
	        		
        }
	else
	if (strcmp((char*)GSM_rx, "NO DIALTONE") == 0) failed = true;
	else
	if (strcmp((char*)GSM_rx, "NO ANSWER") == 0) failed = true;
	if (strncmp((char*)GSM_rx, "NO CARRIER",10) == 0) 
	{                      //  NO CARRIER  
		 CallState=CallState_Idle;	
		  Speak_OFF;// �رչ��� 
		 failed = true;
		 rt_kprintf("\r\n Callstate=Idle\r\n");    
	}   
	if(strcmp((char*)GSM_rx, "BUSY") == 0)  
	{ 
		// ZHinfotype=ZH_01H;//  ��̨���ն˷�������ָʾ
		 //subtype=0x12;	   //�Ҷ�
		 CallState=CallState_Idle;	
		 failed = true; 
		// Off_speaker;		// �رչ��� 
		 //Clip_flag=0;
		 //Clip_counter=0; 
	
	}
	else
	if(strncmp((char*)GSM_rx,"+CSQ:",5)==0)	//��ǿ��Ϣ
	{ //+CSQ: 27,99
				if ( GSM_rx[7] == ',' )
				{
						ModuleSQ = GSM_rx[6] - 0x30;   
				}
				else if ( GSM_rx[8] == ',' )
				{
						ModuleSQ = ( GSM_rx[6] - 0x30 ) * 10 + GSM_rx[7] - 0x30;
	
						if ( ModuleSQ == 99 )
						{
								ModuleSQ = 0;
						}
				}
	          //-------------------------------------- 
	 	      //  You must   register on 
	 		  if(CommAT.Total_initial==1)
	 			{
	                if((ModuleSQ>10)&&(CommAT.Initial_step==17))      
	 				{
	 				  CommAT.Initial_step++;
					  rt_kprintf("\r\n CSQ check Pass : %s\r\n",GSM_rx);      
	                }
	 			} 				 
		
	}
	else
	if ((strlen((char*)GSM_rx) >= 7) && (strncmp((char*)GSM_rx, "CONNECT", 7) == 0))
	{
	      connect=true;
	}
	
#ifdef MC8332_CDMA
	else
	if (strncmp((char*)GSM_rx, "ERROR",5) == 0) 
	{
	   error = true;
	   CallState=CallState_Idle;  
	   //----------------------
	   if(TTS_ACK_Error_Process()==true)
               {     rt_kprintf("\r\n  TTS ack  error \r\n");  Speak_OFF; }
	   else 
	   if(DataLink_Status())        //   Online  state  Error    , End Link and Redial    
	       {
	                   Online_error_counter++;
				  if(Online_error_counter>3)	 
			         {   
			                DataLink_EndFlag=1; //AT_End();    
	                              Online_error_counter=0;
				  }
			  rt_kprintf("\r\n Online error\r\n"); 	   
	   	}
	   rt_kprintf("\r\n ERROR\r\n");      
	}
#endif	
	else
	if (strncmp((char*)GSM_rx, "Unknown",7) == 0) 
		error = true;	

	//-------------  End  need	process ---------------------------------------
RXOVER:
       memset(GSM_rx, 0, sizeof(GSM_rx));
	GSM_rx_Wr = 0; 
	
	//---------------------  Function  Process Below --------------------------
	if ((DataDial.Dial_step != Dial_MainLnk) && failed) 
	{ 
		 Dial_Stage(Dial_Idle); 
		return;
	}
	
	switch (DataDial.Dial_step)
	{
		case Dial_DialInit0:	
			                     #ifndef MC8332_CDMA		  
			                         if (ok) 
                                 #endif
						         Dial_Stage(Dial_DialInit1);
								break;
		case Dial_DialInit1   : 			                   						     
			                    if (ok) Dial_Stage(Dial_DialInit2); 
								break;
		case Dial_DialInit2	:	if (ok)   Dial_Stage(Dial_DialInit3);
								break;
		case Dial_DialInit3	:	if (ok)   Dial_Stage(Dial_DialInit4);
								break;
		case Dial_DialInit4	:	
			                    #ifdef MC8332_CDMA
                                    if(connect)    
										   Dial_Stage(Dial_DialInit5);      
								#endif	
								break;
		case Dial_DialInit5	:	if (ok)  Dial_Stage(Dial_DialInit6);
								break;
		case Dial_DialInit6	:	if (ok)  
			                  /*
                                                            ��ʼ����ɺ�ѡ���״����ӵķ�ʽ
			                                */
			                   if(Vechicle_Info.Link_Frist_Mode==1)
							   	      Dial_Stage(Dial_MainLnk);
							   else
							   	      Dial_Stage(Dial_DNSR1); 

		                       /*
					                    #ifdef MULTI_LINK
					                          Dial_Stage(Dial_DNSR1);   // ������  
								#else
								      Dial_Stage(Dial_MainLnk);
						             #endif
						             */
								break;						
		case Dial_DNSR1	:	if (ok)
			                           {
                                                         Dial_Stage(Dial_DNSR2);
			                           }   
								break;		
	    case Dial_DNSR2	:	if (ok)
			                           {
                                                          Dial_Stage(Dial_MainLnk); 
			                           }   
								break;							
		case Dial_MainLnk		:		
	    case Dial_AuxLnk	    :	if (failed || error)
								{
									DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;				// try again in 6 seconds
									break;
								} 
								 if (connect) 						
								{   //  below  run once  since  online
								     if(DataDial.Dial_step ==Dial_MainLnk)    //mainlink 
								             rt_kprintf("\r\n���ӳɹ�TCP---\r\n");
								     if(DataDial.Dial_step ==Dial_AuxLnk)    //auxlink 	 
								                rt_kprintf("\r\n Aux ���ӳɹ�TCP---\r\n");
								   //     1.   ��½�ɹ�����ز���	 
								    // <--  ע��״̬
								    if(1==JT808Conf_struct.Regsiter_Status)   
								        {
								           DEV_Login.Operate_enable=1;  
										   if(DEV_Login.Sd_counter==0)
                                               DEV_Login.Enable_sd=1;    
								    	}
									else
										{
										  JT808Conf_struct.Regsiter_Status=0;  
										  if(DEV_regist.Sd_counter==0)
										     DEV_regist.Enable_sd=1; 
										}
									 
								   // connect = true;
								         //  -----  Data  Dial Related ------
									    if((DataDial.Dial_ON)&&(DataDial.Dial_step<Max_DIALSTEP)) 
									    	{
							                   Dial_Stage(Dial_Idle);
										       DataDial.Dial_step_Retry=0;
										       DataDial.Dial_step_RetryTimer=0;   			
									    	}
							              //-----------------------------------
							           //   DataDial.Dial_ON=0; 
								       DataLink_Online=enable;	  
									  DialLink_TimeOut_Clear(); 	  
									  
                                      #ifdef	MULTI_LINK
											   TCP2_ready_dial=1;	   
                                       #endif
									  
                                   //--------------------------------------------------------------------->
									 Dial_Stage(Dial_Idle); //  state  convert   
									 RTC_TimeShow();	
						             }						           
							      //------------------------------------------							
								 break;  
		     case Dial_ISP:
			 	                if (failed || error)
								{
									DataDial.Dial_step_RetryTimer=Dial_Dial_Retry_Time;
									break;
								}
								 if (connect) 
								{
									 Dial_Stage(Dial_Idle); //	In EM310 Mode										
									TCP2_Connect=1;		
									TCP2_sdFlag=1;
									TCP2_ready_dial=0;  
									 DataDial.Dial_ON=0;  // 
								}							 
								break;		 						
		default 			:	 Dial_Stage(Dial_Idle);
	                                          DataDial.Dial_step_Retry=0;	
								break;
	}
    //-----------
}

void  IMSIcode_Get(void)
{

        if(GSM_PWR.GSM_power_over==1)
		{
               IMSIGet.Checkcounter++;
		       if(IMSIGet.Checkcounter>20)     //  15*30=450ms      
		   	{
                 IMSIGet.Checkcounter=0; 
			    if(IMSIGet.Get_state==1)
	               {
	                 rt_hw_gsm_output(IMSI_Check_str);    // ��ѯ IMSI_CODE ���� 
	                 rt_kprintf(IMSI_Check_str);  
		         }  
			  else 
			   {
			  	   rt_hw_gsm_output(SIM_Check_Str);    // �ȼ�� SIM ���Ĵ���
		                 rt_kprintf(SIM_Check_Str);    
			   }
		   	} 
		}  
}

#if 0 
 void Rx_in(u8* instr)
 {
	 u16  inlen=0;
 
	   if(strncmp(instr,"can1",4)==0)
		 {
			Get_GSM_HexData("7E8103003B013601300001864F060000010004000001F40000010102000A0000010204000000000000010302000000000110080000000058FFD11700000111080000006458FFD017EA7E",148,0);
			OutPrint_HEX("ģ��1", GSM_HEX, GSM_HEX_len); 
 
	  }
		 else
		 {
			  inlen=strlen((const char*)instr);
			  Get_GSM_HexData(instr,inlen,0);  
			   OutPrint_HEX("ģ��", GSM_HEX, GSM_HEX_len); 
		 }	  
 
 }
 FINSH_FUNCTION_EXPORT(Rx_in, Rx_in);
 #endif

void  cplay(u8 in)
 {
      // rt_hw_gsm_output("AT+ZTTS=1,\"2c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d9965\"\r\n");   
     if(in==1)
	  rt_hw_gsm_output("AT+ZTTS=1,\"EA818C4EF696004E094E745E4351E6654750E565895B92636C51035E0E540CFF76511C20114F4750F69663651D208C541C2082820E54858D7F95E55D2866E5651D20D7533052288D917502304B4E0E540CFF776D575301773F659C5E1A90C78F985B517FD153035E1A90E577F079065C3E4E9E5268510177226B504E\"\r\n");
     else
	 if(in==2)	
		 rt_hw_gsm_output("AT+ZTTS=2,\"CDE2BDBBB2BFB8B1B2BFB3A4D5C5D6BEBEFCD7F2CCECB4F2B5E7BBB0B8F8C8D5B1BECDE2CEF1CAA1B3A3CEF1B8B1CDE2B3A4D7F4D7F4BDADCFCDD2BBC0C9\"\r\n");
	 else	 	  
	 rt_hw_gsm_output("AT+ZTTS=0,\"EA818C4EF696004E094E745E4351E6654750E565895B92636C51035E0E540CFF76511C20114F4750F69663651D208C541C2082820E54858D7F95E55D2866E5651D20D7533052288D917502304B4E0E540CFF776D575301773F659C5E1A90C78F985B517FD153035E1A90E577F079065C3E4E9E5268510177226B504E\"\r\n");
 } 
 FINSH_FUNCTION_EXPORT(cplay, cplay);  

void  catd(void)
 {
      // rt_hw_gsm_output("AT+ZTTS=1,\"2c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d99652c67448d9965\"\r\n");   
	   rt_hw_gsm_output("ATD112;\r\n");
 } 
 FINSH_FUNCTION_EXPORT(catd, catd);  


void AT(char *str)
{
	rt_hw_gsm_output((const char*)str);     
	rt_hw_gsm_output("\r\n"); 
	rt_kprintf("%s\r\n",str);
	   
} 
FINSH_FUNCTION_EXPORT(AT, AT);



void  rt_hw_gsm_init(void)
{

       GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	


      RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD, ENABLE );
      RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4, ENABLE );



/*uart4 �ܽ�����*/

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
       GPIO_InitStructure.GPIO_Pin = GSM_TX_PIN| GSM_RX_PIN;
	GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);   

	

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    #ifdef MC8332_CDMA
	USART_InitStructure.USART_BaudRate =115200;   // new M66  57600  
	#endif
	
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
	USART_Init(UART4, &USART_InitStructure);
	/* Enable USART */
	USART_Cmd(UART4, ENABLE);
	USART_ITConfig( UART4, USART_IT_RXNE, ENABLE );   

			 
   /* -------------- GPRS  GSM	ģ�� EM310 ������� -----------------	*/
   /*
		   STM32 Pin	 SIM300 gprs  Pin		   Remark
			 PD12		   Power				  PE2 set 1: Power On  set	0: Poweroff 
			 PD13	   PWRKEY				   ���˷���������	PA5 set 1:Off	   0: On 
	*/
  
   /*  �ܽų�ʼ�� ����Ϊ ������� */

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);


  GPIO_InitStructure.GPIO_Pin = GPRS_GSM_Power ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);	
   
   GPIO_InitStructure.GPIO_Pin = GPRS_GSM_PWKEY;
   GPIO_Init(GPIOD, &GPIO_InitStructure);	
   
   GPIO_InitStructure.GPIO_Pin =GPRS_GSM_RST;		//-----  Reset ��̬���õ�   ����Ч  
   GPIO_Init(GPIOD, &GPIO_InitStructure);   


   GPIO_InitStructure.GPIO_Pin =Speak_Ctrl;		//-----  ����
   GPIO_Init(GPIOD, &GPIO_InitStructure);   	 
   //==================================================================== 
   #ifdef  MC8332_CDMA
       GPIO_SetBits(GPIOD,GPRS_GSM_RST);   // ��̬�ø߿ص�  
   #endif   
   
   GPIO_ResetBits(GPIOD,GPRS_GSM_Power);
   GPIO_ResetBits(GPIOD,GPRS_GSM_PWKEY);    //GPIO_SetBits(GPIOD,GPRS_GSM_PWKEY);       
   
   Speak_OFF;//  �ر���Ƶ���� 

/*
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;		//GPS  ��������
   GPIO_Init(GPIOD, &GPIO_InitStructure);   

     GPIO_InitStructure.GPIO_Pin =GPIO_Pin_12;		//GPS  ��������
   GPIO_Init(GPIOC, &GPIO_InitStructure);   
     GPIO_ResetBits(GPIOC,GPIO_Pin_12);      
    GPIO_ResetBits(GPIOD,GPIO_Pin_2);      	   

 // Speak_ON;  
 */
}
