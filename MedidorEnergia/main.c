/***
**** Arquivo main do projeto
***/

// define para modulos assumirem hw do kit para o PI
// fico udefinido nas opções de cada arquivo .c
//#define PROJETO_PI
//ste define tambem foi usado nas bibliotes do senai que foram daptadas para testar e usar notacao de acesso a hw que é diferentes


///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////
#include <p18cxxx.h>	// inclui as definições dos ios do pic
// para temporizações
#include <delays.h>
//#include <usart.h>

#define _XTAL_FREQ 20000000   // define frequencia do cristal para delay

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "..\lib\base\base.h"

//barramento
#include "barra.h"



#include "..\lib\base\base.h"
#include "..\lib\kitmcu\tipos.h"
#include "..\lib\kitmcu\utils.h"
#include "..\lib\kitmcu\lcd.h"

#ifdef modulo_rtc
#include "..\lib\senai\rtc_1302.h"
#endif
#include "..\lib\senai\serial.h"
#include "..\lib\senai\eeprom.h"

//definicoes globais e macros
#include "global.h"
//mudoulos do projeto
#include "captura.h"

#ifdef modulo_dados
#include "dados.h"
#endif

#include "protocolo.h"

#ifdef modulo_dados                            
#include "m25pxx.h"
#endif

#include "barcode.h"


//largura maxima para entrada do id/lote
#define LARGURA_CAMPO_ID 8
//timeout esperando entrada dos dados
#define TIMEOUT_CAMPO_ID 5
//timeout da espera por confirmacao do usuari oe autorizacao
#define TIMEOUT_SEG_CONFIRMA_INICIO 30
//timeout segundos tempo sem consumo de energia (2 minutos)
#define TIMEOUT_SEG_SEM_CONSUMO_ENERGIA ((60) * 2)
//para indica sem comunicacao
#define QTD_PASSAGENS_LED_COM 10000
//qtd de passagens para piscar led indicadorr de que esta rodando
//para indicar rodando(invertendo led)
#define QTD_PASSAGENS_LED_RODANDO 10000

//valor maximo apra consumo simulado
#define   MAX_VALOR_CONSUMO_SIM 100000 

char cc[60];

//configuracoes globais
TCfgModuloCapturas cfg;

//dados decodificados de msg do protocolo de escrita ou leitura an memoria
unsigned long gulEndereco;
unsigned int guiQtd;
unsigned char gucCI, gucCIAnterior;
//flag indicando que deve impromir no display caracter recebido
unsigned char gucEventoCharSerialImprimir = 0;
//o caracter em si
unsigned char gucCharSerialImprimir = 0;

unsigned int gucPassoParado;

//guarda o valor de segundso lidos anteriormente
unsigned char gucSegundosAnteriores = 0;

//evento apra indica fina lde captura e que deve mostrar dados
unsigned char gucEventoDisplaySerialImprimir = 0;


//mascaras para os bits no espelho das saidas da automacao(reais e de funcoes especiais)
#define MASCARA_SAIDAS_Q0_0 0x01
#define MASCARA_SAIDAS_Q0_1 0x02
#define MASCARA_SAIDAS_Q0_2 0x04
#define MASCARA_SAIDAS_Q0_3 0x08
#define MASCARA_SAIDAS_AUTORIZA_OPERACAO 0x10
#define MASCARA_SAIDAS_DEBUG1 0x20
#define MASCARA_SAIDAS_DEBUG2 0x40
#define MASCARA_SAIDAS_DEBUG3 0x80
//espelho das saidas digitais(bobinas para o modbus)
unsigned char gucEspelhoSaidasDigitais = 0x00;


//espelho das entradas digitais(bobinas para o modbus)
unsigned char gucEspelhoEntradasDigitais = 0x00;

//flag que indica que houve mudanca em gucEspelhoSaidasDigitais
//e que assim deve atualziar os valoes na saida em si
unsigned char gucAtualizarSaidasDigitais = 0;

//espelho dos leds indicadores
unsigned char gucEspelhoLEDS = 0;   


//para debug
//vai retornar no endereco de reg. 6 do modbus honding register
unsigned char gucBarcodeUltimoCaracter = 0;

//objeto de ponteiros de eventos  se passado para os modulos
TBaseEventos gbeBaseEventos = {NULL, NULL, NULL, NULL};

//possiveis estados de operacao da edstacao
//  case 0: return "Inicializando...";
#define ESTADOOPER_INICIALIZANDO 0
//  case 1: return "Parada.";
#define ESTADOOPER_PARADA 1
//  case 2: return "Trabalhando";
#define ESTADOOPER_TRABALHANDO 2
//  case 3: return "Finalizando...";
#define ESTADOOPER_FINALIZANDO 3
//  return "(desconhecido)";
#define ESTADOOPER_DESCONHECIDO 4


//estado e operação da estação a qual este equip. está ligada
unsigned char gucEstadoOperacao = ESTADOOPER_DESCONHECIDO;
//ultimo valor numerico lido do barcode (ou teclado)
union
{
  //como unsigned long 
  unsigned long ulValor;
  //como buffer de caracteres
  unsigned char ucBuffer[4];
  //como buffer de int
  unsigned int uiValor[2];
} gBarCodeBCD = 0;

//consumo corrente, em kilowats horas, com 2 casa decimais presumidas
union
{
  //como unsigned long
  unsigned long ulValor;
  //como buffer de long
  unsigned int uiValor[2];
} gConsumoOperAtual = 0;

//para o cunsumo anterior
unsigned long gulValorConsumoAnterior = 0;
   
//consumo acumulado, em kilowats horas, com 2 casa decimais presumidas
union
{
  //como unsigned long
  unsigned long ulValor;
  //como buffer de long
  unsigned int uiValor[2];
  //como buffer de caracter
  unsigned char ucValor[4];
} gConsumoAcumulado = 0;
     
//endereco de comunicação
unsigned char gucEnderecoComunicacao = 1;     

//ultimo caracter do barcode o uteclado
unsigned char gucUltimoCaracter = 0;

//para controel de timeout
unsigned long gulTimeOutOper = 0;
//flag que indica que esta ativo
unsigned long gucTimeOutOperAtivo = 0;

//indica o indice da saida que deve ser liga e desligada antes de deposi do consumo
//se maior que 3 indica nenhuma, como 0xff
unsigned char gucSaidaAssociadaOperacao = 0; 


//delau em ms para esperar chavear saida
//valor 0 apra nao esperar nada
#define DELAY_MS_CHAVEANDO_SAIDA 0

//macro apra ligar ou desligar led de debug (jah selecionando saida do led)
#define setLedDebug(VAL) { barra_setSaidaAtual(GLOBAL_SEL_NENHUM); delay_ms(DELAY_MS_CHAVEANDO_SAIDA); setLed(GLOBAL_MASCARA_LED_DEBUG, VAL); barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS);  }
                           
#define inverteLedDebug() { barra_setSaidaAtual(GLOBAL_SEL_NENHUM); delay_ms(DELAY_MS_CHAVEANDO_SAIDA); setLed(GLOBAL_MASCARA_LED_DEBUG, !getLed(gucEspelhoLEDS, GLOBAL_MASCARA_LED_DEBUG)); barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS); }

//incluo header do módulo de AD
//#include "..\lib\senai\ad.h"

void main();
void montaRespostaModbusAscii(unsigned char mucCmd, unsigned char *pucArrayDadosBinarios,
   unsigned char mucTamanho, unsigned char *pucArrayDadosTestoSaida);
//manipulador de evento para Antes do módulo colocar dados na saida
int OnAntesSaidaDados(unsigned char mucIdModulo);
//manipulador de evento para Após o módulo colocar dados na saida
int OnDepoisSaidaDados(unsigned char mucIdModulo);
//manipulador de evento para Antes do módulo ler dados da entrada
int OnAntesEntradaDados(unsigned char mucIdModulo);
//manipulador de evento para Após o módulo ler dados da entrada
int OnDepoisEntradaDados(unsigned char mucIdModulo);
//escreve default na eeprom (inclusive endereco)
void salvaCfgDaEepromEMemoria(void);

// variáveis globais

/** V E C T O R  R E M A P P I N G *******************************************/
extern void _startup (void);        // See c018i.c in your C18 compiler dir
#pragma code _RESET_INTERRUPT_VECTOR = 0x000800
void _reset (void)
{
    _asm goto _startup _endasm
}

#pragma code


#pragma interruptlow trataLowISR save=section(".tmpdata"),PROD
void trataLowISR(void)
{
	//verifica se recebeu um byte pela serial
	//if(DataRdyUSART())
	if(PIR1bits.RCIF)
	{
		//salva byte recebido
		byte_recebido = RCREG;
		PIR1bits.RCIF = 0;
    proto_notificaByteRecebido(byte_recebido);
	}
	
  //por tx byte
  //como nao consegui config. sem trava a int por envio, fica testando no loop principal
  //se terminpo ude enviar byte
	//if(PIE1bits.TXIE && PIR1bits.TXIF)
	//{
	//  PIR1bits.TXIF = 0;
  //  proto_notificaByteTransmitido();
  //}
}
	


#pragma code _LOW_INTERRUPT_VECTOR = 0x000818
void _low_ISR (void)
{
   _asm goto trataLowISR _endasm; // Desvio para a rotina de "tratamento" de interrupções local
}


#pragma code



#define true  1
#define false 0

//
void putch(char c) 
{ 
	lcd_putc(c);
} 

//funcao apra mostrar msg co mtelporizacao no lugar de GLOBAL_TELA_ABERTURA
void exibeMensagem_const(auto const rom char *mpcMsg, unsigned int muiTempo)
{
  lcd_puts_const(mpcMsg);
  delay_ms(muiTempo);
}


//seta valor
void setLed(unsigned char mucMascaraLed, unsigned char mucValue)
{ 
  //seta ou reseta bit no espelho
  if(mucValue)
  {
    gucEspelhoLEDS |= mucMascaraLed;
  }
  else
  {
    gucEspelhoLEDS &= ~mucMascaraLed;  
  }   
  //joga na porta
  PORTD = gucEspelhoLEDS;
}
    
unsigned char getLed(unsigned char mucMascaraLed)
{
  return (gucEspelhoLEDS&mucMascaraLed) == mucMascaraLed;   
}




