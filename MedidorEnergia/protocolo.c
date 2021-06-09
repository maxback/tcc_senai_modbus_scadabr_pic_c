/**
*** Implementação do modulo de protocolo de comunicacao serial para a aplicacao
**/

#include <p18cxxx.h>	// inclui as definições dos ios do pic
//#include <usart.h>


#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

//definir para imprimir dados de 
//#define IMPRIME_DEBUG


#define PROTO_C

#include "global.h"
#include "..\lib\base\base.h"
#include "..\lib\senai\serial.h"
#include "protocolo.h"

//evento que flag que tem uam mensagem a ser tratada.
unsigned char proto_bEventoMsgRecebida;
//valor da mensagem recebida
unsigned char proto_ucMsgRecebida;
//define tamanho do buffer
//#define PROTO_TAMANHO_DADOS 10
//unsigned char proto_ucBufferDados[PROTO_TAMANHO_DADOS];


//buffer para envio pela serial (byte a byte)
//indicador e envio
unsigned char gucIndiceEnvio = 0;
//total de bytes a enviar
unsigned char gucTotalEnvio = 0; 


//contador de bytes recebidos
unsigned char gucTotalRecebido;
//endereco na rede
unsigned char gucEndereco;
//qtd de bytes de inicio que devem ser enviados
unsigned char gucQtdBytesInicioPacote;
//implementando um checksum para garatir que pacotes co merro n~ao estao sendo aceitos


//por como 2 bytes para permitir escontaro byte de checksum somando indevidamente sem eprde info
unsigned long gulCheckSumRx = 0;


//tipo do protocolo
unsigned char gucTipo = PROTO_PI2009;

//se esta ocupado (na inicializaçã do modulo muda para 0)
unsigned char gucOcupado = 1;

//mensagens recebidas
//#define PROTO_MSG_MODO	0	//Define modo de trabalho -> recebe 1 byte em proto_ucBufferDados com o modo
//#define PROTO_MSG_PARAR	1	//Faz parar a captura, teste ou medicao ate que uma mensagem PROTO_MSG_RODAR seja recebida, se o modo for captura o equp. jah inicia parado
//#define PROTO_MSG_RODAR	2	//Faz retomar a captura. medicao ou teste
//#define PROTO_MSG_APAGA	3	//apaga toda a memoria flash
//#define PROTO_MSG_LE	4	//le n bytes de determinado endereco de determinado CI -> proto_ucMsgRecebida[0]:Quantidades proto_ucMsgRecebida[1]:CI
//							//														  proto_ucMsgRecebida[2..4]: Endereço (3 bytes)
//#define PROTO_MSG_CONFIG_LE	5 //Le a configuracao in dicada pelo primeiro byte da area de dados -> Valores: 0-ValorADZeroCorrente, 1-FatorTrafo, 2-DescontoDiodos
//							//																			  3-Resistor1, 4-Resistor2
//
//#define PROTO_MSG_CONFIG_ESCREVE 6	//Escreve configuracao na memoria conforme valores da leitura
//

//inicializa modulo
//recebe por primeiro parametro o protocolo a ser usado PROTO_PI2009 ou PROTO_MODBUS_ASCII 
void proto_iniciar(unsigned char mucTipo, unsigned char mucEndereco, 
  unsigned char mucQtdBytesInicioPacote, TBaseEventos *mpbeBaseEventos)
{
	//evento que flag que tem uam mensagem a ser tratada.
	proto_bEventoMsgRecebida = 0;
	//valor da mensagem recebida
	proto_ucMsgRecebida = 0xff;
	//contador de bytes recebidos
	gucTotalRecebido = 0;
	//endereco na rede
	gucEndereco = mucEndereco;
    //tamanho da area de daods
    proto_ucTamanhoDados = 0;
	//salva qtd de bytes a ser enviado
	gucQtdBytesInicioPacote = mucQtdBytesInicioPacote;
	//tipo de protoclo
	gucTipo = mucTipo;
	//inicia desocupado
	gucOcupado = 0;
}


