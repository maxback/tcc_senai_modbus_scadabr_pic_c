// Defini��es no Modulo
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas op��es do arquivo no proceto //////////

#include <p18cxxx.h>	// inclui as defini��es dos ios do pic

#else

#include <htc.h>

#endif

#include "ad.h"
#include "bits.h"

// Inicia o AD
void ad_iniciar(TBaseEventos *mpbeBaseEventos){
   //configura como entrada os 3 primeiros pinos do AD
   set_bit(TRISA, 0);
   set_bit(TRISA, 1);
   set_bit(TRISA, 2);
   
   ADCON1 = 0b00001110;
   ADCON0 = 0b11000001;

}

// Espera pelo tempo de aquisi��o +-5us
void delay_ad(void){
unsigned char i;

  for (i=0;i< 30;i++);

}

// Ler o AD
unsigned char ad_ler(void){
   set_bit(TRISA, 0);
   ADCON1 = 0b00001110;
   ADCON0 = 0b11000001;
   delay_ad();
   GODONE = 1;
   while(GODONE);
   return(ADRESH);
}

