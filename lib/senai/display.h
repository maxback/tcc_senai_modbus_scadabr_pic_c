// Arquivo header do módulo display.c
#ifndef DISPLAY_H
#define DISPLAY_H

//id unico do modulo para passar nos eventos previstos em base.h
#define DISPLAY_ID_MODULO 6


// Funções disponíveis no módulo

void display_iniciar(TBaseEventos *mpbeBaseEventos);			 // Inicia a configuração da porta do display	
void display_mostrar(unsigned char numero);     // Mostra um numero no display

#endif
