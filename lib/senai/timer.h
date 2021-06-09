// Interface do M�dulo Timer.c
#ifndef TIMER_H
#define TIMER_H

//id unico do modulo para passar nos eventos previstos em base.h
#define TIMER0_ID_MODULO 15

// Vari�veis dispon�veis no M�dulo
extern unsigned char contador_1ms;
extern unsigned char contador_100ms;

// Fun��es dispon�veis no M�dulo
void timer0_iniciar(TBaseEventos *mpbeBaseEventos);
void timer0_int(void);
void delay_ms(unsigned char tempo);



#endif
