/* //////////////////////////////////////////////////////////////////////////
// autor: RODRIGO P. A. :
// data: 23/01/2003
// você pode ligar o lcd na porta que vc quizer, é só definir onde vc quer ligar
// no seu código e incluir este arquivo, se vc nao fizer as definições o compilador utiliza
// os valores padrões
	Alterado em 04/2006 para funcionar com o C18 da Microchip
*/

#define LCD_C

#include <p18cxxx.h>	// inclui as definições dos ios do pic
#include "..\lib\base\base.h"
#include "tipos.h"
#include "utils.h"
#include "lcd.h"
#include <pwm.h>		/// biblioteca da microchip do C18, consulte a documentacao


//foi preciso trazer def. direto para o modulo
//lcd (o módulo testa se ja hesta definido e se estiver, acata, senao define)
// definição dos I/Os onde estão ligados o LCD
#define  lcd_rs      PORTCbits.RC2
#define  lcd_rw      PORTCbits.RC1
#define  lcd_enable  PORTCbits.RC0
//ficaram iguais na verdade:
//#define  lcd_D0      PORTDbits.RD0
//#define  lcd_D1      PORTDbits.RD1
//#define  lcd_D2      PORTDbits.RD2
//#define  lcd_D3      PORTDbits.RD3
#define  lcd_D4      PORTDbits.RD4
#define  lcd_D5      PORTDbits.RD5
#define  lcd_D6      PORTDbits.RD6          
#define  lcd_D7      PORTDbits.RD7

unsigned char main_dummy;
//coloca macro apontando para uma variavel para modulo nao mecher sem necessidade com a def e saida
#define  lcd_rs_tris      main_dummy
#define  lcd_rw_tris      main_dummy
#define  lcd_enable_tris  main_dummy
//ficaram iguais na verdade:
#define  lcd_D0_tris      main_dummy
#define  lcd_D1_tris      main_dummy           
#define  lcd_D2_tris      main_dummy
#define  lcd_D3_tris      main_dummy
#define  lcd_D4_tris      main_dummy
#define  lcd_D5_tris      main_dummy
#define  lcd_D6_tris      main_dummy          
#define  lcd_D7_tris      main_dummy


//max para controle de salvar e restaurar posicao do cursor
unsigned char gucSavedX;
unsigned char gucSavedY;

unsigned char gucX;
unsigned char gucY;

//ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
static TBaseEventos *gpbeBaseEventos = NULL;

//macro para eventos
        
#define  lcd_comando() { doEventoSaidaDados(0); lcd_rs=0; doEventoSaidaDados(1); }
#define  lcd_dado()    { doEventoSaidaDados(0); lcd_rs=1; doEventoSaidaDados(1); }

#define	delay_lcd()	delay_us(10);

void limpaEspelho(void)
{
  unsigned char i;
  for(i=0; i<sizeof(lcd_ucEspelhoDisplay); i++)
    lcd_ucEspelhoDisplay[i] = ' ';
}

void doEventoSaidaDados(unsigned char mucEventoDepois)
{
  if(gpbeBaseEventos)
  {
    if(mucEventoDepois)
    {
      base_doOnDepoisSaidaDados(gpbeBaseEventos, LCD_ID_MODULO);
    }  
    else
    {
      base_doOnAntesSaidaDados(gpbeBaseEventos, LCD_ID_MODULO);
    }
  }    
} 


/*
	void lcd_led(int8 valor)
	Acende o LCD, com o duty cycle informado
	uso da biblioteca da microchip
*/
/*
void lcd_led(int8 valor){
	ClosePWM1();
	SetDCPWM1(valor);	// duty cycle
	OpenPWM1(0xff);
}
*/

//
void lcd_envia_byte(int8 n) {
      restart_wdt();

      //evento antes de alterar dados
      doEventoSaidaDados(0);      
      lcd_enable=0;	  
      doEventoSaidaDados(1);
      doEventoSaidaDados(0);      
      if ( n&0x10 ) lcd_D4=1; else lcd_D4=0;
      if ( n&0x20 ) lcd_D5=1; else lcd_D5=0;
      if ( n&0x40 ) lcd_D6=1; else lcd_D6=0;
      if ( n&0x80 ) lcd_D7=1; else lcd_D7=0;
      //evento depois de alterar dados
      doEventoSaidaDados(1);
            
      delay_lcd();

      //evento antes de alterar dados
      doEventoSaidaDados(0);      
      lcd_enable=1;
      //evento depois de alterar dados
      doEventoSaidaDados(1);

      delay_lcd();

      //evento antes de alterar dados
      doEventoSaidaDados(0);      
      lcd_enable=0;
      if ( n&0x01 ) lcd_D4=1; else lcd_D4=0;
      if ( n&0x02 ) lcd_D5=1; else lcd_D5=0;
      if ( n&0x04 ) lcd_D6=1; else lcd_D6=0;
      if ( n&0x08 ) lcd_D7=1; else lcd_D7=0;
      //evento depois de alterar dados
      doEventoSaidaDados(1);

      delay_lcd();

      //evento antes de alterar dados
      doEventoSaidaDados(0);      
      lcd_enable=1;
      //evento depois de alterar dados
      doEventoSaidaDados(1);

      delay_lcd();

      //evento antes de alterar dados
      doEventoSaidaDados(0);      
      lcd_enable=0;
      //evento depois de alterar dados
      doEventoSaidaDados(1);
}

