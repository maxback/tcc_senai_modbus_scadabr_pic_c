// Implementação - Módulo Serial.c
// Módulos Incluídos
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
#include <p18cxxx.h>	// inclui as definições dos ios do pic
#else
#include <htc.h>
#endif
#include "..\lib\base\base.h"
#include "serial.h"

// Variáveis do Módulo
unsigned char flag_rx;
unsigned char byte_recebido;
//indica que esta usando interrupcao de envio de byte
unsigned char gucUsarIntTx;

//*************** Funções do Módulo *********************

// Inicia serial
#ifdef nada
void serial_iniciar(TBaseEventos *mpbeBaseEventos){

	//RC6/TX COMO SAIDA
//	TRISCbits.TRISC6 = 0;
	//RC7/RX COMO ENTRADA
//	TRISCbits.TRISC7 = 1;
/*
  SPBRG = 129;	 // Configura velocidade da serial 9600 a 20 MHz	

  TXSTAbits.BRGH = 1;
  BAUDCONbits.BRG16 = 0;		
  TXSTAbits.SYNC = 0;

  RCSTAbits.SPEN = 1;

  //PIE1bits.RCIE = 0; //1 - para int. de recepcao

  TXSTAbits.TXEN = 1;

  //RCSTAbits.CREN = 1;


  //para interrrupcoes
  INTCONbits.PEIE = 1;
  INTCONbits.GIE = 1;


  TRISCbits.TRISC7 = 1;
  TRISCbits.TRISC6 = 0;	

  //teste max(jah eh default em 0)
  
*/

  //ale mde alta velociadade e mmodo de 16 bits
  //SPBRGH = (259 >> 8);
  //SPBRG = 259;	 // Configura velocidade da serial 9600 a 20 MHz		

  


              TRISC  |= (1 << 6); // Seta bit 6 do trisc
             TRISC  |= (1 << 7); // Seta bit 7 do trisc
	SPBRG = 129;	 // Configura velocidade da serial 9600 a 20 MHz	
//	BRGH = 1; 	//Seleciona modo de alta velocidade
//	SYNC = 0;               //Seleciona modo assincrono
//	TXEN = 1; 	//Ativa transmissão 
    TXSTA = 0b00100100; // Ativa de uma vez TXEN e BRGH (SYNC=0)
//  CREN = 1; 	//Habilita Recepção Contínua
//	SPEN = 1; 	//Ativa a serial(pinos TX e RX)
	RCSTA = 0b10010000; // Ativa de uma vez SPEN e CREN
//	RCIE =  1;             // Ativar interrupção serial quando um byte for recebido`

//	PIE1bits.RCIE = 1;

//	TXIE =  1;             // Ativar interrupção serial no final da transmissão	
//	GIE = 1; 		// Habilita interrupções globais
//	INTCONbits.GIE = 1;

    	flag_rx = 0;	// flag de bytes recebidos
}
#endif

