// Modulo de Controle do Display de 7segmentos
#include "display.h"
#include <htc.h>


// Inicia a configuração da porta do display
void display_iniciar(TBaseEventos *mpbeBaseEventos){

   TRISB = 1;  // Configura porta Trisb como saída(menos primeiro bit)
   PORTB = 0;   // Apaga todos os digitos do display

}


// Mostra um número no display
void display_mostrar(unsigned char numero){
unsigned char valor;

   switch(numero){  // Verifica qual número tem que ser exibido no display
      case 0: valor = 0x3F;
              break;
      case 1: valor = 0x30;	// Liga os segmentos para mostrar o numero 1
              break;	
      case 2: valor = 0x5B;	// Liga os segmentos para mostrar o 2..
              break;
      case 3: valor = 0x79;
              break; 
      case 4: valor = 0x74;
	      break;
      case 5: valor = 0x6D;
              break;
      case 6: valor = 0x6F;
              break;
      case 7: valor = 0x38;
              break;
      case 8: valor = 0x7F;
	      break;
      case 9: valor = 0x7D;
              break;
      default:valor = 0x40;   // Numero inválido, mostra um traco
              break;
   };
   PORTB = (valor << 1);
}

