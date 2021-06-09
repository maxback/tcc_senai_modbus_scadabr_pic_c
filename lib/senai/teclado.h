#ifndef TECLADO_H
#define TECLADO_H

//id unico do modulo para passar nos eventos previstos em base.h
#define TECLADO_ID_MODULO 14

// Módulos incluidos
#define NENHUMA_TECLA 0xFF

// Variáveis e estruturas disponíveis do módulo 
// ex: extern unsigned char valor;

// Funções disponíveis no módulo
void teclado_iniciar(TBaseEventos *mpbeBaseEventos);
unsigned char teclado_ler(void);
char teclado_getch(void);
char teclado_getche(void);
void teclado_scanf(char * texto);
void teclado_scanf_num(unsigned char * valor);


#endif
