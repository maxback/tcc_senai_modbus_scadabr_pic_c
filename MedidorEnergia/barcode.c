/***                  
**** Arquivo de implementação do módulo de controle de leitura de barcode no padrão de teclado ps2
**** Barcode reading control module implementation file in ps2 keyboard pattern
***/
#include <p18cxxx.h>	// inclui as definições dos ios do pic / includes the ios definitions from the pic

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


//definir para imprimir dados de Debug / set to print Debug data
//#define IMPRIME_DEBUG


#define BARCODE_C

#include "..\lib\base\base.h"
#include "barcode.h"
#include "dados.h"
#include "global.h"
#include "..\lib\kitmcu\utils.h"

//pinos
//1 - amarelo=clk / yellow=clk
#define BARCODE_CLK PORTBbits.RB0
#define BARCODE_CLK_TRIS TRISBbits.TRISB0
//2 = cinza=gnd / gray=gnd
//3 - vermelho=data / red=date
#define BARCODE_DATA PORTBbits.RB1
#define BARCODE_DATA_TRIS TRISBbits.TRISB1
//5= marrom=vcc / brown=vcc

//pino de debub / debub pin
//#define BARCODE_DEBUG_PINO 1
//#define BARCODE_DEBUG_PIN PORTBbits.RB2
//#define BARCODE_DEBUG_TRIS TRISBbits.TRISB2

//pino de debub / debub pin
//#define INVPINO() if(BARCODE_DEBUG_PINO){ BARCODE_DEBUG_PIN = !BARCODE_DEBUG_PIN; }

#define delayus(t) Delay10TCYx(t)


//comando para o teclado / keyboard command
//ED	Set Status LED's - This command can be used to turn on and off the Num Lock, Caps Lock & Scroll Lock LED's. After Sending ED, keyboard will reply with ACK (FA) and wait for another byte which determines their Status. Bit 0 controls the Scroll Lock, Bit 1 the Num Lock and Bit 2 the Caps lock. Bits 3 to 7 are ignored.
#define KBD_SETSTATUSLED 0xed	
//EE	Echo - Upon sending a Echo command to the Keyboard, the keyboard should reply with a Echo (EE)
#define KBD_ECHO 0xee	
//F0	Set Scan Code Set. Upon Sending F0, keyboard will reply with ACK (FA) and wait for another byte, 01-03 which determines the Scan Code Used. Sending 00 as the second byte will return the Scan Code Set currently in Use
#define KBD_SETSCANCODESET 0xf0
//F3	Set Typematic Repeat Rate. Keyboard will Acknowledge command with FA and wait for second byte, which determines the Typematic Repeat Rate.
#define KBD_SETREPEATRATE 0xf3
//F4	Keyboard Enable - Clears the keyboards output buffer, enables Keyboard Scanning and returns an Acknowledgment.
#define KBD_ENABLE 0xf4
//F5	Keyboard Disable - Resets the keyboard, disables Keyboard Scanning and returns an Acknowledgment.
#define KBD_DISABLE 0xf5
//FE	Resend - Upon receipt of the resend command the keyboard will re- transmit the last byte sent.
#define KBD_RESEND 0xfe
//FF	Reset - Resets the Keyboard.
#define KBD_RESET 0xff

//ultimo caracter recebido / last character received
unsigned char gucUltimoCaracterRecebido = 0;
//indica se deixou em idle o device / indicates if you left the device idle
unsigned char gucEstahEmEspera = 0;
//para timeouts de entrada / for input timeouts
unsigned long gulTimeOut = 0;

//eventos base / base events
TBaseEventos *gpbeBaseEventos = NULL;

//controle de envio de comandos com mais de um byte / control sending commands with more than one byte
struct 
{
  unsigned char ucLista[3];
  unsigned char ucIndice;
  unsigned char ucTotal;
  unsigned char ucEsperandoAck;
} gEnvioCmd;

//prototipos / prototypes
unsigned char ucEnviaComandoKdb(unsigned char mucCmd, unsigned char *mpucCmdsAdicionais, 
  unsigned char mucQtdCmdsAdicionais);
void funcCallbackTickTimerParaTimeout(void);  