//funcao para ler se esta autorizado a iniciar operacao
unsigned char operacaoAutorizada(void)
{
  if(gucEspelhoSaidasDigitais&MASCARA_SAIDAS_AUTORIZA_OPERACAO)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

  
//uncai para setar o uresetar bit de autorizacao
void setOperacaoAutorizada(unsigned char mucValor)
{
  if(mucValor)
  {
    gucEspelhoSaidasDigitais |= MASCARA_SAIDAS_AUTORIZA_OPERACAO;
  }
  else
  {
    gucEspelhoSaidasDigitais &= ~MASCARA_SAIDAS_AUTORIZA_OPERACAO;
  }  
}  



//monta resposta
void montaRespostaModbusAscii(unsigned char mucCmd, unsigned char *pucArrayDadosBinarios, 
  unsigned char mucTamanho, unsigned char *pucArrayDadosTestoSaida)
{                        
  unsigned char ucVal, i;
  unsigned char *pSaida;

  //tamanho
  ucVal = mucTamanho;
  //menos sig
  pucArrayDadosTestoSaida[1] = (ucVal&0x0f)>9 ? 'A'+(ucVal&0x0f)-0x0a : '0'+(ucVal&0x0f);
  //mais sig
  ucVal >>= 4;
  pucArrayDadosTestoSaida[0] = (ucVal&0x0f)>9 ? 'A'+(ucVal&0x0f)-0x0a : '0'+(ucVal&0x0f);


  pSaida = &pucArrayDadosTestoSaida[2];
  //percore dados adicionando ao byffer
  for(i = 0; i < mucTamanho; i++)
  {
    ucVal = pucArrayDadosBinarios[i];
    //menos sig
    pSaida[1] = (ucVal&0x0f)>9 ? 'A'+(ucVal&0x0f)-0x0a : '0'+(ucVal&0x0f);
    //mais sig
    ucVal >>= 4;
    pSaida[0] = (ucVal&0x0f)>9 ? 'A'+(ucVal&0x0f)-0x0a : '0'+(ucVal&0x0f);
    //inc ponteiro
    pSaida += 2;
  }
  //coloca caracter nulo no final
  *pSaida = 0;
  

}




void retornaValoresBitsIO(unsigned long mulEndereco, unsigned char mucOrigemEhSaida, 
  unsigned long mulQuantidade, unsigned char *pucArrayDadosBinariosSaida)
{


    
    ///////// ********** coil ou input coil
        //Operação autorizada	Boolean	Indica que o supervisório autorizou o inicio de uma operação e assim deve-se zerar o valor de Consumo e começar uma nova medida.	Leitura e escrita	Sim
        //Saída Q0.0	Boolean	Valor da primeira saída digital.	Leitura e escrita	não
        //Saída Q0.1	Boolean	Valor da segunda saída digital.	Leitura e escrita	não
        //Saída Q0.2	Boolean	Valor da terceira saída digital.	Leitura e escrita	não
        //Saída Q0.3	Boolean	Valor da quarta saída digital.	Leitura e escrita	não
        //Entrada I0.0	Boolean	Valor lido da primeira entrada digital.	Somente Leitura	não
        //Entrada I0.1	Boolean	Valor lido da segunda entrada digital.	Somente Leitura	Não
        //Entrada I0.2	Boolean	Valor lido da terceira entrada digital.	Somente Leitura	Não
        //Entrada I0.3	Boolean	Valor lido da quarta entrada digital.	Somente Leitura	Não
     



//  unsigned char *pSaida;
  //o nedereco é consderado an faixa de device compativel com modbus
  
 //se  é coils
 if(mucOrigemEhSaida)
 {
   //pega do espelho das saidas digitais(bobinas para o modbus)
   pucArrayDadosBinariosSaida[0] = gucEspelhoSaidasDigitais; 
   //tem que jogar fora os enderecos ante do bit desejado
   //soh suporta 8 saidas, sendo 4 reais
   pucArrayDadosBinariosSaida[0] >>= mulEndereco&0x0007; 
   
 }
 //senao considera inputs
 else
 {
   //pega do espelho das entradas digitais(bobinas para o modbus)
   pucArrayDadosBinariosSaida[0] = gucEspelhoEntradasDigitais; //input[mulEndereco] = 1 
   //tem que jogar fora os enderecos ante do bit desejado
   //soh suporta 8 saidas, endo 4 reais
   pucArrayDadosBinariosSaida[0] >>= mulEndereco&0x0007; 
 }
}  

void forcaBobina(unsigned long mulEndereco, unsigned char mucLigada)
{
  unsigned char ucBits;
  //na faz por hora
  //soh suporta 8 saidas, sendo 4 reais, e os 3 ultimos leds do kbd no primeiro byte
  if(mulEndereco < 8)
  {
    //teste
    //lcd_gotoxy(14,0);
    //lcd_putc('>');
    
    ucBits = 1;
    //deslica para a esquerda de acordo com o bit
    ucBits <<= mulEndereco&0x0007; 
    //se seta
    if(mucLigada)
    {
      //teste
      //lcd_gotoxy(15,0);
      //lcd_putc('1');
      //seta apenas o bit correspondente
      gucEspelhoSaidasDigitais |= ucBits;
    }
    else
    {
      //teste
      //lcd_gotoxy(15,0);
      //lcd_putc('0');
      //Zera apenas o bit correspondente
      gucEspelhoSaidasDigitais &= ~ucBits;
    }
    
    //o bit 4 (mulEndereco == 4) indica a autorização para iniciar a medição
    //o bit eh testado a partir desta variavel de espelho, nao precisando fazer nada aqui
    /*
    if(mulEndereco == 4)
    {
      setOperacaoAutorizada(X);
    }                                                              
    */
    
    
    //5, 6 e 7 LEDs d oteclado, se for o caso (não fo ifeito funcionar isto)
    if((mulEndereco >= 5) && (mulEndereco <= 7))
    {
      //os 3 ultimos bits (enderecos 5, 6 e 7) são os leds doteclado eventualemtne ligado ao hw
      //constantes respectivas(BARCODE_KBDLED_SCROLLLOCK, BARCODE_KBDLED_NUMLLOCK,
      //BARCODE_KBDLED_CAPSLOCK) 
      //desloca 5 bits, pega is 3 bits menso sig  e passa apra funcao do modulo
      
      //tirado para liberar espaco jah que nao esta enviando cmd para teclado
      //barcode_alteraEstadoLeds((gucEspelhoSaidasDigitais>>5)&0x07);
    }
    //seta flag que indica necessidade de atualziar saidas
    gucAtualizarSaidasDigitais = 1;
    
  }
}

//preset dos registros em memoria
void presetRegistro(unsigned int muiEndereco, unsigned int muiValor)
{
  unsigned char *pucDado;
  unsigned int *puiDado;
  unsigned char ucInd;

  //zera ponteiros para testar apos if  e switch
  pucDado = NULL;
  puiDado = NULL;
  //indice da eeprom
  ucInd = 0xff; //se mudar para indice dif de 0xff salva na eerpom tambem
  
  //mapeamento dos registros de entrada (input registers): voláteis
  switch(muiEndereco)
  {
      //endereco atual
      case 0: pucDado = &gucEnderecoComunicacao; break;
      
      //Status	Byte	Define o status atual do medidor: "Inicializando"; 1: "Parada."; 2: "Trabalhando" e 3:"Finalizando trabalho"	Somente leitura	sim
      case 1: 
        pucDado = &gucEstadoOperacao; 
        ucInd = GLOBAL_INDEX_CFG_ENDERECO_REDE; 
      break;  

      //Identificador	Alfanumérico	Último identificador do produto ou lote fornecido via ADC ou digitação via teclado.	Leitura e escrita 	sim
      //long mais sig - 4 digitos BCD mais sig.
      case 2: 
        puiDado = &gBarCodeBCD.uiValor[0]; break;
      //long menos sig - 4 digitos BCD menos sig.
      case 3: puiDado = &gBarCodeBCD.uiValor[1]; break;
      
      //Consumo	Double	Valor, em KWh do consumo corrente, se trabalhando, 
      //ou do último consumo realizado.	Somente leitura	sim
      //long com a parte mais significativa do consumo parcial
      case 4: puiDado = &gConsumoOperAtual.uiValor[0]; break;  
      //long com a parte menos significativa do consumo parcial
      case 5: puiDado = &gConsumoOperAtual.uiValor[1]; break;  

      //Offset de nível 0 no TC	Word	Indica o valor de Leitura do canal de A/D 
      //ligado ao TC que corresponde ao valor de repouso, ou seja, 
      //corrente igual a zero. Aceita valores entre 0 e 1023, equivalente a 
      //faixa de 0a 5V.	Leitura e escrita	Não 
      case 6: 
        puiDado = &cfg.uiValorADZeroCorrente; 
        ucInd = GLOBAL_INDEX_CFG_VALORADZEROCOR; 
      break; 

      //Fator do TP	Byte	Indica qual o fator de redução do valor de tensão é empregado para no TP instalado.. Aceita valores entre 1 e 254	Leitura e escrita	Não
      case 7: 
        pucDado = &cfg.ucFatorTrafo; 
        ucInd = GLOBAL_INDEX_CFG_FATORTRAFO; 
      break;

      //Queda de tensão após o TP	Byte	Indica que valor de queda de tensão, em mVs, deve ser considerados nos cálculos da tensão lida, após dividor o valor leal pelo Fator do TP e antes de dividi-lo pelo Fator do divisor resistivo do TP.	Leitura e escrita	Não
      case 8: 
        puiDado = &cfg.uiDescontoDiodos;
        ucInd = GLOBAL_INDEX_CFG_DESCONTODIODOS; 
      break;
      
      //Fator do divisor resistivo do TP	Byte	Indica qual a parcela do valor de tensão entregue na saída do TP (depois de dividido pelo Fator do TP e subtraído da Queda de tensão após o TP).	Leitura e escrita	Não

      case 9: 
        puiDado = &cfg.uiFatorDivResistivo;
        ucInd = GLOBAL_INDEX_CFG_FATORDIVRESISTIVO; 
      break;
      
      //#define GLOBAL_INDEX_CFG_SENSIBILIDADE_TC 20	
      //sensibilidade do TC - Indica qual a sensibilidade do sensor TC em mV/A. 
      //Este valor indica para o medidor em quantos mV deve aumentar a tensão na saída do TC 
      //para uma variação de 1A em sua entrada.
      //3 casas decimais presumidas
      /// parte mais significativa
      case 10: 
        puiDado = &((unsigned int *)&cfg.ulSensibilidadeTC)[0]; 
        ucInd = GLOBAL_INDEX_CFG_SENSIBILIDADE_TC; 
      break;
      //parte menos significativa
      case 11: 
        puiDado = &((unsigned int *)&cfg.ulSensibilidadeTC)[1]; 
        ucInd = GLOBAL_INDEX_CFG_SENSIBILIDADE_TC+2; 
      break;
      
      //int com a parte mais significativa do consumo acumulado
      case 12: 
        puiDado = &gConsumoAcumulado.uiValor[0];
        ucInd = GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO; 
      break;
      //long com a parte menos significativa do consumo acumulado
      case 13: 
        puiDado = &gConsumoAcumulado.uiValor[1];
        ucInd = GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO+2; 
      break;
      
      case 14: 
        pucDado = &gucSaidaAssociadaOperacao; 
        ucInd = GLOBAL_INDEX_CFG_SAIDA_ASSOCIADA_OPER; 
      break;        

        
  }
  //e acordo com o ponteiro atribui e se tiver indice definido salva na eeprom
  //unsigned char
  if(pucDado)
  {
    //um byte apenas
    *pucDado = muiValor&0x00ff;
    //salva na eeprom se necessário
    if(ucInd != 0xff)
    {
      eeprom_escrever(ucInd, *pucDado);
    }
  }
  //unsigned long
  else if(puiDado)
  {
    //dois bytes apenas
    *puiDado = muiValor; 
    //salva na eeprom se necessário
    if(ucInd != 0xff)
    {
      eeprom_escrever(ucInd, (*pucDado)>>8);
      eeprom_escrever(ucInd+1, (*pucDado)&0x00ff);
    }
  }
}

void presetMultiplosRegistros(unsigned int muiEndereco, unsigned int muiQuantidade,
  unsigned char *mpucBuffer, unsigned char mucQtdBytesIndicados)
{
  unsigned int uiVal;
  //vai escrevendo endereco a endereco, diminuind oa qtd, andando no endereco e no buffer de entrada
  //cuida para noa ultrapassar bytes passados e mmucQtdBytesIndicados
  for(; mucQtdBytesIndicados && muiQuantidade; muiQuantidade--)
  {
    //pega valor
    //MSB
    uiVal = mpucBuffer[0];
    uiVal <<= 8; 
    mucQtdBytesIndicados--;
    //LSB
    if(mucQtdBytesIndicados)
    {
      uiVal += mpucBuffer[1];
      mucQtdBytesIndicados--;
    } 

    //grava
    presetRegistro(muiEndereco, uiVal);
    //manda...
    muiEndereco++;
    mpucBuffer += 2;
  }

}


//retorna "muiQuantidade" registros solicitasos (mucEhRegistroEntrada=1->input regisres,
//senão hold register) a partir do endereço "muiEndereco" (considerando sempre regs de 16 bits)
//armaznando os daods em pucArrayDadosBinariosSaida limitando a mucQtdBytesIndicados
//bytes.
//Faz aqui um mapeamento das variaveis internas de acordo com os enderecos definidos
//pela aplicação.  
void retornaValoresRegistros(unsigned int muiEndereco, unsigned char mucEhRegistroEntrada, 
  unsigned int muiQuantidade, unsigned char *pucArrayDadosBinariosSaida,
  unsigned char mucQtdBytesIndicados)
{
	static unsigned char rucContRegister = 0;
	static unsigned char rucContPedidos = 0;       
  unsigned char i;
  unsigned char iIndice;
  unsigned char *pucDado;
  unsigned int *puiDado;
  //apenas para teste
  rucContPedidos++;
  if(rucContPedidos > 1)
  {
    rucContRegister++;
    rucContPedidos = 0;
  }
  
  //pecorre os n enderecos requeridos ou ateh encher o indicado 
  for(iIndice = 0, i = 0; 
      (i < muiQuantidade) && (iIndice < mucQtdBytesIndicados); 
      i++, iIndice += 2, muiEndereco++
     )
  {
    //zera ponteiros para testar apos if  e switch
    pucDado = NULL;
    puiDado = NULL;

    //mapeamento dos registros de entrada (input registers)
    //mapeia dados basicos como o espelho das entradas
    //e valroes de leitura de AD, mas por hora apenas o espelho das entradas
    if(mucEhRegistroEntrada)
    {
      //de acordo com o endereco decodifica as variaveis
      switch(muiEndereco)
      {
        case 0: pucDado = &gucEspelhoEntradasDigitais;  break;
        //valores de AD como inteiros
        case 1: puiDado = &captura_uiUltimaLeituraADCMedicao[0]; break; 
        case 2: puiDado = &captura_uiUltimaLeituraADCMedicao[1]; break; 
        case 3: puiDado = &captura_uiUltimaLeituraADCMedicao[2]; break; 
        //nos proximos retorna os valores das portas (mais para debig)
        case 4: pucDado = &PORTA; break; 
        case 5: pucDado = &PORTB; break; 
        case 6: pucDado = &PORTC; break; 
        case 7: pucDado = &PORTD; break; 
        case 8: pucDado = &PORTE; break; 
        case 9: pucDado = &gucUltimoCaracter; break;
        
        default:
          //do 10 ate o 41 rretorna espelho do display
          if(
              (muiEndereco >= 10) && 
              (muiEndereco < (10+sizeof(lcd_ucEspelhoDisplay))) 
            )
          {
            //aponta para o caracteres requerido
            pucDado = &lcd_ucEspelhoDisplay[muiEndereco-10]; 
          }
                  
      }
    }
    //senão é hold regsiter
    else
    {
      //de acordo com o endereco decodifica as variaveis
      switch(muiEndereco)
      {
        //endereco atual
        case 0: pucDado = &gucEnderecoComunicacao; break;
        
        //Status	Byte	Define o status atual do medidor: "Inicializando"; 1: "Parada."; 2: "Trabalhando" e 3:"Finalizando trabalho"	Somente leitura	sim
        case 1: pucDado = &gucEstadoOperacao; break;

        //Identificador	Alfanumérico	Último identificador do produto ou lote fornecido via ADC ou digitação via teclado.	Leitura e escrita 	sim
        //long menos sig - 4 digitos BCD menos sig.
        case 2: puiDado = &gBarCodeBCD.uiValor[1]; break;
        //long mais sig - 4 digitos BCD mais sig.
        case 3: puiDado = &gBarCodeBCD.uiValor[0]; break;
        
        //Consumo	Double	Valor, em KWh do consumo corrente, se trabalhando, 
        //ou do último consumo realizado.	Somente leitura	sim
        //long com a parte menos significativa do consumo parcial
        case 4: puiDado = &gConsumoOperAtual.uiValor[1]; break;  
        //long com a parte mais significativa do consumo parcial
        case 5: puiDado = &gConsumoOperAtual.uiValor[0]; break;  

        //Offset de nível 0 no TC	Word	Indica o valor de Leitura do canal de A/D 
        //ligado ao TC que corresponde ao valor de repouso, ou seja, 
        //corrente igual a zero. Aceita valores entre 0 e 1023, equivalente a 
        //faixa de 0a 5V.	Leitura e escrita	Não 
        case 6: puiDado = &cfg.uiValorADZeroCorrente; break; 

        //Fator do TP	Byte	Indica qual o fator de redução do valor de tensão é empregado para no TP instalado.. Aceita valores entre 1 e 254	Leitura e escrita	Não
        case 7: pucDado = &cfg.ucFatorTrafo; break;

        //Queda de tensão após o TP	Byte	Indica que valor de queda de tensão, em mVs, deve ser considerados nos cálculos da tensão lida, após dividor o valor leal pelo Fator do TP e antes de dividi-lo pelo Fator do divisor resistivo do TP.	Leitura e escrita	Não
        case 8: puiDado = &cfg.uiDescontoDiodos; break;
        
        //Fator do divisor resistivo do TP	Byte	Indica qual a parcela do valor de tensão entregue na saída do TP (depois de dividido pelo Fator do TP e subtraído da Queda de tensão após o TP).	Leitura e escrita	Não

        case 9: puiDado = &cfg.uiFatorDivResistivo; break;
        
        //#define GLOBAL_INDEX_CFG_SENSIBILIDADE_TC 20	
        //sensibilidade do TC - Indica qual a sensibilidade do sensor TC em mV/A. 
        //Este valor indica para o medidor em quantos mV deve aumentar a tensão na saída do TC 
        //para uma variação de 1A em sua entrada.
        //3 casas decimais presumidas
        //parte menos significativa
        case 10: puiDado = &((unsigned int *)&cfg.ulSensibilidadeTC)[1]; break;
        /// parte mais significativa
        case 11: puiDado = &((unsigned int *)&cfg.ulSensibilidadeTC)[0]; break;
        
        //long com a parte menos significativa do consumo acumulado
        case 12: puiDado = &gConsumoAcumulado.uiValor[1]; break;
        //int com a parte mais significativa do consumo acumulado
        case 13: puiDado = &gConsumoAcumulado.uiValor[0]; break;
        //cfg da saida associada
        case 14: pucDado = &gucSaidaAssociadaOperacao; break;
        

        
      }
    }  

    //e acordo com o ponteiro atribui
    //unsigned char
    if(pucDado)
    {
      pucArrayDadosBinariosSaida[iIndice] = 0;
      pucArrayDadosBinariosSaida[iIndice+1] = *pucDado; 
    }
    //unsigned long
    else if(puiDado)
    {
      pucArrayDadosBinariosSaida[iIndice] = (*puiDado)>>8;
      pucArrayDadosBinariosSaida[iIndice+1] = (*puiDado)&0x00ff; 
    }
    //senao deixa valores calculados de teste
    else
    {
      pucArrayDadosBinariosSaida[iIndice] = ((i+1)/2) * 10;
      pucArrayDadosBinariosSaida[iIndice+1] += rucContRegister;
    }
  }
}  




void trataMensagemProtocolo(void)
{
	unsigned int i, j;
	unsigned int uiVal;
	unsigned int uiVal2;   
	//aponta para o do buffer dd envio para reaproveitar aqui a memoria
	unsigned char *ucBufferDados;
	//aloca de acordo com o tamanho maxmo necessario
	unsigned char cBufferValoresHex[(PROTO_MAX_DADOSBIN_MODBUS_ASCII*2)+1];

  //************* anula trata pacote pro hora para teste
  

  //atribui endereco
  ucBufferDados = proto_ucBufferEnvio;

  //lcd_clear();
  //sprintf(cc, "msg modbus\rrec: %02X", proto_ucMsgRecebida); 
  //lcd_puts(cc);
  //delay_ms(2000);
  //exibeMensagem_const("msg modbus\rrecebida", 1000);
  //lcd_clear();
    
   /////// calc enderco e qtd de itens (usar se mensagem realemtne tem estes campos) 
  //proto_ucBufferDados[0..1]-> endereco inicial
  uiVal = proto_ucBufferDados[0];
  uiVal <<= 8; 
  uiVal += proto_ucBufferDados[1]; 
  //proto_ucBufferDados[2..3]-> qtd de itens
  
  uiVal2 = proto_ucBufferDados[2];
  uiVal2 <<= 8; 
  uiVal2 += proto_ucBufferDados[3]; 
    
  //trata de acordo com o comando modbus
  switch(proto_ucMsgRecebida)
  {
    //01	Read coil status
    case PROTO_MODBUS_READCOILSTATUS:
    //02	Read input status
    case PROTO_MODBUS_READINPUTSTATUS:
    {
      //monta valores na saida a partir de cBufferValoresHex[1]
      retornaValoresBitsIO(uiVal,  
        proto_ucMsgRecebida == PROTO_MODBUS_READCOILSTATUS, uiVal2,
        &ucBufferDados[1]);
      
      //em cBufferValoresHex[0] vai a qtd de bytes dos bits
      ucBufferDados[0] = (unsigned char)(uiVal2/8)+(uiVal2%8?1:0);
      //se passou do que pode suprtar, retorna erro
      if(ucBufferDados[0] > PROTO_MAX_DADOSBIN_MODBUS_ASCII)
      {
        //retorna erro de endereco, na falta de outro melhor
        proto_enviaExceptionModbus(proto_ucMsgRecebida, 
          PROTO_MODBUS_EXCEPTION_ILLEGALADDRESS);          
      }
      //senao monta retorno e responde
      else
      {
        //monta em formato ascii
        montaRespostaModbusAscii(proto_ucMsgRecebida, &ucBufferDados[1], 
          ucBufferDados[0], cBufferValoresHex);          
        //manda resposta
        proto_enviaPacote(proto_ucMsgRecebida, cBufferValoresHex);
      }  

    }
    break;                                    
    
    //03	Read holding registers
    case PROTO_MODBUS_READHOLDINGREGISTERS:
    //04	Read input registers
    case PROTO_MODBUS_READINPUTREGISTER:
    {
      
      //em cBufferValoresHex[0] vai a qtd de bytes dos bits
      ucBufferDados[0] = (unsigned char)(uiVal2 * 2);
      //se passou do que pode suprtar, retorna erro
      if(ucBufferDados[0] > PROTO_MAX_DADOSBIN_MODBUS_ASCII)
      {
        //retorna erro de endereco, na falta de outro melhor
        proto_enviaExceptionModbus(proto_ucMsgRecebida, 
          PROTO_MODBUS_EXCEPTION_ILLEGALADDRESS);          
      }
      //senao monta retorno e responde
      else
      {
        retornaValoresRegistros(uiVal, proto_ucMsgRecebida == PROTO_MODBUS_READINPUTREGISTER, 
          uiVal2, &ucBufferDados[1], ucBufferDados[0]);          
        
        
        montaRespostaModbusAscii(proto_ucMsgRecebida, &ucBufferDados[1], 
          ucBufferDados[0], cBufferValoresHex);          
        //manda resposta
        proto_enviaPacote(proto_ucMsgRecebida, cBufferValoresHex);
      }  
    
    }
    break;
    //05	Force single coil
    case PROTO_MODBUS_FORCESINGLECOIL:
    //06	Preset single register
    case PROTO_MODBUS_PRESETSINGLEREGISTER:
    {
      //se eh forca bobina
      if(proto_ucMsgRecebida == PROTO_MODBUS_FORCESINGLECOIL)
      {
        //recebe em uiVal o endereco da bobina e em uiVal2 0xff00 para setar bobina
        //ou 0x0000 para zerar
        forcaBobina(uiVal, (uiVal2==0xFF00)?1:0);
      }
      //senao eh regsitro
      else
      {
         //pela faixa de endereco diferencia holdregister de registro normal
         //uiVal fica com o endereco e uiVal2 com o valor
         presetRegistro(uiVal, uiVal2);
      }
      //devolve o que recebeu (echo)
      //tamanho
      montaRespostaModbusAscii(proto_ucMsgRecebida, proto_ucBufferDados, 
        proto_ucTamanhoDados, cBufferValoresHex);          
      //manda resposta
      proto_enviaPacote(proto_ucMsgRecebida, cBufferValoresHex);

    }
    break;
    
    //16	Preset multiple registers
    case PROTO_MODBUS_PRESETMULTIPLEREGISTERS:
    {
      //recebe o endereço(já em uiVal)
      //a qtd de regsitradores (já em uiVal2)
      //a qtd de bytes (em proto_ucBufferDados[3])
      //e depois os bytes para os registros(proto_ucBufferDados[4..(4+proto_ucBufferDados[3]-1)])
      presetMultiplosRegistros(uiVal, uiVal2,
        &proto_ucBufferDados[4], proto_ucBufferDados[3]);
      
      //na resposta vão o endereco e o contador de registros
      montaRespostaModbusAscii(proto_ucMsgRecebida, proto_ucBufferDados,
        4, cBufferValoresHex);
      //manda resposta
      proto_enviaPacote(proto_ucMsgRecebida, cBufferValoresHex);
    }
    break;

    //07	Read exception status
    case PROTO_MODBUS_READEXCEPTIONSTATUS:
    //15	Force multiple coils
    case PROTO_MODBUS_FORCEMULTIPLECOILS:
    //17	Report slave ID
    case PROTO_MODBUS_REPORTSLAVEID:    
    {
      //retorna excessão indicando que estes comandos não são suportados
      proto_enviaExceptionModbus(proto_ucMsgRecebida, 
        PROTO_MODBUS_EXCEPTION_ILLEGALFUNCTION);
    }
    break;
  }  
	
}


//escreve default na eeprom (inclusive endereco)
void salvaCfgDaEepromEMemoria(void)
{ 
 	unsigned char *pucCfg, i;
  //escreve na memoria primeiro
	//default: medicao
	cfg.ucModo = CAPTURA_MODO_MEDICAO;
	// intervalo entre capturas, em useg. de 512 (32 amostras por onda de 60 Hz)	
  cfg.uiTempoEntreCapturas = 512;
	//asume valor no centro
	cfg.uiValorADZeroCorrente = 0x01FF; //(511)
	//assume 36 vez (220v / 6v) casa semiciclo co mtap centra de trafo de 6+6 no secundario
	cfg.ucFatorTrafo = 36;
	//* O desconto dos diodos preve dos 5 digitos os tris a direita após a virgula
	//assume 0.7 voltes
	cfg.uiDescontoDiodos = 700;
	//assume 1/2 (0,5 com 3 casas presumidas)
	cfg.uiFatorDivResistivo = 500;
	//assume 12,5 mV (3 casas presumidas)
	cfg.ulSensibilidadeTC = 12500;

	//encara como buffer e escreve todos os bytes a partir do endereco 0 minimo ateh o maxim oda cfg
	pucCfg = (unsigned char *)&cfg;

	for(i=GLOBAL_INDEX_CFG_MIN; i<= GLOBAL_INDEX_CFG_MAX; i++)
	{
		//escreve byte a byte a byte
		eeprom_escrever(i, pucCfg[i]);
	}
	
	//o endereco de comunicacao e o consumo acumulado eh separado
  gucEnderecoComunicacao = 1;
  gConsumoAcumulado.ucValor[i] = 0;
  
  //salva endereco
  eeprom_escrever(GLOBAL_INDEX_CFG_ENDERECO_REDE, gucEnderecoComunicacao);
   
  //salva bytes
  for(i=0; i < 4; i++)
  {
    eeprom_escrever(GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO+i, gConsumoAcumulado.ucValor[i]);  
	}
	
	gucSaidaAssociadaOperacao = 0;
	eeprom_escrever(GLOBAL_INDEX_CFG_SAIDA_ASSOCIADA_OPER, gucSaidaAssociadaOperacao);

}


//jah le cfgs
void leCfgDaEeprom(void)
{	
	unsigned char *pucCfg, i;

	//encara como buffer e le todos os bytes a partir do endereco 0 minimo ateh o maxim oda cfg
	pucCfg = (unsigned char *)&cfg;

	for(i=GLOBAL_INDEX_CFG_MIN; i<= GLOBAL_INDEX_CFG_MAX; i++)
	{
		//le byte a bute
		pucCfg[i] = eeprom_ler(i);
	}

	//se modo invalido eh teste
	if(cfg.ucModo > CAPTURA_MODO_MEDICAO)
	{
		//default: medicaos
		cfg.ucModo = CAPTURA_MODO_MEDICAO;
	}

	// intervalo entre capturas, em useg. de 512 (32 amostras por onda de 60 Hz)	
	if(cfg.uiTempoEntreCapturas == 0xffff)
	{
		cfg.uiTempoEntreCapturas = 512;
	}

	//coloca valores presumidos do hw se lido tudo ff

	//se maior que 10 bits nao eh valor de ad valido (1023)
	if(cfg.uiValorADZeroCorrente > 0x03ff)
	{
		//asume valor no centro
		cfg.uiValorADZeroCorrente = 0x01FF; //(511)
	}


	if((cfg.ucFatorTrafo == 0xff) || (cfg.ucFatorTrafo == 0))
	{
		//assume 36 vez (220v / 6v) casa semiciclo co mtap centra de trafo de 6+6 no secundario
		cfg.ucFatorTrafo = 36;
	}

	//* O desconto dos diodos preve dos 5 digitos os tris a direita após a virgula
	//aceita como valor maximo 6 volts
	if((cfg.uiDescontoDiodos > 6000) || (cfg.uiDescontoDiodos < 100))
	{
		//assume 0.7 voltes
		cfg.uiDescontoDiodos = 700;
	}

	//valida relacao do divisor resistivo
	if((cfg.uiFatorDivResistivo == 0) || (cfg.uiFatorDivResistivo == 0xffff))
	{
		//assume 1/2 (0,5 com 3 casas presumidas)
		cfg.uiFatorDivResistivo = 500;
	}
	
	//sensibilidade do TC
	if((cfg.ulSensibilidadeTC == 0xffffffff) || (cfg.ulSensibilidadeTC == 0))
	{
		//assume 12,5 mV (3 casas presumidas)
		cfg.ulSensibilidadeTC = 12500;
	}
	
	//le ainda o endereco de comunicaco
	gucEnderecoComunicacao = eeprom_ler(GLOBAL_INDEX_CFG_ENDERECO_REDE);
	if((gucEnderecoComunicacao == 0) || (gucEnderecoComunicacao == 0xff))
	{
	  gucEnderecoComunicacao = 1;
  }
  
  //le totalizador acumulado de consumo
  for(i=0; i < 4; i++)
  {
    gConsumoAcumulado.ucValor[i] = eeprom_ler(GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO+i);  
  }	
  
 	gucSaidaAssociadaOperacao = eeprom_ler(GLOBAL_INDEX_CFG_SAIDA_ASSOCIADA_OPER);
 	if(!((gucSaidaAssociadaOperacao>=0) && (gucSaidaAssociadaOperacao<=3)))
 	{
    gucSaidaAssociadaOperacao = 0; 
  }

}




/*
	rotina principal
*/
#pragma code



unsigned char ucPressionouBotao(unsigned char mucMascaraBitBotaoDesejado)
{
  //entrada anterior
  unsigned char ucEntradaAnt;
  unsigned int i;
  unsigned char ucRet;
  
  //por default retorna que nao foi press
  ucRet = 0; 
   
  //seleciona entrada para botão, se não estiver
  ucEntradaAnt = barra_getEntradaAtual();
  if(ucEntradaAnt != GLOBAL_SEL_ENT_IU_BOTOES)
  {                                          
    barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
  }
  else
  {
    ucEntradaAnt = GLOBAL_SEL_ENT_IU_BOTOES; 
  }
  
  //testa se esta pressionad o o botao (bit zerado)
  if(!(PORTB&mucMascaraBitBotaoDesejado))
  {
    //debug
    setLedDebug(1);
    //confirma
    i = 10;
    //permite ateh 3 pulsos contradizendo
    ucRet = 3;
    while(i)
    {
      //se soltou, desiste
      if(PORTB&mucMascaraBitBotaoDesejado)
      {
        if(!ucRet)
        {
          break;
        }
        ucRet--;  
      }
      i--;
    }
    //se chegoua  zero confirma pressionado, sena nao pressionou
    if(i == 0)
    {
      //para retornar verdadeiro
      ucRet = 1;
      //antes espera soltar a tecla (por no maximo 10 segundo)
      i = 1000;
      while(i && (!(PORTB&mucMascaraBitBotaoDesejado)))
      {
        delay_ms(10);
        i--;
      }
    }
    else
    {
      //sinaliza brevemente que nao aceitou a tecla
      setLedDebug(0);
      delay_ms(100);
      setLedDebug(1);
      delay_ms(100);
    }
    //debug
    setLedDebug(0);
    
  }

  //restaura entrada anterior
  if(ucEntradaAnt != GLOBAL_SEL_ENT_IU_BOTOES)
  {
    barra_setEntradaAtual(ucEntradaAnt);
  }
  //retorna resultado
  return ucRet;
  

}


unsigned char ucMenuEscolha(unsigned char *mpucPrompt, unsigned char *mpucListaOpcoes,
  unsigned char mucQtdOpcoes, unsigned char *mpucIndiceOpAtual, unsigned char mucEntradaEhViaTeclado)
{
  unsigned char iOpExibida;
  unsigned char *pucItem;
  unsigned char ucUltimoCaracter;
  unsigned char i;
  
  //seleciona entrada botoes e saida display
  if(mucEntradaEhViaTeclado)
  {
    barra_setEntradaAtual(GLOBAL_SEL_ENT_IA);
    //zera flag de evento
    barcode_ucEventoCaracterRecebido = 0;
    barcode_execute();
    serialsinc_setEstado(MAQGERAL_RECEBENDO);
  }
  else
  {
    barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
  }

  barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_DISPLAY);
  
  //se tem prompt limpa tela
  if(mpucPrompt)
  {
    lcd_clear();
  }
  //se opatual invalida, vai para primeira
  if(*mpucIndiceOpAtual >= mucQtdOpcoes)
  {
    *mpucIndiceOpAtual = 0;
  }
  
  
  //imprime prompt
  iOpExibida = 0;
  pucItem = mpucListaOpcoes;
  
  if(mpucPrompt)
  {
    lcd_puts(mpucPrompt);
  }
  //senao salva posicao atual no display para usar como ponto as opcoes
  else
  {
    lcd_savexy();
  }
  
  while(1)
  {
		//restart watchdog
		restart_wdt();
  
    //se diferente
    if(iOpExibida != *mpucIndiceOpAtual)
    {
      //procura na lista e posiciona na opcao atual
      iOpExibida = 0;
      pucItem = mpucListaOpcoes;
      while(iOpExibida < *mpucIndiceOpAtual)
      {
        //se for o caracter nulo, anda 1 e sai
        if(!(*pucItem))
        {
          pucItem++;
          iOpExibida++;
          //se eh o item pretendido, sai
          if(iOpExibida == *mpucIndiceOpAtual)
          {
            break;
          }  
        }
        else
        {
          pucItem++;
        }
      }
    }  

    //posiciona na segunda linha sem tem prompt, senao imprime sempre a partir do ponto salvo
    if(mpucPrompt)
    {
      lcd_gotoxy(0, 1);
    } 
    else
    {
      lcd_restorexy();
    }   
    //exibe o texto da opcao atual
    lcd_puts(pucItem);
    for(i=strlen(pucItem); i < 16; i++)
    {
      lcd_putc(' ');
    }
    
    //debug
    //lcd_gotoxy(0, 0);
    //lcd_putc(':');

    
    //espera teclas
    if(mucEntradaEhViaTeclado)
    {
      //lcd_putc('B');
      while(!barcode_ucEventoCaracterRecebido)
      {
        restart_wdt();
      }
      //lcd_putc('b');
      ucUltimoCaracter = barcode_ucUltimoCaracter();
      barcode_ucEventoCaracterRecebido = 0;
      barcode_execute();
      //lcd_putc('>');
      //lcd_putc(ucUltimoCaracter);
      //lcd_putc('<');
    }
    else
    { 
      //lcd_putc('*');
      //espera zerar o bit do botao
      while(1)
      { 
        while((PORTB&0x0f)==0x0f)
        {
          restart_wdt();
        }
        //lcd_putc('b');
        i = 100;
        while(i && ((PORTB&0x0f) != 0x0f))
        {
          restart_wdt();
          i--;
        }  
        //lcd_putc('c');
        //se cgeou a 0, confirma tecla
        if(i==0) break;
        //lcd_putc('d');
      }  
      //lcd_putc('>');
      
      //zera car.
      ucUltimoCaracter = 0;
      //se foi enter
      if(!(PORTB&GLOBAL_MASCARA_BTN_ENTER))
      {
        ucUltimoCaracter = 0x0d;
      }
      //se foi up (+)
      else if(!(PORTB&GLOBAL_MASCARA_BTN_UP))
      {
        ucUltimoCaracter = '+';
      }
      //se foi down (-)
      else if(!(PORTB&GLOBAL_MASCARA_BTN_DOWN))
      {
        ucUltimoCaracter = '-';
      }
      //se foi cancela
      else if(!(PORTB&GLOBAL_MASCARA_BTN_CANCEL))
      {
        ucUltimoCaracter = 0x1b;
      }
      
      //lcd_putc('<');
      //espera soltar
      while(1)
      { 
        while((PORTB&0x0f)!=0x0f)
        {
          restart_wdt();
        }
        i = 100;
        while(i && ((PORTB&0x0f) == 0x0f))
        {
          restart_wdt();
          i--;
        } 
        //se cgeou a 0, confirma tecla
        if(i==0) break;
      }
      //lcd_putc('E');      
    }
    
    //lcd_putc('z');
    
    //se foi enter sai
    if(ucUltimoCaracter == 0x0d)
    {
      //true pois confirmou
      return 1;
    }
    //se foi cancel sai
    else if(ucUltimoCaracter == 0x1b)
    {
      //false pois cancelou
      return 0;
    }
    //se foi down, inc indice
    else if(ucUltimoCaracter == '-')
    {
      (*mpucIndiceOpAtual)++;
      //se passo uda ultima vai para primeira
      if(*mpucIndiceOpAtual >= mucQtdOpcoes)
      {
        *mpucIndiceOpAtual = 0;      
      }
    }
    //se foi up, dec indice
    else if(ucUltimoCaracter == '+')
    {
      if(*mpucIndiceOpAtual == 0)
      {
        //se estava na primeira vai apra ultima
        *mpucIndiceOpAtual = mucQtdOpcoes-1;
      }
      else
      {
        (*mpucIndiceOpAtual)--;
      }
    }
    //enao se for um numero entre '0' e'9' tenta ir para o item
    else if((ucUltimoCaracter >= '0') && (ucUltimoCaracter <= '9'))
    {
      //se opatual invalida, vai para primeira
      if((ucUltimoCaracter&0x0f) < mucQtdOpcoes)
      {
        *mpucIndiceOpAtual = ucUltimoCaracter&0x0f;
      }
    }
     
  }  

}  

