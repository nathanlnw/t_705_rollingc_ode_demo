#include "App_moduleConfig.h"




//--------   ˳���ȡ�������  ------------
u8    ReadCycle_status=RdCycle_Idle;  
u8    ReadCycle_timer=0;   // ��ʱ�ж�


u32     cycle_write=0, cycle_read=0,delta_0704_rd=0;  // ѭ���洢��¼
u32    AvrgSpdPerMin_write=0,AvrgSpdPerMin_Read=0; // ����ÿ����ƽ���ٶȼ�¼
u32    AvrgMintPosit_write=0,AvrgMintPosit_Read=0; // ������λСʱ��ÿ����λ�ü�¼
u32    ErrorLog_write=0,ErrorLog_Read=0;           // �豸�쳣��¼
u32    Login_write=0,Login_Read=0;                 // ��¼��¼
u32    Powercut_write=0,Powercut_read=0;           // �ⲿ��Դ�Ͽ�
u32    Settingchg_write=0,Settingchg_read=0;       // �����޸�
u32    TiredDrv_write=0, TiredDrv_read=0;  // ƣ�ͼ�ʻ�洢��¼
u32    ExpSpdRec_write=0, ExpSpdRec_read=0;  // ���ٱ����洢��¼
u32    pic_write=0,pic_read=0,pic_current_page=0,pic_PageIn_offset=0,pic_size=0;;       // ͼƬ�洢��¼ 
u32   	Distance_m_u32=0;	 // �г���¼�����о���	  ��λ��
u32     DayStartDistance_32=0; //ÿ����ʼ�����Ŀ


//-----------------------------------------------------------------------------------------------------------------------------
u8 SaveCycleGPS(u32 cyclewr,u8 *content ,u16 saveLen) 
{
  /*
         //old  NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
         NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 128 Bytes ;  1page= 4Records   1Sector=8Page=32Records
  */
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
//	u8   reg[1]={0};
	u8   rd_back[128];
	u16  i=0,retry=0;

  //----------------------------------------------------------------------------------------------
  //   1. Judge  Whether  needs to Erase page 
  
     pageoffset=(u32)(cycle_write>>2);                // ����� Page ƫ��  ����4 (ÿ��page�ܷ�4����¼)
     InPageoffset=cycle_write-(u32)(pageoffset<<2);   // ����� ҳ��ƫ�Ƶ�ַ 
     InPageAddr=(u16)(InPageoffset<<7);           // �����ҳ�� �ֽ�   ���� 128 (ÿ����¼128���ֽ�)
     if(((pageoffset%8)==0)&&(InPageoffset==0))  // �ж��Ƿ���Ҫ����Sector  ���Ƴ�����һ��Sector  1Sector=8Page  
     {
        WatchDog_Feed();
		SST25V_SectorErase_4KByte((pageoffset+CycleStart_offset)*PageSIZE);      // erase Sector		
		DF_delay_ms(20); 
	    rt_kprintf("\r\n Erase Cycle Sector : %d\r\n",(pageoffset>>3));       
	 }
  //	   2. write  and read back    
  SV_RTRY:
      if(retry>=2)
	  	  return false;
	   delay_ms(5);
	   WatchDog_Feed(); 
	   DF_WriteFlashDirect(pageoffset+CycleStart_offset,InPageAddr,content,saveLen);  //   д����Ϣ
	   DF_delay_us(30);   
       DF_ReadFlash(pageoffset+CycleStart_offset,InPageAddr,rd_back,saveLen);  //   ��ȡ��Ϣ
  //  compare 
       for(i=0;i<saveLen;i++)
       	{
             if(content[i]!=rd_back[i])
			 {
			     cycle_write++;		  
		         if(cycle_write>=Max_CycleNum)
			       cycle_write=0; 
				 if(retry==0)
				 {
				     retry++;
			         goto SV_RTRY;
				 }
				 else
				 {
				   //---------------------------
				  PositionSD_Enable(); 
				  Current_State=1; // ʹ�ܼ�ʱ�ϱ� 		
				  Current_UDP_sd=1;
				  rt_kprintf("\r\n wrte error current\r\n");
				    return false;
				 }
             }
       	}
		return true;  
  //-------------------------------------------------------------------------------------------- 
}  