typedef struct
{
  unsigned char ScanCode;
  unsigned char VirtualCode;
  unsigned char AsciiCode;
  unsigned char AsciiCodeShift;  
} TItemListaTrad;
  
//tabela na area de codigo com a traducao dos codigos de rastreamente / table in code area with translation of tracking codes
const TItemListaTrad gltListaTradEspeciais[]  = {
{0x11, BARCODE_ALT, 0},  
 {0x14, VK_RCONTROL, 0} 
};


const TItemListaTrad gltListaTradNormais[] = {
{0x11,  BARCODE_ALT , 0,    0},  
{0x14,  VK_LCONTROL , 0,    0},
{0x12,  VK_LSHIFT   , 0,    0},
{0x59,  VK_RSHIFT   , 0,    0},

{0x66,  VK_BACK     , 0x08, 0},
/*
{0x0D,  VK_TAB      , 0x09, 0},
*/
{0x5A,  VK_RETURN   , 0x0d, 0},
/*
{0x58,  VK_CAPITAL  , 0,    0},
*/
{0x76,  VK_ESCAPE   , 0x1b, 0},
/*
{0x29,  VK_SPACE    , ' ',  0},
{0x45,  VK_0        , '0',  ')'},
{0x16,  VK_1        , '1',  '!'},
{0x1e,  VK_2        , '2',  '@'},
{0x26,  VK_3        , '3',  '#'},
{0x25,  VK_4        , '4',  '$'},
{0x2e,  VK_5        , '5',  '%'},
{0x36,  VK_6        , '6',  '^'},
{0x3d,  VK_7        , '7',  '&'},
{0x3e,  VK_8        , '8',  '*'},
{0x46,  VK_9        , '9',  '('},

{0x1c,  VK_A        , 'a',  'A'},
{0x32,  VK_B        , 'b',  'B'},
{0x21,  VK_C        , 'c',  'C'},
{0x23,  VK_D        , 'd',  'D'},
{0x24,  VK_E        , 'e',  'E'},
{0x2b,  VK_F        , 'f',  'F'},

{0x34,  VK_G        , 'g',  'G'},
{0x33,  VK_H        , 'h',  'H'},
{0x43,  VK_I        , 'i',  'I'},
{0x3b,  VK_J        , 'j',  'J'},
{0x42,  VK_K        , 'k',  'K'},
{0x4b,  VK_L        , 'l',  'L'},
{0x3a,  VK_M        , 'm',  'M'},                    
{0x31,  VK_N        , 'n',  'N'},
{0x44,  VK_O        , 'o',  'O'},
{0x4d,  VK_P        , 'p',  'P'},
{0x15,  VK_Q        , 'q',  'Q'},
{0x2d,  VK_R        , 'r',  'R'},
{0x1b,  VK_S        , 's',  'S'},
{0x2c,  VK_T        , 't',  'T'},
{0x3c,  VK_U        , 'u',  'U'},
{0x2a,  VK_V        , 'v',  'V'},
{0x1d,  VK_W        , 'w',  'W'},
{0x22,  VK_X        , 'x',  'X'},
{0x35,  VK_Y        , 'y',  'Y'},
{0x1a,  VK_Z        , 'z',  'Z'},          
*/          
{0x70,  VK_NUMPAD0  , '0', 0},
{0x69,  VK_NUMPAD1  , '1', 0},
{0x72,  VK_NUMPAD2  , '2', 0},
{0x7A,  VK_NUMPAD3  , '3', 0},
{0x6B,  VK_NUMPAD4  , '4', 0},
{0x73,  VK_NUMPAD5  , '5', 0},
{0x74,  VK_NUMPAD6  , '6', 0},
{0x6C,  VK_NUMPAD7  , '7', 0},
{0x75,  VK_NUMPAD8  , '8', 0},
{0x7D,  VK_NUMPAD9  , '9', 0},
{0x7C,  VK_MULTIPLY , '*', 0},
{0x79,  VK_ADD      , '+', 0},
{0x6D,  VK_SEPARATOR, '.', 0},
{0x7B,  VK_SUBTRACT , '-', 0},
{0x71,  VK_DECIMAL  , ',', 0},
/*
,
{0x05,  VK_F1       , 0  , 0},
{0x06,  VK_F2       , 0  , 0},
{0x04,  VK_F3       , 0  , 0},
{0x0C,  VK_F4       , 0  , 0},
{0x03,  VK_F5       , 0  , 0},
{0x0B,  VK_F6       , 0  , 0},
{0x83,  VK_F7       , 0  , 0},
{0x0A,  VK_F8       , 0  , 0},
{0x01,  VK_F9       , 0  , 0},
{0x09,  VK_F10      , 0  , 0},
{0x78,  VK_F11      , 0  , 0},
{0x07,  VK_F12      , 0  , 0},


{0x77,  VK_NUMLOCK  , 0  , 0},
{0x7E,  VK_SCROLL   , 0  , 0},
          
{0x2f,  VK_RMENU    , 0  , 0}   
*/
};