//pede configuracoes e salva
void configurarParametros()
{
  unsigned char ucIndiceOpAtual;
  unsigned char ucEntradaTeclado;
  unsigned char ucDigitosEndereco; 
  unsigned char i;

  barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
  barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_DISPLAY);

  lcd_clear();
                      //0123456789abcdef  0123456789abcdef
  exibeMensagem_const("Tela de Config. \nde parametros.  ", 1000);
  lcd_clear();
  //exibeMensagem_const("Botão cancela   \npara sair...    ", 1000);

  //espera soltar
  while((PORTB&0x0f) != 0x0f)
  {
    restart_wdt();
  }

  //0 - via botoes, 1-via kbd(modulo barcode)
  ucEntradaTeclado = 0;
  
  ucIndiceOpAtual = 0;
  while(1)
  {
    restart_wdt();
    //menu de opcoes  0123456789abcdef
    sprintf(&cc[0 ], "Escolha:");
                    //9abcdef0123456789abcdef
    sprintf(&cc[9 ], "Endereco");
                    //89abcdef0123456789abcdef
    sprintf(&cc[18], "Redef.Params.");

                    //23456789abcdef0123456789abcdef
    sprintf(&cc[32], "Modo Oper.");  

    if(ucEntradaTeclado)
    {
      //poe opcoes para mudar para entrada via botoes denovo
                      //3456789abcdef0123456789abcdef
      sprintf(&cc[43], "botoes ");
    }
    else  
    {
      //poe opcoes para mudar para entrada via teclado denovo
                      //3456789abcdef0123456789abcdef
      sprintf(&cc[43], "teclado");
    }
    
  //ate pressionar a tecla cancela
                     //0123456789abcdef
    if(!ucMenuEscolha(cc, &cc[9 ], 4, &ucIndiceOpAtual, ucEntradaTeclado))
    {
      break;
    }
    
    //limpa a tela
    lcd_clear();
    
    //ve a opcao
    switch(ucIndiceOpAtual)
    {
      //endereco
      case 0:
      
        //pega 2 digitos o endereco
        gucEnderecoComunicacao = 0;
        for(ucDigitosEndereco=0; ucDigitosEndereco<2; ucDigitosEndereco++)
        {
          //desloca digito decimal
          gucEnderecoComunicacao *= 10;
          //monta opcoes de endereco
                     //0123456789abcdef
          sprintf(cc, "Endereco");
          for(ucIndiceOpAtual=0; ucIndiceOpAtual < 10; ucIndiceOpAtual++)
          {
            i=0;
            if(ucDigitosEndereco> 0)
            {
              cc[9+(ucIndiceOpAtual*(2+ucDigitosEndereco))+i] = '0'+
                (gucEnderecoComunicacao/10);
              i++;
            }
            cc[9+(ucIndiceOpAtual*(2+ucDigitosEndereco))+i] = '0'+ucIndiceOpAtual;
            i++;
            cc[9+(ucIndiceOpAtual*(2+ucDigitosEndereco))+i] = 0;
          }
          //pega valor
          ucIndiceOpAtual = 0;
          if(ucMenuEscolha(cc, &cc[9], 10, &ucIndiceOpAtual, ucEntradaTeclado))
          {
            gucEnderecoComunicacao += ucIndiceOpAtual; 
          }
          //se cancelou, fica como zero
        }
        ucIndiceOpAtual = 0;

        //salva cfg
        eeprom_escrever(GLOBAL_INDEX_CFG_ENDERECO_REDE, gucEnderecoComunicacao); 
                            //0123456789abcdef
        exibeMensagem_const("Endereco salvo. ", 1000);
      break;  
      case 1:
        salvaCfgDaEepromEMemoria();
                            //0123456789abcdef  0123456789abcdef
        exibeMensagem_const("Valores de conf.\nredefinidos.", 1000);
      break;
      /// modo de operacao
      case 2:
                     //0123456789abcdef
          sprintf(cc, "Modo Opereracao:");
          //opcoes
                          //789abc
          sprintf(&cc[17], "Teste");
                          //3456789a  
          sprintf(&cc[23], "Captura");
                          //12345678
          sprintf(&cc[31], "Medicao");
           
          //pede para escolher
          ucIndiceOpAtual = 0;
          if(ucMenuEscolha(cc, &cc[17], 3, &ucIndiceOpAtual, ucEntradaTeclado))
          { 
            lcd_clear();
            //nao aceita captura
            if(ucIndiceOpAtual != CAPTURA_MODO_CAPTURA)
            {
              eeprom_escrever(GLOBAL_INDEX_CFG_MODO, ucIndiceOpAtual);
                                  //0123456789abcdef  0123456789abcdef
              exibeMensagem_const(" Novo modo oper. \n Salvo.         ", 1000);            
            
            }
            else
            {
                                  //0123456789abcdef 0123456789abcdef
              exibeMensagem_const("    Modo nao    \n implementado. ", 1000);            
            }
          }
          //seta indice da op atial denovo
          ucIndiceOpAtual = 2;
       break;
       
      //origem teclado ou botoes
      case 3:
        ucEntradaTeclado = !ucEntradaTeclado;
        //mostra atual
        if(ucEntradaTeclado)
        {
                              //0123456789abcdef
          exibeMensagem_const("origem: teclado!", 1000);
        }
        else  
        {
                              //0123456789abcdef
          exibeMensagem_const("origem: botoes! ", 1000);
        }  
      break;  
    }
  }

  barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
  
}


