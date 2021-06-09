// Modulo LCD.C

#define USA_RW

// Definições no Modulo
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
///// INDICA TAMBEM O USUO DO COMPILADOR c18
#include <p18cxxx.h>	// inclui as definições dos ios do pic

//inclui para temporizações de display
#include <delays.h>

#define LCD_DATA PORTD
//defineo  tris tambem
#define LCD_DATA_TRIS TRISD
// indica se uma os 8 bits
#define LCD_8BITS 0
//se nao usa, indica se usa os 4 mais sig
#define LCD_4BITSMSB 1

//define TRIS da outra porta com os bits de controle
#define LCD_CONTROL_TRIS TRISE

//define pino e indice para os 3 pinos de controle
#define LCD_RS PORTEbits.RE2
#define LCD_RS_INDEX 2

#if defined(USA_RW)
#define LCD_RW PORTEbits.RE1
#define LCD_RW_INDEX 1
#endif
#define LCD_EN PORTEbits.RE0
#define LCD_EN_INDEX 0

#else
// Modulos incluídos
#include <htc.h> 

#define LCD_DATA PORTB
//defineo  tris tambem
#define LCD_DATA_TRIS TRISB
// indica se uma os 8 bits
#define LCD_8BITS 0
//se nao usa, indica se usa os 4 mais sig
#define LCD_4BITSMSB 1

//define TRIS da outra porta com os bits de controle
#define LCD_CONTROL_TRIS TRISC

//define pino e indice para os 3 pinos de controle
#define LCD_RS RC3
#define LCD_RS_INDEX 3

#if defined(USA_RW)
#define LCD_RW RC4
#define LCD_RW_INDEX 4

#endif
#define LCD_EN RC5
#define LCD_EN_INDEX 5


#endif

#include "lcd.h"
//inclui bits para melhor acesso generico ao cfg dos bits
#include "bits.h"

//ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
static TBaseEventos *gpbeBaseEventos = NULL;


// ************** Funções do Módulo ************************


// Envia um comando de 4bit para o LCD (apenas para inicializacao)
void lcd_comando4bit(unsigned char comando){

  if(LCD_4BITSMSB)
  {
    LCD_DATA &= 0x0F; 
    LCD_DATA |= comando<<4;    // Coloca Comando(hi) no barramento	
  }
  else
  { 
	LCD_DATA = comando;    // Coloca Comando(hi) no barramento	
  }
#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 0;		// Ativa comando(RS=0)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)
}	

// Inicia o LCD
void lcd_iniciar(TBaseEventos *mpbeBaseEventos){

  //ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
  gpbeBaseEventos = mpbeBaseEventos;


  // indica se uma os 8 bits
  if(LCD_8BITS)
  {
	LCD_DATA_TRIS = 0x00;
  }
  //senao eh 4 bits - sao os mais sig?
  else if(LCD_4BITSMSB)
  {
	LCD_DATA_TRIS &= 0x0F;			// Configura os  bits  mais sgig como saida (demais mantem)
  }
  //entao sao os 4 menos sig
  else
  {
	LCD_DATA_TRIS &= 0xF0;			// Configura os  bits  menso sgig como saida (demais mantem)

  }



    //LCD_CONTROL_TRIS &= 0b11000111;	// Configura os bits RC 3,4,5 como saídas (demais mantém)
    clear_bit(&LCD_CONTROL_TRIS, LCD_RS_INDEX); //configurou RS como saida
#if defined(USA_RW)
    clear_bit(&LCD_CONTROL_TRIS, LCD_RW_INDEX); //configurou RW como saida
#endif
    clear_bit(&LCD_CONTROL_TRIS, LCD_EN_INDEX); //configurou EN como saida


  //seta todos os bits com 1
  // indica se uma os 8 bits
  if(LCD_8BITS)
  {
	LCD_DATA = 0xFF;
  }
  //senao eh 4 bits - sao os mais sig?
  else if(LCD_4BITSMSB)
  {
	LCD_DATA = 0xF0;			
  }
  //entao sao os 4 menos sig
  else
  {
	LCD_DATA = 0x0F;

  }

	
	LCD_EN = 0;
	LCD_RS = 0;
	lcd_delay(250); 		// espera 2,5ms
	lcd_delay(250); 		// espera 2,5ms
	lcd_comando4bit(0x03);	// Inicia configuração do Display
	lcd_delay(250); 		// espera 2,5ms
	lcd_delay(250); 		// espera 2,5ms
	lcd_comando4bit(0x03);	// Sequencia inicial
	lcd_delay(20);			// espera 200us
	lcd_comando4bit(0x03);	// Sequencia inicial
	lcd_delay(10);			// espera 100us
	lcd_comando4bit(0x02);	// Sequencia inicial	
	lcd_delay(10);			// espera 100us
	lcd_comando(0x28);		// Configura 2 linhas e 4 bits
 	lcd_comando(0x0E);		// Liga LCD e cursor
	lcd_comando(0x06);		// Texto esquerda->direita
	lcd_comando(0x01);		// Limpa LCD
	lcd_comando(0x02);		// Posiciona LCD na primeira linha/coluna
	lcd_delay(250);			// espera mais 2,5ms
}