//notifica o modulo do byte recebido
void proto_notificaByteRecebido(unsigned char mucByteRecebido)
{
    static unsigned char ucBytesRecebidosAnteriores[2] = {0, 0};
    static unsigned char ucValorAnterior;
    static unsigned char ucValor;

	//se evento ainda nao tratado, ignora bytes recebidos
	if(proto_bEventoMsgRecebida)
	{
		return;
	}

	//trata recebimento do pacote ascii

  //Trata de acordo com o tip ode protocolo
  
  /////////////////////// modbus ascii ////////////////////////
  if(gucTipo == PROTO_MODBUS_ASCII)
  {
  	//se foi flag de inicio de pacote, reinicia
  	if(mucByteRecebido == PROTO_MODBUS_ASCII_CARACTER_INICIO)
  	{
  		//reinicia total recebido, jah contando o de inicio
  		gucTotalRecebido = 1;
  		proto_ucTamanhoDados = 0;
      gulCheckSumRx = 0;
  	}
  	//senao, se jah recebeu o de inicio verifica os proximos
  	else if(gucTotalRecebido)
  	{
  		//mais um byte recebido
  		gucTotalRecebido++;

    	//decodifica ascii para bin se qtd impar
    	if(gucTotalRecebido%2)
    	{
    	  //salva valor anterior
    	  ucValorAnterior = ucValor;
    	  //calcula novo valor
      	ucValor = ucBytesRecebidosAnteriores[1]<0x41 ? (ucBytesRecebidosAnteriores[1]&0x0f):
          (ucBytesRecebidosAnteriores[1]-0x41+0x0a);
        ucValor <<= 4;  
        ucValor |= mucByteRecebido<0x41 ? (mucByteRecebido&0x0f): (mucByteRecebido-0x41+0x0a);
  		}
  
  		//verifica se acordo com o byte recebido
  		switch(gucTotalRecebido)
  		{
  			//endereco (2 e 3)
  			case 2:
  			  //não faz nada aqui
  			break;
  			case 3:
          //se não for o endereço ignora pacote            
  				if(ucValor != gucEndereco)
  				{
  					gucTotalRecebido = 0;
  				}
  				
  			break;
  
  			//comando modbus no cmapo mensagem(testado nos outros modulo) (4 e 5)
  			case 4:
        break;
  			case 5:
          //proto_ucMsgRecebida |= mucByteRecebido<0x41 ? (mucByteRecebido&0x0f): (mucByteRecebido-0x41+0x0a);
          
          proto_ucMsgRecebida = ucValor;
          //deste ponto em diante começara a salvar os dados recebidos 
  			break;		
  
  			//demais bytes sao armazenados como dados ate receber o de final de linha '\r' ou '\n'
  			default:
  			  //pode ser final do pacote
        	//tem que ter recebido pelo menos o inicio(1), o endereco(2) e o comando(2), checksum(2) e fim1(1))
        	if((gucTotalRecebido>8) && 
             ((mucByteRecebido == PROTO_MODBUS_ASCII_CARACTER_FIM1) ||
              (mucByteRecebido == PROTO_MODBUS_ASCII_CARACTER_FIM2)) && 
             ((ucBytesRecebidosAnteriores[1] == PROTO_MODBUS_ASCII_CARACTER_FIM1) ||
              (ucBytesRecebidosAnteriores[1] == PROTO_MODBUS_ASCII_CARACTER_FIM2)) ) 
          {
  				  
  				  //tem que subtrair o ultimo valor inteiro que foi o proximo checksum
  				  gulCheckSumRx -= (unsigned long)ucValorAnterior;
  					//faz o complemento de 2 ante de comaprar
  					gulCheckSumRx = 0xFFFF - gulCheckSumRx;
  					gulCheckSumRx++;
            
  					//compara byte recebido antes dos finalziadores com checksu mcamculado
  					if(ucValorAnterior == (unsigned char)(gulCheckSumRx&0x00FF))
  					{
  					  //se na verdade nao salvou todos os dados, retoran erro
  					  if(proto_ucTamanhoDados >= PROTO_TAMANHO_DADOS)
  					  {
                proto_enviaExceptionModbus(proto_ucMsgRecebida, 
                  PROTO_MODBUS_EXCEPTION_ILLEGALDATAVALUE);
              }
              //se ocupado retorna resposta exception correspondente
  					  else if(gucOcupado)
  					  {
                proto_enviaExceptionModbus(proto_ucMsgRecebida, 
                  PROTO_MODBUS_EXCEPTION_SLAVEBUSY);
              }
  					  //senão ativa evento para nova msg ser tratada
  					  else
  					  {
    						//flag para indicar que recebeu mensagem
    						proto_bEventoMsgRecebida = 1;
      					//desconta do tamanho o caracter de fim 1 
      					proto_ucTamanhoDados--;
      				}	
  					}
  					//senao ignora pacote
  					else
  					{
  						proto_ucTamanhoDados = 0;
  					}
  					//reinicia total recebido, jah contando o de inicio
  					gucTotalRecebido = 0;
  					
  				}
  				//se tiver espaco ainda, e fechou par de bytes (ucValor ok)
  				else if (gucTotalRecebido%2)
  				{
            //se tem espeço apra salvar dados que vao sendo recebidos salva
  					if(proto_ucTamanhoDados < PROTO_TAMANHO_DADOS)
  					{
        		  //salva valor
  						proto_ucBufferDados[proto_ucTamanhoDados] = ucValor;
  					}
            //senao continua recebendo o resto do pacote para alidar o CRC ainda
            //inc ponteiro (se no fina lfor maior ou igual a PROTO_TAMANHO_DADOS -> erro)
 		        proto_ucTamanhoDados++;
  				}
  			break;
  		}
  		//soma LRC
  		if(gucTotalRecebido%2)
  		{
				//decodifica endereco ascii e compara
				/*
				ucValor = ucBytesRecebidosAnteriores[1]<0x41 ? (ucBytesRecebidosAnteriores[1]&0x0f):
          (ucBytesRecebidosAnteriores[1]-0x41+0x0a);
        ucValor <<= 4;  
        ucValor |= mucByteRecebido<0x41 ? (mucByteRecebido&0x0f): (mucByteRecebido-0x41+0x0a);
        */
  		  //depois de recebido que soma o checksum
  		  gulCheckSumRx += (unsigned long)ucValor;
  		}
  		//salva anterior
  		ucBytesRecebidosAnteriores[1] = mucByteRecebido;
  	}
  }
  //////////////////////////////////////// pi2009 //////////////////////////
  else if(gucTipo == PROTO_PI2009)
  {
#ifdef nada  
  	//se foi flag de inicio de pacote, reinicia
  	if(mucByteRecebido == PROTO_CARACTER_INICIO)
  	{
  		//reinicia total recebido, jah contando o de inicio
  		gucTotalRecebido = 1;
      gulCheckSumRx = (unsigned long)PROTO_CARACTER_INICIO;
  
  	}
  	//senao, se jah recebeu o de inicio verifica os proximos
  	else if(gucTotalRecebido)
  	{
  		//mais um byte recebido
  		gucTotalRecebido++;
  
  		//verifica se acordo com o byte recebido
  		switch(gucTotalRecebido)
  		{
  			//endereco, por hora nao implementado, tem que ser igual
  			case 2:
  				//se na ofor, zera total recebido
  				if(mucByteRecebido != gucEndereco)
  				{
  					gucTotalRecebido = 0;
  				}
  			break;
  
  			//recebe a mensagem, setandoa na bariavel de comunicacao com so outros modulos
  			case 3:
  				proto_ucMsgRecebida = mucByteRecebido;
                  proto_ucTamanhoDados = 0;
  			break;		
  
  			//demais bytes sao armazenados como dados ate receber o de final de linha '\r' ou '\n'
  			default:
  				if(mucByteRecebido == '\r' || mucByteRecebido == '\n')
  				{
  					//desconta os 2 bytes do crc que foram somandos sem intencao
  					gulCheckSumRx -= (unsigned long)ucBytesRecebidosAnteriores[1];
  					gulCheckSumRx -= (unsigned long)ucBytesRecebidosAnteriores[0];
  					
  					//desconta do tamanho tambem
  					proto_ucTamanhoDados -= 2;
  
  					//calcula chacksum dos ultimos 2 caracteres
  					ucBytesRecebidosAnteriores[0] = ucBytesRecebidosAnteriores[0]<0x41 ? (ucBytesRecebidosAnteriores[0]&0x0f) :  (ucBytesRecebidosAnteriores[0]-0x41+0x0a);
  					ucBytesRecebidosAnteriores[0] <<= 4;
  					ucBytesRecebidosAnteriores[1] = ucBytesRecebidosAnteriores[1]<0x41 ? (ucBytesRecebidosAnteriores[1]&0x0f) :  (ucBytesRecebidosAnteriores[1]-0x41+0x0a);
  					//junta em um byte so
  					ucBytesRecebidosAnteriores[1] = ucBytesRecebidosAnteriores[0] | (ucBytesRecebidosAnteriores[1]&0x0f);
  					
  					//agora comapra
  					if(ucBytesRecebidosAnteriores[1] == (unsigned char)gulCheckSumRx)
  					{
  						//flag para indicar que recebeu mensagem
  						proto_bEventoMsgRecebida = 1;
  					}
  					//senao ignora pacote
  					else
  					{
  						proto_ucTamanhoDados = 0;
  					}
  					//reinicia total recebido, jah contando o de inicio
  					gucTotalRecebido = 0;
  				}
  				//se tiver espaco ainda, armazena
  				else 
  				{
  
  					if(proto_ucTamanhoDados < PROTO_TAMANHO_DADOS)
  					{
  		               //tamanho d area de dados, sem conta cab. e finalziador
  						proto_ucBufferDados[proto_ucTamanhoDados] = mucByteRecebido;
  		                proto_ucTamanhoDados++;
  					}
  					//senao encheu
  					else
  					{
  						//passou do tamanho permitido
  						//voltap ara o inicio
  						proto_ucTamanhoDados = 0;
     					    //reinicia total recebido, jah contando o de inicio
  					    gucTotalRecebido = 0;
  					}
  				}
  			break;
  		}
  		//depois de recebido que soma o checksum
  		gulCheckSumRx += (unsigned long)mucByteRecebido;
  		//salva sem pre os dois anterioresanterior para quando chegar o \n ou \r ser o checksum
  		ucBytesRecebidosAnteriores[0] = ucBytesRecebidosAnteriores[1]; 
  		ucBytesRecebidosAnteriores[1] = mucByteRecebido;
  	}
#endif  	
 } 	
}