u8 ReadCycleGPS(u32 cycleread,u8 *content ,u16 ReadLen)
{
  /*
     NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
  */
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
	u8  i=0,FCS=0;
	u8  Len_read=0;  // ��Ϣ����

  /*
      �ϱ���ÿһ������һ���ֽ�����Ч��Ϣ�ĳ��ȣ��ӵڶ����ֽ�����Ϣ���ݣ�
      ��Ϣ���ݵĺ����һ���ֽڶ�У��(У��ӳ��ȿ�ʼ���������һ���ֽ�)
  */
  //----------------------------------------------------------------------------------------------
  //   1. caculate address 
     pageoffset=(u32)(cycle_read>>2);                 // ����� Page ƫ��  ����4 (ÿ��page�ܷ�4����¼)
     InPageoffset=cycle_read-(u32)(pageoffset<<2);   // ����� ҳ��ƫ�Ƶ�ַ 
     InPageAddr=(u16)(InPageoffset<<7);            // �����ҳ�� �ֽ�   ���� 128 (ÿ����¼128���ֽ�)
  //   2. Write Record Content 
     DF_ReadFlash(pageoffset+CycleStart_offset,InPageAddr,content,ReadLen); 
     DF_delay_us(20);
	 //  ��ȡ��Ϣ����
	 Len_read=content[0];
	 
  if(DispContent==2)
  {
   	 OutPrint_HEX("��ȡCycleGPS ����Ϊ ",content,Len_read+1);
  }	 
  //  3. Judge FCS	
	//--------------- �����Ѿ����͹�����Ϣ ------- 
	  FCS = 0;
	   for ( i = 0; i < Len_read; i++ )   //�����ȡ��Ϣ������
	   {
			   FCS ^= *( content + i );  
	   } 
	   //rt_kprintf("\r\n  FCS=%d \r\n",FCS); 
	  if(((content[Len_read]!=FCS)&&(content[0]!=0xFF))||(content[0]==0xFF))  // �ж�����   
	    { 	      
		  if(content[0]==0xFF)
		  {
			 rt_kprintf("\r\n  content[0]==0xFF   read=%d,  write=%d  \r\n",cycle_read,cycle_write);   
			 //cycle_read=cycle_write;//�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���
                       
			  cycle_read++;	
		        if(cycle_read>=Max_CycleNum)
		  	      cycle_read=0; 
			 ReadCycle_status=RdCycle_Idle;  
			 return false;	
		  }  
		  //------------------------------------------------
          cycle_read++;	
		  if(cycle_read>=Max_CycleNum)
		  	cycle_read=0;
		  ReadCycle_status=RdCycle_Idle; 
		  return false; 
     	}	  
	//------------------------------------------------------------------------------------------   	 
	    return true;     
  //-------------------------------------------------------------------------------------------- 
} 

