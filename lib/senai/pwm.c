#include <htc.h>
#include "pwm.h"
 
void pwm_iniciar(TBaseEventos *mpbeBaseEventos){

    TRISC &= 0b11111011;    // Configura portas (saidas)
	pwm_ligar(0);
}
 
// Liga o PWM 
void pwm_ligar(unsigned char duty_cicle){

	PR2 = 0xFF;
	CCPR1L = duty_cicle;  // Configura Duty Cicle
	T2CON = 0b00000101;   // Liga Timer 2 (prescaler 4) 4Khz	
	CCP1CON = 0b00001111;   // Configura PWM (canal e clock) e liga
}
