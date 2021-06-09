/**
*** Cabe'calho do m'odulo de acesso a mem'oria
**/

#ifndef FLASH_M25PXX_H
#define FLASH_M25PXX_H

//id unico do modulo para passar nos eventos previstos em base.h
#define FLASH_M25PXX_ID_MODULO 104


//valores para primeiro parametro(mucModo) de m25pxx_apagaMemoria()
#define APAGA_SETOR 0
#define APAGA_TUDO 1



#ifndef FLASH_M25PXX_C
extern
#endif
//ultimo endereco acessado (usado tambme para calcular endereco quando passado apra fncoes com enbdereco de 2 ou 1 byte
//eh usado tambem para situar o endereco dentro do setor a ser apagado antes de chamar m25pxx_apagaMemoria()
unsigned long int m25pxx_ulUltimoEndereco;

//da memória para endereco de 8 e 16 bits
#ifndef FLASH_M25PXX_C
extern
#endif
unsigned int m25pxx_uiEndereco2BytesMSB;

//variavel que indica os 3 bytes mais significativos de endereco dos 3 e que é usado para as funcoes de escrita e leitura
//indica que esta em modo de leitura rapida se true
#ifndef FLASH_M25PXX_C
extern
#endif
unsigned char m25pxx_gucModoLeituraRapida;


///////////////////////////////////// funcoes do modulo ///////////////////////////////

//indices para seleciona()
#define SEL_CI0 0
#define SEL_CI1 1
#define SEL_NENHUM 0xff


//retorna quantos CIS de memoria tem
#define M25PXX_QUANTIDADE_CI 2

//retorna o tamanho, em bytes do CI atualmenteselecionado
//por hora cnstante em 64k
#define m25pxx_gulTamanhoMemoriaAtual() (65536)
//retorna a quantidade de setores
//por hora 2
#define m25pxx_gucQuantidadeSetores() (2)
//Retorna a quantidade de pagina
//por hora 512
#define m25pxx_guiQuantidadePaginas() (512)
//retorna quantos bytes tem cada setor
// divide o tamanho pela quantidade de paginas
//#define  m25pxx_guiTamanhoPagina() ( m25pxx_gulTamanhoMemoriaAtual() / m25pxx_guiQuantidadePaginas() )
#define  m25pxx_guiTamanhoPagina() (128)



//para iniciar o modulo -> Inicia com o primeiro CI e com m25pxx_gulUltimoEndereco = 0
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_iniciar(TBaseEventos *mpbeBaseEventos);

//para indicar o CI que esta trabalhando: 0 ou 1
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_seleciona(unsigned char mucIndice);

//para indicar o CI que esta trabalhando: 0 ou 1
#ifndef FLASH_M25PXX_C
extern
#endif
unsigned char m25pxx_ucSelecionado(void);

//Le Identificação da memoria
//recebe tre ponteiros para os bytes
//pucIDFabricante 	-> 	Manufacturer identification
//pucTipoMemoria	->	Device identification	->	Memory type 
//pucCapacidade		->	Device identification	->	Memory capacity
//
// Exceto o primeiro ponteiro(pucIDFabricante) os demais ponteiros podem ser passados nulos se a informação não interessar
void m25pxx_leIdentificacao(unsigned char *pucIDFabricante, unsigned char *pucTipoMemoria, unsigned char *pucCapacidade);



//le da memória no endereco de 3 bytes passado
// As questoes de tamanho de setor e de impedir que volte ao primeiro byte dentro do setor são de responsabilidade
//de quem usa a função apra escritas de mais de um byte muiTamanho > 1
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_leDadosEndereco3Bytes(unsigned long int mulEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);



//le da memória no endereco de 3 bytes passado utilizando os dois bytes passados em muiEndereco com os menos sig. dos 3
// e o byte menos sig. de m25pxx_uiEndereco2BytesMSB para completar como o terceiro ais sig. do endereco de 3 bytes
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_leDadosEndereco2Bytes(unsigned int muiEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);


//le da memória no endereco de 3 bytes passado utilizando o byte passado em mucEndereco com o menos sig. dos 3
// e os 2 bytes de m25pxx_uiEndereco2BytesMSB para completar como o terceiro e o segundo mais sig. do endereco de 3 bytes
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_leDadosEndereco1Bytes(unsigned char mucEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);


//esreve na memória no endereco de 3 bytes passado
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_escreveDadosEndereco3Bytes(unsigned long int mulEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);


//escreve na memória no endereco de 3 bytes passado utilizando os dois bytes passados em muiEndereco com os menos sig. dos 3
// e o byte menos sig. de m25pxx_uiEndereco2BytesMSB para completar como o terceiro ais sig. do endereco de 3 bytes
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_escreveDadosEndereco2Bytes(unsigned int muiEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);


//escreve na memória no endereco de 3 bytes passado utilizando o byte passado em mucEndereco com o menos sig. dos 3
// e os 2 bytes de m25pxx_uiEndereco2BytesMSB para completar como o terceiro e o segundo mais sig. do endereco de 3 bytes
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_escreveDadosEndereco1Bytes(unsigned char mucEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho);


//le o status register e retorna dado num buffer como string (tamanho minimo do buffer: 12)
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_leStringStatus(char *mpcBuffer);


//apaga o setor ou toda a memoria, se for seto usa o setor apontado pela variavel m25pxx_ulUltimoEndereco
//valores para primeiro parametro(mucModo) de m25pxx_apagaMemoria()
//#define APAGA_SETOR 0
//#define APAGA_TUDO 1
#ifndef FLASH_M25PXX_C
extern
#endif
void m25pxx_apagaMemoria(unsigned char mucModo);


#endif