//-----------------------------------------------------------------------------------------------------------
u8  Common_WriteContent(u32 In_write,u8 *content ,u16 saveLen, u8 Type) 
{
  //-----------------------------------------------------
  u8     reg[1];
  //u8   regStr[25];   
  //-----------------------------------------------------
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
	u32  Start_offset=0; 

   //--------------------------------------------------	
    //memset(regStr,0,sizeof(regStr)); 
  //  1.   Classify
    switch(Type)
    {
	
		case TYPE_TiredDrvAdd:
							 Start_offset=TiredDrvStart_offset;
							// memcpy(regStr,"ƣ�ͼ�ʻ",25);
							 break;
		case TYPE_ExpSpdAdd:
							 Start_offset=ExpSpdStart_offset;
							// memcpy(regStr,"���ٱ���",25);
							 break; 					 
		default :
							 return false;		  					 
    }
	//----------------------------------------------------------------------------------------------
	//	 2. caculate address 
	 
		pageoffset=(u32)(In_write>>4);				 // ����� Page ƫ��  ����16  
		InPageoffset=In_write-(u32)(pageoffset<<4);	 // ����� ҳ��ƫ�Ƶ�ַ 
		InPageAddr=(u16)(InPageoffset<<5);			 // �����ҳ�� �ֽ�   ���� 32 (ÿ����¼32���ֽ�)
		if(((pageoffset%8)==0)&&(InPageoffset==0))  // �ж��Ƿ���Ҫ����Block  ���Ƴ�����һ��Block	1Block=8Page  
		{
        SST25V_SectorErase_4KByte((pageoffset+Start_offset)*PageSIZE);      // erase Sector	
        DF_delay_ms(70);
		//  rt_kprintf("\r\n Common --- Erase Cycle Block : %d\r\n",(pageoffset>>6));    
		}
	 // 	  2. Filter write  area    
		DF_ReadFlash(pageoffset+Start_offset,InPageAddr,reg,1); 
		  if(reg[0]!=0xff)	// ���Ҫд������� dirty  �����ַ��Ȼ����¿�ʼѰ��֪���ҵ�Ϊֹ
		   {
			 In_write++;
			 if(In_write>=Max_CommonNum)
				  In_write=0;  
			         // rt_kprintf("\r\n	 *******   Common д���� Write area : %d	is	 Dirity!  \r\n",In_write);  
				 return false;
		   }	   
	 //   3. Write Record Content 
        DF_WriteFlashDirect(pageoffset+Start_offset,InPageAddr,content,saveLen);  //   д����Ϣ
		DF_delay_ms(10);
		//rt_kprintf("\r\n Common Starpageoffset=%d	PageOffset= %d ,  InPageAddr= %d  TYPE= %s \r\n",CycleStart_offset,pageoffset,InPageoffset,regStr);  

     //   4. end  
        switch(Type) 
    {
	
		case TYPE_TiredDrvAdd:							 
							 TiredDrv_write=In_write;
							 break;
		case TYPE_ExpSpdAdd:
							 ExpSpdRec_write=In_write;
							 break; 					 
		case TYPE_AvrgSpdAdd:
							 AvrgSpdPerMin_write=In_write;
							 break;
		case TYPE_ErrorLogAdd:
							 ErrorLog_write=In_write;  
							 break; 
		case TYPE_LogInAdd:
							 Login_write=In_write;
							 break; 
		case TYPE_SettingChgAdd:
							 Settingchg_write=In_write;     
							 break; 					 
		default :
							 return false;	 						 
    }

	return true; 
	     	     
}