// Envia um comando para o LCD
void lcd_comando(unsigned char comando){
  
	//LCD_DATA = (comando >> 4);    // Coloca Comando(hi) no barramento	
  //Envia o xomando todo de uam vez um de 4 em 4 na parte mais sig. ou menos, dependendo da configuração
  // indica se uma os 8 bits
  if(LCD_8BITS)
  {
	LCD_DATA = comando;
#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 0;		// Ativa comando(RS=0)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)
	lcd_delay(180);		// Espera 1,8ms
  }
  //senao eh 4 bits - sao os mais sig?
  else 
  {
    if(LCD_4BITSMSB)
    {
      //manda a parte mais significativa do comando nos 4 bits mais sig da porta
	  LCD_DATA = (comando & 0xf0);			
    }
    //entao sao os 4 menos sig
    else
    {
      //manda a parte mais significativa do comando nos 4 bits menos sig da porta
	  LCD_DATA = (comando >> 4);			

    }

#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 0;		// Ativa comando(RS=0)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)

    if(LCD_4BITSMSB)
    {
      //manda a parte menos significativa do comando nos 4 bits mais sig da porta
	  LCD_DATA = (comando << 4);			
    }
    //entao sao os 4 menos sig
    else
    {
      //manda a parte menos significativa do comando nos 4 bits menos sig da porta
	  LCD_DATA = (comando & 0x0f);			

    }
#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 0;		// Ativa comando(RS=0)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)
	lcd_delay(180);		// Espera 1,8ms
  }


/* Codigo em Assembler equivalente 
  movwf LCD_DATA	; DATA = 0x03 (4 bits)
  bcf   LCD_RS		; RS = 0
  bcf   LCD_RW		;
  bsf   LCD_EN
  nop 				; pulso de enable
  bcf   LCD_EN
*/

}

// Envia um dado para o LCD
void lcd_dado(unsigned char dado){

  //Envia o dado todo de uam vez um de 4 em 4 na parte mais sig. ou menos, dependendo da configuração
  // indica se uma os 8 bits
  if(LCD_8BITS)
  {
	LCD_DATA = dado;
#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 1;		// Ativa comando(RS=0)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)
	lcd_delay(180);		// Espera 1,8ms
  }
  //senao eh 4 bits - sao os mais sig?
  else 
  {
    if(LCD_4BITSMSB)
    {
      //manda a parte mais significativa do dado nos 4 bits mais sig da porta
	  LCD_DATA = (dado & 0xf0);			
    }
    //entao sao os 4 menos sig
    else
    {
      //manda a parte mais significativa do dado nos 4 bits menos sig da porta
	  LCD_DATA = (dado >> 4);			

    }

#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 1;		// Ativa dado(RS=1)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)

    if(LCD_4BITSMSB)
    {
      //manda a parte menos significativa do dado nos 4 bits mais sig da porta
	  LCD_DATA = (dado << 4);			
    }
    //entao sao os 4 menos sig
    else
    {
      //manda a parte menos significativa do dado nos 4 bits menos sig da porta
	  LCD_DATA = (dado & 0x0f);			

    }
#if defined(USA_RW)
	LCD_RW = 0;  	// Ativa escrita(RW=0)
#endif
	LCD_RS = 1;		// Ativa dado(RS=1)
	LCD_EN = 1;	   // Gera pulso de enable (EN=1)
	LCD_EN = 0;	// Desliga pulso de enable (EN=0)
	lcd_delay(5);		// Espera 50us
  }



}