void setTimeOutSegundosOperAtivo(unsigned char mucAtivo, unsigned int muiValorSeg)
{
  gucTimeOutOperAtivo = 0;
  //se ativando seta valor e ativa
  if(mucAtivo)
  {
    //valo em segundos 
    gulTimeOutOper = muiValorSeg;
    //mult por 4000 pois sao 4 contagens por ms
    gulTimeOutOper *= 4000; 
    //ativa
    gucTimeOutOperAtivo = 1;
  }  
}

//callback do tick do timer associado apenas quando se está recebendo pel obarcode
//e necessita de timeout com controle por aqui
void funcCallbackTickTimerParaTimeoutOper(void)
{
  //trata flag de bye enviado se tem mais a enviar
  if(serial_ucUsarIntTx() && proto_getExistemBytesPedentesTransmissao())
  {
    if(TXSTAbits.TRMT)
    {
      TXSTAbits.TRMT = 0;
      //notifica o protocolo de que mais um byte foi enviado
      proto_notificaByteTransmitido();
    }
  }


  if(gucTimeOutOperAtivo && gulTimeOutOper)
  {
    gulTimeOutOper--;
  }
}




void mostraConsumo(unsigned long *mpulVal, unsigned char mucEvitaZerosAntes)
{
  unsigned char i;
  
  //mostra ocupando 5 caracteres e inteiro
  for(i=0; i<5; i++)
  {
    switch(i)
    {
      case 0: cc[i] = ((*mpulVal / 10000) % 10) + '0'; break;  
      case 1: cc[i] = ((*mpulVal / 1000 ) % 10) + '0'; break;  
      case 2: cc[i] = ((*mpulVal / 100  ) % 10) + '0'; break;  
      case 3: cc[i] = ((*mpulVal / 10   ) % 10) + '0'; break;  
      case 4: cc[i] = ((*mpulVal / 1    ) % 10) + '0'; break;  
    }
    //se ainaesta soh nos zeros e este eh zero, troca por espaço (exceto se for o ultimo)
    if(mucEvitaZerosAntes && (cc[i] == '0') && i < 4)
    {
      cc[i] = ' ';
    
    }
    else
    {
      mucEvitaZerosAntes = 0;
    }
  }
  //põe nulo no final e imprime
  cc[5] = 0;
  lcd_puts(cc);
}

