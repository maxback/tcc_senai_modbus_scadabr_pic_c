/*
	utils.c
	Rotinas diveras para usar com o pic
	Autor: Rodrigo  P A 
	visite meu site: www.kitmcu.com.br

*/                     

#define UTIL_C

#include <p18cxxx.h>	// inclui as defini??es dos ios do pic
#include <timers.h>
#include "..\lib\base\base.h"
#include "tipos.h"
#include <delays.h>
#include "utils.h"


#define SERIAL_CLK PORTBbits.RB0
#define SERIAL_CLK_TRIS TRISBbits.TRISB0
#define SERIAL_DATA PORTBbits.RB5
#define SERIAL_DATA_TRIS TRISBbits.TRISB5


//estados para maquina de transmissao
#define MAQENV_PARADA 0
#define MAQENV_INICIANDO 1
#define MAQENV_IDLE 2
#define MAQENV_FIMIDLE 3
#define MAQENV_STARTBIT 4
#define MAQENV_DADOS 5
#define MAQENV_PARIDADE 6
#define MAQENV_PREPARA_ESPERA_STOPBIT 7
#define MAQENV_ESPERA_STOPBIT 8
#define MAQENV_ESPERA_ACK 9
#define MAQENV_ESPERA_IDLEFINAL 10


//#define MAQGERAL_PARADA 0
//#define MAQGERAL_ENVIANDO_CTS 1
//#define MAQGERAL_ENVIANDO_BYTE 2
//#define MAQGERAL_ESPERANDO_ACK 3
//#define MAQGERAL_RECEBENDO 4

//maquian de estago geral da comunica??o
unsigned char gucMaqGeral = MAQGERAL_PARADA; 

//este passo eh usado pelo controle interno nos estaddos e ? zerado por serialsinc_setEstado();
unsigned char gucPassoNoEstado = 0;

//se maior que 0 interrupcao vai mandando dados
unsigned char gucBitsAEnviar = 0;
unsigned char gucCmdEnvio = 0;
unsigned char gucByteEnvioAtual = 0;
unsigned char gucMaqEnvio = MAQENV_PARADA;
unsigned char gucTimeOutEnv = 0;
unsigned char gucContBitsAtivosEnv = 0;

//ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
static TBaseEventos *gpbeBaseEventos = NULL;


//ponteiro para funcao void(void) que se definida ? chaamda para ao ocorre interrup??o
void (*gpFuncHighInt)(void) = 0;              
//flag que garante que esta funcao nao seja executada varias vezes
//(pois eh passada por ponteiro para o vetor de alta prioridade)
unsigned char gucJahChamouPonteiroFuncHighInt = 0;

//ponteiro para funcao void(void) que se definida ? chamada ao concluir amda para ao ocorre interrup??o
void (*gpFuncCallbackResultadoRecSerial)(unsigned char mucDados, unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado) = 0;
//para o fi mdo envio
//ponteiro para funcao void(void) que se definida ? chamada ao concluir amda para ao ocorre interrup??o
void (*gpFuncCallbackResultadoEnvSerial)(unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado) = 0;