u8  Common_ReadContent(u32 In_read,u8 *content ,u16 ReadLen, u8 Type) 
{
  //-----------------------------------------------------
  //u8    regStr[25];   
  //-----------------------------------------------------
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
	u32  Start_offset=0;     
	u8   i=0,FCS=0;;
   //--------------------------------------------------	
 
	// memset(regStr,0,sizeof(regStr)); 
   //  1.	Classify
	 switch(Type)
	 {
	 
		 case TYPE_TiredDrvAdd:
							  Start_offset=TiredDrvStart_offset;
							//  memcpy(regStr,"ƣ�ͼ�ʻ",25);
							  break;
		 case TYPE_ExpSpdAdd:
							  Start_offset=ExpSpdStart_offset;
							//  memcpy(regStr,"���ٱ���",25);
							  break;					  
		 default :
							  return false; 						  
	 } 
 
	//----------------------------------------------------------------------------------------------
	//	 2. caculate address 
	 
		   pageoffset=(u32)(In_read>>4);				  // ����� Page ƫ��  ����64  
		   InPageoffset=In_read-(u32)(pageoffset<<4);   // ����� ҳ��ƫ�Ƶ�ַ 
		   InPageAddr=(u16)(InPageoffset<<5);			 // �����ҳ�� �ֽ�   ���� 32 (ÿ����¼32���ֽ�)
		  //	 2. Write Record Content 
		   DF_ReadFlash(pageoffset+Start_offset,InPageAddr,content,ReadLen);    
		   DF_delay_us(10);
		 //  rt_kprintf("\r\n  Common Starpageoffset=%d	PageOffset= %d ,  InPageAddr= %d  TYPE= %s\r\n",Start_offset,pageoffset,InPageoffset,regStr);  
		 if(DispContent)
		 {
			   rt_kprintf("\r\n  ��ȡCommon ����Ϊ :\r\n");  
				for(i=0;i<ReadLen;i++)
				  rt_kprintf("%2X ",content[i]);  
			   rt_kprintf("\r\n");   
		 }   
		//	3. Judge FCS  
		  //--------------- �����Ѿ����͹�����Ϣ -------
			FCS = 0;
			 for ( i = 0; i < ReadLen-1; i++ )	 //�����ȡ��Ϣ������
			 {
					 FCS ^= *( content + i );  
			 }				
			 if(((content[ReadLen-1]!=FCS)&&(content[0]!=0xFF))||(content[0]==0xFF))  // �ж����� 
			  { 
			   
			   if(content[0]==0xFF)
			   {
				  //�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���	
				         switch(Type)
					    {
						
							case TYPE_TiredDrvAdd:							 
												 TiredDrv_read=TiredDrv_write;
												 break;
							case TYPE_ExpSpdAdd:
												 ExpSpdRec_read=ExpSpdRec_write;
												 break; 					 
							case TYPE_AvrgSpdAdd:
												 AvrgSpdPerMin_Read=AvrgSpdPerMin_write;
												 break;
							case TYPE_ErrorLogAdd:
												 ErrorLog_Read=ErrorLog_write;        
												 break; 
							 case TYPE_LogInAdd:
												 Login_Read=Login_write;
												  break; 
							 case TYPE_SettingChgAdd:
							 	                 Settingchg_read=Settingchg_write;
												  break;					  
												 
							default :
												 return false;							 
					    
				         	}	
						return false; 
			   }
				In_read++; 
				if(In_read>=Max_CommonNum)
				  In_read=0;
				rt_kprintf("\r\n   *******	������¼���ݲ��� *******  \r\n");  
				return false;
			  }
			 if(content[0]==0xFF)
			 {
               rt_kprintf("\r\n  ��ȡ����Ϊ 0xFF \r\n");
			        switch(Type)
					    {
						
							case TYPE_TiredDrvAdd:							 
												 TiredDrv_read=TiredDrv_write;
												 break;
							case TYPE_ExpSpdAdd:
												 ExpSpdRec_read=ExpSpdRec_write;
												 break; 					 
							case TYPE_AvrgSpdAdd:
												 AvrgSpdPerMin_Read=AvrgSpdPerMin_write;
												 break;
							case TYPE_ErrorLogAdd:
												 ErrorLog_Read=ErrorLog_write;        
												 break; 
							 case TYPE_LogInAdd:
												 Login_Read=Login_write;
												  break; 
							 case TYPE_SettingChgAdd:
												 Settingchg_read=Settingchg_write;
												  break;					  
												 
							default :
												 return false;							 
					    
				         	}	
               return false;
			 } 
			 
			 	
     //   4. end  
        switch(Type)
    {
	
		case TYPE_TiredDrvAdd:							 
							 TiredDrv_read=In_read;
							 break;
		case TYPE_ExpSpdAdd:
							 ExpSpdRec_read=In_read;
							 break; 					 
		case TYPE_AvrgSpdAdd:
							 AvrgSpdPerMin_Read=In_read;
							 break;
		case TYPE_ErrorLogAdd:
							 ErrorLog_Read=In_read;        
							 break; 
		 case TYPE_LogInAdd:
							 Login_Read=In_read;
							  break; 
		 case TYPE_SettingChgAdd:
		 	                 Settingchg_read=In_read; 
							  break;					  
							 
		default :
							 return false;							 
    }

	return true; 
	     	     
}