void encodeByteOnAsciiPair(unsigned char mucValue, unsigned char *mpucTextoDados)
{
  unsigned char ucPartes[2], i;
  
  //parte mais significativa
  ucPartes[0] = mucValue >> 4;
  //parte menos significativa
  ucPartes[1] = mucValue & 0x0F;
  
  for(i=0; i<2; i++)
  {
    if(ucPartes[i] >= 0x0A)
    {
      ucPartes[i] -= 0x0A;
      ucPartes[i] += 0x41; //'A'
    }
    else
    {
      ucPartes[i] &= 0x0F;
      ucPartes[i] += 0x30; //'0'
    }
    mpucTextoDados[i] = ucPartes[i];
  }
}


unsigned char ucDecodeByteFromAsciiPair(unsigned char *mpucTextoDados, unsigned char *mpucValue)
{
  unsigned char ucPartes[2], i;
  
  //parte mais significativa
  ucPartes[0] = mpucTextoDados[0];
  //parte menos significativa
  ucPartes[1] = mpucTextoDados[1];
  
  for(i=0; i<2; i++)
  {
    if((ucPartes[i] >= 0x30) && (ucPartes[i] <= 0x39))
    {
      ucPartes[i] &= 0x0F;
    }
    else
    if((ucPartes[i] >= 0x41) && (ucPartes[i] <= 0x46))
    {
      ucPartes[i] -= 0x41;
      ucPartes[i] += 0x0A;
    }
    else
    {
       //retorna erro pois veio caracter invalido
       return 0;
    }
  }
  *mpucValue = ucPartes[0];
  *mpucValue <<= 4;
  *mpucValue += ucPartes[1];     
  //retorna ok, pois decodificou
  return 1;   
}


