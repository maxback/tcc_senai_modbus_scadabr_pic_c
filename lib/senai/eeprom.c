// Defini��es no Modulo
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas op��es do arquivo no proceto //////////

#include <p18cxxx.h>	// inclui as defini��es dos ios do pic

#else

#include <htc.h>

#endif

#include "..\lib\base\base.h"

#include "eeprom.h"


unsigned char endereco_atual;

void eeprom_iniciar(TBaseEventos *mpbeBaseEventos){

	endereco_atual = 0;
}

// Le dado da mem�ria e2prom
unsigned char eeprom_ler(unsigned char endereco){

#ifdef PROJETO_PI

  //EECON1bits.EEPGD = 0; 
  //EECON1bits.CFGS = 0; 
  EECON1 = 0x00;	
	
  EEADR = endereco;

  EECON1bits.RD = 1; 

  EECON1bits.RD = 0; 

  return(EEDATA); 

#else

  EECON1 = 0x00;      // Acessa mem�ria de dados
  EEADR =  endereco; // Define o endere�o da leitura
  EECON1 = 0x01; // Ativa a leitura
  EECON1 = 0x00; 
  return(EEDATA);       // Retorna dado lido
#endif
}

// Escreve um byte na mem�ria e2prom
void eeprom_escrever(unsigned char endereco,unsigned char dado){

#ifdef PROJETO_PI
  ///// config para projeto PI 2009 usando KIT PIC 18F d MCU
  EECON1bits.EEPGD = 0; 

  EEADR = endereco;	 
  EEDATA = dado; 

  INTCONbits.GIE = 0;

  EECON1bits.WREN = 1;	

  EECON2 = 0x55; 
  EECON2 = 0xAA; 

  EECON1bits.WR = 1;	 //Starts writing 

  while(EECON1bits.WR);  // aguarda final grava��o
  
  EECON1bits.WREN = 0;   //WREN = 0;

  INTCONbits.GIE = 1;    // Religa interrup��es

#else
   //como fo ifeito no hlc
   EEPGD = 0;      // Acessa mem�ria de dados
   EEADR =  endereco;
   EEDATA = dado;
   GIE = 0;	// Desliga interrup��es
   EECON1 = 0x04; //WREN = 1;    // Permite grava��o
   EECON2=0x55;                  // Sequencia exigida pelo fabricante
   EECON2=0xAA;
   EECON1 = 0x06; // WREN = 1 , WR = 1;
   while(WR);  // aguarda final grava��o
   EECON1 = 0x00; //WREN = 0;
   GIE = 1;         // Religa interrup��es
   endereco_atual++;
#endif

}

// Envia uma string pela serial (similar ao printf)
void eeprom_printf(const char * texto){
char ch;

	ch = *texto++;
	while(ch != '\0'){
		eeprom_escrever(endereco_atual,ch);
		ch = *texto++;	
	}
}

void eeprom_escreverBuffer(unsigned char endereco,unsigned char *mpucBuffer, unsigned char mucTamanho)
{
	while(mucTamanho)
	{
		eeprom_escrever(endereco, *mpucBuffer);

		mpucBuffer++;
		endereco++;
		mucTamanho--;
	}
}

void eeprom_print_num(unsigned int valor){
        
	eeprom_escrever(endereco_atual,valor/100+'0');	// Imprimir (centena)   
	valor = valor%100;
	eeprom_escrever(endereco_atual,valor/10+'0');		// Imprimir (dezena)
	eeprom_escrever(endereco_atual,valor%10+'0');     // Imprimir (unidade);
}