//evento de tick do timer (aprox. 250 uS) / timer tick event (approx. 250 uS)
//void funcCallbackTickTimer(void)
//{


//}

//evento de byte enviado ou cts
void funcCallbackResultadoEnvSerial(unsigned char mucEstadoAtual, unsigned char *mpucProximoEstado)
{
  //se estado é cts muda sempre para recebendo
  if(mucEstadoAtual == MAQGERAL_ENVIANDO_CTS)
  {
    //passa a receber
    *mpucProximoEstado = MAQGERAL_RECEBENDO;
  }
  //senao, se for envi ode byte, verifica nos controles deste modilo se precisa mandar outro
  //em seguinda ou esperar ack
  else if(mucEstadoAtual == MAQGERAL_ENVIANDO_BYTE)
  {
    //inc indice
    gEnvioCmd.ucIndice++;
    //se tem a enviar ainda
    if(gEnvioCmd.ucIndice < gEnvioCmd.ucTotal)
    {
      //seta byte a ser enviado
      serialsinc_setByteEnvio(gEnvioCmd.ucLista[gEnvioCmd.ucIndice], 0);
      //muda estado para enviando byte a fi dme enviar o novo byte
      *mpucProximoEstado = MAQGERAL_ENVIANDO_BYTE;
    }
    //senao espera ack ou volta para recepcao
    else if(gEnvioCmd.ucEsperandoAck)
    {
      //vai para espera do ack (kah seria o padroa)
      *mpucProximoEstado = MAQGERAL_ESPERANDO_ACK;
    }
    //senao muda para recepcao
    else
    { 
      *mpucProximoEstado = MAQGERAL_RECEBENDO;
    }
  }
}

