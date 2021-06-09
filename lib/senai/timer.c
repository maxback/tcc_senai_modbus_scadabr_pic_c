// Implementação - Módulo Timer.c
// Módulos Incluídos
// Módulos Incluídos
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
#include <p18cxxx.h>	// inclui as definições dos ios do pic
#else
#include <htc.h>
#endif

#include "timer.h"


// Definições do Módulo
#define TEMPO 1000		// Tempo desejado em us (1000 = 1ms)
#define CLOCK 4000000		// Clock do sistema
#define PRESCALER 256
#define FOSC CLOCK/4/PRESCALER	// Fosc
#define TIMER_AUX ((FOSC*TEMPO)/1000000) // Calcular valor e divide por 1000 000(porque tempo está em us(E-6)
#define TIMER_MAX 256 // Valor máximo do timer (MODO 2) 16bits	
#define TIMER0 (TIMER_MAX-TIMER_AUX) // Valor do timer (65536-contagens)

// Variáveis do Módulo
unsigned char contador_1ms;
unsigned char contador_100ms;


// ******** Funções do Módulo ***********

// Inicia Timer
void timer0_iniciar(TBaseEventos *mpbeBaseEventos){
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU
					// Configuração do Timer
   T0CONbits.T0CS = 0;		// Fonte de Clock Interno
   T0CONbits.PSA = 0;			// Usa o prescaler
   T0CONbits.T0PS0 = 1; 		 
   T0CONbits.T0PS1 = 1; 		// Prescaler = 256
   T0CONbits.T0PS2 = 1;	 	
   //poe valor de recarga
   TMR0H = (TIMER0>>8)&0x00FF;
   TMR0L = TIMER0&0x00FF;

   T0CONbits.TMR0ON = 1;
   INTCONbits.T0IF = 0;		// Habilita contagem
   INTCONbits.T0IE = 1;		// Habilita interrupção do timer
   INTCONbits.GIE = 1; 		// Habilita uso de interrupções 	
  
#else

					// Configuração do Timer
   T0CS = 0;		// Fonte de Clock Interno
   PSA = 0;			// Usa o prescaler
   PS0 = 1; 		 
   PS1 = 1; 		// Prescaler = 256
   PS2 = 1;	 	
   TMR0 = TIMER0;
   T0IF = 0;		// Habilita contagem
   T0IE = 1;		// Habilita interrupção do timer
   GIE = 1; 		// Habilita uso de interrupções 	
#endif

   contador_1ms = 0;
   contador_100ms = 0;

}

// Rotina de interrupção do Timer
void timer0_int(void){

#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU
   //poe valor de recarga
   TMR0H = (TIMER0>>8)&0x00FF;
   TMR0L = TIMER0&0x00FF;
   //Zera Flag de contagem do timer para proxima interrupção
   INTCONbits.T0IF = 0;

#else
   TMR0 = TIMER0;			// Recarrera a contagem do timer
   T0IF = 0;  				//Zera Flag de contagem do timer para proxima interrupção
#endif

   contador_1ms++;			// Contagem de 1ms
   if (contador_1ms >= 100){
      contador_100ms++;    // Contagem de 100ms
      contador_1ms=0;
   }
}

// Rotina de Delay 
void delay_ms(unsigned char tempo){

   contador_1ms = 0;
   while (contador_1ms < tempo);  // Aguarda a contagem do timer até o valor do tempo

}