/*
*/
void lcd_init(TBaseEventos *mpbeBaseEventos) {
    int8 i;
    
  //ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
  gpbeBaseEventos = mpbeBaseEventos;
    
  //ativa saida para madar tudo direto
  doEventoSaidaDados(1);
	/*
  PORTE=0x00;
	PORTD=0x00;
	CMCON=0x07;
	TRISE=0x00;
	TRISD=0x00;
	LATD=0x00;
	LATE=0x00;
	ADCON1=0x0A;
	*/
	
	lcd_rw=0;
	
	//lcd_led(20);
	
    lcd_D4=0;
    lcd_D5=0;
    lcd_D6=0;
    lcd_D7=0;
    lcd_rs=0; // comando
    lcd_enable=0;
    restart_wdt();
    delay_ms(15);
    lcd_comando();
    lcd_D4=1;
    lcd_D5=1;
    lcd_D6=0;
    lcd_D7=0;
    for (i=0;i<3;i++)
    {
       lcd_enable=1;
       delay_ms(1);
       lcd_enable=0;
       delay_ms(1);
   }
    lcd_D4=0;
    lcd_D5=1;
    lcd_D6=0;
    lcd_D7=0;
    lcd_enable=1;
    delay_ms(1);
    lcd_enable=0;
    delay_ms(1);
    restart_wdt();
    lcd_comando();
    lcd_envia_byte(0x28);
    lcd_envia_byte(0x06);
    lcd_envia_byte(0x0C);
    lcd_envia_byte(0x01);

	//para controlar pos do cursor
	gucSavedX = 0;
	gucSavedY = 0;
	gucX = 0;
	gucY = 0;

  limpaEspelho();


}

void lcd_putc(char c) {
   restart_wdt();
   switch (c) {
      case '\f':              // limpa o lcd
         lcd_comando();
         lcd_envia_byte(0x01);
         restart_wdt();
         delay_ms(2);
         lcd_envia_byte(0x80);

       limpaEspelho();
		 //atualiza ultima posicao
	     gucX = 0;
	     gucY = 0;

         break;
     case '\n':               // vai para a segunda linha
	 case '\r':               // vai para a segunda linha
	       //TCC: usa macro ao inves do pino
         lcd_comando();     
         //lcd_rs=0; // comando
         lcd_envia_byte(0xC0);

		 //atualiza ultima posicao
	     gucX = 0;
	     gucY = 1;

         break;
     case '\b':               // volta um caracter para es esquerda (nao verif. se jah eh o primeiro)
         lcd_incxy(-1,0);

         break;
     default:
          lcd_dado(); // caractere		  
          lcd_envia_byte(c);
     
       if(((gucY*16)+gucX) < sizeof(lcd_ucEspelhoDisplay))     
         lcd_ucEspelhoDisplay[(gucY*16)+gucX] = c;    
          
		 //atualiza ultima posicao
	     gucX++;
          break;
     }
     restart_wdt();
}

void  lcd_goto_xy(int8 x,int8 y)
{

}


void lcd_puts(char *s){
	while( *s!=0x00 ){
		lcd_putc(*s);
		s++;
	}	
}


//versao que  recebe uma string constante para evetar codigo extra
void lcd_puts_const(auto const rom char *s)
{
	while( *s!=0x00 ){
		lcd_putc(*s);
		s++;
    }   
}




//adicionados ao módulo original
void lcd_clear(void)
{
  lcd_putc('\f');
}


// Liga o cursor on
void lcd_cursor_on(void)
{
  //TCC: usa macro ao inves do pino
  lcd_comando();     
  //lcd_rs=0; // comando
  lcd_envia_byte(0x0F);
}

// Liga o cursor off
void lcd_cursor_off(void)
{
  //TCC: usa macro ao inves do pino
  lcd_comando();     
  //lcd_rs=0; // comando
  lcd_envia_byte(0x0C);
}




void lcd_gotoxy(unsigned char x,unsigned char y)
{
  //TCC: usa macro ao inves do pino
  lcd_comando();     
  //lcd_rs=0; // comando
  lcd_envia_byte(0x80+y*0x40+x);

  //atualiza ultima posicao
  gucX = x;
  gucY = y;
	
}

void lcd_incxy(char incx,char incy)
{
  lcd_gotoxy((char)gucX+incx, (char)gucY+incy);
}


void lcd_savexy(void)
{
  //salva ultima posicao
  gucSavedX = gucX;
  gucSavedY = gucY;
}

void lcd_restorexy(void)
{
	//restaura
	lcd_gotoxy(gucSavedX, gucSavedY);
}

// Imprime numero no display
/*
void lcd_printf_int(int numero)
{

  lcd_rs=0; // dado
  lcd_envia_byte(numero/1000+'0');
  numero = numero % 1000;
  lcd_envia_byte(numero/100+'0');
  numero = numero % 100;
  lcd_envia_byte(numero/10+'0');
  numero = numero % 10;
  lcd_envia_byte(numero+'0');
}
*/
