#ifndef AD_H
#define AD_H

//id unico do modulo para passar nos eventos previstos em base.h
#define AD_ID_MODULO 4

// Inicia o AD
void ad_iniciar(TBaseEventos *mpbeBaseEventos);

// Espera pelo tempo de aquisição +-5us
void delay_ad(void);

// Ler o AD
unsigned char ad_ler(void);

#endif


