// Definições no Modulo
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
#include <p18cxxx.h>	// inclui as definições dos ios do pic
#else
#include <htc.h>
#endif

#include "..\lib\base\base.h"
#include "m3wlib.h"

// Variáveis do módulo
unsigned char rtc_segundo, rtc_minuto, rtc_hora, rtc_semana, rtc_mes, rtc_dia, rtc_ano;


void rtc_iniciar(TBaseEventos *mpbeBaseEventos){
    init_3w(mpbeBaseEventos);          //Init 3wire
    reset_3w();
    wbyte_3w(0x8e); /* control register */
    wbyte_3w(0);        /* disable write protect */
    reset_3w();
    wbyte_3w(0x90); /* trickle charger register */
    wbyte_3w(0x00); /* enable, 2 diodes, 8K resistor */
    reset_3w();
    wbyte_3w(0xbe); /* clock burst write (eight registers) */
    wbyte_3w(0x50); //sec
    wbyte_3w(0x09);//min
    wbyte_3w(0x16);//hour
    wbyte_3w(0x01);//date
    wbyte_3w(0x09);//mon
    wbyte_3w(24);//day
    wbyte_3w(0x09);//year
    wbyte_3w(0);        /* must write control register in burst mode */
    reset_3w();
}


// Converte um valor hex (ex: 0x24 em 24 decimal
unsigned char conv_hex_dec(unsigned char byte){
unsigned char numero;
   numero = (byte>>4)*10; 
   return(numero+(byte%16));   
}

// Converte um valor dec (ex: 0x24 em 24 decimal
unsigned char conv_dec_hex(unsigned char byte){
unsigned char numero;
   numero = (byte/10)<<4; 
   return(numero+(byte%10));   
}

// Le a hora do RTC para as variáveis
void rtc_ler(void){
   reset_3w();     // Reset 3wire
   wbyte_3w(0xBF); // Clock burst, tells rtc to send date/time info on 3w bus
   rtc_segundo = conv_hex_dec(rbyte_3w());
   rtc_minuto = conv_hex_dec(rbyte_3w());
   rtc_hora = conv_hex_dec(rbyte_3w());
   rtc_dia = conv_hex_dec (rbyte_3w());
   rtc_mes = conv_hex_dec (rbyte_3w());
   rtc_semana = conv_hex_dec(rbyte_3w());
   rtc_ano  = conv_hex_dec(rbyte_3w());
   reset_3w();
}


// Escreve a hora desejada (que estão nas variáveis no RTC)
void rtc_gravar(void){
    reset_3w();
    wbyte_3w(0x8e); /* control register */
    wbyte_3w(0);        /* disable write protect */
    reset_3w();
    wbyte_3w(0x90); /* trickle charger register */
    wbyte_3w(0x00); /* enable, 2 diodes, 8K resistor */
    reset_3w();
    wbyte_3w(0xbe); /* clock burst write (eight registers) */
    wbyte_3w(conv_dec_hex(rtc_segundo)); //sec
    wbyte_3w(conv_dec_hex(rtc_minuto));//min
    wbyte_3w(conv_dec_hex(rtc_hora));//hour
    wbyte_3w(conv_dec_hex(rtc_dia));//semana(dia)
    wbyte_3w(conv_dec_hex(rtc_mes));//mon
    wbyte_3w(conv_dec_hex(rtc_semana));//day
    wbyte_3w(conv_dec_hex(rtc_ano));//year
    wbyte_3w(0);        /* must write control register in burst mode */
    reset_3w();
}