//decodifica tecla a partir da seq de teclas
//funcao com variaveis staticas para comntrole e que retorna valor > 0 se for tecla comum
//tambem atualiza
//decodifica tecla a partir da seq de teclas
//funcao com variaveis staticas para comntrole e que retorna valor > 0 se for tecla comum
//tambem atualiza
unsigned char ucDecodificaTecla(unsigned char mucByteRecebido, unsigned char mucReset)
{
  static unsigned char rucSoltou = 0;
  static unsigned char rucEspecial = 0;
  unsigned char ucRet;
  unsigned char ucCodigoVirtual;
  unsigned char ucTeclaShift = 0;
  unsigned char i;
  unsigned char ucAchouTecla;
  
  ucRet = 0;
  ucCodigoVirtual = 0;
  ucAchouTecla = 0; 
  //falta tratar o capslock pressionado ainda
  ucTeclaShift = barcode_ucFlagsTeclasCombinacao & BARCODE_SHIFT; 
  
  
  if(mucReset)
  {
    rucSoltou = 0;
    rucEspecial = 0;
  }
  else
  {
    //se o anterior foi flag de especial
    if(rucEspecial)
    {
      //se esta soltando
      if(mucByteRecebido == 0xF0)
      {
        //desmarca pressionada e deixa especial amrcada
        rucSoltou = 1;
      }
      //senao trata o codigo da tecla epecial pressionada ou solta
      else
      {
        //desmarca especial
        rucEspecial = 0;
        
        //verifica casos especiais do contro, alt e shift para atualziar 
        //barcode_ucFlagsTeclasCombinacao
        switch(mucByteRecebido)
        {
          //alt
          case 0x11:
            //se soltou desmarca, senao marca
            barcode_ucFlagsTeclasCombinacao = rucSoltou? 
              (barcode_ucFlagsTeclasCombinacao&(~BARCODE_ALT)):
              (barcode_ucFlagsTeclasCombinacao|BARCODE_ALT);
          break;  
          //conrol
          case 0x14:
            //se soltou desmarca, senao marca
            barcode_ucFlagsTeclasCombinacao = rucSoltou? 
              (barcode_ucFlagsTeclasCombinacao&(~BARCODE_CTRL)): 
              (barcode_ucFlagsTeclasCombinacao|BARCODE_CTRL);
          break;  
        }
        
        //procura entre as especiais mapeadas
        for(i = 0; i < (sizeof(gltListaTradNormais) / sizeof(TItemListaTrad)); i++)
        {
          //compara o scancode
          if(gltListaTradEspeciais[i].ScanCode == mucByteRecebido)
          {
            ucCodigoVirtual = gltListaTradEspeciais[i].VirtualCode;
            //se pressionada tecla shift e tem alternativa
            if(ucTeclaShift && gltListaTradEspeciais[i].AsciiCodeShift)
            {
              ucRet = gltListaTradEspeciais[i].AsciiCodeShift;
            }
            //senao codigo ascii princ.
            else
            {
              ucRet = gltListaTradEspeciais[i].AsciiCode;
            }
            ucAchouTecla = 1;
            //sai do loop
            break;
          }
        }
        //se achou a tecla
        if(ucAchouTecla)
        {
          //se soltou, atualiza tecla solta
          if(rucSoltou)
          {
            barcode_ucEventoTeclasSolta = ucCodigoVirtual;
            //se soltou nao manda ascii
            ucRet = 0; 
          }
          //senao pressionada
          else
          {
            barcode_ucEventoTeclasPressionada = ucCodigoVirtual;
          }
        }
        //zera flag de tecla solta
        rucSoltou = 0;
      }
    }
    //senao eh normal
    else
    {
      //debug:
      //barcode_ucEventoTeclasPressionada = mucByteRecebido;
      
      //se 0xe0 - tecla especial
      if(mucByteRecebido == 0xe0)
      {
        rucEspecial = 1;
      }
      else if (mucByteRecebido == 0xf0)
      {
        rucSoltou = 1;    
      }
      else
      {
      
        //verifica casos especiais do contro, alt e shift para atualziar 
        //barcode_ucFlagsTeclasCombinacao
        switch(mucByteRecebido)
        {
          //alt
          case 0x11:
            //se soltou desmarca, senao marca
            barcode_ucFlagsTeclasCombinacao = rucSoltou? 
              (barcode_ucFlagsTeclasCombinacao&(~BARCODE_ALT)): 
              (barcode_ucFlagsTeclasCombinacao|BARCODE_ALT);
          break;  
          //shift
          case 0x12:
          case 0x59:
            //se soltou desmarca, senao marca
            barcode_ucFlagsTeclasCombinacao = rucSoltou? 
              (barcode_ucFlagsTeclasCombinacao&(~BARCODE_SHIFT)):
              (barcode_ucFlagsTeclasCombinacao|BARCODE_SHIFT);
          break;  
          //conrol
          case 0x14:
            //se soltou desmarca, senao marca
            barcode_ucFlagsTeclasCombinacao = rucSoltou? 
              (barcode_ucFlagsTeclasCombinacao&(~BARCODE_CTRL)):
              (barcode_ucFlagsTeclasCombinacao|BARCODE_CTRL);
          break;  
        }
      
        //procura entre as especiais mapeadas
        for(i = 0; i < (sizeof(gltListaTradNormais) / sizeof(TItemListaTrad)); i++)
        {
          //compara o scancode
          if(gltListaTradNormais[i].ScanCode == mucByteRecebido)
          {
            ucCodigoVirtual = gltListaTradNormais[i].VirtualCode;
            //se pressionada tecla shift e tem alternativa
            if(ucTeclaShift && gltListaTradNormais[i].AsciiCodeShift)
            {
              ucRet = gltListaTradNormais[i].AsciiCodeShift;
            }
            //senao codigo ascii princ.
            else
            {
              ucRet = gltListaTradNormais[i].AsciiCode;
            }
            ucAchouTecla = 1;
            //sai do loop
            break;
          }
        }

        //se achou a tecla
        if(ucAchouTecla)
        {
          //se soltou, atualiza tecla solta
          if(rucSoltou)
          {
            barcode_ucEventoTeclasSolta = ucCodigoVirtual;
            //se soltou nao manda ascii
            ucRet = 0; 
          }
          //senao pressionada
          else
          {
            barcode_ucEventoTeclasPressionada = ucCodigoVirtual;
          }
        }
        
        //zera flag de tecla solta
        rucSoltou = 0;

      }
              
    }  
  }

  return ucRet;

} 



