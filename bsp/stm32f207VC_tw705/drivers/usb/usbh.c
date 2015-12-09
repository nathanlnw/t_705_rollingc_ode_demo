/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// �ļ���
 * Author:			// ����
 * Date:			// ����
 * Description:		// ģ������
 * Version:			// �汾��Ϣ
 * Function List:	// ��Ҫ�������书��
 *     1. -------
 * History:			// ��ʷ�޸ļ�¼
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/

#include <stdio.h>

#include "stm32f2xx.h"
#include <board.h>
#include <rtthread.h>

#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "dfs_init.h"
#include "dfs_elm.h"
#include "dfs_romfs.h"
#include "dfs_fs.h"


#ifdef RT_USING_DFS_ROMFS 
#include <dfs_romfs.h> 
#endif 



ALIGN(RT_ALIGN_SIZE)
	
USB_OTG_CORE_HANDLE      USB_OTG_Core ;
USBH_HOST                USB_Host;
static char thread_usbmsc_stack[1024];
struct rt_thread thread_usbmsc;
static void rt_thread_entry_usbmsc(void* parameter)
{

	dfs_init();
    elm_init();

	USBH_Init(&USB_OTG_Core,
			  USB_OTG_FS_CORE_ID,
			  &USB_Host,
			  &USBH_MSC_cb, 
			  &USR_cb);
	
	rt_kprintf("\r\nUSBH_Init\r\n");


#if defined(RT_USING_DFS_ROMFS) 
		dfs_romfs_init(); 
		if (dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) == 0) 
		{ 
			rt_kprintf("Root File System initialized!\n"); 
		} 
		else 
			rt_kprintf("Root File System initialzation failed!\n"); 
#endif







	//USB_OTG_Core.regs.GREGS->GUSBCFG|=(1<<29);

	//USB_OTG_WRITE_REG32(USB_OTG_Core.regs.GREGS->GUSBCFG,value)
	rt_thread_delay(RT_TICK_PER_SECOND);
	
    while (1)
    {
    	//GPIO_SetBits(GPIOD, GPIO_Pin_15);
		USBH_Process(&USB_OTG_Core,&USB_Host);
		rt_thread_delay(RT_TICK_PER_SECOND/20);    // ����д���ٶȵ���
		//GPIO_ResetBits(GPIOD, GPIO_Pin_15);
    }

}





void usbh_init(void)
{

    rt_thread_init(&thread_usbmsc,
                   "usbmsc",
                   rt_thread_entry_usbmsc,
                   RT_NULL,
                   &thread_usbmsc_stack[0],
                   sizeof(thread_usbmsc_stack),19,5);
    rt_thread_startup(&thread_usbmsc);
}


/************************************** The End Of File **************************************/