u8 Save_PerMinContent(u32 In_wr,u8 *content ,u16 saveLen)
{
	/*
	   NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
	*/
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
	u8   reg[1]={0};
  //----------------------------------------------------------------------------------------------
  //   1. Judge  Whether  needs to Erase page 
  
     pageoffset=(u32)(In_wr/7);                // ����� Page ƫ��  ����7  
     InPageoffset=In_wr-(u32)(pageoffset*7);   // ����� ҳ��ƫ�Ƶ�ַ 
     InPageAddr=(u16)(InPageoffset*70);           // �����ҳ�� �ֽ�   ���� 70 (ÿ����¼70���ֽ�)    
     if(((pageoffset%8)==0)&&(InPageoffset==0))  // �ж��Ƿ���Ҫ����Block  ���Ƴ�����һ��Block  1Block=8Page  
     {
	  SST25V_SectorErase_4KByte((pageoffset+AverageSpdStart_offset)*PageSIZE);	  // erase Sector 
	  DF_delay_ms(80);
	  // rt_kprintf("\r\n Erase Cycle Block : %d\r\n",(pageoffset>>6));    
	 }
  //	   2. Filter write  area    
      DF_ReadFlash(pageoffset+AverageSpdStart_offset,InPageAddr,reg,1);
	   if(reg[0]!=0xff)  // ���Ҫд������� dirty  �����ַ��Ȼ����¿�ʼѰ��֪���ҵ�Ϊֹ
	   	{
	   	  In_wr++;
		  AvrgSpdPerMin_write++;		  
		  if(AvrgSpdPerMin_write>=Max_SPDSperMin)
			   AvrgSpdPerMin_write=0;  
		 // rt_kprintf("\r\n    *******  ÿ�����ٶ� д���� Write area : %d   is   Dirity!  \r\n",AvrgSpdPerMin_write);  
          //--------------------------------
		  	return false;
	   	}       
  //   3. Write Record Content 
     DF_WriteFlashDirect(pageoffset+AverageSpdStart_offset,InPageAddr,content,saveLen);  //   д����Ϣ
     DF_delay_ms(5);
	// rt_kprintf("\r\n Write PerMit Starpageoffset=%d  PageOffset= %d ,  InPageAddr= %d \r\n",AverageSpdStart_offset,pageoffset,InPageoffset); 
		return true; 
  //-------------------------------------------------------------------------------------------- 
}

