/**
*** Arquivo de implementação do módulo de controle de barramento/multiplexaçào das portas do MCU
**/
#include <p18cxxx.h>	// inclui as definições dos ios do pic

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


#define BARRA_C

#include "..\lib\base\base.h"
#include "barra.h"
#include "global.h"

//globais
unsigned char gucIndiceEntrada;
unsigned char gucIndiceSaida;
unsigned char gucSaidaHabilitada;

//macro para aplciar mascara pelo bit
#define APLICABITSAIDA(IND, MASCIND, IDP, MASCIDP) IDP = (IND&MASCIND) == MASCIND ?  IDP|MASCIDP : IDP&(~MASCIDP)


//atualzia saida com base em gucIndiceSaida e gucSaidaHabilitada
void atualizaSaida(void)
{
  unsigned char ucValorPorta;
  
  //pega valor da entrada
#ifdef BARRA_PORTA_SEL_SAIDA  
  ucValorPorta = BARRA_PORTA_SEL_SAIDA;
#endif
  //seta e reseta cada bit de acordo com o valor de gucIndiceSaida para os bits definidos

  //bit 0  
#ifdef BARRA_MASCARABIT_SEL_SAI0
  APLICABITSAIDA(gucIndiceSaida, 0x01, ucValorPorta, BARRA_MASCARABIT_SEL_SAI0);
#endif     
  //bit 1  
#ifdef BARRA_MASCARABIT_SEL_SAI1
  APLICABITSAIDA(gucIndiceSaida, 0x02, ucValorPorta, BARRA_MASCARABIT_SEL_SAI1);
#endif     
  //bit 2  
#ifdef BARRA_MASCARABIT_SEL_SAI2
  APLICABITSAIDA(gucIndiceSaida, 0x04, ucValorPorta, BARRA_MASCARABIT_SEL_SAI2);
#endif     
  //bit 3  
#ifdef BARRA_MASCARABIT_SEL_SAI3
  APLICABITSAIDA(gucIndiceSaida, 0x08, ucValorPorta, BARRA_MASCARABIT_SEL_SAI3);
#endif     
  //bit 4  
#ifdef BARRA_MASCARABIT_SEL_SAI4
  APLICABITSAIDA(gucIndiceSaida, 0x10, ucValorPorta, BARRA_MASCARABIT_SEL_SAI4);
#endif     
  //bit 5  
#ifdef BARRA_MASCARABIT_SEL_SAI5
  APLICABITSAIDA(gucIndiceSaida, 0x20, ucValorPorta, BARRA_MASCARABIT_SEL_SAI5);
#endif     
  //bit 6  
#ifdef BARRA_MASCARABIT_SEL_SAI6
  APLICABITSAIDA(gucIndiceSaida, 0x40, ucValorPorta, BARRA_MASCARABIT_SEL_SAI6);
#endif     
  //bit 7  
#ifdef BARRA_MASCARABIT_SEL_SAI7
  APLICABITSAIDA(gucIndiceSaida, 0x80, ucValorPorta, BARRA_MASCARABIT_SEL_SAI7);
#endif     
  //aplica a mascara para desligar os bits nao selecionados e marcar os selecionados
#ifdef BARRA_PORTA_SEL_SAIDA  
  BARRA_PORTA_SEL_SAIDA = ucValorPorta;
#endif  
  
  //atualzia habilitacao (habilita em 0 no circuito)
#ifdef BARRA_HAB_SAIDA  
  BARRA_HAB_SAIDA = !gucSaidaHabilitada;
#endif   
}  

//atialzia entradas
void atualizaEntrada(void)
{
  unsigned char ucValorPorta;
  
  //pega valor da entrada
#ifdef BARRA_PORTA_SEL_ENTRADA  
  ucValorPorta = BARRA_PORTA_SEL_ENTRADA;
#endif
  //seta e reseta cada bit de acordo com o valor de gucIndiceEntrada para os bits definidos

  //bit 0  
#ifdef BARRA_MASCARABIT_SEL_ENT0
  APLICABITSAIDA(gucIndiceEntrada, 0x01, ucValorPorta, BARRA_MASCARABIT_SEL_ENT0);
#endif     
  //bit 1  
#ifdef BARRA_MASCARABIT_SEL_ENT1
  APLICABITSAIDA(gucIndiceEntrada, 0x02, ucValorPorta, BARRA_MASCARABIT_SEL_ENT1);
#endif     
  //bit 2  
#ifdef BARRA_MASCARABIT_SEL_ENT2
  APLICABITSAIDA(gucIndiceEntrada, 0x04, ucValorPorta, BARRA_MASCARABIT_SEL_ENT2);
#endif     
  //bit 3  
#ifdef BARRA_MASCARABIT_SEL_ENT3
  APLICABITSAIDA(gucIndiceEntrada, 0x08, ucValorPorta, BARRA_MASCARABIT_SEL_ENT3);
#endif     
  //bit 4  
#ifdef BARRA_MASCARABIT_SEL_ENT4
  APLICABITSAIDA(gucIndiceEntrada, 0x10, ucValorPorta, BARRA_MASCARABIT_SEL_ENT4);
#endif     
  //bit 5  
#ifdef BARRA_MASCARABIT_SEL_ENT5
  APLICABITSAIDA(gucIndiceEntrada, 0x20, ucValorPorta, BARRA_MASCARABIT_SEL_ENT5);
#endif     
  //bit 6  
#ifdef BARRA_MASCARABIT_SEL_ENT6
  APLICABITSAIDA(gucIndiceEntrada, 0x40, ucValorPorta, BARRA_MASCARABIT_SEL_ENT6);
#endif     
  //bit 7  
#ifdef BARRA_MASCARABIT_SEL_ENT7
  APLICABITSAIDA(gucIndiceEntrada, 0x80, ucValorPorta, BARRA_MASCARABIT_SEL_ENT7);
#endif     
  //aplica a mascara para desligar os bits nao selecionados e marcar os selecionados
#ifdef BARRA_PORTA_SEL_ENTRADA  
  BARRA_PORTA_SEL_ENTRADA = ucValorPorta;
#endif  
}

