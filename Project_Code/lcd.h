#ifndef _LCD_H
#define _LCD_H

void lcd_init(void);
void lcd_clear(void);
void lcd_movetoline(int);
void lcd_stringout( char str[]);
void lcd_writecommand(unsigned char);
void lcd_writedata(unsigned char);
void lcd_nextLine(void);

#endif