u8 Read_PerMinContent(u32 In_read,u8 *content ,u16 ReadLen) 
{
	/*
	   NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
	*/
    u32  pageoffset=0;   //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr=0;   //ҳ�� ��ַƫ�� 
	u8   FCS=0;
	u16  i=0;  

  /*
      �ϱ���ÿһ������λ31���ֽڣ� ÿ����¼Ϊ32���ֽڣ���ÿ����¼�����һ���ֽ�����Ϊ�Ƿ��ϱ����ı�־λ��û���ϱ��ñ�־Ϊ0xFF
      �ϱ�����ñ�־Ϊ��дΪ0x01
  */
  //----------------------------------------------------------------------------------------------
  //   1. caculate address 
  pageoffset=(u32)(In_read/7);				 // ����� Page ƫ��  ����7  
  InPageoffset=In_read-(u32)(pageoffset*7);	 // ����� ҳ��ƫ�Ƶ�ַ  
  InPageAddr=(u16)(InPageoffset*70);		   // �����ҳ�� �ֽ�	���� 70 (ÿ����¼70���ֽ�)	  
  //   2. Write Record Content 
    DF_ReadFlash(pageoffset+AverageSpdStart_offset,InPageAddr,content,ReadLen);     
    DF_delay_us(10);
	// rt_kprintf("\r\n   Read PerMintSPD  Starpageoffset=%d  PageOffset= %d ,  InPageAddr= %d   ÿ����ƽ���ٶ� \r\n",AverageSpdStart_offset,pageoffset,InPageoffset);  
   if(DispContent==2)
   {
	rt_kprintf("\r\n  >>>>>>>>>>��ȡÿ����ƽ���ٶ�   PerMintSPD ����Ϊ :\r\n");       
   	  for(i=0;i<ReadLen;i++)
   	  	rt_kprintf("%2X ",content[i]);   
   	 rt_kprintf("\r\n");    
   }	 
  //  3. Judge FCS	
	//--------------- �����Ѿ����͹�����Ϣ -------
	  FCS = 0;
	   for ( i = 0; i < ReadLen-1; i++ )   //�����ȡ��Ϣ������
	   {
			   FCS ^= *(content + i );  
	   }			  
	   if(((content[ReadLen-1]!=FCS)&&(content[0]!=0xFF))||(content[0]==0xFF))	// �ж����� 
	    { 
	    
		    if(content[0]==0xFF)
	 		{             
	 		       rt_kprintf("\r\n  ��ȡ����Ϊ0xFF --1\r\n"); 	 		    
				   AvrgSpdPerMin_Read=AvrgSpdPerMin_write;//�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���   		  
	               return false; 
			} 
          AvrgSpdPerMin_Read++;	
		  if(AvrgSpdPerMin_Read>=Max_SPDSperMin) 
		  	AvrgSpdPerMin_Read=0; 
		  //rt_kprintf("\r\n   ******* PerMit  ������¼���ݲ��� *******  \r\n");    
		  return false; 
     	}  
	    if(content[0]==0xFF)
 		{             
 		       rt_kprintf("\r\n  ��ȡ����Ϊ0xFF \r\n"); 	 		    
			   AvrgSpdPerMin_Read=AvrgSpdPerMin_write;//�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���   		  
               return false; 
		} 
	//------------------------------------------------------------------------------------------  
    return true; 
    //-------------------------------------------------------------------------------------------- 
}
u8 Save_MintPosition(u32 In_write,u8 *content ,u16 saveLen)
{    // ��¼��λСʱ��ÿ���ӵ�λ����Ϣ
	/*
	   NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
	*/
    u32  pageoffset=0;   //Page ƫ��
	u8   reg[1]={0};
  //----------------------------------------------------------------------------------------------
  //   1. Judge  Whether  needs to Erase page     ÿ����¼512���ֽ�
  
     pageoffset=In_write;                // ����� Page ƫ��  ����4  
        // ����� ҳ��ƫ�Ƶ�ַ 0
        // �����ҳ�� �ֽ�   ���� 512 (ÿ����¼512���ֽ�)  485(content)+1(FCS)
     if((pageoffset%8)==0)  // �ж��Ƿ���Ҫ����Block  ���Ƴ�����һ��Block  1Block=8 Page  
     {
	  SST25V_SectorErase_4KByte((pageoffset+AvrgMintPosit_offset)*PageSIZE);	  // erase Sector 
	  DF_delay_ms(50);
	  // rt_kprintf("\r\n Erase Cycle Block : %d\r\n",(pageoffset>>6));   
	 }
  //	   2. Filter write  area    
       DF_ReadFlash(pageoffset+AvrgMintPosit_offset,0,reg,1); 
	   if(reg[0]!=0xff)  // ���Ҫд������� dirty  �����ַ��Ȼ����¿�ʼѰ��֪���ҵ�Ϊֹ
	   	{
          In_write++;
		  AvrgMintPosit_write++;		  
		  if(AvrgMintPosit_write>=Max_MintPos) 
			   AvrgMintPosit_write=0;  
		 // rt_kprintf("\r\n    *******   �г���¼��д���� Write area : %d   is   Dirity!  \r\n",In_write);  
		  //----------------------------
		  	return  false;
	   	}       
  //   3. Write Record Content 
       DF_WriteFlashDirect(pageoffset+AvrgMintPosit_offset,0,content,saveLen);  //   д����Ϣ
       DF_delay_ms(10);
     return true; 
  //-------------------------------------------------------------------------------------------- 
}