//inicializa o módulo
void barra_iniciar(unsigned char mucIndiceEntrada, unsigned char mucIndiceSaida,
  unsigned char mucSaidaHabilitada, TBaseEventos *mpbeBaseEventos)
{
  gucIndiceEntrada = mucIndiceEntrada;
  gucIndiceSaida = mucIndiceSaida;
  gucSaidaHabilitada = mucSaidaHabilitada;

  //configura pinos para saida
  //selecao da entrada
#ifdef BARRA_SEL_ENT0_TRIS
  BARRA_SEL_ENT0_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT1_TRIS
  BARRA_SEL_ENT1_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT2_TRIS
  BARRA_SEL_ENT2_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT3_TRIS
  BARRA_SEL_ENT3_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT4_TRIS
  BARRA_SEL_ENT4_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT5_TRIS
  BARRA_SEL_ENT5_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT6_TRIS
  BARRA_SEL_ENT6_TRIS = 0;
#endif
#ifdef BARRA_SEL_ENT7_TRIS
  BARRA_SEL_ENT7_TRIS = 0;
#endif

  //selecao a saida
#ifdef BARRA_SEL_SAI0_TRIS
  BARRA_SEL_SAI0_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI1_TRIS
  BARRA_SEL_SAI1_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI2_TRIS
  BARRA_SEL_SAI2_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI3_TRIS
  BARRA_SEL_SAI3_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI4_TRIS
  BARRA_SEL_SAI4_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI5_TRIS
  BARRA_SEL_SAI5_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI6_TRIS
  BARRA_SEL_SAI6_TRIS = 0;
#endif
#ifdef BARRA_SEL_SAI7_TRIS
  BARRA_SEL_SAI7_TRIS = 0;
#endif

//habiltiacao da saida
#ifdef BARRA_HAB_SAIDA_TRIS
  BARRA_HAB_SAIDA_TRIS = 0;
#endif  


//configuracoes pinos de dados por bits (10 bits previstos)
#ifdef BARRA_SAI_B0_TRIS
  BARRA_SAI_B0_TRIS = 0;
#endif
#ifdef BARRA_SAI_B1_TRIS
  BARRA_SAI_B1_TRIS = 0;
#endif
#ifdef BARRA_SAI_B2_TRIS
  BARRA_SAI_B2_TRIS = 0;
#endif
#ifdef BARRA_SAI_B3_TRIS
  BARRA_SAI_B3_TRIS = 0;
#endif
#ifdef BARRA_SAI_B4_TRIS
  BARRA_SAI_B4_TRIS = 0;
#endif
#ifdef BARRA_SAI_B5_TRIS
  BARRA_SAI_B5_TRIS = 0;
#endif
#ifdef BARRA_SAI_B6_TRIS
  BARRA_SAI_B6_TRIS = 0;
#endif
#ifdef BARRA_SAI_B7_TRIS
  BARRA_SAI_B7_TRIS = 0;
#endif
#ifdef BARRA_SAI_B8_TRIS
  BARRA_SAI_B8_TRIS = 0;
#endif
#ifdef BARRA_SAI_B9_TRIS
  BARRA_SAI_B9_TRIS = 0;
#endif
#ifdef BARRA_SAI_B10_TRIS
  BARRA_SAI_B10_TRIS = 0;
#endif
#ifdef BARRA_SAI_B11_TRIS
  BARRA_SAI_B11_TRIS = 0;
#endif
#ifdef BARRA_SAI_B12_TRIS
  BARRA_SAI_B12_TRIS = 0;
#endif
#ifdef BARRA_SAI_B13_TRIS
  BARRA_SAI_B13_TRIS = 0;
#endif
#ifdef BARRA_SAI_B14_TRIS
  BARRA_SAI_B14_TRIS = 0;
#endif
#ifdef BARRA_SAI_B15_TRIS
  BARRA_SAI_B15_TRIS = 0;
#endif

  //copia para saida
  atualizaSaida();
  //atualzia entrada atual
  atualizaEntrada();
}  


//seta o indice da saida atual que deverá receber os daodos na saida do MCU
void barra_setSaidaAtual(unsigned char mucIndiceSaida)
{
  gucIndiceSaida = mucIndiceSaida;
  //copia para saida
  atualizaSaida();
}

//seta o estado de habilitação da saída selecionada (0 - alta impedancia, 1 - saida habilitada)
void barra_setSaidaAtiva(unsigned char mucSaidaAtiva)
{
  gucSaidaHabilitada = mucSaidaAtiva;
  //copia para saida
  atualizaSaida();
}

//retorna o indice da saida atual que deverá receber os daodos na saida do MCU
unsigned char barra_getSaidaAtual(void)
{
  return gucIndiceSaida;
}

//retorna  o estado de habilitação da saída selecionada (0 - alta impedancia, 1 - saida habilitada)
unsigned char barra_getSaidaAtiva(void)
{
  return gucSaidaHabilitada;
}


//seta o indice da entrada atual que deverá ter os dados copiados para o micrcocontrolador
void barra_setEntradaAtual(unsigned char mucIndiceEntrada)
{
  gucIndiceEntrada = mucIndiceEntrada;
  //atualzia entrada atual
  atualizaEntrada();
}

//retorna o indice da entrada atual que deverá ter os dados copiados para o micrcocontrolador
unsigned char barra_getEntradaAtual(void)
{
  return gucIndiceEntrada;
}