//evento de byte recebido com sucesso ou erro do byte ou ack
void funcCallbackResultadoRecSerial(unsigned char mucDado, unsigned char mucEstadoAtual, 
  unsigned char *mpucProximoEstado)
{
  unsigned char ucChar;
  //se estado atual eh esperando ack
  if(mucEstadoAtual == MAQGERAL_ESPERANDO_ACK)
  {
    //se envio estava esperando ack
    if(gEnvioCmd.ucEsperandoAck)
    {
      //zera flag para indica resposta
      gEnvioCmd.ucEsperandoAck = 0;
    }
    //senao por jah para recepcao
    else
    {
      *mpucProximoEstado = MAQGERAL_RECEBENDO;
    }
  
  }
  //senao trata evento de repcao
  else if(mucEstadoAtual = MAQGERAL_RECEBENDO)
  {
    switch(util_ucEventoSerialSinc)
    {
      case UTIL_EVT_SERIAL_RECEBIDO:
        //decodifica tecla
        ucChar = ucDecodificaTecla(mucDado, 0);
        //se <> 0 eh nova tecla
        if(ucChar)
        {
          if(!barcode_ucEventoCaracterRecebido)
          {
            gucUltimoCaracterRecebido = ucChar;
            barcode_ucEventoCaracterRecebido = 1;
            //poe em espera para que teclado nao mande nada ateh tratar o bye recebido
            gucEstahEmEspera = 1;
            serialsinc_setEstado(MAQGERAL_PARADA);
          }
          else
            BARCODE_DEBUG("<evt_trancado>");
        }  
      break;
      case UTIL_EVT_SERIAL_ERRO_PARIDADE:
        BARCODE_DEBUG("(PAR_ERRO)");
      break;
      case UTIL_EVT_SERIAL_ERRO_STOP:
        BARCODE_DEBUG("(SB_ERRO)");
      break;
    }
  }  
}




// Inicializa o módulo
//recebe no primeiro parametro objeto de eventos e no segundo o estado inicial para
//a comunicação sincrona com os valores possiveis da maquina de estado
//definidos com os nomes começando com MAQGERAL_... em utils.h
//caso deseje-se iniciar recebendo e com os flags de evento pegar cada caracter no loop principal
//passar MAQGERAL_ENVIANDO_CTS que ira liberar o dispositivo apra começar a enviar os dados
//senão passar MAQGERAL_PARADA para que funcoes como serial_gets
//ativem a comunicação pontualmente 
void barcode_iniciar(TBaseEventos *mpbeBaseEventos, unsigned char mucEstadoInicialCom)
{
  //copia ponteiro
  gpbeBaseEventos = mpbeBaseEventos; 

	//zera event ode caracter recebido
  barcode_ucEventoCaracterRecebido = 0;
  barcode_ucEventoTeclasSolta = 0;
  barcode_ucEventoTeclasPressionada = 0;
  gucUltimoCaracterRecebido = 0;
  barcode_ucFlagsTeclasCombinacao = 0;
  
  //RESETA DECODIFICAÇÃO DAS TECLAS
  ucDecodificaTecla(0, 1);
  
  //call back apra recepcao de bye com sucesso ou não
 	serialsinc_setFuncCallbackResultadoRecSerial(&funcCallbackResultadoRecSerial);
 	//callback apra envio
 	serialsinc_setFuncCallbackResultadoEnvSerial(&funcCallbackResultadoEnvSerial);
 	//inicialzia a recepcao em si
 	serialsinc_iniciar(gpbeBaseEventos);
 	
   gulTimeOut = 0;
  //associa funcao de callback para poder pegar notificação do timer e tratar aqui
  utils_setFuncCallbackTickTimer(&funcCallbackTickTimerParaTimeout);


	//manda comand ode reset para teclado
	//ucEnviaComandoKdb(KBD_RESET, 0, 0);

  //de acordo com o estado passado por parametro seta comunicação
  serialsinc_setEstado(mucEstadoInicialCom);

}

