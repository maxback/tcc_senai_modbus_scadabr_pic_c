/**
*** Definicao de tipos basicos para as bibliotecas / Defining Basic Types for Libraries
**/

#ifndef BASE_H
#define BASE_H

//id unico do modulo para passar nos eventos previstos em base.h / unique module id to pass the events provided in base.h
#define BASE_ID_MODULO 0

//definicao de NULL para ponteiros / NULL definition for pointers
#ifndef NULL
#define NULL 0
#endif


//estrutura para que os modulos possam receber da aplicacao os ponteiros que deve
//chamar de acordo com eventos que queira notificacao e que o modulo suporte
//todos os eventos tem por padrao um primeiro parametro com o id num�rico unico do modulo
//(segundo id de responsabilidade do modulo)
//tambem retornam todos um valor inteiro que deve ser interpretado de acordo com o evento e o modulo

//structure so that modules can receive from the application the pointers they should
// call according to events that you want notification and that the module supports
//all events have by default a first parameter with the module's unique numeric id
//(second module responsibility id)
//also all return an integer value that must be interpreted according to the event and the module
typedef struct
{
  //Ap�s m�dulo inicializado / After module initialized
  int (*OnInicializar)(unsigned char mucIdModulo);
  //Antes do m�dulo colocar dados na saida
  int (*OnAntesSaidaDados)(unsigned char mucIdModulo);
  //Ap�s o m�dulo colocar dados na saida / Before the module outputs data
  int (*OnDepoisSaidaDados)(unsigned char mucIdModulo);
  //Antes do m�dulo ler dados da entrada / Before the module reads input data
  int (*OnAntesEntradaDados)(unsigned char mucIdModulo);
  //Ap�s o m�dulo ler dados da entrada / After the module reads input data
  int (*OnDepoisEntradaDados)(unsigned char mucIdModulo);
} TBaseEventos; 


//macros para facilitar chamada dos eventos / macros to facilitate calling of events
#define base_doOnInicializar(PTRVAR, IDMODULO) (PTRVAR->OnInicializar != NULL ? (*PTRVAR->OnInicializar)(IDMODULO) : 0)
#define base_doOnAntesSaidaDados(PTRVAR, IDMODULO) (PTRVAR->OnAntesSaidaDados != NULL ? (*PTRVAR->OnAntesSaidaDados)(IDMODULO) : 0)
#define base_doOnDepoisSaidaDados(PTRVAR, IDMODULO) (PTRVAR->OnDepoisSaidaDados != NULL ? (*PTRVAR->OnDepoisSaidaDados)(IDMODULO) : 0)
#define base_doOnAntesEntradaDados(PTRVAR, IDMODULO) (PTRVAR->OnAntesEntradaDados != NULL ? (*PTRVAR->OnAntesEntradaDados)(IDMODULO) : 0)
#define base_doOnDepoisEntradaDados(PTRVAR, IDMODULO) (PTRVAR->OnDepoisEntradaDados != NULL ? (*PTRVAR->OnDepoisEntradaDados)(IDMODULO) : 0)


#endif
