/**
*** Cabeçalho do modulo de controle de barramento/multiplexaçào das portas do MCU
**/

#ifndef BARRA_H
#define BARRA_H

//id unico do modulo para passar nos eventos previstos em base.h
#define BARRA_ID_MODULO 101


//configuracao dos pinos disponiveis para selecao da entrada
//qtd de bits
//CONFIG DOS PINOS e PINOS(TRIS) DE ENTRADA
#define BARRA_PORTA_SEL_ENTRADA PORTE
#define BARRA_SEL_ENT0_TRIS TRISEbits.TRISE0
#define BARRA_MASCARABIT_SEL_ENT0 0x01
//se tivesse mais bits bastaria definiar a mascara  com valor dif. de 0 e nomes como BARRA_MASCARABIT_SEL_ENT1, ...3, ...4
//tambem definindo as cfgs de tristate BARRA_SEL_ENT1_TRIS, ..2.., ..3..  

//CONFIG DOS PINOS e PINOS(TRIS) DE SAIDA
#define BARRA_PORTA_SEL_SAIDA PORTE
#define BARRA_SEL_SAI0_TRIS TRISEbits.TRISE1
#define BARRA_MASCARABIT_SEL_SAI0 0x02
#define BARRA_SEL_SAI1_TRIS TRISEbits.TRISE2
#define BARRA_MASCARABIT_SEL_SAI1 0x04
//se tivesse mais bits bastaria definiar a mascara  com valor dif. de 0 e nomes como BARRA_MASCARABIT_SEL_SAI2, ...3, ...4
//tambem definindo as cfgs de tristate BARRA_SEL_SAI2_TRIS, ..3.., ..4..  

//bit de habilitacao da saida
//nao eh usada neste hw, ficand osempre habilitada
//#define BARRA_HAB_SAIDA_TRIS TRISCbits.TRISC7
//#define BARRA_HAB_SAIDA PORTCbits.RC7

//pinos de dados por bits (10 bits previstos)
#define BARRA_SAI_B0 PORTDbits.RD0
#define BARRA_SAI_B1 PORTDbits.RD1
#define BARRA_SAI_B2 PORTDbits.RD2
#define BARRA_SAI_B3 PORTDbits.RD3
#define BARRA_SAI_B4 PORTDbits.RD4
#define BARRA_SAI_B5 PORTDbits.RD5
#define BARRA_SAI_B6 PORTDbits.RD6
#define BARRA_SAI_B7 PORTDbits.RD7
#define BARRA_SAI_B8 PORTCbits.RC0
#define BARRA_SAI_B9 PORTCbits.RC1
#define BARRA_SAI_B10 PORTCbits.RC2


//configuracoes pinos de dados por bits (10 bits previstos)
#define BARRA_SAI_B0_TRIS TRISDbits.TRISD0
#define BARRA_SAI_B1_TRIS TRISDbits.TRISD1
#define BARRA_SAI_B2_TRIS TRISDbits.TRISD2
#define BARRA_SAI_B3_TRIS TRISDbits.TRISD3
#define BARRA_SAI_B4_TRIS TRISDbits.TRISD4
#define BARRA_SAI_B5_TRIS TRISDbits.TRISD5
#define BARRA_SAI_B6_TRIS TRISDbits.TRISD6
#define BARRA_SAI_B7_TRIS TRISDbits.TRISD7
#define BARRA_SAI_B8_TRIS TRISCbits.TRISC0
#define BARRA_SAI_B9_TRIS TRISCbits.TRISC1
#define BARRA_SAI_B10_TRIS TRISCbits.TRISC2


//condig dos bits


//seta o indice da saida atual que deverá receber os daodos na saida do MCU
#ifndef BARRA_C
extern
#endif
void barra_setSaidaAtual(unsigned char mucIndiceSaida);

//seta o estado de habilitação da saída selecionada (0 - alta impedancia, 1 - saida habilitada)
#ifndef BARRA_C
extern
#endif
void barra_setSaidaAtiva(unsigned char mucSaidaAtiva);

//retorna o indice da saida atual que deverá receber os daodos na saida do MCU
#ifndef BARRA_C
extern
#endif
unsigned char barra_getSaidaAtual(void);

//retorna  o estado de habilitação da saída selecionada (0 - alta impedancia, 1 - saida habilitada)
#ifndef BARRA_C
extern
#endif
unsigned char barra_getSaidaAtiva(void);


//seta o indice da entrada atual que deverá ter os dados copiados para o micrcocontrolador
#ifndef BARRA_C
extern
#endif
void barra_setEntradaAtual(unsigned char mucIndiceEntrada);

//retorna o indice da entrada atual que deverá ter os dados copiados para o micrcocontrolador
#ifndef BARRA_C
extern
#endif
unsigned char barra_getEntradaAtual(void);


//inicializa o módulo
#ifndef BARRA_C
extern
#endif
void barra_iniciar(unsigned char mucIndiceEntrada, unsigned char mucIndiceSaida,
  unsigned char mucSaidaHabilitada, TBaseEventos *mpbeBaseEventos);

#endif

