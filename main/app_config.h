/*
 *		Global config file
 *			- Pins
 *			- Frequinces
 *			- FB size
 *			- etc.
 * TODO: Move to KConfig.
 */

#pragma once

#define DISP_WIDTH   		320
#define DISP_HEIGHT  		480
#define DISP_BUS_WIDTH 		16
#define BITS_PER_PIXEL		DISP_BUS_WIDTH
#define FB_SIZE 			((DISP_WIDTH * DISP_HEIGHT) / 10)

#define LCD_DB0   			4
#define LCD_DB1   			5
#define LCD_DB2   			6
#define LCD_DB3   			7
#define LCD_DB4   			8
#define LCD_DB5   			9
#define LCD_DB6   			10
#define LCD_DB7   			11
#define LCD_DB8   			12
#define LCD_DB9   			13
#define LCD_DB10  			14
#define LCD_DB11  			15
#define LCD_DB12  			16
#define LCD_DB13  			17
#define LCD_DB14  			18
#define LCD_DB15  			19

#define LCD_RS    			35
#define LCD_WR    			20
#define LCD_CS    			-1
#define LCD_RST   			21

#define LCD_DATA_BUS_MASK 	(0xFFFF << 4)

#define LCD_FREQUENCY_HZ 	10000000

#define DMA_BURST_SIZE		64