//inicia recebendo o valor de velocidade e config. para velocidades altas (0-lentas, 1- altas)
void serial_iniciarEx(unsigned char mucValorSPBRG, unsigned char mucUsaVelAlta, 
  unsigned char mucUsarIntRx, unsigned char mucUsarIntTx, TBaseEventos *mpbeBaseEventos)
{ 



              TRISC  |= (1 << 6); // Seta bit 6 do trisc
             TRISC  |= (1 << 7); // Seta bit 7 do trisc
	SPBRG = mucValorSPBRG; //129;	 // Configura velocidade da serial 9600 a 20 MHz	
//	BRGH = 1; 	//Seleciona modo de alta velocidade
//	SYNC = 0;               //Seleciona modo assincrono
//	TXEN = 1; 	//Ativa transmissão

	if(mucUsaVelAlta)
	{
      TXSTA = 0b00100100; // Ativa de uma vez TXEN e BRGH (SYNC=0)
	}
	else
	{
    //teste - deixa pra habilitar  TXEN depois
	  TXSTA = 0b00100000; // Ativa de uma vez TXEN e desativa BRGH (SYNC=0)	
	}	
	
//  CREN = 1; 	//Habilita Recepção Contínua
//	SPEN = 1; 	//Ativa a serial(pinos TX e RX)
	RCSTA = 0b10010000; // Ativa de uma vez SPEN e CREN

	//	TXIE =  1;             // Ativar interrupção serial no final da transmissão	

  //esta travando, por algum motivo. Seta apenas propriedade do modulo
  gucUsarIntTx = mucUsarIntTx;

	if(mucUsarIntRx || mucUsarIntTx)
	{

		RCONbits.IPEN=1; // Interrupt Priority Enable bit

		if(mucUsarIntRx)
		{
      PIE1bits.RCIE = 1; // Ativar interrupção serial quando um byte for recebido
      IPR1bits.RCIP = 0; // poe em prioridade baixa para pegar manipilador jah implem. na fincao mais
		}
		else
		{
		  PIE1bits.RCIE = 0;
    }
    
    //esta travando, por algum motivo. Seta apenas propriedade do modulo
//		if(mucUsarIntTx)
//		{
//      PIE1bits.TXIE = 1; // Ativar interrupção serial quando um byte for transmitido
//      IPR1bits.TXIP = 0; // poe em prioridade baixa para pegar manipilador jah implem. na fincao mais
//		}
//		else
//		{
		  PIE1bits.TXIE = 0;
//    }
		
  	
  INTCONbits.GIEL=1; // habilita todas as ints biaxas
  
//	TXEN = 1; 	//Ativa transmissão e bit e velocidade alta se for o caso
/*
	if(mucUsaVelAlta)
	{
      TXSTA = 0b00100100; // Ativa de uma vez TXEN e BRGH (SYNC=0)
	}
	else
	{
    //teste - deixa pra habilitar  TXEN depois
	  TXSTA = 0b00100000; // Ativa de uma vez TXEN e desativa BRGH (SYNC=0)	
	}	
*/  

/*
	    //RCIE =  1;             // Ativar interrupção serial quando um byte for recebido`
		PIE1bits.RCIE = 1;

	    //	GIE = 1; 		// Habilita interrupções de perifericos globais
	    //	GIE = 1; 		// Habilita interrupções globais
		INTCONbits.PEIE = 1;
		INTCONbits.GIE = 1;
*/
	}
   	flag_rx = 0;	// flag de bytes recebidos
}

//TCC: retorna valor da proriedade
unsigned char serial_ucUsarIntTx(void)
{
  return gucUsarIntTx; 
}


// Tranmite um dado pela serial
void serial_enviar(unsigned char dado){

#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F e MCU:
   while(!TXSTAbits.TRMT); // aguarda flag de transmissão          
   //while(!PIR1bits.TXIF);	// aguarda flag de recepção
#else
   while(!TXIF); // aguarda flag de transmissão          
#endif
   TXREG = dado;
}

// Recebe dado pela serial (sem interrupção)
/*
unsigned char serial_receber(void){
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F e MCU:
   while(!PIR1bits.RCIF);	// aguarda flag de recepção
#else
   while(!RCIF);	// aguarda flag de recepção
#endif
   return(RCREG);
}

// Interrupção serial (caso seja usada)
void serial_int(void){
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F e MCU:
   if (PIR1bits.RCIF){ 		 // Verifica se um dado foi recebido
      flag_rx = 1;                    // Sinaliza que um dado foi recebido
      byte_recebido = RCREG;   // Salva dado recebido
	//zera flag
	//PIR1bits.RCIF = 0;	
   }		
#else

   if (RCIF){ 		 // Verifica se um dado foi recebido
      flag_rx = 1;                    // Sinaliza que um dado foi recebido
      byte_recebido = RCREG;   // Salva dado recebido
   }		
#endif
// if (TXIF) TXIF = 0;
}
*/

// Envia uma string pela serial (similar ao printf)
void serial_printf(const char * texto){
char ch;

    ch = *texto++;
    while(ch != '\0'){
       serial_enviar(ch);
       ch = *texto++;	
   }
}	

// Envia uma string pela serial (similar ao printf)
/*
void serial_scanf(char * texto){
char ch;

    ch = serial_receber();   // recebe o primeira tecla
    while(ch != 13){	           // continua se tecla diferente de Enter
         *texto=ch;	           // salva caracter digitado
          serial_enviar(ch);    // ecoa caracter na tela
          texto++;	           // prepara para o próximo caracter
         ch = serial_receber();   // recebe o primeira tecla
   }
   *texto='\0';	          // final da string
}	
*/


/*
void serial_scanf_num(unsigned char * valor){
char ch;
unsigned int leitura;

    leitura = 0;            
    ch = serial_receber();   // recebe o primeira tecla
    while(ch != '#'){	    // continua se tecla diferente de Enter (#)
         serial_enviar(ch);    	// ecoa caracter na tela
         leitura = leitura*10+(ch-'0');	 // calcula valor digitado (valor atual (deslocado na base 10) + as unidades
         ch = serial_receber();   // recebe o próxima tecla
   }
   *valor = leitura;      // retorna a leitura final do valor difgitado

}
*/