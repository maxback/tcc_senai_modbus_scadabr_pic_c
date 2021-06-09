#ifndef PWM_H
#define PWM_H

//id unico do modulo para passar nos eventos previstos em base.h
#define PWM_ID_MODULO 11

 
void pwm_iniciar(TBaseEventos *mpbeBaseEventos);
// Liga o PWM 
void pwm_ligar(unsigned char duty_cicle);


#endif