// retorna ultimo caracter recebido
unsigned char  barcode_ucUltimoCaracter(void)
{
  return gucUltimoCaracterRecebido;
}                                         



unsigned char ucEnviaComandoKdb(unsigned char mucCmd, unsigned char *mpucCmdsAdicionais, 
  unsigned char mucQtdCmdsAdicionais)
{
  unsigned char i, ucRet;

  gEnvioCmd.ucLista[0] = mucCmd;
  //ate 2 cmds adicionais
  if(mucQtdCmdsAdicionais > 2)
  {
    mucQtdCmdsAdicionais = 2;
  }
  //copia 
  for(i=0; i < mucQtdCmdsAdicionais; i++)
  {
    gEnvioCmd.ucLista[1+i] = mpucCmdsAdicionais[i];
  } 
  //total
  gEnvioCmd.ucTotal = mucQtdCmdsAdicionais + 1;
  //inidice atual
  gEnvioCmd.ucIndice = 0;
  gEnvioCmd.ucEsperandoAck = 1;
  //envia o primeiro byte
  serialsinc_setByteEnvio(gEnvioCmd.ucLista[0], 1);
  
  ucRet = 0;
  //fica esperando receber o ack
  for(i=10; i; i--)
  {
    //se jah recebeu retorno, sai
    if(!gEnvioCmd.ucEsperandoAck)
    {
      //se ack 0 - ok (true)
      ucRet = !serialsinc_ucValorACKRecebido;
      break;
    }  
    delayus(10);
  }
  
  //faz mauina entrar em modo de recepcao
  serialsinc_setEstado(MAQGERAL_RECEBENDO);
  
  //timeout
  return ucRet;  
} 

// manda alterar estado dos leds do teclado, se for u mteclado
/*
void barcode_alteraEstadoLeds(unsigned char mucSetOfLedsMask)
{
  unsigned char ucLista[1];
  //ED	Set Status LED's - This command can be used to turn on and off the Num Lock, 
  //Caps Lock & Scroll Lock LED's. After Sending ED, keyboard will reply with ACK (FA) 
  //and wait for another byte which determines their Status. Bit 0 controls the Scroll Lock, 
  //Bit 1 the Num Lock and Bit 2 the Caps lock. Bits 3 to 7 are ignored.
  BARCODE_DEBUG("[cmd_led]");
  
  ucLista[0] = mucSetOfLedsMask;
  
//  ucEnviaComandoKdb(KBD_SETSTATUSLED, 0);
//  ucEnviaComandoKdb(mucSetOfLedsMask, 1);
  
  ucEnviaComandoKdb(KBD_SETSTATUSLED, ucLista, 1);
}
*/

//execute do loop para barcode
void barcode_execute(void)
{
  // senao, se está em idle, espera processar evento
  if(gucEstahEmEspera)
  {
    //se processou o caracter, tira do modo idle
    if(!barcode_ucEventoCaracterRecebido)
    {
      gucEstahEmEspera = 0;
      //manda enviar cts novamente
     serialsinc_setEstado(MAQGERAL_ENVIANDO_CTS); 
    }
  }
}

//callback do tick do timer associado apenas quando se está recebendo pel obarcode
//e necessita de timeout com controle por aqui
void funcCallbackTickTimerParaTimeout(void)
{
  if(gulTimeOut)
  {
    gulTimeOut--;
  }
}


//funcao que espera faz uma leitura ateh que um caracter final(se definido)
//seja recebido(nao eh incluido n oresultado),um timout ocorra, ou o buffer encha
 //o timeout é considerado em segundos e determina o tmpo máximo que vai esperar