//seta saida associada ao consumo
void setSaidaAssociadaConsumo(unsigned char mucEstado)
{
  if((gucSaidaAssociadaOperacao>=0) && (gucSaidaAssociadaOperacao<=3))
  {
    forcaBobina(gucSaidaAssociadaOperacao, mucEstado);
  }
}  


//imprime sol. do ID/lote e dasinstala func .da callback de timeout
//além de inicialziar o barcode
void  iniciaSolicitacaoID(unsigned char mucLagCampoEntrada)
{
  unsigned char i, ucCol;
  
  ucCol = 0;
  
  lcd_clear();
  
  //seleciona barcode como entrada atual
  barra_setEntradaAtual(GLOBAL_SEL_ENT_IA);


  //pega todos os caracteres do buffer do barcode
  while(barcode_ucEventoCaracterRecebido)
  {
    barcode_ucEventoCaracterRecebido = 0;
    barcode_execute();
    delay_ms(10);
  }
  
                 //0123456789abcdef 0123456789abcdef  
  lcd_puts_const("Informe ID/Lote:\n");
  //se tem especo para o "Valor:[" (6 digitos), mostra, senao apenas [
  if( (16-mucLagCampoEntrada) > 6)
  {
    lcd_puts_const("Valor:[");
    ucCol = 7;
  }
  else
  {
    lcd_putc('[');
    ucCol = 1;
  }
  //imprime e mbranco no especo reservado a digitacao
  for(i=0; i < mucLagCampoEntrada; i++)
  {
    lcd_putc(' ');
  }

  lcd_putc(']');
  //posiciona
  lcd_gotoxy(ucCol, 1);
  lcd_cursor_on();
  
  //aqui muda estado para iniciar recepcao de caracteres do barcode pela serial sinc.
  barcode_ucEventoCaracterRecebido = 0;
  serialsinc_setEstado(MAQGERAL_ENVIANDO_CTS);
}


