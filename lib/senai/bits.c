#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
#include <p18cxxx.h>	// inclui as definições dos ios do pic
#else
#include "htc.h"
#endif
// Seta um bit de um registrador
void set_bit(unsigned char * REG,unsigned char BIT){

   *REG |= (1 << BIT);
}	

// Seta um bit de um registrador
void clear_bit(unsigned char * REG,unsigned char BIT){

   *REG &= (~(1 << BIT));
}

// Seta um bit de um registrador
unsigned char test_bit(unsigned char * REG,unsigned char BIT){

   if (*REG & (1 << BIT)) return(1);
   else return(0);
}

// Final do Módulo

