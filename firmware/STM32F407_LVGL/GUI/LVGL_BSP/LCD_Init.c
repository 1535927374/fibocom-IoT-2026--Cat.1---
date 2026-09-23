#include "LCD_Init.h"

void LCD_GPIO_Init(void)
{
	// HAL库中GPIO初始化已经在CubeMX生成的代码中完成
	// 这里可以留空或者添加额外的初始化代码
	// GPIO配置在MX_GPIO_Init()函数中已经完成
}


/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{	
	uint8_t i;
	LCD_CS_Clr();
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr();
		if(dat&0x80)
		{
		   LCD_MOSI_Set();
		}
		else
		{
		   LCD_MOSI_Clr();
		}
		LCD_SCLK_Set();
		dat<<=1;
	}	
  LCD_CS_Set();	
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint32_t dat)
{
	LCD_Writ_Bus(dat>>16);
	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	LCD_WR_REG(0x2a);//列地址设置
	LCD_WR_DATA8(x1>>8);
	LCD_WR_DATA8(x1);
	LCD_WR_DATA8(x2>>8);
	LCD_WR_DATA8(x2);
	LCD_WR_REG(0x2b);//行地址设置
	LCD_WR_DATA8(y1>>8);
	LCD_WR_DATA8(y1);
	LCD_WR_DATA8(y2>>8);
	LCD_WR_DATA8(y2);
	LCD_WR_REG(0x2c);//储存器写
}

void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	
	LCD_RES_Clr();//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
//	//************* Start Initial Sequence **********//
//	以下为ILI9488驱动初始化代码
//	
//	//************* Start Initial Sequence **********// 	
	LCD_BLK_Set();//打开背光
	delay_ms(100);
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA8(0x0F); 
	LCD_WR_DATA8(0x13); 
	LCD_WR_DATA8(0x1D); 
	LCD_WR_DATA8(0x09); 
	LCD_WR_DATA8(0x18); 
	LCD_WR_DATA8(0x0A); 
	LCD_WR_DATA8(0x43); 
	LCD_WR_DATA8(0x66); 
	LCD_WR_DATA8(0x4F); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x0F); 
	LCD_WR_DATA8(0x0E); 
	LCD_WR_DATA8(0x18); 
	LCD_WR_DATA8(0x1A); 
	LCD_WR_DATA8(0x03);  
	
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA8(0x0F); 
	LCD_WR_DATA8(0x1A); 
	LCD_WR_DATA8(0x1D); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_DATA8(0x0F); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_DATA8(0x31); 
	LCD_WR_DATA8(0x14); 
	LCD_WR_DATA8(0x43); 
	LCD_WR_DATA8(0x03); 
	LCD_WR_DATA8(0x0D); 
	LCD_WR_DATA8(0x0C); 
	LCD_WR_DATA8(0x26); 
	LCD_WR_DATA8(0x29); 
	LCD_WR_DATA8(0x00); 
	
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA8(0x14); 
	LCD_WR_DATA8(0x0E); 
	
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA8(0x43); 
	
	LCD_WR_REG(0xC5); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x36); 
	LCD_WR_DATA8(0x80); 
	
	LCD_WR_REG(0x36);    // Memory Access Control 
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x48);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0x88);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x28);
	else LCD_WR_DATA8(0xE8);

	
	LCD_WR_REG(0x3A); //Interface Mode Control，此处ILI9486为0X55
	LCD_WR_DATA8(0x66);
		
	LCD_WR_REG(0XB0);  //Interface Mode Control  
	LCD_WR_DATA8(0x00); 
	LCD_WR_REG(0xB1);   //Frame rate 70HZ  
	LCD_WR_DATA8(0xB0); 
	LCD_WR_DATA8(0x11); 
	LCD_WR_REG(0xB4); 
	LCD_WR_DATA8(0x02);   
	LCD_WR_REG(0xB6); //RGB/MCU Interface Control
	LCD_WR_DATA8(0x02); 
	LCD_WR_DATA8(0x02); 
	
	LCD_WR_REG(0xE9); 
	LCD_WR_DATA8(0x00);
	
	LCD_WR_REG(0XF7);    
	LCD_WR_DATA8(0xA9); 
	LCD_WR_DATA8(0x51); 
	LCD_WR_DATA8(0x2C); 
	LCD_WR_DATA8(0x82);
	
	LCD_WR_REG(0x11); 
	delay_ms(120); 
	LCD_WR_REG(0x29); 
} 





