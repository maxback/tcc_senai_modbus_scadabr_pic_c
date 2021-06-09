/**
** Cabeçalho do Módulo de captura da corrente e tensao via canais do AD
**/

#ifndef CAPTURA_H
#define CAPTURA_H

//id unico do modulo para passar nos eventos previstos em base.h
#define CAPTURA_ID_MODULO 102

//configuracoes globais
typedef struct
{
											//Valores(tamanho): 
//#define GLOBAL_INDEX_CFG_MODO 0				//0-modoOperacao(1) - embora armazenado aqui tem ser apssado na funcao setaModo para aplica se mudar no meio da 
												//execucao
	unsigned char ucModo;
	unsigned int uiTempoEntreCapturas;
	unsigned int uiValorADZeroCorrente;
	unsigned char ucFatorTrafo;
	unsigned int uiDescontoDiodos;
	unsigned int uiFatorDivResistivo;
//sensibilidade do TC - Indica qual a sensibilidade do sensor TC em mV/A. 
//Este valor indica para o medidor em quantos mV deve aumentar a tensão na saída do TC 
//para uma variação de 1A em sua entrada.
//3 casas decimais prewsumidas	
  unsigned long ulSensibilidadeTC; 
//											//  * O desconto dos diodos preve dos 5 digitos os tris a direita após a virgula
//

} TCfgModuloCapturas;


// tipos de valor apra passar por parametros para a funcao captura_iniciar() ou para a funcao captura_setaModo()
#define CAPTURA_MODO_TESTE 0
#define CAPTURA_MODO_CAPTURA 1
#define CAPTURA_MODO_MEDICAO 2


//ultimos valores de captura dos ads 0, 1 e 2
#ifndef CAPTURA_C
extern
#endif
unsigned int captura_uiUltimaLeituraADCMedicao[3];


// bit de sinali para modulo main gravar uma informacao 
#ifndef CAPTURA_C
extern
#endif
unsigned char captura_ubEventoNovoDado;


// flag que inica que fo iativada a captura
#ifndef CAPTURA_C
extern
#endif
unsigned char captura_ubEventoAtivado;


// contador de capturas - zerado ao setar o modo captura e incrementado ao setar flag de que tem dados
#ifndef CAPTURA_C
extern
#endif
unsigned long int captura_ulQuantidadeCapturas;


//inicializa o modulo com o modo passado por parametro: CAPTURA_MODO_TESTE, CAPTURA_MODO_CAPTURA, CAPTURA_MODO_MEDICAO  
// pcBuffer espera um ponteiro para um array de no minimo 40 caracteres para uso geral
//muiIntervaloCaptura indica o tempo a ser esperado entre capturas em microsegundos (apenas para o modo = CAPTURA_MODO_CAPTURA)
//***** tudo passa a ser passado via ponteiro para estrutura do tipo TCfgModuloCapturas ***
//exceto o buffer
#ifndef CAPTURA_C
extern
#endif
void captura_iniciar(TCfgModuloCapturas *mpcmCfg, char *mcBuffer, TBaseEventos *mpbeBaseEventos);

//altera o modo de trabalho para : CAPTURA_MODO_TESTE, CAPTURA_MODO_CAPTURA, CAPTURA_MODO_MEDICAO
#ifndef CAPTURA_C
extern
#endif
void captura_setaModo(unsigned char mucModo);

//retorna o modo
#ifndef CAPTURA_C
extern
#endif
unsigned char captura_ucRetornaModo(void);

//seta se deve rodar ou parar - true roda, false - para
#ifndef CAPTURA_C
extern
#endif
void captura_rodar(unsigned char mucRodar);


//seta se deve rodar ou parar - true roda, false - para
#ifndef CAPTURA_C
extern
#endif
unsigned char captura_ucRodando(void);


//execucao a cada ciclo do main - faz as capturas realmente
#ifndef CAPTURA_C
extern
#endif
void captura_execute(void);

//notifica que trabscorreu um segundo (chamar se em modo de medicao)
/*
#ifndef CAPTURA_C
extern
#endif
void captura_notificaSegundoTranscorrido(void);
*/


#endif
