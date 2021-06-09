/*
	timer.c
	Autor: Rodrigo P A
	visite meu site: www.kitmcu.com.br
	
	Este arquivo configura o timer0 do pic para gerar uma base
	de tempo para ser usada na maioria dos programas
*/

#include <p18cxxx.h>
#include <timers.h>

#define KITMCU_TIMER_C
#include "utils.h"

unsigned int tempo=46875; // 1 second 
unsigned char ledstatus=1;

/*
	Configura o timer para gerar uma interrupção a cada 50uS
*/
#pragma code
void Iniciar_Timer(void)
{
	INTCON=0x00; // Todas interrupções desabilitadas // <en> All interrupts disabled
	
	RCONbits.IPEN=1; // Interrupt Priority Enable bit
	INTCONbits.TMR0IE=1; // TMR0 Overflow Interrupt Enable bit
	INTCON2bits.TMR0IP=1; // TMR0 Overflow Interrupt Priority bit (1=HIGH)
	
	OpenTimer0(T0_16BIT & T0_SOURCE_INT & T0_PS_1_256 ); // Setando: T0CON=0b10110111; // <en> Setting: T0CON=0b10110111;
	WriteTimer0(65536-tempo);
	
	INTCONbits.GIEH=1; // Global Interrupt Enable bit	
}	

#pragma interrupt high_int
void high_int (void)
{

	if(INTCONbits.TMR0IF) // A interrupção foi desencadeada pelo "overflow" do TMR0 ? // <en> Was it a TMR0 overflow ?
	{	
		if(ledstatus)
		{
			PORTD=4;
			ledstatus=0;
		}
		else
		{
			PORTD=8;
			ledstatus=1;
		}
	WriteTimer0(65536-tempo); // Restart TMR0
	INTCONbits.TMR0IF = 0; // Clear TMR0 Overflow flag
	}
}
