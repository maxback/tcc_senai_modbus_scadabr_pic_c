/**
*** Cabeçalho do modulo de protocolo de comunicacao serial para a aplicacao
**/

#ifndef PROTO_H
#define PROTO_H

//id unico do modulo para passar nos eventos previstos em base.h
#define PROTO_ID_MODULO 105

//flag de que tem mensagem recebida
#ifndef PROTO_C
extern
#endif
unsigned char proto_bEventoMsgRecebida;

//// tipos de protocolos suportados (passado para funcao de inicialzia)
#define PROTO_PI2009 0
#define PROTO_MODBUS_ASCII 1


///////////// protocolo pi2009 /////////////// 

//CARACTER DE INICIO DE PACOTE
#define PROTO_CARACTER_INICIO ':'

//mensagens recebidas e enviadas como resposta
#define PROTO_MSG_MODO	'm'	//Define modo de trabalho -> recebe 1 byte em proto_ucBufferDados com o modo
#define PROTO_MSG_PARAR	'p'	//Faz parar a captura, teste ou medicao ate que uma mensagem PROTO_MSG_RODAR seja recebida, se o modo for captura o equp. jah inicia parado
#define PROTO_MSG_RODAR	'r'	//Faz retomar a captura. medicao ou teste
#define PROTO_MSG_APAGA	'a'	//apaga toda a memoria flash
#define PROTO_MSG_LE	'l'	//le n bytes de determinado endereco de determinado CI (tudo em ascii)-> proto_ucMsgRecebida[0..4]:Quantidades proto_ucMsgRecebida[5]:CI
							//														  proto_ucMsgRecebida[6..13]: Endereço (3 bytes)
#define PROTO_MSG_CONFIG_LE	'L' //Le a configuracao in dicada pelo primeiro byte da area de dados -> Valores(tamanho): 0-modoOperacao(1), 1-TempoEntreCapturas(2),
									// 2-ValorADZeroCorrente(2), 3-FatorTrafo(1), 4-DescontoDiodos(2)*, 5-Resistor1(4)**, 6-Resistor2(4)**
									// o desconto dos diodos preve dos 5 digitos os tris a direita após a virgula
									// o valor do registros é em ohms, ou seja, 100000 é 100k

#define PROTO_MSG_CONFIG_ESCREVE 'E'	//Escreve configuracao na memoria conforme valores da leitura
	


//mensagens enviadas
#define PROTO_MSG_REALIZADO	'!'	//indica que foi realizada a operação parada ou reinicio, de cfg de modo, escrita na memoria ou de configuracao ou de apagar dados
#define PROTO_MSG_DADOS	'd'	//indica que é resposta dos dados solicitados, sendo o primeiro byte de dados a mensagem que o solicitou e após os dados
#define PROTO_MSG_MSG_DESCONHECIDA	'?'	//indica que não entendeu a mensagem recebida
#define PROTO_MSG_PARAMETRO_INVALIDO	'*'	//indica que parametro passado n~ao fo iaceito


//////////////////////// modbus ascii ///////////////////
#define PROTO_MODBUS_ASCII_CARACTER_INICIO ':'

#define PROTO_MODBUS_ASCII_CARACTER_FIM1 0x0D
#define PROTO_MODBUS_ASCII_CARACTER_FIM2 0x0A

//comandos suportados pelo modbus ascii
//01	Read coil status
#define PROTO_MODBUS_READCOILSTATUS 1
//02	Read input status
#define PROTO_MODBUS_READINPUTSTATUS 2
//03	Read holding registers
#define PROTO_MODBUS_READHOLDINGREGISTERS 3
//04	Read input registers
#define PROTO_MODBUS_READINPUTREGISTER 4
//05	Force single coil
#define PROTO_MODBUS_FORCESINGLECOIL 5
//06	Preset single register
#define PROTO_MODBUS_PRESETSINGLEREGISTER 6
//07	Read exception status
#define PROTO_MODBUS_READEXCEPTIONSTATUS 7
//15	Force multiple coils
#define PROTO_MODBUS_FORCEMULTIPLECOILS 15
//16	Preset multiple registers
#define PROTO_MODBUS_PRESETMULTIPLEREGISTERS 16
//17	Report slave ID
#define PROTO_MODBUS_REPORTSLAVEID 17


//exception para o modbus
//(passado por parametros para proto_enviaExceptionModbus)
//Illegal Function
//	The function code received in the query is not an allowable action for the slave.  
//This may be because the function code is only applicable to newer devices, and was not implemented in the unit selected.  It could also indicate that the slave is in the wrong state to process a request of this type, for example because it is unconfigured and is being asked to return register values. If a Poll Program Complete command was issued, this code indicates that no program function preceded it.
#define PROTO_MODBUS_EXCEPTION_ILLEGALFUNCTION 0x01

//Illegal Data Address
// The data address received in the query is not an allowable address for the slave.
// More specifically, the combination of reference number and transfer length is invalid. For a controller with 100 registers, a request with offset 96 and length 4 would succeed, a request with offset 96 and length 5 will generate exception 02.
#define PROTO_MODBUS_EXCEPTION_ILLEGALADDRESS 0x02


//Illegal Data Value
// 	A value contained in the query data field is not an allowable value for the slave.  
//This indicates a fault in the structure of remainder of a complex request, such as that the implied length is incorrect. It specifically does NOT mean that a data item submitted for storage in a register has a value outside the expectation of the application program, since the MODBUS protocol is unaware of the significance of any particular value of any particular register.
#define PROTO_MODBUS_EXCEPTION_ILLEGALDATAVALUE 0x03

