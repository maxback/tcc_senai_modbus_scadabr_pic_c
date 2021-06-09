#ifndef MOTOR_H
#define MOTOR_H

//id unico do modulo para passar nos eventos previstos em base.h
#define MOTOR_ID_MODULO 10


#define HORARIO 1
#define ANTI_HORARIO 2

extern unsigned char motor_etapa;
extern unsigned int motor_posicao;


void motor_iniciar(TBaseEventos *mpbeBaseEventos);
void motor_movimentar(unsigned char sentido,unsigned char passos);

#endif