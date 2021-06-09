#include <htc.h>
#include "motor.h"

#define _XTAL_FREQ 4000000

#define POLO_1A RC0
#define POLO_1B RC1
#define POLO_2A RC2
#define POLO_2B RC4
#define ANGULO_PASSO 15
//  A -|     |-A
//  1 -| ( ) |-2
//  B -|     |-B

unsigned char motor_etapa;
unsigned int motor_posicao;

void motor_iniciar(TBaseEventos *mpbeBaseEventos){

// Configura portas 1A, 1B,2A,2B como saida 
  PORTC = 0;
  TRISC &= 0b11101000; // apenas bits 0,1,2 e 4 são resetados(0)
// Coloca portas em 0
  POLO_1A = 0;
  POLO_1B = 0;
  POLO_2A = 0;
  POLO_2B = 0;
  motor_etapa = 1;
  motor_posicao = 0;
}


void motor_movimentar(unsigned char sentido,unsigned char passos){


   while(passos > 0){	
	   if (sentido == HORARIO){ 
		  // Aciona portas na sequencia 1A,2A,1B,2B
	      switch(motor_etapa){
		      case 0:{
			      POLO_1A = 1;
			  	  POLO_1B = 0;
			      POLO_2A = 1;
			      POLO_2B = 0;
			      __delay_ms(3);
		         motor_etapa++;
		         break;
		      };
		      // Etapa 1
		      case 1:{
				  POLO_1A = 0;
			  	  POLO_1B = 0;
			      POLO_2A = 1;
			      POLO_2B = 1;
			      __delay_ms(3);
		         motor_etapa++;
			      break;
		      };
		      // Etapa 2
		      case 2:{
				  POLO_1A = 0;
			  	  POLO_1B = 1;
			      POLO_2A = 0;
			      POLO_2B = 1;
			      __delay_ms(3);
		         motor_etapa++;
		         break;
		      };
			      // Etapa 3
		      case 3:{
				  POLO_1A = 1;
			  	  POLO_1B = 1;
			      POLO_2A = 0;
			      POLO_2B = 0;
			      __delay_ms(3);   
		          motor_etapa=0;
			      break;
		      };
		      default:{
		         motor_etapa=0;
		        break;
		      }
          }    
	      motor_posicao=motor_posicao+ANGULO_PASSO;
	      if (motor_posicao >= 360) motor_posicao = motor_posicao%360;
	
	   }
	   else{
		  // Aciona portas na sequencia 2B,1B,2A,1A
	      switch(motor_etapa){
		      case 0:{
			      POLO_1A = 0;
			  	  POLO_1B = 1;
			      POLO_2A = 0;
			      POLO_2B = 1;
			      __delay_ms(3);
		         motor_etapa=3;
		         break;
		      };
		      // Etapa 3
		      case 3:{
				  POLO_1A = 0;
			  	  POLO_1B = 0;
			      POLO_2A = 1;
			      POLO_2B = 1;
			      __delay_ms(3);
		         motor_etapa=2;
			      break;
		      };
		      // Etapa 2
		      case 2:{
				  POLO_1A = 1;
			  	  POLO_1B = 0;
			      POLO_2A = 1;
			      POLO_2B = 0;
			      __delay_ms(3);
		         motor_etapa=1;
		         break;
		      };
		      case 1:{
				  POLO_1A = 1;
			  	  POLO_1B = 1;
			      POLO_2A = 0;
			      POLO_2B = 0;
			      __delay_ms(3);   
		          motor_etapa=0;
			      break;
		      };
		      default:{
		         motor_etapa=0;
		        break;
		      }
          }    
	      if (motor_posicao <= ANGULO_PASSO) motor_posicao += 360;
	       motor_posicao=motor_posicao-ANGULO_PASSO;
	   }	
       passos--; // Decrementa numero de passos
      __delay_ms(150);   
   }
}