//Slave Device Failure
//	An unrecoverable error occurred while the slave was attempting to perform the requested action.
#define PROTO_MODBUS_EXCEPTION_SLAVEDEVICEFAILURE 0x04

//Acknowledge 	Specialized use in conjunction with programming commands.
//   The slave has accepted the request and is processing it, but a long duration of time will be required to do so.  This response is returned to prevent a timeout error from occurring in the master. The master can next issue a Poll Program Complete message to determine if processing is completed.
#define PROTO_MODBUS_EXCEPTION_ACKNOWLEDGE 0x05

//Slave Device Busy 	Specialized use in conjunction with programming commands.
//   The slave is engaged in processing a long-duration program command.  The master should retransmit the message later when the slave is free..
#define PROTO_MODBUS_EXCEPTION_SLAVEBUSY 0x06

//Negative Acknowledge
// 	The slave cannot perform the program function received in the query. This code is returned for an unsuccessful programming request using function code 13 or 14 decimal. The master should request diagnostic or error information from the slave.
#define PROTO_MODBUS_EXCEPTION_NEGATIVEACKNOWLEDGE 0x07

//Memory Parity Error
// 	Specialized use in conjunction with function codes 20 and 21 and reference type 6, to indicate that the extended file area failed to pass a consistency check.
//The slave attempted to read extended memory or record file, but detected a parity error in memory. The master can retry the request, but service may be required on the slave device.
#define PROTO_MODBUS_EXCEPTION_MEMORYPARITYERROR 0x08

//Gateway Path Unavailable
// 	Specialized use in conjunction with gateways, indicates that the gateway was unable to allocate an internal communication path from the input port to the output port for processing the request. Usually means the gateway is misconfigured or overloaded.
#define PROTO_MODBUS_EXCEPTION_GATEWAYPATHUNAVALIABLE 0x0A

//Gateway Target Device Failed to Respond
// 	Specialized use in conjunction with gateways, indicates that no response was obtained from the target device. Usually means that the device is not present on the network.
#define PROTO_MODBUS_EXCEPTION_GATEWAYTARGETFAILEDTORESPONSE 0x0B

//valor da mensagem recebida
#ifndef PROTO_C
extern
#endif
unsigned char proto_ucMsgRecebida;

//tamanho da area de parametros
#ifndef PROTO_C
extern
#endif
unsigned char proto_ucTamanhoDados;


//define tamanho do buffer
#define PROTO_TAMANHO_DADOS 30

//define o hovehead, o useja, quantos bytes do apcote de envio sao usados para o pacote
#define PROTO_TAMANHO_HOVERHEAD_MODBUS_ASCII 9

//BUFFER PARA ENVI ODE DADOS PARAO HOST (tamanho dos dados ) + ovehead(no caso do modbis ascii)
#define PROTO_TAMANHO_DADOS_ENVIO (100 + PROTO_TAMANHO_HOVERHEAD_MODBUS_ASCII)   

//define a qtd de bytes (jah an repre binaria), que podem ser enc. para envio
#define PROTO_MAX_DADOSBIN_MODBUS_ASCII ((PROTO_TAMANHO_DADOS_ENVIO - PROTO_TAMANHO_HOVERHEAD_MODBUS_ASCII)/2) 

//buffer de dados recebidos junto com os pacotes
#ifndef PROTO_C
extern
#endif
unsigned char proto_ucBufferDados[PROTO_TAMANHO_DADOS];

//buffer para envio pela serial (byte a byte)
#ifndef PROTO_C
extern
#endif
unsigned char proto_ucBufferEnvio[PROTO_TAMANHO_DADOS_ENVIO];


//notifica o modulo do byte recebido
#ifndef PROTO_C                                            
extern
#endif
void proto_notificaByteRecebido(unsigned char mucByteRecebido);

//notifica o modulo do byte transmitido
#ifndef PROTO_C
extern
#endif
void proto_notificaByteTransmitido(void);

//permite saber se tem bytes a enviar ainda, a fi dme saber
//se deve notificar bytes. Retorna true ou false
#ifndef PROTO_C
extern
#endif
unsigned char proto_getExistemBytesPedentesTransmissao(void);


//retorna o tipo
#ifndef PROTO_C
extern
#endif
unsigned char proto_ucTipo(void);


//Envia pacote com mensagem para o host
#ifndef PROTO_C
extern
#endif
void proto_enviaPacote(unsigned char mucMensagem, unsigned char *mpucTextoDados);

                            

//inicializa modulo
#ifndef PROTO_C
extern
#endif
//recebe por primeiro parametro o protocolo a ser usado PROTO_PI2009 ou PROTO_MODBUS_ASCII 
void proto_iniciar(unsigned char mucTipo, unsigned char mucEndereco, 
  unsigned char mucQtdBytesInicioPacote, TBaseEventos *mpbeBaseEventos);

//retorna exception modbus
#ifndef PROTO_C
extern
#endif
//recebe por primeiro parametro o protocolo a ser usado PROTO_PI2009 ou PROTO_MODBUS_ASCII
//espera no segund ocampo um dos defines PROTO_MODBUS_EXCEPTION_...  
void proto_enviaExceptionModbus(unsigned char mucCmdOrig, unsigned char mucCodException);


//set status de ocupado
#ifndef PROTO_C
extern
#endif
//set status de ocupado (se true, no caso de modbus passa a retornar uma resp. de excessão)  
void proto_setOcupado(unsigned char mucOcupado);


#endif