u8 Read_MintPosition(u32 In_read,u8 *content ,u16 ReadLen)
{  // ��ȡ��λСʱ��ÿ���ӵ�λ����Ϣ
	/*
	   NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
	*/
    u32  pageoffset=0;   //Page ƫ�� 
	u8   FCS=0;
	u16  i=0;    

  /*
      �ϱ���ÿһ������λ31���ֽڣ� ÿ����¼Ϊ32���ֽڣ���ÿ����¼�����һ���ֽ�����Ϊ�Ƿ��ϱ����ı�־λ��û���ϱ��ñ�־Ϊ0xFF
      �ϱ�����ñ�־Ϊ��дΪ0x01
  */
  //----------------------------------------------------------------------------------------------
  //   1. caculate address 
  pageoffset=In_read;				// ����� Page ƫ��  ����1	
  	// ����� ҳ��ƫ�Ƶ�ַ 
    // �����ҳ�� �ֽ�	���� 512 (ÿ����¼512���ֽ�)  485(content)+1(FCS)
  //   2. Write Record Content 
    DF_ReadFlash(pageoffset+AvrgMintPosit_offset,0,content,ReadLen);	  
    DF_delay_us(10);
    // rt_kprintf("\r\n  ��λСʱÿ����λ�ö�ȡ  Position ����Ϊ :\r\n");   
	//  for(i=0;i<ReadLen;i++)
	//	rt_kprintf("%2X ",content[i]);   
	//  rt_kprintf("\r\n");         
    
  //  3.  Judge FCS	
	//--------------- �����Ѿ����͹�����Ϣ -------
	  FCS = 0;
	   for ( i = 0; i < ReadLen-1; i++ )   //�����ȡ��Ϣ������
	   {
			   FCS ^= *( content + i );  
	   }			  
	   if(((content[ReadLen-1]!=FCS)&&(content[0]!=0xFF))||(content[0]==0xFF))	// �ж����� 
	    { 	      
		  if(content[0]==0xFF)
		  {
			 AvrgMintPosit_Read=AvrgMintPosit_write;//�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���			
			 return false;
		  }
          AvrgMintPosit_Read++;	
		  if(AvrgMintPosit_Read>=Max_MintPos)    
		  	AvrgMintPosit_Read=0; 
		//  rt_kprintf("\r\n   *******  DrvRecoder ������¼���ݲ��� *******  \r\n"); 
		  return false;
     	} 
	   	if(content[0]==0xFF)
	   	{
           AvrgMintPosit_Read=AvrgMintPosit_write;//�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���           
		   return false;
	   	}
	//--------------------------------------------------------------  	
		 return true; 		    
  //-------------------------------------------------------------------------------------------- 
}

//----------------------------------------------------------------------  
void  CHK_ReadCycle_status(void) 
{  
  		if(RdCycle_Idle==ReadCycle_status)
			  {
			       
				   if(cycle_write!=cycle_read)
					 {
						ReadCycle_status=RdCycle_RdytoSD; 
					 }
				    else
						ReadCycle_status=RdCycle_Idle; 
				
			  } 
}


void MediaIndex_Init(void)  
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

void Save_Common(u32 In_write,u8 Type)
{
  u8 content[32],regDateTime[6];
  u8 wr_add=0,FCS=0,i;

  
 
   //-----------------
    memset(content,0,sizeof(content));
    wr_add=0; 
   //-----------------
   switch(Type)
   	{
   	   
	   case	 TYPE_ExpSpdAdd:  // ��¼A ��û�г��ټ�¼����������������
	                           break;
	   case  TYPE_TiredDrvAdd:
		                      break;
    
						  
	  case  TYPE_LogInAdd:
	  case	TYPE_SettingChgAdd:
	  	                        Time2BCD(regDateTime);  
	                            memcpy(content+wr_add,regDateTime,6);
							    wr_add+=6; 
								memcpy(content+wr_add,JT808Conf_struct.Driver_Info.DriverCard_ID,18);
							    wr_add+=18;
								switch(Type)  
									{
                                       case  TYPE_LogInAdd:  
									   	                     content[wr_add++]=Login_Status;
															 break;
									   case	TYPE_SettingChgAdd:
									   	                     content[wr_add++]=Settingchg_Status;
															 break;
									}
								break;
     default:
	 	         rt_kprintf("\r\n  Save common Type Error!\r\n");
	 	         return;
   	}
   //-----------------------
   FCS = 0;
   for ( i = 0; i < 32; i++ )  
   {
		 FCS ^=content[i];
   }			  //���ϱ����ݵ�����
   content[wr_add++] = FCS;	  // ��31�ֽ�    
    if(DispContent)
   rt_kprintf("\r\n Common  WriteLen:  %d  \r\n ",wr_add);   
   Common_WriteContent( In_write, content, wr_add, Type);       	
}