//seta funcao de call back ao ocorrer evento de timer (config. a cada 250 uS)
//passa a ser um array para os modulos interessandos adicionarem suas funcoes
#define MAX_QTD_ITENS_CALLBACKTICKTIMER 2
void (*gpFuncCallbackTickTimer[MAX_QTD_ITENS_CALLBACKTICKTIMER])(void);
//QTD DE ITENS USADOS
unsigned char gucQtdItensCallbackTickTimer = 0; 
//flag para salber se jah esta tratando callback (para ocaso da func, de int ser chamda recursivamente)  
unsigned char gucQtdItensCallbackTickTimerSendoChamados = 0; 



                                                                   
#pragma interrupt high_int save=section(".tmpdata"),PROD
void high_int (void)
{
  unsigned char ucDado, i, par; 
  unsigned char ucFlagTimeout = 0, ucFlagEventoInto0;
  
  //testa flag de timer aqui para manipular timeotus da comunica??o
  if(INTCONbits.TMR0IF)
  {
    //timeout de envio, ousado tambem para espera do ack e envio de cts
    if(gucTimeOutEnv)
    {
      gucTimeOutEnv--;
      if(!gucTimeOutEnv)
      {
        ucFlagTimeout = 1;
      }
    }
    
    //se nao vai controlar timeout da serial, dispaa callback de tick do timer
    //caso definido
    //if(!ucFlagTimeout && gpFuncCallbackTickTimer)
    if(gucQtdItensCallbackTickTimer)
    {
      //so faz loop se j? n?o est? sendo tratado
      if(!gucQtdItensCallbackTickTimerSendoChamados)
      {
        //indica que esta tratando
        gucQtdItensCallbackTickTimerSendoChamados = 1;

        //chama intes do array
        for(i=0; i < gucQtdItensCallbackTickTimer; i++)
        {
         (*gpFuncCallbackTickTimer[i])();
        } 
        //indica que n?o esta mais tratando
        gucQtdItensCallbackTickTimerSendoChamados = 0;
      }  
    }
  }
  

  //se int 0 trata dados vindso pela serial sinc. ou flag de timeout ou passo == 0

  //se gucPassoNoEstado ? 0, entra nela independente no estado
  //(cada estad odeve iniciar sua operacao neste pssso e mudado para outro valor)
  if(INTCONbits.INT0IF || ucFlagTimeout || !gucPassoNoEstado )
  {
    ucDado = SERIAL_DATA;
    ucFlagEventoInto0 = INTCONbits.INT0IF;
    //apos salvar estado do flag, zera 
    INTCONbits.INT0IF = 0;

    //se recebendo
    switch(gucMaqGeral)
    {
      /////////////////////////////// ENVIANDO CTS //////////////////////////////
      case MAQGERAL_ENVIANDO_CTS:
      {   
#ifdef SERIALSINC_APENASENTRADA
        //muda maquina geral para parada e chama funcao de callback que dever?
        //colocar em modo de recepcao
        gucMaqGeral = MAQGERAL_PARADA;
        //passo = 0 para iniciarlizar maq, se for o caso
        gucPassoNoEstado = 0;
        //chama evento de enviado antes de mudar a maquina
        if(gpFuncCallbackResultadoEnvSerial)
        {
          //chamada
          (*gpFuncCallbackResultadoEnvSerial)(MAQGERAL_ENVIANDO_CTS, &gucMaqGeral);
        }

#else      
        //de acordo com o passo
        switch(gucPassoNoEstado)
        {
          //inicializacao
          case 0:
          {
            //configura saida do clock e data como saidas
            SERIAL_CLK_TRIS = 0;
            SERIAL_DATA_TRIS = 0;
            //coloca 1 no data (para garantir) e 0 no click para mandar o cts
            SERIAL_DATA = 1;
            SERIAL_CLK = 0;
            //temporiza com o timeout de envio
            gucTimeOutEnv = 1;
            //passa para prox. passo que esperar o timeout
            gucPassoNoEstado = 1;  
          }
          break;
          
          //// fim da espera pelo tempo com clk em 0
          case 1:
          {
            //coloca clock em 1 novamente
            SERIAL_CLK = 1;
            //config. clock e data apra entrada novamente
            SERIAL_CLK_TRIS = 1;
            SERIAL_DATA_TRIS = 1;
            
            //muda maquina geral para parada
            gucMaqGeral = MAQGERAL_PARADA;
            //passo = 0 para iniciarlizar maq, se for o caso
            gucPassoNoEstado = 0;
            
            //chama evento de enviado antes de mudar a maquina
            if(gpFuncCallbackResultadoEnvSerial)
            {
              //chamada
              (*gpFuncCallbackResultadoEnvSerial)(MAQGERAL_ENVIANDO_CTS, &gucMaqGeral);
            }
          }
          break;
        }
#endif        
      }
      break;
      
      /////////////////////////////// ENVIANDO /////////////////////////////////
      case MAQGERAL_ENVIANDO_BYTE: 
      {
#ifdef SERIALSINC_APENASENTRADA
        //como ? bidirecional, pura direto apra evento de byte enviado
        //muda maq para esperda do ack e poe um timeout minimo para em seguida
        //sair deste estado (como se o ACK n?o fosse recebido)
        gucMaqGeral = MAQGERAL_ESPERANDO_ACK;
        //poe passo > 0 para jah entrar tratando a borda do clock e nao inicialziando
        gucPassoNoEstado = 1;
        //aqui chama callback de envio
        if(gpFuncCallbackResultadoEnvSerial)
        {
          //chama
          (*gpFuncCallbackResultadoEnvSerial)(MAQGERAL_ENVIANDO_BYTE, &gucMaqGeral);
          //aqui teste se mudou maq, se for ocaso, muda passo apra 0
          if(gucMaqGeral != MAQGERAL_ESPERANDO_ACK)
          {
            //passo = 0 para iniciarlizar maq, se for o caso
            gucPassoNoEstado = 0;
          }
        }
        //coloca um timeout minimo
        gucTimeOutEnv = 1;    

#else                    
        //trata maquina de envio de dados
        
        //se passo == 0 poe maquina enviando e muda passo, senao manda a maquina
        if(!gucPassoNoEstado)
        {
          gucPassoNoEstado++;
          gucMaqEnvio = MAQENV_INICIANDO;
        }
        
        //processa maquina de estados
        switch(gucMaqEnvio)
        {  
          case MAQENV_INICIANDO: 
            //inicializa controles
            gucBitsAEnviar = 8;     
            //copia comando apra byte de envio
            gucByteEnvioAtual = gucCmdEnvio;
          
            //muda portas para modo de saida (pois ? bidirecional)
            SERIAL_CLK_TRIS = 0;
            SERIAL_DATA_TRIS = 0;
            //clk em 0 e data em 1
            SERIAL_CLK = 0;
            SERIAL_DATA = 1;

            //cont para paridade
            gucContBitsAtivosEnv = 0;
            //timeout
            gucTimeOutEnv = 5;
            gucMaqEnvio = MAQENV_IDLE;
          break;
          
          case MAQENV_FIMIDLE:
          
            //fim da sinalzia??o que libera o device para enviar            

            //data em 0 e clock em 1
            SERIAL_DATA = 0;
            SERIAL_DATA = 0;
            SERIAL_DATA = 0;
            SERIAL_DATA = 0; //para dar um tempinho
            SERIAL_CLK = 1;
            SERIAL_CLK = 1; //para dar um tempinho
            
            //clock volta a ser entrada
            SERIAL_CLK_TRIS = 1;

            //muda interrupcao para ser na borda de subida
            INTCON2bits.INTEDG0 = 1;
            //mud maqui para iniciar envio dos bits
            gucMaqEnvio = MAQENV_DADOS;
            //o data jah em 0 ? o start bit
            
            //coloca um timeout para desistir dos dados se as bordas nao vierem
            //gucTimeOutEnv = 50;
          break;
          
          case MAQENV_DADOS:
  
  
            //a cada borda de subida coloca o data, enquanto houver
            if(gucByteEnvioAtual&0x01)
            {
              SERIAL_DATA = 1;
              gucContBitsAtivosEnv++; 
            }
            else
            {
              SERIAL_DATA = 0;
            }
            //desloca dado
            gucByteEnvioAtual >>= 1;
            //dec qtd de bits faltantes
            gucBitsAEnviar--;
            //se foi o ultimo vai apra paridade
            if(!gucBitsAEnviar)
            {
              gucMaqEnvio = MAQENV_PARIDADE;
            }
            
          break;
          case MAQENV_PARIDADE:
            //se qtd de bits impar por 0, senao 1
            SERIAL_DATA = gucContBitsAtivosEnv&0x01 ? 0 : 1;
            //vai para a espera do stop o stop bit
            gucMaqEnvio = MAQENV_PREPARA_ESPERA_STOPBIT;
            //coloca um timeout para desistir dos dados se as bordas nao vierem
            gucTimeOutEnv = 50;
                      
          break;
          
          case MAQENV_PREPARA_ESPERA_STOPBIT:
          
            //aqui poe data em 1
            SERIAL_DATA = 1;
            
           //muda interrupcao para ser na borda de descida novamente
            INTCON2bits.INTEDG0 = 0;
            //e data para ser entrada novamente
            SERIAL_DATA_TRIS = 1;
            
            //vai apra maquina apos stop lido
            gucMaqEnvio = MAQENV_ESPERA_STOPBIT;
  
            //coloca um timeout para desistir dos dados se as bordas nao vierem
            gucTimeOutEnv = 50;        
          break;
          
          case MAQENV_ESPERA_STOPBIT:
          
            //muda maq para esperda do ack
            gucMaqGeral = MAQGERAL_ESPERANDO_ACK;
            //poe passo > 0 para jah entrar tratando a borda do clock e nao inicialziando
            gucPassoNoEstado = 1;
          
            //aqui chama callback de envio
            if(gpFuncCallbackResultadoEnvSerial)
            {
              //chama
              (*gpFuncCallbackResultadoEnvSerial)(MAQGERAL_ENVIANDO_BYTE, &gucMaqGeral);
              //aqui teste se mudou maq, se for ocaso, muda passo apra 0
              if(gucMaqGeral != MAQGERAL_ESPERANDO_ACK)
              {
                //passo = 0 para iniciarlizar maq, se for o caso
                gucPassoNoEstado = 0;
              }
            }
            
            //coloca um timeout para desistir do recebimento
            gucTimeOutEnv = 50;    
                        
          break;
        }
#endif            
      }
      break;
      /////////////////////////////// ESPERANDO ACK ////////////////////////////
      case MAQGERAL_ESPERANDO_ACK:
      {
#ifdef SERIALSINC_APENASENTRADA
        //passado o timeout volta
        gucMaqEnvio = MAQENV_PARADA;
        gucMaqGeral = MAQGERAL_PARADA; 

        //cham a callback passando o valor da ack recebio como bem sucedido (0)
        if(gpFuncCallbackResultadoRecSerial)
        {
          (*gpFuncCallbackResultadoRecSerial)(0, MAQGERAL_ESPERANDO_ACK,
           &gucMaqGeral);
        }
        
#else              
        //se passo 0 inc
        if(!gucPassoNoEstado)
        {
          //coloca clock e data como entradas
          SERIAL_CLK_TRIS = 1;
          SERIAL_DATA_TRIS = 1;
          gucPassoNoEstado++;
        }
        //se foi timeout poe 1 (que indica falha)
        else if(ucFlagTimeout)
        {
          serialsinc_ucValorACKRecebido = 1;
        }
        //senao trata borda
        else
        {
          //senao pega valor
          serialsinc_ucValorACKRecebido = SERIAL_DATA;
        }  

        //FINALZIA MAQUINA
        gucMaqEnvio = MAQENV_PARADA;
        gucMaqGeral = MAQGERAL_PARADA; 

        //cham a callback passando o valor da ack recebio
        if(gpFuncCallbackResultadoRecSerial)
        {
          (*gpFuncCallbackResultadoRecSerial)(serialsinc_ucValorACKRecebido, MAQGERAL_ESPERANDO_ACK,
           &gucMaqGeral);
        }
#endif        
      }
      break;
            
      /////////////////////////////// RECEBENDO ///////////////////////////////// 
      case MAQGERAL_RECEBENDO:
      {
        //se passo == 0 poe contador de bits recebidos no inicio e muda passo para
        //daqui em diante mandar o contador de bits
        if(!gucPassoNoEstado)
        {
          gucPassoNoEstado++;
          util_ucQtdBitsRecebidos = 0;
#ifndef SERIALSINC_APENASENTRADA          
          //coloca o clock em 1 novamente, se estiver como saida
          if(!SERIAL_CLK_TRIS)
          {
            //coloca saida em 1 para ficar em idle
            SERIAL_CLK = 1; 
          }

          //coloca o data em 1 novamente, se estiver como saida
          if(!SERIAL_DATA_TRIS)
          {
            //coloca saida em 1 para ficar em idle
            SERIAL_DATA = 1; 
          }          
          //coloca clock e data como entradas
          SERIAL_CLK_TRIS = 1;
          SERIAL_DATA_TRIS = 1;          
#endif
          //se n?o foi evento borda junto, sai
          if(!ucFlagEventoInto0)
          {
            return;
          }
        }
        
  
        INVPINO();    
    
        //salva dado
        util_ulValorRecebido >>= 1;
        if(ucDado)
        {
          //(11 bits)
          util_ulValorRecebido |= 0x0400;   
        }
        //se esperando inicio, verifica se ? start bit senao deixa de contar
        switch(util_ucQtdBitsRecebidos)
        {
          //start bit
          case 0:
            if(ucDado) return;
          break;
          //stop bit
          case 10:
            //se errado retorna isto 
            if(!ucDado)
            {
              util_ucEventoSerialSinc = UTIL_EVT_SERIAL_ERRO_STOP;
              //mesmo assim desloca para passar o valor do byte recebido
              //tira o start bit fora
              util_ulValorRecebido >>= 1;
            }
            //senao verifica paridade
            else
            {
              //tira o start bit fora
              util_ulValorRecebido >>= 1;
              //conta bits em 1 dos dados ateh a paridade (que ? par) e poe em par
              par = 0;
              //copia dos dados
              ucDado = util_ulValorRecebido&0x00ff;
              for(i=9; i; i--)
              {
                if(ucDado&0x01)
                {
                  par++;
                }
                //desloca
                ucDado >>= 1;
              }
              //ao final, soma-se com o bit de paridade
              if(util_ulValorRecebido&0x100)
              {
               par++;
              }
              //se somando com o bit de paridade der inpar, indica erro
              if(par&0x01)
              {
                util_ucEventoSerialSinc = UTIL_EVT_SERIAL_ERRO_PARIDADE;
              }
              //senao dados ok
              //tira o bit de paridade para ficar o valor puro
              util_ulValorRecebido &= 0x00ff;
              util_ucEventoSerialSinc = UTIL_EVT_SERIAL_RECEBIDO;
            }

#ifndef SERIALSINC_APENASENTRADA          
            //coloca clock em 0 para impedir que venha outro byte (como e bidirecional)
            SERIAL_CLK_TRIS = 0;
            SERIAL_CLK = 0;
#endif            
            //poe passo para 0 para um novo inicio de recepcao
            //(ou outro estado que psosa ser alterado)
            gucPassoNoEstado = 0;
            
            //cham a callback passando o valor recebido recebio
            if(gpFuncCallbackResultadoRecSerial)
            {
              (*gpFuncCallbackResultadoRecSerial)((unsigned char)util_ulValorRecebido,
                MAQGERAL_RECEBENDO, &gucMaqGeral);
            }
            
            //reseta contador de bits
            util_ucQtdBitsRecebidos = 0;
            
            //sai
            return;
        }
        util_ucQtdBitsRecebidos++;
      }
      break;
    }  
  }    
  
	if(INTCONbits.TMR0IF) // A interrup??o foi desencadeada pelo "overflow" do TMR0 ? // <en> Was it a TMR0 overflow ?
	{	
		tempo++;	// incremento a vari?vel tempo
		WriteTimer0(65536-tempo_int); // Restart TMR0
		INTCONbits.TMR0IF = 0; // Clear TMR0 Overflow flag
	}
	
}