void main(){

    unsigned int uiLoopsSemComunicacao;
    unsigned int uiContadorPassagensLoopPrinc;
    unsigned char i;
    unsigned char iCarAtual;
    unsigned char iIndiceBCD;
    //controle do pedido de confirmacao para inicio
    struct
    {
      unsigned char ucOpcaoSelecionada : 6;
      unsigned char ucPressionouEnter  : 1;
      unsigned char ucPressionouCancel : 1;      
    } ControleOpcoesLinha;

  //selecao generica de opcoes
  ControleOpcoesLinha.ucOpcaoSelecionada = 0; // 0=nao, 1=sim 
  ControleOpcoesLinha.ucPressionouEnter = 0;
  ControleOpcoesLinha.ucPressionouCancel = 0;      
    
  /*
    Configura manipular de eventos
  */    
  gbeBaseEventos.OnInicializar = NULL;
  //manipulador de evento para Antes do módulo colocar dados na saida
  gbeBaseEventos.OnAntesSaidaDados = &OnAntesSaidaDados;
  //manipulador de evento para Após o módulo colocar dados na saida
  gbeBaseEventos.OnDepoisSaidaDados = &OnDepoisSaidaDados;
  //manipulador de evento para Antes do módulo ler dados da entrada
  gbeBaseEventos.OnAntesEntradaDados = &OnAntesEntradaDados;
  //manipulador de evento para Após o módulo ler dados da entrada
  gbeBaseEventos.OnDepoisEntradaDados = &OnDepoisEntradaDados;
    
    
    
	/*
		configurar os ports
	*/
	//desconfigura todos os finos de AD (senao configurados pelos modulos que usarem)
	   ADCON1 = 0xff;
	
	
	//Desabilita todas as interrupoes ()depois cada modul ovia ativando as suas
	INTCON=0x00;	// desabilito todas interrupções

	TRISD=0x00;	 // configuro todo port d como saída
	LATD=0x00;
	PORTD=0x00;

	// port E -> LCD
	TRISE=0x00;	 // configuro todo port E como saída
	LATE=0x00;
	PORTE=0x00;
	
	//PORTB É A ENTRADA DO BARRAMENTO
	//PORTB=0xFF;
	TRISB=0xFF;
	
  //estado de operacao
  gucEstadoOperacao = ESTADOOPER_PARADA;


  barra_iniciar(GLOBAL_SEL_ENT_IU_BOTOES, GLOBAL_SEL_SAI_IU_DISPLAY, 0,
    &gbeBaseEventos);
                                        

	Iniciar_Timer();

  //timeout para operacao inicia desativado
	setTimeOutSegundosOperAtivo(0,0);
	
  //associa funcao de callback para controle de timeout
  utils_setFuncCallbackTickTimer(&funcCallbackTickTimerParaTimeoutOper);

  barra_setSaidaAtiva(1);

	lcd_init(&gbeBaseEventos);	// inicializa o LCD

  //inicia barcode com comunicação em recepcao
  barcode_iniciar(&gbeBaseEventos, MAQGERAL_RECEBENDO);


  //verifica se o botao F2 está pressionado, se estiver fica no modo de teste
  //verifica se o botao ENTER está pressionado, e estiver abre opcoes cfg
  i=100;
  while(i && ((!(PORTB&GLOBAL_MASCARA_BTN_F2)) || (!(PORTB&GLOBAL_MASCARA_BTN_ENTER))))
  {
     i--;
  }
  //i = 0;
  //se chego ua 0 aceita tecla
  if(i == 0)
  {
    //se ENTER, vai apra menu
    if(!(PORTB&GLOBAL_MASCARA_BTN_ENTER))
    {
      configurarParametros();
    }
    //senao modeo de teste
    else
    /////////////////////// inicio do modo de teste de I/O /////////////////////////  
    {
      lcd_clear();
                    //0123456789abcdef
      lcd_puts_const(" Desabilitado.  ");
      delay_ms(2000);
/*      
  
      //inicia na saida inicializada
      i= barra_getSaidaAtual();
      iCarAtual = 0; //contador para os leds
      iIndiceBCD = 1; //para desloar saidas da automacao
      uiContadorPassagensLoopPrinc = 10; //para dar u mtempo maior para a mudança das saidas
    
      lcd_clear();
  
      //inicio da segunda linha
      lcd_gotoxy(0,1);
                     //0123456789abcdef
      lcd_puts_const("BC:[ag. entrada]");
      //lcd_gotoxy(3,1);
      //lcd_savexy();
  
    
      while(1)
      {
        //seleciona entrada
        barra_setEntradaAtual(i&0x01);
        //pequeno delay para poder carregar as entradas
        delay_ms(1);
    
        //mostra estado das entradas
        switch(barra_getEntradaAtual())
        {
          case GLOBAL_SEL_ENT_IA:
            //posiciona na primeira linha
            lcd_gotoxy(0,0);
                           //0123456789abcdef
            lcd_puts_const("IA:");
    
            //pega vlaores 
            cc[0] = PORTB&0x02 ? '1' : '0';
            cc[1] = PORTB&0x04 ? '1' : '0';
            cc[2] = PORTB&0x08 ? '1' : '0';
            cc[3] = PORTB&0x10 ? '1' : '0';
            cc[4] = 0;
            lcd_puts(cc);
            
            //se recebeu caracter do barcode
            if(barcode_ucEventoCaracterRecebido)
            {
              //pega valor  que vai para pegar no endereco 6 dos holding register
              gucUltimoCaracter = barcode_ucUltimoCaracter();
              //zera flag de evento
              barcode_ucEventoCaracterRecebido = 0;
              //chama execute para perceber que foi tratado o caracter
              barcode_execute();
  
              //inicio da segunda linha
              lcd_gotoxy(0,1);
                             //0123456789abcdef
              lcd_puts_const("BC:[           ]");
              lcd_gotoxy(4,1);
              
              //lcd_restorexy();
              
              //poe primeiro caracter no buffer
              cc[0] = gucUltimoCaracter;
              cc[1] = 0;
              
              //lcd_puts(cc); 
                
              lcd_cursor_on();
              
              //lcd_savexy();
  
              //chama funcao para pegar o restante dos caracteres do código
              //(no maximo 11 digitos)
              //termina com enter, e espera no máxmo 10 segundo, ecoando para tela
              barcode_gets(cc, 11, "\r\n", NULL, 10, 1);
  
              lcd_cursor_off();
              
            }            
          break;
          
          case GLOBAL_SEL_ENT_IU_BOTOES:
            //posiciona na segunda linha
            lcd_gotoxy(7,0);
            
                    //0123456789abcdef
            lcd_puts_const("IU:");
    
            //pega vlaores 
            cc[0] = PORTB&GLOBAL_MASCARA_BTN_ENTER ? '_' : 'E';
            cc[1] = PORTB&GLOBAL_MASCARA_BTN_UP ? '_' : 'U';
            cc[2] = PORTB&GLOBAL_MASCARA_BTN_DOWN ? '_' : 'D';
            cc[3] = PORTB&GLOBAL_MASCARA_BTN_CANCEL ? '_' : 'C';
            cc[4] = PORTB&GLOBAL_MASCARA_BTN_F1 ? '_' : '1';
            cc[5] = PORTB&GLOBAL_MASCARA_BTN_F2 ? '_' : '2';
            cc[6] = 0;
            lcd_puts(cc);
          break;
        }
    
    
        //selecioan saida alguma apra poder evitar mudanca indevida
        barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
        delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
    
        //zera saida
        PORTD = 0; 
    
        //sempre que a saida for ser a de leds, muda valor para contar
        if(i == GLOBAL_SEL_SAI_IU_LEDS)
        {
          iCarAtual++;
          //copia para o leds as entradas
          PORTD = iCarAtual;
        }
        //se for saidas de automacao vai deslocando
        else 
        if(i == GLOBAL_SEL_SAI_IA)
        {
          uiContadorPassagensLoopPrinc--;
          if(!uiContadorPassagensLoopPrinc) //para dar u mtempo maior para a mudança das saidas
          {
            uiContadorPassagensLoopPrinc = 10;
            //se fora da faixa, inicia
            if((!iIndiceBCD) || (iIndiceBCD>8))
            {
              iIndiceBCD = 0x01;
            }
            else
            {
              iIndiceBCD <<= 1;
            }
          }
          PORTD = iIndiceBCD;  
        }
    
        //seleciona a saida que vai pegar os dados postos em PORTD
        barra_setSaidaAtual(i);
    
        //temporiza 
       	delay_ms(100);	
    
        //muda entrada e saida
        i++;
        //i vai ate o valor maximo de saida usado
        i %= (GLOBAL_SEL_SAI_IU_LEDS+1);
      }
*/      
    }  
    /////////////////////// fim do modo de teste de I/O ///////////////////////  
  }
  

  //pega as entradas digitais
  //seleciona entrada
  barra_setEntradaAtual(GLOBAL_SEL_ENT_IA);
  gucEspelhoEntradasDigitais = PORTB;
  //desloca 1 para direita para ficar na ordem
  gucEspelhoEntradasDigitais >>= 1;
  //tira bits mais sig, para ficar apenas com as 4 entradas
  gucEspelhoEntradasDigitais &= 0x0f;

  
  //deseleciona as saidas parap doer mudar porta sem afetar
  barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
  delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
  //flag que indica que deve atualzair saida pelo espelho
  gucAtualizarSaidasDigitais = 0;
  //coloca saidas desligadas
  gucEspelhoSaidasDigitais = 0;
  //teste - inicia autorizado
  setOperacaoAutorizada(1);

  //muda dados da saida
  PORTD = gucEspelhoSaidasDigitais;
  //seleciona no barramento as saidas da interface de automação para zerar 
  //elas realmente                         
  barra_setSaidaAtual(GLOBAL_SEL_SAI_IA);
  
  
  
  //seleciona nenhuma saida
  barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
  delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
  //altera led
  setLed(GLOBAL_MASCARA_LED_RODANDO, 0);
  setLed(GLOBAL_MASCARA_LED_MEDINDO, 0);
  setLed(GLOBAL_MASCARA_LED_DEBUG, 0);
  setLed(GLOBAL_MASCARA_LED_COM, 1);
  //carrega para saida
  barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS);

	
  gucPassoParado = 0xff;

                      


	eeprom_iniciar(&gbeBaseEventos);
	//jah le cfgs
	leCfgDaEeprom();	
	
	//lcd_incxy(-2, 0);

  //lcd_puts_const("Ok");
 

#ifdef modulo_rtc
    //modulo de rtc
	lcd_restorexy();
                        //0123456789abcdef  0123456789abcdef
    exibeMensagem_const("Relogio..  ", 10); 

	rtc_iniciar(&gbeBaseEventos);

	lcd_incxy(-2, 0);
  lcd_puts_const("Ok"); 
