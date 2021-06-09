// Interface do Módulo Serial.c
#ifndef  SERIAL_H
#define SERIAL_H

//id unico do modulo para passar nos eventos previstos em base.h
#define SERIAL_ID_MODULO 13

// Variáveis disponíveis no Módulo
extern unsigned char flag_rx;
extern unsigned char byte_recebido;

// Funções disponíveis no Módulo


//void serial_iniciar(TBaseEventos *mpbeBaseEventos);			// Inicia serial
void serial_iniciarEx(unsigned char mucValorSPBRG, unsigned char mucUsaVelAlta, 
unsigned char mucUsarIntRx, unsigned char mucUsarIntTx, TBaseEventos *mpbeBaseEventos); //inicia recebendo o valor de velocidade e config. para velocidades altas (0-lentas, 1- altas)

void serial_enviar(unsigned char dado);	// Tranmite um dado pela serial
//unsigned char serial_receber(void);		// Recebe dado pela serial (sem interrupção)
//void serial_int(void);			// Interrupção serial (caso seja usada)
void serial_printf(const char * texto);		// Envia uma string pela serial (similar ao printf)
//void serial_scanf(char * texto);		// Le uma string pela serial (similar ao scanf)
//void serial_scanf_num(unsigned char * valor);
//TCC: para indica que esta usando int. de tx (esta travando e por isso apenas seta a propriedade)
unsigned char serial_ucUsarIntTx(void);
#endif
