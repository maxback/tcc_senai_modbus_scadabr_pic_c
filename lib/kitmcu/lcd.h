/*
	Autor: Rodrigo  P A 
	visite meu site: www.kitmcu.com.br
	Prototipo das funções do arquivo que controla o LCD
	
	- Alterado por Max Back
*/

#ifndef LCD_H
#define LCD_H

//id unico do modulo para passar nos eventos previstos em base.h
#define LCD_ID_MODULO 1


//espelho dos bytes nos lcs 2x16
#ifndef LCD_C
extern
#endif
unsigned char lcd_ucEspelhoDisplay[32];

#ifndef LCD_C
extern
#endif
void lcd_init(TBaseEventos *mpbeBaseEventos);


#ifndef LCD_C
extern
#endif
void lcd_putc(char c);

#ifndef LCD_C
extern
#endif
void lcd_puts(char *s);

//versao que  recebe uma string constante para evetar codigo extra
#ifndef LCD_C
extern
#endif
void lcd_puts_const(auto const rom char *s);

/*
#ifndef LCD_C
extern
#endif
void lcd_led(int8 valor);
*/
//adicionados ao módulo original
#ifndef LCD_C
extern
#endif
void lcd_clear(void);

#ifndef LCD_C
extern
#endif
void lcd_cursor_on(void);

#ifndef LCD_C
extern
#endif
void lcd_cursor_off(void);


#ifndef LCD_C
extern
#endif
void lcd_savexy(void);

#ifndef LCD_C
extern
#endif
void lcd_restorexy(void);

#ifndef LCD_C
extern
#endif
void lcd_gotoxy(unsigned char x,unsigned char y);

#ifndef LCD_C
extern
#endif
void lcd_incxy(char incx, char incy);

// Imprime numero no display
/*
#ifndef LCD_C
extern
#endif
void lcd_printf_int(int numero);
*/
#endif