#endif	

    
    
    



	serial_iniciarEx(81,  //Valor de SPBRG obtido experimentalmente para velocidade de 9600
                     0,   //BRGH = 0
					 1, 1, &gbeBaseEventos);  // int. por Rx data habilitada



  proto_iniciar(PROTO_MODBUS_ASCII, gucEnderecoComunicacao, 4, &gbeBaseEventos); 
 lcd_puts_const("Ender. Rede: ");
   
  //poe como string
  i=0;
  cc[i++] = (gucEnderecoComunicacao/100)%10+'0';
  cc[i++] = (gucEnderecoComunicacao/10 )%10+'0';
  cc[i++] = (gucEnderecoComunicacao    )%10+'0';
  cc[i] = 0;
  
  lcd_puts(cc);
  //espera um pouco
  delay_ms(1000); 

  //CAPTURA_MODO_TESTE 0 //CAPTURA_MODO_CAPTURA 1 //CAPTURA_MODO_MEDICAO 2 
  captura_iniciar(&cfg, &cc[0], &gbeBaseEventos);
  

	//para controle de indicadores de leds
  uiLoopsSemComunicacao = QTD_PASSAGENS_LED_COM;
	uiContadorPassagensLoopPrinc = QTD_PASSAGENS_LED_RODANDO;
	
  //imprime sol. do ID/lote e dasinstala func .da callback de timeout
  //além de inicialziar o barcode
   iniciaSolicitacaoID(LARGURA_CAMPO_ID);
	

  for(;;)
  {
  
		//restart watchdog
		restart_wdt();
		
    //por tx byte - se bytes a enviar e esta com int por trnsmissão
    //como nao consegui config. sem trava a int por envio, fica testando no loop principal
    //se terminpo ude enviar byte
    /**** testand por isto no callback do timer
    if(serial_ucUsarIntTx() && proto_getExistemBytesPedentesTransmissao())
    {
      if(TXSTAbits.TRMT)
      {
        TXSTAbits.TRMT = 0;
        //notifica o protocolo de que mais um byte foi enviado
        proto_notificaByteTransmitido();
      }
    }
    //se nao esta enviando bytes preocupa-se com os leds
    else
    */
    {
      //a cada n passagens pisca led indicados
      if(uiContadorPassagensLoopPrinc)
      {
        uiContadorPassagensLoopPrinc--;
        if(!uiContadorPassagensLoopPrinc)
        {
          //muda led
          //seleciona nenhuma saida, seta estado led e sel. saida de led
          barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
          delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
          setLed(GLOBAL_MASCARA_LED_RODANDO, !getLed(GLOBAL_MASCARA_LED_RODANDO));
          barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS);
          delay_ms(DELAY_MS_CHAVEANDO_SAIDA);
          //recarrega
          uiContadorPassagensLoopPrinc = QTD_PASSAGENS_LED_RODANDO;
        }
      }
      //se loop sem comunicacao muito tempo apaga led
      if(uiLoopsSemComunicacao)
  		{
  		  uiLoopsSemComunicacao--;
  		  //se chegou em 0 cende led indicador
  		  if(!uiLoopsSemComunicacao)
  		  {
          //seleciona nenhuma saida, seta estado led e sel. saida de led
          barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
          delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
          setLed(GLOBAL_MASCARA_LED_COM, 0);
          barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS);
          delay_ms(DELAY_MS_CHAVEANDO_SAIDA);
        }
      }      
    }  
  
		//restart watchdog
		restart_wdt();
		
		//considera qu a entrada atual eh, na maoria das vezes a de automacao
		//pois eh onde é lido o barcode e as entradas digitais
		//se for necessário ler um botão a rotina que faz isso deve mudar a entrada
    barra_setEntradaAtual(GLOBAL_SEL_ENT_IA);
    gucEspelhoEntradasDigitais = PORTB;
    //desloca 1 para direita para ficar na ordem
    gucEspelhoEntradasDigitais >>= 1;
    //tira bits mais sig, para ficar apenas com as 4 entradas
    gucEspelhoEntradasDigitais &= 0x0f;
    
  
		//restart watchdog
		restart_wdt();
		
    
		//ve se completou mensagem recebida trata, independente do estado
		if(proto_bEventoMsgRecebida)
		{
			//trata nova mensagem em passos
			//trataMensagemProtocoloEmPassos(0);
			trataMensagemProtocolo();

			//zera flag na func agora
			proto_bEventoMsgRecebida = 0;

			//para controle de nao ficar imprindo cisa oc mdelai nas infor de parado
	    uiLoopsSemComunicacao = QTD_PASSAGENS_LED_COM;

      //se desligado, liga led
      if(!getLed(GLOBAL_MASCARA_LED_COM))
      {
        //seleciona nenhuma saida
        barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
        delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
        setLed(GLOBAL_MASCARA_LED_COM, 1);
        barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_LEDS);
        delay_ms(DELAY_MS_CHAVEANDO_SAIDA);
      }
		}
		
    //seleciona no barramento as saidas da interface de automação para atualizar seus valores
    //testa antes flag que indica que deve atualzair saida pelo espelho
    if(gucAtualizarSaidasDigitais)
    {
      //limpa flag
      gucAtualizarSaidasDigitais = 0;
      //deseleciona as saidas parap doer mudar porta sem afetar
      barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
      delay_ms(DELAY_MS_CHAVEANDO_SAIDA); 
      //muda dados da saida
      PORTD = gucEspelhoSaidasDigitais;
      //elas realmente
      barra_setSaidaAtual(GLOBAL_SEL_SAI_IA);
    }  
		

		//trata de acordo com o estado atual de operacao
		switch(gucEstadoOperacao)
		{
      //  case 0: return "Inicializando...";
      case ESTADOOPER_INICIALIZANDO:
         //lcd_gotoxy(0,0);
         //lcd_putc('@');
         
        //se usuario na oconfirmou ainda
        if(!ControleOpcoesLinha.ucPressionouEnter)
        {
          //se pressionou enter confirma, se cancl cancela se up ou down muda opcao
          if(ucPressionouBotao(GLOBAL_MASCARA_BTN_ENTER))
          {
            ControleOpcoesLinha.ucPressionouEnter = 1;
          }
          else if(ucPressionouBotao(GLOBAL_MASCARA_BTN_CANCEL))
          {
            ControleOpcoesLinha.ucPressionouCancel = 1;
          }
          else if(ucPressionouBotao(GLOBAL_MASCARA_BTN_UP) || ucPressionouBotao(GLOBAL_MASCARA_BTN_DOWN))
          {
            lcd_gotoxy(10,0);
            //0=nao, 1=sim
            //muda opcao
            ControleOpcoesLinha.ucOpcaoSelecionada = !ControleOpcoesLinha.ucOpcaoSelecionada;
            if(ControleOpcoesLinha.ucOpcaoSelecionada == 0)
            {                                    
                            //abcdef
              lcd_puts_const("[N] S ");
            }
            else 
            {
                            //abcdef
              lcd_puts_const(" N [S]");
            }
            //reset do timeout
            setTimeOutSegundosOperAtivo(1, TIMEOUT_SEG_CONFIRMA_INICIO);
            
          }

          //se cancelou ou na oconfirmou
          if(ControleOpcoesLinha.ucPressionouCancel)
          {
            //SIMULA TIMEOUT (nao pis zera pois int esta hab)
            setTimeOutSegundosOperAtivo(1, 0);
          }
          
          //senão se press enter
          else if(ControleOpcoesLinha.ucPressionouEnter)
          {
            //se sim esta pronto (depede apenas da autoriz. geral)
            if(ControleOpcoesLinha.ucOpcaoSelecionada == 1)
            {
              //se falta autoriz. mostra isto
              if(!operacaoAutorizada())
              {
                lcd_clear();
                              //0123456789ABCDEF\n0123456789ABCDEF
                lcd_puts_const("Ag. autorizacao \npara operacao...");
              }
            }
            //se nao, simula timeout tambem
            else
            {
              //SIMULA TIMEOUT proximo (nao pis zera pois int esta hab)
              setTimeOutSegundosOperAtivo(1, 0);
            }
          }
        }
        //senao esta aqui ainda apenasp or noa ter sido autorizado
        else
        {
          //nste caso se press cancel, cancela
          if(ucPressionouBotao(GLOBAL_MASCARA_BTN_CANCEL))
          {
            //SIMULA TIMEOUT proximo (nao pis zera pois int esta hab)
            setTimeOutSegundosOperAtivo(1, 0);
          }
        }
        
        
        //se foi autorizada e conf. pelo usiário, inicia medicao
        if(operacaoAutorizada() && (ControleOpcoesLinha.ucPressionouEnter && 
             (ControleOpcoesLinha.ucOpcaoSelecionada)))
        {
          lcd_clear();
                              //0123456789ABCDEF\n0123456789ABCDEF
          exibeMensagem_const("Inicio  operacao\n  autorizado.   ", 500);
          
          //zera medicao parcial
          gConsumoOperAtual.ulValor = 0;
          gulValorConsumoAnterior = 0;
           
          //vai aora estado de trabalho
          gucEstadoOperacao = ESTADOOPER_TRABALHANDO;
          
          //poe na primeira linah do lcd que está trabalhando e mula para a 
          //segunda
        	//escreve prompt
        	lcd_clear();
        	               //0123456789abcdef  0123456789abcdef  
        	lcd_puts_const("Est. em operacao \nCons.:    0(kWh)");
        	//posicina no inicio da area onde deverá mostrar o consumo
        	//e salva posicao
        	lcd_gotoxy(6, 1);
        	lcd_savexy();
        	
        	//carrega timeout para controle de tempo sem consumo (TIMEOUT_SEG_SEM_CONSUMO_ENERGIA minutos)
        	setTimeOutSegundosOperAtivo(1, TIMEOUT_SEG_SEM_CONSUMO_ENERGIA);
        	
        	//ativa saida associada a operacao
        	setSaidaAssociadaConsumo(1);
        	
        	//ativa captura
    			captura_rodar(1);

        }
        //senão, se timeout chegou ao fim, desiste e volta ao estado de parado
        else if(!gulTimeOutOper)
        {
          //informa timeout
          lcd_clear();
                              //0123456789ABCDEF\n0123456789ABCDEF
          exibeMensagem_const("Inicio  operacao\n   cancelado.   ", 500);
          //volta a ficar parada pois autorização não veio
          gucEstadoOperacao = ESTADOOPER_PARADA; 
        	//escreve prompt
        	//lcd_clear();
        	//lcd_putc('_');
          //pega todos os caracteres do buffer do barcode
          while(barcode_ucEventoCaracterRecebido)
          {
            barcode_ucEventoCaracterRecebido = 0;
            barcode_execute();
            delay_ms(10);
          }
          //imprime sol. do ID/lote e dasinstala func .da callback de timeout
          //além de inicialziar o barcode
          iniciaSolicitacaoID(LARGURA_CAMPO_ID);
          
        }
        
      break;
      
      //  case 1: return "Parada.";
      case ESTADOOPER_PARADA:

        //se esta com o estado de operacao parado, e pegou evento de caracter recebido
        if(barcode_ucEventoCaracterRecebido)
        {
          //pega valor  que vai para pegar no endereco 6 dos holding register
          gucUltimoCaracter = barcode_ucUltimoCaracter();
          //zera flag de evento
          barcode_ucEventoCaracterRecebido = 0;
          //chama execute para perceber que foi tratado o caracter
          barcode_execute();
          
          //poe primeiro caracter no buffer
          cc[0] = gucUltimoCaracter;
          cc[1] = 0; 
            
          //chama funcao para pegar o restante dos caracteres do código
          //(no maximo LARGURA_CAMPO_ID digitos)
          //termina com enter, e espera no máxmo TIMEOUT_CAMPO_ID segundos, ecoando para tela
          barcode_gets(cc, LARGURA_CAMPO_ID, "\r\n", "0123456789", TIMEOUT_CAMPO_ID, 1);
          
          lcd_cursor_off();
          
          //se foicou em branco solicita novamente
          if(strlen(cc) == 0)
          {
            iniciaSolicitacaoID(LARGURA_CAMPO_ID);
          }
          //senão algo foi efetivamente informado
          else
          { 
            //para maquina de comunicacao
            serialsinc_setEstado(MAQGERAL_PARADA);
            
            //atualzia ultimo caracter (a partir da string retornada)
            gucUltimoCaracter = cc[strlen(cc)-1];
            
            
            //valor de teste
            /*
            gBarCodeBCD.ucBuffer[3] = 0x01;
            gBarCodeBCD.ucBuffer[2] = 0x23;
            gBarCodeBCD.ucBuffer[1] = 0x45;
            gBarCodeBCD.ucBuffer[0] = 0x67;
            */
  
            //de codifica o numero aqui como BCD
            gBarCodeBCD.ulValor = 0;
            iIndiceBCD = 0;
            iCarAtual = 0;
            for(i=strlen(cc); i; i--)
            {
              //se eh o impar (muda de indice no BCD)
              if(iCarAtual&0x01)
              {
                //se impar poe na parte mais sig
                gBarCodeBCD.ucBuffer[iIndiceBCD] |= cc[i-1]<<4;
                //ajusta o indice do buffer BCD
                //se o indice jah eh 3, sai
                if(iIndiceBCD == 3)
                {
                  break;
                }  
                else
                {  
                  iIndiceBCD++;
                }  
              }
              //senão, se par põe na parte menos sig
              else
              {
                gBarCodeBCD.ucBuffer[iIndiceBCD] = cc[i-1] & 0x0f;
              }  
  
              iCarAtual++;
            }
            
            //muda estado para iniciando a fim de esperar receber autorização do inicio
            gucEstadoOperacao = ESTADOOPER_INICIALIZANDO;
            
            //mostra mss de conformacao para o valor
            lcd_gotoxy(0, 0);
                          //0123456789abcdef
            lcd_puts_const("Confirma?  N [S]");
            ControleOpcoesLinha.ucOpcaoSelecionada = 1; // 0=nao, 1=sim 
            ControleOpcoesLinha.ucPressionouEnter = 0;
            ControleOpcoesLinha.ucPressionouCancel = 0;      
            
            //timeout 
            setTimeOutSegundosOperAtivo(1, TIMEOUT_SEG_CONFIRMA_INICIO);
          }  
        }   
          
      break;
      //  case 2: return "Trabalhando";
      case ESTADOOPER_TRABALHANDO:
        //lcd_gotoxy(0,0);
        //lcd_putc('#');
        
        //chama execute do modul ode captura
        captura_execute();
        
        //de acordo com o mod trata
        switch(captura_ucRetornaModo())
        {
  				case CAPTURA_MODO_TESTE:
  				  //o modulo jah imprime os dados, aqui espera apenas o cancela
            if(ucPressionouBotao(GLOBAL_MASCARA_BTN_CANCEL))
            {
              //vai apra o estado de finalziação da operação para salvar totalizador
              //acumulado.
              gucEstadoOperacao = ESTADOOPER_FINALIZANDO; 
            	//escreve prompt
            	lcd_clear();
            	               //0123456789abcdef  0123456789abcdef  
            	lcd_puts_const("Finalizando teste\ncaptura ADC.    ");
            	delay_ms(1000);
            }
  				break;
  				case CAPTURA_MODO_CAPTURA:
  				  gucEstadoOperacao = ESTADOOPER_FINALIZANDO;
  				  lcd_clear();
                       //0123456789abcdef  0123456789abcdef
  					sprintf(cc, "Captura de dados\nnão habilitada..");
          	delay_ms(1000);
          	//finaliza
  				break;
  				case CAPTURA_MODO_MEDICAO:
  				
            //no caso de medicao, pdoe ser pressionado Enter, sendo que será pedido
            //no passo finalziando
            //confirmacao
            if(ucPressionouBotao(GLOBAL_MASCARA_BTN_ENTER))
            {
              //vai para o estado de finalziacao que neste caso, pro ser medicao, estar autoriza
              //e sem timeout vai pedir confirmacao do usuario
              gucEstadoOperacao = ESTADOOPER_FINALIZANDO;
              
            }
            else
            {
              //por enquanto da um delay grande aqui para poder ficar vendo os valores
              //delay_ms(1000); 
            
              /////////////////// debug 0 inc. valor sozinho //////////////////
              
              barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
              //chama funcao do modulo de medicao para tatar valores
              //  debug:
              if(!(PORTB&GLOBAL_MASCARA_BTN_UP))
              {
                if((gConsumoOperAtual.ulValor+100) <= MAX_VALOR_CONSUMO_SIM)
                {
                  gConsumoOperAtual.ulValor += 100;
                  delay_ms(1000);
                }
              }  
              else if(!(PORTB&GLOBAL_MASCARA_BTN_DOWN))   
              {
                if((gConsumoOperAtual.ulValor+1) <= MAX_VALOR_CONSUMO_SIM)
                {                           
                  gConsumoOperAtual.ulValor++;
                  delay_ms(1000);
                }
              }  
              else
              {
                if((gConsumoOperAtual.ulValor+10) <= MAX_VALOR_CONSUMO_SIM)
                {
                  gConsumoOperAtual.ulValor += 10;  
                  delay_ms(100);
                }  
              }
              
              //////////////////////////////////// debug ///////////////////////  
              
              
              //verificao de eventos do modulo de mendicao para calcular o valor de 
              //gConsumoOperAtual.ulValor
              
              
              //se consumo dif. do anterior reset do timeout, senao deixa correr
              if(gConsumoOperAtual.ulValor != gulValorConsumoAnterior)
              {
                //iguala
                gulValorConsumoAnterior = gConsumoOperAtual.ulValor; 
              	//carrega timeout para controle de tempo sem consumo (TIMEOUT_SEG_SEM_CONSUMO_ENERGIA minutos)
              	setTimeOutSegundosOperAtivo(1, TIMEOUT_SEG_SEM_CONSUMO_ENERGIA);
    
                //atualiza no display (de tempos em tempos apenas)
                if(1)
                {
                  //posiciona na posicao de inicio do campo na tela
                  lcd_restorexy();
                  //mostra consumo atual
                  mostraConsumo(&gConsumoOperAtual.ulValor, 1);
                }	
              }  
              
              //se timeout chegou ao fim, para operacao pois ja passou um tempo
              //sem energia
              //tambem para se a autorização for retirada
              if((!gulTimeOutOper) || (!operacaoAutorizada()))
              {
                         
                //vai apra o estado de finalziação da operação para salvar totalizador
                //acumulado.
                gucEstadoOperacao = ESTADOOPER_FINALIZANDO; 
              	//escreve prompt
              	lcd_clear();
              	               //0123456789abcdef  0123456789abcdef  
              	lcd_puts_const("Finalizando...  \nTotal: ");
              	
              }              				
  				break;
  				default:
  				  gucEstadoOperacao = ESTADOOPER_FINALIZANDO;
  				  lcd_clear();
                       //0123456789abcdef  0123456789abcdef
  					sprintf(cc, "Modo de operacao\ninvalido...     ");
          	delay_ms(1000);
          	//finaliza  
        }
          

        }  
      
      break;
      //  case 3: return "Finalizando...";
      case ESTADOOPER_FINALIZANDO:
      
        //se em modo medicao, pede confirmacao
        if(captura_ucRetornaModo() == CAPTURA_MODO_MEDICAO)
        {
        
          //desativa captura
    			captura_rodar(0);
          
        	//desativa saida associada a operacao
          setSaidaAssociadaConsumo(0);
        
          //no entanto apenas se nao foi por timeout ou autorizacao retirada
          if(gulTimeOutOper && operacaoAutorizada())
          {
            //imprime pergunda
            lcd_gotoxy(0, 0);
                          //0123456789abcdef
            lcd_puts_const("Confirma? ");  
            //prepara para usat funcao de menu
                           //0123456789abcdef
            sprintf(&cc[0], "[N] S ");
            sprintf(&cc[7], " N [S]");
            //i indica a opcao (inicia com o nao -> 0)
            i = 0;
            if(!ucMenuEscolha(NULL, cc, 2, &i, 0))
            {
              //cancelado é o mewmso que ter escolhido nao
              i = 0;
            }
            //se cancelou, imprime primeria linha e volta para estado trabalhando
            if(!i)
            {
              lcd_gotoxy(0, 0);
                        	  //0123456789abcdef  
            	lcd_puts_const("Est. em operacao");
            	//muda estado e reibicia timeout
              gucEstadoOperacao = ESTADOOPER_TRABALHANDO;
              setTimeOutSegundosOperAtivo(1, TIMEOUT_SEG_SEM_CONSUMO_ENERGIA);
              //sai do case aqui
              break;
            }
          }
          //senao eh finalzado direto

          //soma o consumo da operacao com o consumo acumulado
          gConsumoAcumulado.ulValor += gConsumoOperAtual.ulValor;
           
          //salva acumulado
          eeprom_escreverBuffer(GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO,
            gConsumoAcumulado.ucValor, 4);
                        //0123456789abcdef  0123456789abcdef
   				lcd_puts_const("Ultimo|Acumulado\n");
   				//ultimo (5 digitos)
   				mostraConsumo(&gConsumoOperAtual.ulValor, 1);
   				lcd_puts_const(" |");
   				mostraConsumo(&gConsumoAcumulado.ulValor, 1);
   				lcd_puts_const(" kWh");
          //tempinho
         	delay_ms(2000);
        }

        //apos um tempinho volta ao estado parado
        gucEstadoOperacao = ESTADOOPER_PARADA;
        
        //prepara inici ode solicitacao de id
        iniciaSolicitacaoID(LARGURA_CAMPO_ID);
        
      break;
		}
  } // while(true)
}	// void main




