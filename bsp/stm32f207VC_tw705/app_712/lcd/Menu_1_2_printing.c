#include  <string.h>
#include "Menu_Include.h"


static void msg( void *p)
{

}
static void show(void)
{

	//rt_kprintf("\r\n------------------打印缺纸----------");
	lcd_fill(0);
	lcd_text12(20,10,"打印中..",8,LCD_MODE_SET);
	lcd_update_all();
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:		
		case KeyValueOk: 
		case KeyValueUP:
		case KeyValueDown:			   
			   pMenuItem=&Menu_1_Idle;
			   pMenuItem->show();
			   break;
		}
	KeyValue=0;
}


static void timetick(unsigned int systick)
{
	CounterBack++;
		if(CounterBack!=MaxBankIdleTime)
			return;
		pMenuItem=&Menu_1_Idle;
		pMenuItem->show();
		CounterBack=0;

}


MENUITEM	Menu_1_1_printing=
{
    "  打印中  ",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


