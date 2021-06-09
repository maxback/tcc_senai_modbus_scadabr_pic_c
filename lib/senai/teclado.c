// Módulo Teclado.c
// Módulos Incluídos
#include <htc.h>
#include "teclado.h"
#include "lcd.h"

// Definições do Módulo (defines)
#define COLUNA_1  RA1
#define COLUNA_2  RA2
#define COLUNA_3  RA3
#define LINHA_1 	 RB4
#define LINHA_2     RB5
#define LINHA_3     RB6
#define LINHA_4     RB7

// Variáveis do Módulo
// ex: unsigned char tempo;

// 
void teclado_iniciar(TBaseEventos *mpbeBaseEventos){

   ADCON1 = 0x0E;	         // Desabilta ADS (Apenas AN0 habilitado)
   TRISA &= 0b11110001;  // Força as portas RA1,RA2,RA3 como saída
   TRISB |= 0b11110000;  // Força as portas RB4,RB5,RB6,RB7 como entradas
   STATUS = 0;
   COLUNA_1 = 1;
   COLUNA_2 = 1;
   COLUNA_3 = 1;
   RBPU = 0;                 // Habilita pull-ups internos do PIC (na PORTB)
}	

// ************** Funções do Módulo ***************
unsigned char teclado_ler(void){
   
   STATUS = 0;	  // Bug do compilador (força banco 0)
   COLUNA_1 = 0;  // Ativa a primeira coluna
   COLUNA_2 = 1;
   COLUNA_3 = 1;
   if (LINHA_1 == 0) return('1'); // Verifica qual das linhas foi ativa (nivel baixo)
   if (LINHA_2 == 0) return('4');
   if (LINHA_3 == 0) return('7');
   if (LINHA_4 == 0) return('*');
   COLUNA_1 = 1;  
   COLUNA_2 = 0;  // Ativa a segunda coluna
   COLUNA_3 = 1;
   if (LINHA_1 == 0) return('2');
   if (LINHA_2 == 0) return('5');
   if (LINHA_3 == 0) return('8');
   if (LINHA_4 == 0) return('0');
   COLUNA_1 = 1;  
   COLUNA_2 = 1;  
   COLUNA_3 = 0;  // Ativa a terceira coluna
   if (LINHA_1 == 0) return('3');
   if (LINHA_2 == 0) return('6');
   if (LINHA_3 == 0) return('9');
   if (LINHA_4 == 0) return('#');
   COLUNA_1 = 1;  
   COLUNA_2 = 1;  // Desativa todas as colunas
   COLUNA_3 = 1;  
   return(NENHUMA_TECLA);
}

// Aguarda uma tecla pessionada
char teclado_getch(void){
unsigned char tecla;
   do{
     tecla = teclado_ler();
   }
   while (tecla == NENHUMA_TECLA);  // Verifica se a tecla foi pressionada
   while (teclado_ler() != NENHUMA_TECLA); // Espera tecla ser solta
   return(tecla);
}

// Le um tecla pressionada com eco (no lcd)
char teclado_getche(void){
unsigned char tecla;

	tecla = teclado_getch();
	lcd_dado(tecla);
	return(tecla);	
}


void teclado_scanf_num(unsigned char * valor){
char ch;
unsigned int leitura;

    leitura = 0;            
    ch = teclado_getch();   // recebe o primeira tecla
    while(ch != '#'){	    // continua se tecla diferente de Enter (#)
         lcd_dado(ch);    	// ecoa caracter na tela
         leitura = leitura*10+(ch-'0');	 // calcula valor digitado (valor atual (deslocado na base 10) + as unidades
         ch = teclado_getch();   // recebe o próxima tecla
   }
   *valor = leitura;      // retorna a leitura final do valor difgitado

}

//  Le uma variavel pelo teclado (texto)
void teclado_scanf(char * texto){
char ch;

    ch = teclado_getch();   // recebe o primeira tecla
    while(ch != '#'){	           // continua se tecla diferente de Enter
         *texto=ch;	           // salva caracter digitado
          lcd_dado(ch);    // ecoa caracter na tela
          texto++;	           // prepara para o próximo caracter
         ch = teclado_getch();   // recebe a proxima tecla
   }
   *texto='\0';	          // final da string
}	





// Final do Módulo