//manipulador de evento para Antes do módulo colocar dados na saida
int OnAntesSaidaDados(unsigned char mucIdModulo)
{
  
  //para o modulo de display jah pode ativar direto
  if(mucIdModulo == LCD_ID_MODULO)
  {
    barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_DISPLAY);
  }
  //para os demais desativa as saidas para poder mudar portas sem refletir no modulo anterior
  else
  {
    barra_setSaidaAtual(GLOBAL_SEL_NENHUM);
    delay_ms(DELAY_MS_CHAVEANDO_SAIDA);    
  }
}
//manipulador de evento para Após o módulo colocar dados na saida
int OnDepoisSaidaDados(unsigned char mucIdModulo)
{
  //de acordo com o id do modulo (definido no seu arquivo .h), decide que saida selecionar
  switch(mucIdModulo)
  {
    //desativa as saidas para poder mudar portas sem refletir no modulo anterior
    case LCD_ID_MODULO: barra_setSaidaAtual(GLOBAL_SEL_SAI_IU_DISPLAY); break;
  } 
}
//manipulador de evento para Antes do módulo ler dados da entrada
int OnAntesEntradaDados(unsigned char mucIdModulo)
{
  //de acordo com o id do modulo (definido no seu arquivo .h), decide que entrada selecionar
  switch(mucIdModulo)
  {
    //desativa as saidas para poder mudar portas sem refletir no modulo anterior
    case BARCODE_ID_MODULO: barra_setEntradaAtual(GLOBAL_SEL_ENT_IA); break;
    default:
      barra_setEntradaAtual(GLOBAL_SEL_ENT_IU_BOTOES);
  } 
    

}
//manipulador de evento para Após o módulo ler dados da entrada
int OnDepoisEntradaDados(unsigned char mucIdModulo)
{


}