//até o primeiro caracter e entre caracteres  
//mpucBuffer -> Tem que iniciar com os vlaores da string jah carregados (string terminada em nulo)
//ou ter o caracter nulo jah no inicio (e inicia sem valores parciais)
//se ativado o echo (mucDisplayEcho) imprime esta inf. parcial an tela antes
//de iniciar a recepção  
//mpucCaracteresAceitos tem a string com os caracteres validos, se nulo, todo aceitos
void barcode_gets(unsigned char *mpucBuffer, unsigned char mucQtdMaximaDigitos,
  auto const rom char *mpcStrOpcoesCarFinal, 
  auto const rom char *mpcCaracteresAceitos,
  unsigned char mucTimeout, unsigned char mucDisplayEcho)
{
  unsigned char ucTamBufferInicial;
  unsigned char ucIndiceTrataBufferInicial;
  unsigned char ucIndice;
  unsigned char ucResetTimeOut;
  unsigned char ucUltimoCaracter;
  unsigned char ucRecebeuCarFinal;
  unsigned char ucCarValido;
  unsigned char i;

  //evento e antes de ler
  if(gpbeBaseEventos && gpbeBaseEventos->OnAntesEntradaDados)
  {
    base_doOnAntesEntradaDados(gpbeBaseEventos, BARCODE_ID_MODULO);
  }
  //carrega timeout
  //como a funcao de callback é chamada a a cada 250us, casa segundo equivale
  //a 4000 contagens
  gulTimeOut = mucTimeout;
  gulTimeOut *= 4000;   
  
  //usa os bits de flags para pegar dados
	//zera evento de caracter recebido
  barcode_ucEventoCaracterRecebido = 0;
  barcode_ucEventoTeclasSolta = 0;
  barcode_ucEventoTeclasPressionada = 0;
  gucUltimoCaracterRecebido = 0;
  barcode_ucFlagsTeclasCombinacao = 0;
  
  //RESETA DECODIFICAÇÃO DAS TECLAS
  ucDecodificaTecla(0, 1);
  
  
  //muda estado para mandar clear to send
   //se mandar clear to send (MAQGERAL_ENVIANDO_CTS )para teclado ele pode mandar algo
  //faz isso mudando estado da maquina geral da serial no callback do envio muda para 
  //proximo estado, que é o de recepcao
  //*** tira daqui para deixar controle fora da funcao 
  //serialsinc_setEstado(MAQGERAL_ENVIANDO_CTS);

  //salva o tamanho original do buffer para que trate os caracteres
  //vundos nele, quep doe ser caracteres finais, ou invalidos
  ucTamBufferInicial = strlen(mpucBuffer);
  //indica que esta para tratar o primeiro car. vindo
  ucIndiceTrataBufferInicial = 0;
  //inicia no 0 sempre, mems ovindo dados  
  ucIndice = 0;

  ucUltimoCaracter = 0;
  ucResetTimeOut = 0;
  ucRecebeuCarFinal = 0;
  
  //monitora os flags
  while((ucIndice < mucQtdMaximaDigitos) && gulTimeOut && 
    (!ucRecebeuCarFinal) )
  {
		//restart watchdog
		restart_wdt();
  
    //se deve restar timeou (por ter pego algo)
    if(ucResetTimeOut)
    {
      //como a funcao de callback é chamada a a cada 250us, casa segundo equivale
      //a 4000 contagens
      gulTimeOut = mucTimeout;
      gulTimeOut *= 4000;   
      //zera flag
      ucResetTimeOut = 0;
    }

    //monitora tecals pressionadas para trata teclas especiais
    //putch()
    if(barcode_ucEventoTeclasPressionada)
    {
      ucUltimoCaracter = barcode_ucEventoTeclasPressionada;
      barcode_ucEventoTeclasPressionada = 0;
      //flag para reset do timeout
      ucResetTimeOut = 1;

      //ignora telcas especiais até processar o que veio (instantaneo)
      if(ucIndiceTrataBufferInicial >= ucTamBufferInicial)
      {
        //testa opcoes de entrada 
        if(ucUltimoCaracter == VK_ESCAPE)      
        {
          //se ecoa, limpa tudo
          if(mucDisplayEcho)
          {
            while(ucIndice)
            {
              //volta - limpa - volta
              putch('\b');
              putch(' ');
              putch('\b');
              //dec indice
              ucIndice--;
            }
          }
          //zera indice
          ucIndice = 0;
          //continua sem preciessar a tecla como entrada
          barcode_ucEventoCaracterRecebido = 0;
          continue;
        }
        else if(ucUltimoCaracter == VK_BACK)
        {
          //tem que ter caracter ainda
          if(ucIndice)
          {
            //se echo mostra na display
            if(mucDisplayEcho)
            {
              //volta - limpa
              putch('\b');
              putch(' ');
              putch('\b');
            }
            //dec indice
            ucIndice--;
          }
          //continua sem preciessar a tecla como entrada
          barcode_ucEventoCaracterRecebido = 0;
          continue;
        }
      }
    }
    ///senão  se veio caracter pel obarcode ou tem caracter no buffer jah de inicio
    else if(barcode_ucEventoCaracterRecebido || (ucIndiceTrataBufferInicial < ucTamBufferInicial))
    {
      //se processando o que vei ono buffer
      if(ucIndiceTrataBufferInicial < ucTamBufferInicial)
      {
        //pega do buffer
        ucUltimoCaracter = mpucBuffer[ucIndiceTrataBufferInicial];
        //inc indice
        ucIndiceTrataBufferInicial++;
      }
      //senão pega valor  que vai para pegar no endereco 6 dos holding register
      else
      {
        ucUltimoCaracter = barcode_ucUltimoCaracter();
        //zera flag de evento
        barcode_ucEventoCaracterRecebido = 0;
      }
      //flag para reset do timeout
      ucResetTimeOut = 1;

      //ignora 0x08 e 0x1b que são o backspace e esc respectivamente
      //e que são tratados apenas no evento de tecla pressionada
      if((ucUltimoCaracter != 0x08) &&(ucUltimoCaracter != 0x1b) )
      {
        //adiciona a string, se na ofor o finalziador
        ucRecebeuCarFinal = 0;
        if(mpcStrOpcoesCarFinal)
        {
          for(i=0; mpcStrOpcoesCarFinal[i]; i++)
          {
            //putch('.');
            if(ucUltimoCaracter == mpcStrOpcoesCarFinal[i])
            {
              //putch('^');
              ucRecebeuCarFinal = 1;
              break;
            }
          }
        } 
        //se não recebeu caracter final
        if(!ucRecebeuCarFinal)
        {
          //verifica se caracter é valido
          if(mpcCaracteresAceitos)
          {
            //se chegar ao final e nao achar o caracter zera
            i=0;
            ucCarValido = 0;
            while(mpcCaracteresAceitos[i])
            {
              //putch(mpcCaracteresAceitos[i]);
              if(ucUltimoCaracter == mpcCaracteresAceitos[i])
              {
                ucCarValido = 1;
                break;
              }
              i++;
            }
            //se saiu antes do final ok, senia zera caracter
            if(!ucCarValido)
            {
              ucUltimoCaracter = 0;
              //putch('n');
            }
            else
            {
              //putch('s');
            }
          }
          
          //se caracte tem valor(eh valido)
          if(ucUltimoCaracter)
          {
            //se tem echo, mostra no display
            if(mucDisplayEcho)
            {
              putch(ucUltimoCaracter);
            }  
    
            //pega caracter
            mpucBuffer[ucIndice] = ucUltimoCaracter;
            //inc indice
            ucIndice++;
          }
        }
      }  
    }
    //se esta não está tratando caracteres vindo no inicio, chama execute do barcode
    if(ucIndiceTrataBufferInicial >= ucTamBufferInicial)
    {
      //chama execute para perceber que foi tratado o caracter
      barcode_execute();
    }
  }
  
  //coloca o caracter de finalização da string (0) para finalizar string
  mpucBuffer[ucIndice] = 0;
      
  //para a maquina
  //*** tira daqui para deixar controle fora da funcao
  //serialsinc_setEstado(MAQGERAL_PARADA);
  //evento depois de ler
  if(gpbeBaseEventos && gpbeBaseEventos->OnDepoisEntradaDados)
  {
    base_doOnDepoisEntradaDados(gpbeBaseEventos, BARCODE_ID_MODULO);
  }
  
}