#pragma code _HIGH_INTERRUPT_VECTOR = 0x000808
void _high_ISR (void)
{
   _asm goto high_int _endasm; // Desvio para a rotina de "tratamento" de interrup??es
}

/*
	Configura o timer para gerar uma interrup??o a cada 50uS
*/
#pragma code
void Iniciar_Timer(void)
{
  //isto noq pode ser assim:
	//INTCON=0x00; // Todas interrup??es desabilitadas // <en> All interrupts disabled
	
	RCONbits.IPEN=1; // Interrupt Priority Enable bit
	INTCONbits.TMR0IE=1; // TMR0 Overflow Interrupt Enable bit
	INTCON2bits.TMR0IP=1; // TMR0 Overflow Interrupt Priority bit (1=HIGH)
	
	OpenTimer0(T0_16BIT & T0_SOURCE_INT & T0_PS_1_64 ); // Setando: T0CON=0b10110111; // <en> Setting: T0CON=0b10110111;
	WriteTimer0(65536-tempo_int);
	
	INTCONbits.GIEH=1; // Global Interrupt Enable bit
  
  //qtd itens callback tick timer
  gucQtdItensCallbackTickTimer = 0; 
    	
}


void serialsinc_iniciar(TBaseEventos *mpbeBaseEventos)
{
  util_ulValorRecebido = 0;
  util_ucEventoSerialSinc = UTIL_EVT_SERIAL_NENNHUM;
  util_ucQtdBitsRecebidos = 0;
  
  serialsinc_ucValorACKRecebido = 0;
  
  //ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
  gpbeBaseEventos = mpbeBaseEventos;


#ifdef SERIALSINC_APENASENTRADA
  //poe pinos como entradas pois apenas isto ? permitido
  SERIAL_CLK_TRIS = 1;
  SERIAL_DATA_TRIS = 1;
#else
  //poe pino de clp como saida no inicio e ata jah como entrada
  SERIAL_CLK_TRIS = 0;
  SERIAL_DATA_TRIS = 1;

  //poe clock em 0
  SERIAL_CLK = 0;
#endif  
  
  //para debug
  if(BARCODE_DEBUG_PINO)
  {
    //poe como saida
    BARCODE_DEBUG_TRIS = 0;
    //inicia co mvalor 1
    BARCODE_DEBUG_PIN = 1;
  }  

  //configura interrupcoes

  
  RCONbits.IPEN=1; // Interrupt Priority Enable bit
  
  //seleciona borda de descida
  INTCON2bits.INTEDG0 = 0;
  
  //habilita interrup??o INT0
	INTCONbits.INT0IE=1; // INT0
	// prioridade alta sempre
	
	INTCONbits.GIEH=1; // Global Interrupt Enable bit	  

#ifndef SERIALSINC_APENASENTRADA
  //poe clk em 1 (assim tira de espera como estava) e muda apra entrada
  SERIAL_CLK = 1;
  SERIAL_DATA_TRIS = 1;
#endif  

}

