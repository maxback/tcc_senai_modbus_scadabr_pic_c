#ifndef RTC_1302H
#define RTC_1302H

//id unico do modulo para passar nos eventos previstos em base.h
#define RTC_ID_MODULO 12


// Variáveis do módulo
#ifndef RTC_1302_C
extern 
#endif
unsigned char rtc_segundo, rtc_minuto, rtc_hora, rtc_semana, rtc_mes, rtc_dia, rtc_ano;


void rtc_iniciar(TBaseEventos *mpbeBaseEventos);
void rtc_ler(void);
void rtc_gravar(void);




#endif