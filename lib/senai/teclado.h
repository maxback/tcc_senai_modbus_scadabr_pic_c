#ifndef TECLADO_H
#define TECLADO_H

//id unico do modulo para passar nos eventos previstos em base.h
#define TECLADO_ID_MODULO 14

// M�dulos incluidos
#define NENHUMA_TECLA 0xFF

// Vari�veis e estruturas dispon�veis do m�dulo 
// ex: extern unsigned char valor;

// Fun��es dispon�veis no m�dulo
void teclado_iniciar(TBaseEventos *mpbeBaseEventos);
unsigned char teclado_ler(void);
char teclado_getch(void);
char teclado_getche(void);
void teclado_scanf(char * texto);
void teclado_scanf_num(unsigned char * valor);


#endif