//seta ponteiro para funcao void(void) que se definida ? chaamda para ao ocorre interrup??o
void serialsinc_setFuncHighInt(void (*mpFuncHighInt)(void))
{
  gpFuncHighInt = mpFuncHighInt;
}


//seta funcao de call back ao terminar com sucesso o uerro a rec. de um byte
void serialsinc_setFuncCallbackResultadoRecSerial(void (*mpFunc)(unsigned char mucDados, unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado))
{
  gpFuncCallbackResultadoRecSerial = mpFunc;
}

//seta funcao de call back ao terminar e enviar um byte
void serialsinc_setFuncCallbackResultadoEnvSerial(void (*mpFunc)(unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado))
{
  gpFuncCallbackResultadoEnvSerial = mpFunc;
}

//inicia envio de um byte pela serial sincrona
void serialsinc_setByteEnvio(unsigned char mucValor, unsigned char mucIniciarEnvio)
{
  //pede reeenvio
  gucCmdEnvio = mucValor; 
  //se inicia aqui jah muda a maquina
  if(mucIniciarEnvio)
  {
    serialsinc_setEstado(MAQGERAL_ENVIANDO_BYTE);
  }              
}






//seta funcao de call back ao ocorrer evento de timer (config. a cada 250 uS)
void utils_setFuncCallbackTickTimer(void (*mpFunc)(void))
{
  //se espaco no array
  if(gucQtdItensCallbackTickTimer < MAX_QTD_ITENS_CALLBACKTICKTIMER)
  {
    gpFuncCallbackTickTimer[gucQtdItensCallbackTickTimer] = mpFunc;
    gucQtdItensCallbackTickTimer++;
  }  
}


