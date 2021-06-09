// Arquivo header do m�dulo display.c
#ifndef DISPLAY_H
#define DISPLAY_H

//id unico do modulo para passar nos eventos previstos em base.h
#define DISPLAY_ID_MODULO 6


// Fun��es dispon�veis no m�dulo

void display_iniciar(TBaseEventos *mpbeBaseEventos);			 // Inicia a configura��o da porta do display	
void display_mostrar(unsigned char numero);     // Mostra um numero no display

#endif