// Delay de 10us x tempo (máximo =
void lcd_delay(unsigned char tempo){ 
#ifndef _XTAL_FREQ
#define _XTAL_FREQ 20000000   // define frequencia do cristal para delay
#endif

#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
///// INDICA TAMBEM O USUO DO COMPILADOR c18

#ifndef LCD_DELAY
                     // Delay of 10ms
                     // Cycles = (TimeDelay * Fosc) / 4
                     // Cycles = (10ms * 20MHz) / 4
                     // Cycles = 50,000 / 1000 = 50

//#define LCD_DELAY (((10 * (_XTAL_FREQ/1000)) / 4) / 1000)
#define LCD_DELAY 50
#endif

  Delay1KTCYx(tempo * LCD_DELAY); // espera de = tempo x 10us

#else

#ifndef LCD_DELAY
#define LCD_DELAY 2   // fixo em 20
#endif

	while(tempo--){	  // espera tempo x 10us
		__delay_us(LCD_DELAY);
	}
#endif

}

// Limpa LCD
void lcd_clear(void){

  lcd_comando(0x01);
}

// Posiciona o cursor na coluna x , linha y
void lcd_gotoxy(unsigned char x,unsigned char y){

  lcd_comando(0x80+y*0x40+x);
	
}

// Posiciona o cursor na coluna x , linha y
void lcd_posiciona(unsigned char linha,unsigned char coluna){

  lcd_comando(0x80+(linha-1)*0x40+coluna-1);
	
}


// Imprime um texto (printf)
void lcd_printf(const char * texto){
char ch;
	
	ch = *texto++;
	while (ch != 0x00){
		lcd_dado(ch);
		ch = *texto++;
	}
}

// Liga o cursor on
void lcd_cursor_on(void){

   lcd_comando(0x0F);
}

// Liga o cursor off
void lcd_cursor_off(void){

   lcd_comando(0x0C);
}

// Imprime numero no display
void lcd_printf_num(unsigned char numero){

   lcd_dado(numero/100+'0');
   numero = numero % 100;
   lcd_dado(numero/10+'0');
   numero = numero % 10;
   lcd_dado(numero+'0');
}