void serialsinc_setEstado(unsigned char mucEstado)
{
  gucMaqGeral = mucEstado;
  gucPassoNoEstado = 0;
  
#ifndef SERIALSINC_APENASENTRADA          
  //se for parado, muda clock pra saida e poe em 0
  if(gucMaqGeral == MAQGERAL_PARADA)
  {
    SERIAL_CLK_TRIS = 0;
    SERIAL_CLK = 0;
  }
#endif    
}

//le o estado
unsigned char  serialsinc_ucGetEstado(void)
{
  return gucMaqGeral;
}



#pragma code
void restart_wdt(void){
	_asm clrwdt _endasm
}

/*
	rotina de delay, sem precis?o, feita 
	somente para testar o exemplo
	Rodrigo P A
	Funcionamento:
		Usa a rotina de delay, da biblioteca da microchip
		a rotina Delay10TCYx() d? um delay de 10 ciclos do 
		pic. Como esse pic roda a 12MIPS, ele executa
		12 x 10 ^6 instrucoes por segundo
		para obtermos um atraso de 1ms, o pic executa 12000 instrucoes
		como a rotina d? um atraso de 10 ciclo, ? s? cham?-la 
		com um delay de 10 x 1200 ciclos
*/
#pragma code
void delay_ms(unsigned int t){
	tempo=0x00;
	while ( tempo<(t*4) ) ;
}


#pragma code
/*
	delay em microsegundos
	utilizando a biblioteca da microchip
	
*/
void delay_us(int16 t){
	unsigned int i,i2;
	for (i=0;i<t;i++) {
		Delay10TCYx(200);
	}
}

