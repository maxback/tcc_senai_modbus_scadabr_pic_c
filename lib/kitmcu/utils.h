/*
	prototipo das funcoes do utils.c
	Autor: Rodrigo  P A 
	visite meu site: www.kitmcu.com.br

  - Alterado por Max Back
*/

#ifndef __utils
	#define	__utils


//id unico do modulo para passar nos eventos previstos em base.h
#define UTIL_ID_MODULO 2

//define que se presente faz o modulo funcionar com os pinos apenas de entrada
#define SERIALSINC_APENASENTRADA 


// definicao dos IOS no Kit PIC18F
#define	LED1	PORTDbits.RD0
#define	LED2	PORTDbits.RD1
#define	LED3	PORTDbits.RD2
#define	LED4	PORTDbits.RD3

#define BARCODE_DEBUG_PINO 0
#define BARCODE_DEBUG_PIN PORTBbits.RB2
#define BARCODE_DEBUG_TRIS TRISBbits.TRISB2

#define INVPINO() if(BARCODE_DEBUG_PINO){ BARCODE_DEBUG_PIN = !BARCODE_DEBUG_PIN; }

#ifndef UTIL_C
extern
#endif
void restart_wdt(void);

#ifndef UTIL_C
extern
#endif
void delay_ms(unsigned int t);

#ifndef UTIL_C
extern
#endif
void delay_us(unsigned int t);

#ifndef UTIL_C
extern
#endif
void Iniciar_Timer(void);

//seta ponteiro para funcao void(void) que se definida é chaamda para ao ocorre interrupção
#ifndef UTIL_C
extern
#endif
void utils_setFuncHighInt(void (*mpFuncHighInt)(void));



// variáveis globais
#define	tempo_int	(unsigned int)43	// 1 interrupção a cada aprox, 250uS

#ifndef UTIL_C
extern
#endif
unsigned int tempo;  					// conta os ticks das interrupções
//unsigned char ledstatus=1;


#ifndef UTIL_C
extern
#endif
unsigned long util_ulValorRecebido;  			


#ifndef UTIL_C
extern
#endif
unsigned char util_ucQtdBitsRecebidos; //con start bit e tudo  		
//tipos de evento
#define UTIL_EVT_SERIAL_RECEBIDO 0
#define UTIL_EVT_SERIAL_ERRO_PARIDADE 1
#define UTIL_EVT_SERIAL_ERRO_STOP 2
#define UTIL_EVT_SERIAL_NENNHUM 3

#ifndef UTIL_C
extern
#endif
unsigned long util_ucEventoSerialSinc;  					// conta os ticks das interrupções

//ulyimo ack recebido
#ifndef UTIL_C
extern
#endif
unsigned char  serialsinc_ucValorACKRecebido;

//seta funcao de call back ao terminar com sucesso o uerro a rec. de um byte
//ou receber um ack
//pode-se ler o estado geral da maquina (que após esta chamada sera mudado apra parada se for ack)
//mucDado -> valor do ack (0 ou 1) ou do byte recebido
#ifndef UTIL_C
extern
#endif
void serialsinc_setFuncCallbackResultadoRecSerial(void (*mpFunc)(unsigned char mucDados, unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado));

//seta funcao de call back ao terminar e enviar um byte, ou mandar um CTS
//pode-se ler o estado geral da maquina (que após esta chamada sera mudado apra parada
//se o envio foi do CTS ou para rec_ack se foi byte

#ifndef UTIL_C
extern
#endif
void serialsinc_setFuncCallbackResultadoEnvSerial(void (*mpFunc)(unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado));

//inicia envio de um byte pela serial sincrona ou apenas indica o proximo byte a ser enviado
#ifndef UTIL_C
extern
#endif
void serialsinc_setByteEnvio(unsigned char mucValor, unsigned char mucIniciarEnvio);


//seta funcao de call back ao ocorrer evento de timer (config. a cada 250 uS)
#ifndef UTIL_C
extern
#endif
void utils_setFuncCallbackTickTimer(void (*mpFunc)(void));

#define MAQGERAL_PARADA 0
#define MAQGERAL_ENVIANDO_CTS 1
#define MAQGERAL_ENVIANDO_BYTE 2
#define MAQGERAL_ESPERANDO_ACK 3
#define MAQGERAL_RECEBENDO 4

//seta esta geral da serial (usar para parar a mesma, mudar ma recepcao, envio, epsera do ack e envi ode cts)
//o estado eh um dos valores de defines MAQGERAL_...
#ifndef UTIL_C
extern
#endif
void serialsinc_setEstado(unsigned char mucEstado);

//le o estado
#ifndef UTIL_C
extern
#endif
unsigned char  serialsinc_ucGetEstado(void);


#ifndef UTIL_C
extern
#endif
void serialsinc_iniciar(TBaseEventos *mpbeBaseEventos);


#endif
