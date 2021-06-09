#ifndef LCD_H
#define LCD_H

//id unico do modulo para passar nos eventos previstos em base.h
#define LCD_ID_MODULO 8


#define A_TIL 240
#define GRAU  241
#define CLOCK  242
#define C_CEDILHA 243
#define RADIOATIVIDADE 244

void lcd_iniciar(TBaseEventos *mpbeBaseEventos);
void lcd_comando(unsigned char comando);
void lcd_dado(unsigned char dado);
void lcd_delay(unsigned char tempo);
void lcd_clear(void);
void lcd_gotoxy(unsigned char x,unsigned char y);
void lcd_posiciona(unsigned char linha,unsigned char coluna);
void lcd_printf(const char * texto);
void lcd_cursor_on(void);
void lcd_cursor_off(void);
void lcd_printf_num(unsigned char numero);
void lcd_desenha(char caracter,unsigned char linha,unsigned char coluna);
void lcd_print_hora(unsigned char hora,unsigned char minuto,unsigned char segundos);
void lcd_print_data(unsigned char dia,unsigned char mes,unsigned char ano);


#endif