// Desenha um caracter 5x8 ( e depois imprime)
void lcd_desenha(char caracter,unsigned char linha,unsigned char coluna){

   lcd_comando(0x40+(caracter%8)*8); // Comando para  desenhar (escrever na CGRAM)
   switch(caracter){
	  case RADIOATIVIDADE:{ // Caracter de radioatividade (simbolo)
	    lcd_dado(0x0A);  //0b01010
	    lcd_dado(0x1B);  //0b11011
	    lcd_dado(0x00);  //0b00000
	    lcd_dado(0x04);  //0b00100
	    lcd_dado(0x00);  //0b00000
	    lcd_dado(0x04);  //0b00100
	    lcd_dado(0x0E);  //0b01110
	    lcd_dado(0x00);  //0b00000
		break;
	  }

      case GRAU:{ // Caracter Grau
		lcd_dado(0x04);  //0b00100 ,  // Os bits em 1 são os pixels que ficam ligados (em preto
		lcd_dado(0x0A);  //0b01010 ,
		lcd_dado(0x11);  //0b10001 ,
		lcd_dado(0x0A);  //0b01010 ,
		lcd_dado(0x04);  //0b00100 ,
		lcd_dado(0x00);  //0b00000 ,
		lcd_dado(0x00);  //0b00000 ,
		lcd_dado(0x00);  //0b00000
		break;
	  }
	  case A_TIL:{ //Caracter A com ~
		lcd_dado(0x0D);  //0b01101 ,
		lcd_dado(0x12);  //0b10010 ,
		lcd_dado(0x06); //0b00110 ,
		lcd_dado(0x01); //0b00001 ,
		lcd_dado(0x0F); //0b01111 ,
		lcd_dado(0x11); //0b10001 ,
		lcd_dado(0x0F); //0b01111 ,
		lcd_dado(0x00); //0b00000
		break;
	  };
	  case CLOCK:{ //Caracter A com ~
		lcd_dado(0x1F); //0b11111 ,
		lcd_dado(0x15); //0b10101 ,
		lcd_dado(0x15); //0b10101 ,
		lcd_dado(0x13); //0b10111 ,
		lcd_dado(0x11); //0b10001 ,
		lcd_dado(0x11); //0b10001 ,
		lcd_dado(0x1F); //0b11111 ,
		lcd_dado(0x00); //0b00000
		break;
	  };
      case C_CEDILHA:{
         lcd_dado(0x1F);  //0b11111
    	lcd_dado(0x10);  //0b10000
    	lcd_dado(0x10);  //0b10000
    	lcd_dado(0x10);  //0b10000
    	lcd_dado(0x1F);  //0b11111
    	lcd_dado(0x04);  //0b00100
    	lcd_dado(0x08);  //0b01000
    	lcd_dado(0x00);  //0b00000
        break;
   }   
	  default:{
		lcd_dado(0x0E);  //0b01110 ,
		lcd_dado(0x0E);  //0b01110 ,
		lcd_dado(0x04);  //0b00100 ,
		lcd_dado(0x1F);  //0b11111 ,
		lcd_dado(0x04);  //0b00100 ,
		lcd_dado(0x0C);  //0b01010 ,
		lcd_dado(0x11);  //0b10001 ,
		lcd_dado(0x11);  //0b10001 ,
		break;
	  }
   }
   lcd_posiciona(linha,coluna); // Termina comando posicionando o diplay
   lcd_dado(0+(caracter%8)); // Imprime o caracter desenhado que está na RAM
}


// Imprime a hora no display
void lcd_print_hora(unsigned char hora,unsigned char minuto,unsigned char segundo){
   lcd_dado(hora/10+'0'); // Imprime as dezenas da hora
   lcd_dado(hora%10+'0'); // Imprime as unidades da hora
   lcd_dado(':'); //Imprime :
   lcd_dado(minuto/10+'0'); // Imprime as dezenas dos minutos
   lcd_dado(minuto%10+'0'); // Imprime as unidades dos minutos
   lcd_dado(':'); //Imprime :
   lcd_dado(segundo/10+'0'); // Imprime as dezenas dos segundos
   lcd_dado(segundo%10+'0'); // Imprime as unidades dos segundos
}

// Imprime a hora no display
void lcd_print_data(unsigned char dia,unsigned char mes,unsigned char ano){
   lcd_dado(dia/10+'0'); // Imprime as dezenas do dia
   lcd_dado(dia%10+'0'); // Imprime as unidades do dia
   lcd_dado('/'); //Imprime :
   lcd_dado(mes/10+'0'); // Imprime as dezenas do mes
   lcd_dado(mes%10+'0'); // Imprime as unidades do mes
   lcd_dado('/'); //Imprime :
   lcd_dado(ano/10+'0'); // Imprime as dezenas do ano
   lcd_dado(ano%10+'0'); // Imprime as unidades do ano
}
