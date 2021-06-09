/*
	Autor: Rodrigo  P A 
	visite meu site: www.kitmcu.com.br
	Prototipo das funções do arquivo que controla o LCD
*/

void lcd_init(void);
void lcd_putc(char c);
void lcd_puts(char *s);
void lcd_led(int8 valor);
//adicionados ao módulo original
void lcd_clear(void);
void lcd_cursor_on(void);
void lcd_cursor_off(void);

void lcd_savexy(void);
void lcd_restorexy(void);
void lcd_gotoxy(unsigned char x,unsigned char y);
void lcd_incxy(char incx, char incy);
// Imprime numero no display
void lcd_printf_int(int numero);