//permite saber se tem bytes a enviar ainda, a fi dme saber
//se deve notificar bytes. Retorna true ou false
unsigned char  proto_getExistemBytesPedentesTransmissao(void)
{
  //tambem considera que tem a enviar apenas se maq. recepcao parada
  return (gucIndiceEnvio < gucTotalEnvio) && (gucTotalRecebido == 0);
}


//notifica o modulo do byte transmitido
void proto_notificaByteTransmitido(void)
{
  //se tem byte a enviar ainda
  if(gucIndiceEnvio < gucTotalEnvio)
  {
    //envia
    serial_enviar(proto_ucBufferEnvio[gucIndiceEnvio]);
    //inc indice
    gucIndiceEnvio++;
  }
}


//Envia pacote com mensagem para o host
void proto_enviaPacote(unsigned char mucMensagem, unsigned char *mpucTextoDados)
{
	unsigned char i, ucValor, iTot;
  unsigned char ucCheckSum;
  
  //zera total a enviar
  gucTotalEnvio = 0;
	
  //Trata de acordo com o tipo de protocolo
  
  /////////////////////// modbus ascii ////////////////////////
  if(gucTipo == PROTO_MODBUS_ASCII)
  {
    //envia inicio
    proto_ucBufferEnvio[0] = PROTO_MODBUS_ASCII_CARACTER_INICIO;
    
    i = gucEndereco;
    //endereco ascii
    //menos sig
    proto_ucBufferEnvio[2] = (i&0x0f)>9 ? ((i&0x0f)-0x0a)+0x41 : 0x30+(i&0x0f);
    //mais sig
    i >>= 4;
    proto_ucBufferEnvio[1] = (i&0x0f)>9 ? ((i&0x0f)-0x0a)+0x41 : 0x30+(i&0x0f);
    
    //envia comando(mensagem)
    i = mucMensagem;
    //endereco ascii
    //menos sig
    proto_ucBufferEnvio[4] = (i&0x0f)>9 ? ((i&0x0f)-0x0a)+0x41 : 0x30+(i&0x0f);
    //mais sig
    i >>= 4;
    proto_ucBufferEnvio[3] = (i&0x0f)>9 ? ((i&0x0f)-0x0a)+0x41 : 0x30+(i&0x0f);
    
    //indicador do contador de bytes no buffer
    iTot = 5;
    //caracter nulo para poder enviar
    //proto_ucBufferEnvio[5] = 0; 
    

    //inicia checksum LCR com o endereco
    
    ucCheckSum = gucEndereco;
    //somado ao comando
    ucCheckSum += mucMensagem;
    
    //se passou dados de texto envia 
    if(mpucTextoDados)
    {
      //tem que somar o buffer
      //coma checksum pel osignificado do byte (ou seja, a cada 2 carac. u mvalor)
      for(i=0; mpucTextoDados[i]; i++, iTot++)
      {
        //copia byte para buffer
        proto_ucBufferEnvio[iTot] = mpucTextoDados[i];
        //se é byte impar, pega valor calculado
        if((i%2)==0)
        {
          if(ucDecodeByteFromAsciiPair(&mpucTextoDados[i], &ucValor))
          {
            /*
      		  ucValor = mpucTextoDados[i]<0x41 ? (mpucTextoDados[-1]&0x0f):
              (mpucTextoDados[i-1]-0x41+0x0a);
              ucValor <<= 4;  
            ucValor |= mpucTextoDados[i+1]<0x41 ? (mpucTextoDados[i]&0x0f): 
                (mpucTextoDados[i]-0x41+0x0a);
             */   
            //soma
            ucCheckSum += ucValor;
          }
        }
      }
    }
    
    //complemento de 2 para ochecksum
     ucCheckSum = 0xFF - ucCheckSum;
     ucCheckSum++;

    
    //envia o checsum e caracteres de final
    encodeByteOnAsciiPair(ucCheckSum, &proto_ucBufferEnvio[iTot]);
    iTot += 2;

    proto_ucBufferEnvio[iTot++] = PROTO_MODBUS_ASCII_CARACTER_FIM1;
    proto_ucBufferEnvio[iTot++] = PROTO_MODBUS_ASCII_CARACTER_FIM2;

    proto_ucBufferEnvio[iTot] = 0;     
    

    //se tem interrupcao ajsuta buffer para ser enviado byte a byte por int. tx
    if(serial_ucUsarIntTx())
    {
      //zera flag
      TXSTAbits.TRMT = 0;
      //total a enviar
      gucTotalEnvio = iTot;
      //envia primeiro byte (os demais vào pe int de envio)
      gucIndiceEnvio = 1;
      serial_enviar(proto_ucBufferEnvio[0]);
    
    }
    //senão envia tudo agora
    else
    {
      serial_printf(proto_ucBufferEnvio);
    }  
    
  }
  //////////////////////////////////////// pi2009 //////////////////////////
  else if(gucTipo == PROTO_PI2009)
  {
#ifdef nada  
    //envia inicio e o endereco do host é o 0x30
    cBufInicio[0] = PROTO_CARACTER_INICIO;
    cBufInicio[1] = 0;
    
    //inici checksum com caracter de inicio somand ocom o endereco do host e a mensagem
    //as repeticoes deste nao contam
    ucCheckSum = PROTO_CARACTER_INICIO;
    ucCheckSum += 0x30;
    ucCheckSum += mucMensagem;
    
    //envia caracter de abertura de pacote algumas vezes
    for(cBufInicio[2] = gucQtdBytesInicioPacote; cBufInicio[2]; cBufInicio[2]--)
    {
      serial_printf(cBufInicio);
    }
    
    cBufInicio[1] = 0x30;
    //mensagem
    cBufInicio[2] = mucMensagem;
    //caracter nulo para poder enviar
    cBufInicio[3] = 0;
    
    
    //envia inicio pela serial
    serial_printf(cBufInicio);
    
    //se passou endereco do texto, envia 
    if(mpucTextoDados)
    {
        //tem que somar o buffer
    for(i=0; mpucTextoDados[i]; i++)
    {
       ucCheckSum += mpucTextoDados[i];
    }
    serial_printf(mpucTextoDados);
    }
   
    
    //envia o checsum
    cBufInicio[0] = (ucCheckSum>>4)<0x0a ? (ucCheckSum>>4)+0x30 : ((ucCheckSum>>4)-0x0a)+0x41;
    cBufInicio[1] = (ucCheckSum&0x0f)<0x0a ? (ucCheckSum&0x0f)+0x30 : ((ucCheckSum&0x0f)-0x0a)+0x41;
    cBufInicio[2] = 0;
    
    serial_printf(cBufInicio);
    
    
    sprintf(cBufInicio, "\r\n");

  	//envia final
	  serial_printf(cBufInicio);
	  
#endif	  
  }
}


//retorna o tipo
unsigned char proto_ucTipo(void)
{
  return gucTipo;
}


//retorna exception modbus
//recebe por primeiro parametro o protocolo a ser usado PROTO_PI2009 ou PROTO_MODBUS_ASCII 
//espera no segund ocampo um dos defines PROTO_MODBUS_EXCEPTION_...  
void proto_enviaExceptionModbus(unsigned char mucCmdOrig, unsigned char mucCodException)
{
  unsigned char ucBuffer[3];
  //manda apenas o codigo do exception na area de dados
  encodeByteOnAsciiPair(mucCodException, ucBuffer);
  ucBuffer[2] = 0;
  //manda pacote ma's setando o bit 7 da mensagem original apra indicar exception 
  mucCmdOrig |= 0x80; 
   proto_enviaPacote(mucCmdOrig, ucBuffer);

}


//set status de ocupado (se true, no caso de modbus passa a retornar uma resp. de excessão)  
void proto_setOcupado(unsigned char mucOcupado)
{
  gucOcupado = mucOcupado;   
}



