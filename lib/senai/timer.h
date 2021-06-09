// Interface do Módulo Timer.c
#ifndef TIMER_H
#define TIMER_H

//id unico do modulo para passar nos eventos previstos em base.h
#define TIMER0_ID_MODULO 15

// Variáveis disponíveis no Módulo
extern unsigned char contador_1ms;
extern unsigned char contador_100ms;

// Funções disponíveis no Módulo
void timer0_iniciar(TBaseEventos *mpbeBaseEventos);
void timer0_int(void);
void delay_ms(unsigned char tempo);



#endif
