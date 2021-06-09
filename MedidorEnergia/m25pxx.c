/**
*** Módulo que implementa o acesso aos CIS de memoria flash modelp M25P05-A
***
**/
#define FLASH_M25PXX_C

#include <p18cxxx.h>	// inclui as definições dos ios do pic
// para temporizações
#include <delays.h>
#include "..\lib\base\base.h"
#include "..\lib\kitmcu\tipos.h"
#include "..\lib\kitmcu\utils.h"
#include "..\lib\kitmcu\lcd.h"

#include "global.h"
#include "m25pxx.h"

//Instrucoes
#define WREN 0x06
#define WRDI 0x04
#define RDID 0x9F
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define FAST_READ 0x0B
#define PP 0x02
#define SE 0xD8
#define BE 0xC7
#define DP 0xB9
#define RES 0xAB


//Mascara para os bits do byte de status
//Definida estrutura para atraves de um union pegar os bits
typedef union
{
	unsigned char Byte;
	struct
	{
		//Table 6. Status Register format
		//b7 a b0
		unsigned char SRWD:1;
		unsigned char ReserverdB6:1;
		unsigned char ReserverdB5:1;
		unsigned char ReserverdB4:1;
		unsigned char BP1:1;
		unsigned char BP0:1;
		unsigned char WEL:1;
		unsigned char WIP:1;
	} Bits;
} TStatusRegister;

#define MASCARA_SRWD 0X80
//		unsigned char ReserverdB6:1;
//		unsigned char ReserverdB5:1;
//		unsigned char ReserverdB4:1;
#define MASCARA_BP1 0x08
#define MASCARA_BP0 0x04
#define MASCARA_WEL 0x02
#define MASCARA_WIP 0x01


//indica o ci selecionado pelo usuario do modulo
unsigned char gucIndiceCIAtual = 0;

//pinos de hardware para a comunicação e bite de configuracao de tristate
#define DATA_OUT 	PORTBbits.RB1
#define DATA_OUT_TRIS 	TRISBbits.TRISB1
#define DATA_IN 	PORTBbits.RB0
#define DATA_IN_TRIS 	TRISBbits.TRISB0
#define CLOCK 		PORTBbits.RB2
#define CLOCK_TRIS 	TRISBbits.TRISB2
#define SELECT_0 	PORTBbits.RB5
#define SELECT_0_TRIS 	TRISBbits.TRISB5
#define SELECT_1	PORTBbits.RB6
#define SELECT_1_TRIS 	TRISBbits.TRISB6


//macro para delay entre bits
// ** tempo em que foi homologado pela ultiam vez, mas é muito lento #define TEMPO_ESPERA 60000
//#define TEMPO_ESPERA 500
#define TEMPO_ESPERA 1

//ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
static TBaseEventos *gpbeBaseEventos = NULL;



//////////////////////// funcoes basicas de comunicação //////////////////////////

//se 0xff deseleciona todos, senao seleciona o 0 ou o 1
//#define SEL_CI0 0
//#define SEL_CI1 1
//#define SEL_NENHUM 0xff
void seleciona(unsigned char mucIndice)
{
	//desabilita todos os 2
	SELECT_0 = 1;
	SELECT_1 = 1;
	//habilita o passado, se for 0 ou 1
	switch(mucIndice)
	{
		case SEL_CI0:	SELECT_0 = 0; break;
		case SEL_CI1:	SELECT_1 = 0; break;
	}
}




//escreve um byte - A Memória le sua entrada de dados quando receber uam borda de SUBIDA do clock
//considera estado de repouso do click em 1
void mandaByte(unsigned char mucDado)
{
	unsigned char i;

	//coloca cada bit, a partir do mais significativo do dado, espera e da um puso de subida
	for(i=8; i; i--)
	{
		//desce o click
		CLOCK = 0;
		//poe dado na saida (bit MSB)
		if(mucDado & 0x80)
		{
			//por valor 1 na saida
			DATA_OUT = 1;
		}
		//senao eh zero
		else
		{
			DATA_OUT = 0;
		}
		//espera 
		Delay10TCYx(TEMPO_ESPERA);
		//sobe clock
		CLOCK = 1;
		//espera em 1 o mesmo tempo para flash pegar bit
		Delay10TCYx(TEMPO_ESPERA);
		//por fim desloca para a direita um bit apra descartar o já enviado
		mucDado <<= 1;
	}
	
	//a saida de dados no final fica em 1
	DATA_OUT = 1;
	
}

//le um byte - A Memória poe o bit de dados em seu pino de saida de dados quando receber uma borda de DESCIDA do clock
//considera estado de repouso do click em 1
unsigned char ucRecebeByte(void)
{
	unsigned char i;
	unsigned char ucDado;

	//dados zerado
	ucDado = 0;

	//debug
	//TRISAbits.TRISA5 = 0;
	
	//coloca cada bit, a partir do mais significativo do dado, espera e da um puso de subida
	for(i=8; i; i--)
	{
		//desloca para a direita um bit livre para receber proximo
		ucDado <<= 1;
		//baixa o clock e espera um pouco
		CLOCK = 0;
		//PORTAbits.RA5 = 0;
		//espera 
		Delay10TCYx(TEMPO_ESPERA);
		//le dado colcoado para saida pela memória
		if(DATA_IN)
		{
			ucDado |= 0x01;
			//debug
			//PORTAbits.RA5 = 1;
		}
		else
		{
			ucDado &= 0xfe;
  			//debug
			//PORTAbits.RA5 = 0;
		}

		//zero j'a esta no bit devido ao deslocamento de 1, logo nao precisa se atribuido se lido 0

		//sobe clock
		CLOCK = 1;
		//PORTAbits.RA5 = 1;
		//espera em 1 o mesmo tempo para flash pegar bit
		Delay10TCYx(TEMPO_ESPERA);
	}

	//retorna dado
	return ucDado;
}


//////////////////////// funcoes de acesso a memória  //////////////////////////
//////////////////////// Acesso as instrucoes da mem. //////////////////////////

// habilita escrita
void habilitaEscrita(void)
{
	//seleciona CI atual
	seleciona(gucIndiceCIAtual);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//manda comando
	mandaByte(WREN);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//deseleciona todos
	seleciona(SEL_NENHUM);

}

//desablita escrita
void desabilitaEscrita(void)
{
	//seleciona CI atual
	seleciona(gucIndiceCIAtual);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//manda comando
	mandaByte(WRDI);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//deseleciona todos
	seleciona(SEL_NENHUM);
}



//le o status register e retorna dado
TStatusRegister leStatus(void)
{
	TStatusRegister Status;

	//seleciona CI atual
	seleciona(gucIndiceCIAtual);
	//manda comando
	mandaByte(RDSR);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//le o byte de status agora e retorna
	Status.Byte = ucRecebeByte();
	//deseleciona todos
	seleciona(SEL_NENHUM);
	
	//retorna valor lido	
	return Status;

}



//escreve status register (altera apenas os bits b6, b5, b4, b1 e b0) Precisa que a escrita seja habilitada
//Ou seja, altera os bits de controle:
//                                    b7 (SRWD) - Status Register Write Protect
//                                    b3 (BP1) -\__>  Block Protect bits
//                                    b2 (BP0) -/
// e apos enviado que o select do chip seja colcoado desabiltido apra que execute (esta funcao não faz isto).
void escreveStatus(unsigned char mucStatus)
{
	//seleciona CI atual
	seleciona(gucIndiceCIAtual);
	//manda comando
	mandaByte(WRSR);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//escreve o status 
	mandaByte(mucStatus);
	//deseleciona todos
	seleciona(SEL_NENHUM);
}


//fica na funcao ate parar a operacao de escrita atual
void esperaTerminarEscrita(void)
{
	TStatusRegister Status;

	//espera terminar operação de escrita anterior
	do
	{
		//le status
		Status = leStatus();
		
	//} while(Status.Bits.WIP);
	} while((Status.Byte&MASCARA_WIP) == MASCARA_WIP);

}

///////////////////////////////////// funcoes do modulo ///////////////////////////////

//para iniciar o modulo -> Inicia com o primeiro CI e com m25pxx_ulUltimoEndereco = 0
void m25pxx_iniciar(TBaseEventos *mpbeBaseEventos)
{

  //ponteiro para estrutura com ponteiros para funcoes que manipulam eventos
  gpbeBaseEventos = mpbeBaseEventos;


	//configura saidas
	DATA_OUT_TRIS = 0;
	CLOCK_TRIS 	  = 0;
	SELECT_0_TRIS = 0;
	SELECT_1_TRIS = 0;

	//entrada
	DATA_IN_TRIS = 1;
  
	//ultimo endereco acessado (usado tambme para calcular endereco quando passado apra fncoes com enbdereco de 2 ou 1 byte
	//eh usado tambem para situar o endereco dentro do setor a ser apagado antes de chamar m25pxx_apagaMemoria()
	m25pxx_ulUltimoEndereco = 0;

	//da memória para endereco de 8 e 16 bits
	m25pxx_uiEndereco2BytesMSB = 0;

	//variavel que indica os 3 bytes mais significativos de endereco dos 3 e que é usado para as funcoes de escrita e leitura
	//indica que esta em modo de leitura rapida se true
	m25pxx_gucModoLeituraRapida = 0;

	//indica o ci selecionado pelo usuario do modulo
	gucIndiceCIAtual = 0;

}




//para indicar o CI que esta trabalhando: 0 ou 1
void m25pxx_seleciona(unsigned char mucIndice)
{
	//indica o ci selecionado pelo usuario do modulo
	gucIndiceCIAtual = mucIndice;
}


//para indicar o CI que esta trabalhando: 0 ou 1
unsigned char m25pxx_ucSelecionado(void)
{
	return gucIndiceCIAtual;
}

//Le Identificação da memoria
//recebe tre ponteiros para os bytes
//pucIDFabricante 	-> 	Manufacturer identification
//pucTipoMemoria	->	Device identification	->	Memory type 
//pucCapacidade		->	Device identification	->	Memory capacity
//
// Exceto o primeiro ponteiro(pucIDFabricante) os demais ponteiros podem ser passados nulos se a informação não interessar
void m25pxx_leIdentificacao(unsigned char *pucIDFabricante, unsigned char *pucTipoMemoria, unsigned char *pucCapacidade)
{
	//seleciona CI atual
	seleciona(gucIndiceCIAtual);
	//manda comando
	mandaByte(RDID);
	//pequena espera extra
	Delay10TCYx(TEMPO_ESPERA);
	//le o byte de ID do fabricante
	*pucIDFabricante = ucRecebeByte();	

	//se pelo menos um dos outros pontiros tiver dados
	if(pucTipoMemoria || pucCapacidade)
	{
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
		
		//deve ter o tipo de meoria
		if(pucTipoMemoria)
		{
			//le info
			*pucTipoMemoria = ucRecebeByte();	
		}
		//senao le sem armazenar, apenas para andar na leitura e pegar o proximo byte
		else
		{
			ucRecebeByte();
		}	

		//se deve ler ultimo byte
		if(pucCapacidade)
		{
			//pequena espera extra
			Delay10TCYx(TEMPO_ESPERA);
			//le info
			*pucCapacidade = ucRecebeByte();
		}
	}
	//deseleciona todos
	seleciona(SEL_NENHUM);
}


//le da memória no endereco de 3 bytes passado
void m25pxx_leDadosEndereco3Bytes(unsigned long int mulEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	//indice do byte na pagina atual
	unsigned int uiIndiceNaPagina;
	

	//preve mais de uma iteração pois pode passar do tamanho da pagina, testa se tem tamanho ainda
	while(muiTamanho)
	{
		//calcula o indice do byte na pagina atual
		if(mulEndereco)
		{
			uiIndiceNaPagina = mulEndereco % m25pxx_guiTamanhoPagina();
		}
		else
		{
			uiIndiceNaPagina = 0;
		}
	

		//copia valor do endereco
		m25pxx_ulUltimoEndereco = mulEndereco;
	
		//espera eventual operacaode escrita
		esperaTerminarEscrita();
	
		//seleciona CI atual
		seleciona(gucIndiceCIAtual);
	
		//manda comando difernete se em leitura rapida
		if(m25pxx_gucModoLeituraRapida)
		{
			mandaByte(FAST_READ);
		}
		//senao eh modo normal
		else
		{
			mandaByte(READ);
		}
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
	
		//manda os 3 bytes do endereço
		//do terceiro para o primeiro
		mandaByte(mulEndereco>>16);
		mandaByte(mulEndereco>>8);
		mandaByte(mulEndereco);
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
	
		//se for fast read tem um byte de dummy que precisa ser enviado
		if(m25pxx_gucModoLeituraRapida)
		{
			//dummy
			mandaByte(0x00);
		}	
	
		//para os n bytes solicitados vai lendo e colocando no buffer
		while(muiTamanho)
		{
			*mpucBuffer = ucRecebeByte();
			//apenta para proximo endereco de memoria
			mpucBuffer++;
			//decrementa contador
			muiTamanho--;
			//se vai ler ainda, muda endereco para o proximo
			if(muiTamanho)
			{
				//inc valor do endereco
				mulEndereco++;

				//inc. indice na pagina
				uiIndiceNaPagina++;
				//se passou do maximo, termina loop local ainda com valor em muiTamanho > 0
				if(uiIndiceNaPagina >= m25pxx_guiTamanhoPagina())
				{
					break;
				}
			}
		}	
		//deseleciona todos
		seleciona(SEL_NENHUM);

		//atualzia ultimo endereco lido
		m25pxx_ulUltimoEndereco = mulEndereco;
	}
}



//le da memória no endereco de 3 bytes passado utilizando os dois bytes passados em muiEndereco com os menos sig. dos 3
// e o byte menos sig. de m25pxx_uiEndereco2BytesMSB para completar como o terceiro ais sig. do endereco de 3 bytes
void m25pxx_leDadosEndereco2Bytes(unsigned int muiEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	//calcula valor do endereco

	//pega apenas menos sig de gucEndereco2BytesMSB
	m25pxx_ulUltimoEndereco = m25pxx_uiEndereco2BytesMSB&0x00ff;
	//e desloca para se tornar o terceiro byte do endereco
	m25pxx_ulUltimoEndereco <<= 16;
	//agora faz um or com o endereco de 2 bytes passado	
	m25pxx_ulUltimoEndereco |= muiEndereco;

	//chama a outra funcao adaptand oendereco
	m25pxx_leDadosEndereco3Bytes(m25pxx_ulUltimoEndereco , mpucBuffer, muiTamanho);
}


//le da memória no endereco de 3 bytes passado utilizando o byte passado em mucEndereco com o menos sig. dos 3
// e os 2 bytes de m25pxx_uiEndereco2BytesMSB para completar como o terceiro e o segundo mais sig. do endereco de 3 bytes
void m25pxx_leDadosEndereco1Bytes(unsigned char mucEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	//calcula valor do endereco

	//pega apenas os 2 bytes de gucEndereco2BytesMSB
	m25pxx_ulUltimoEndereco = m25pxx_uiEndereco2BytesMSB;
	//e desloca para se tornar o terceiro e o segundo byte do endereco
	m25pxx_ulUltimoEndereco <<= 8;
	//agora faz um or com o endereco de 1 byte passado	
	m25pxx_ulUltimoEndereco |= mucEndereco;

	//chama a outra funcao adaptand oendereco
	m25pxx_leDadosEndereco3Bytes(m25pxx_ulUltimoEndereco , mpucBuffer, muiTamanho);
}



//escreve na memória no endereco de 3 bytes passado
void m25pxx_escreveDadosEndereco3Bytes(unsigned long int mulEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	TStatusRegister Status;
	//indice do byte na pagina atual
	unsigned int uiIndiceNaPagina;
	

	//preve mais de uma iteração pois pode passar do tamanho da pagina, testa se tem tamanho ainda
	while(muiTamanho)
	{
		//calcula o indice do byte na pagina atual
		if(mulEndereco)
		{
			uiIndiceNaPagina = mulEndereco % m25pxx_guiTamanhoPagina();
		}
		else
		{
			uiIndiceNaPagina = 0;
		}
	
		//espera eventual operacaode escrita
		esperaTerminarEscrita();
		
	
		//manda comando para habilitar escrita
		habilitaEscrita();
	
		//espera setar bit indicando que esta habilitado
		do
		{
			//le status
			Status = leStatus();

			
			
		//} while(!Status.Bits.WEL);
		} while((Status.Byte&MASCARA_WEL) != MASCARA_WEL);
	
		//copia valor do endereco
		m25pxx_ulUltimoEndereco = mulEndereco;
	
		//seleciona CI atual
		seleciona(gucIndiceCIAtual);
	
		//manda comando de escrita
		mandaByte(PP);
	
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
	
	
		//manda os 3 bytes do endereço
		//do terceiro para o primeiro
		mandaByte(mulEndereco>>16);
		mandaByte(mulEndereco>>8);
		mandaByte(mulEndereco);
	
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
	
	
		//para os n bytes solicitados vai mandando a partir do buffer
		while(muiTamanho)
		{
			//manda byte da pos. atual do buffer
			mandaByte(*mpucBuffer);
			//apenta para proximo endereco de memoria
			mpucBuffer++;
			//decrementa contador
			muiTamanho--;
			//se vai ler ainda, muda endereco para o proximo
			if(muiTamanho)
			{
				//inc valor do endereco
				mulEndereco++;
				//inc. indice na pagina
				uiIndiceNaPagina++;
				//se passou do maximo, termina loop local ainda com valor em muiTamanho > 0
				if(uiIndiceNaPagina >= m25pxx_guiTamanhoPagina())
				{
					break;
				}
			}
		}	
		//deseleciona todos
		seleciona(SEL_NENHUM);

		//atualzia ultima endereco
		m25pxx_ulUltimoEndereco = mulEndereco;
	}

}



//escreve na memória no endereco de 3 bytes passado utilizando os dois bytes passados em muiEndereco com os menos sig. dos 3
// e o byte menos sig. de m25pxx_uiEndereco2BytesMSB para completar como o terceiro ais sig. do endereco de 3 bytes
void m25pxx_escreveDadosEndereco2Bytes(unsigned int muiEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	//calcula valor do endereco

	//pega apenas menos sig de gucEndereco2BytesMSB
	m25pxx_ulUltimoEndereco = m25pxx_uiEndereco2BytesMSB&0x00ff;
	//e desloca para se tornar o terceiro byte do endereco
	m25pxx_ulUltimoEndereco <<= 16;
	//agora faz um or com o endereco de 2 bytes passado	
	m25pxx_ulUltimoEndereco |= muiEndereco;

	//chama a outra funcao adaptand oendereco
	m25pxx_escreveDadosEndereco3Bytes(m25pxx_ulUltimoEndereco , mpucBuffer, muiTamanho);
}


//escreve na memória no endereco de 3 bytes passado utilizando o byte passado em mucEndereco com o menos sig. dos 3
// e os 2 bytes de m25pxx_uiEndereco2BytesMSB para completar como o terceiro e o segundo mais sig. do endereco de 3 bytes
void m25pxx_escreveDadosEndereco1Bytes(unsigned char mucEndereco, unsigned char *mpucBuffer, unsigned int muiTamanho)
{
	//calcula valor do endereco

	//pega apenas os 2 bytes de gucEndereco2BytesMSB
	m25pxx_ulUltimoEndereco = m25pxx_uiEndereco2BytesMSB;
	//e desloca para se tornar o terceiro e o segundo byte do endereco
	m25pxx_ulUltimoEndereco <<= 8;
	//agora faz um or com o endereco de 1 byte passado	
	m25pxx_ulUltimoEndereco |= mucEndereco;

	//chama a outra funcao adaptand oendereco
	m25pxx_escreveDadosEndereco3Bytes(m25pxx_ulUltimoEndereco , mpucBuffer, muiTamanho);
}




//apaga o setor ou toda a memoria, se for seto usa o setor apontado pela variavel m25pxx_ulUltimoEndereco
//valores para primeiro parametro(mucModo) de m25pxx_apagaMemoria()
//#define APAGA_SETOR 0
//#define APAGA_TUDO 1
void m25pxx_apagaMemoria(unsigned char mucModo)
{
	TStatusRegister Status;

	//se modo invalido sai
	if((mucModo != APAGA_SETOR) && (mucModo != APAGA_TUDO))
	{
		return;
	}


	//espera eventual operacaode escrita
	esperaTerminarEscrita();
	

	//manda comando para habilitar escrita
	habilitaEscrita();

	//espera setar bit indicando que esta habilitado
	do
	{
		//le status
		Status = leStatus();
		
	//} while(!Status.Bits.WEL);
	} while((Status.Byte&MASCARA_WEL) != MASCARA_WEL);

	//seleciona CI atual
	seleciona(gucIndiceCIAtual);

	//manda comando de apagar

	//setor:
	if(mucModo == APAGA_SETOR)
	{
		mandaByte(SE);

		//manda endereco dentro do setor a ser apagado
		//pequena espera extra
		Delay10TCYx(TEMPO_ESPERA);
	
		//manda os 3 bytes do endereço
		//do terceiro para o primeiro
		mandaByte(m25pxx_ulUltimoEndereco>>16);
		mandaByte(m25pxx_ulUltimoEndereco>>8);
		mandaByte(m25pxx_ulUltimoEndereco);

	}
	//toda a memoria
	else //if(mucModo == APAGA_TUDO) 
	{
		mandaByte(BE);
	}
	

	//deseleciona todos
	seleciona(SEL_NENHUM);

	//fica presa paagando
	esperaTerminarEscrita();
}



//le o status register e retorna dado num buffer como string (tamanho minimo do buffer: 12)
void m25pxx_leStringStatus(char *mpcBuffer)
{
	TStatusRegister Status;

	//so para teste
	habilitaEscrita();

	Status = leStatus();

	//byte e mhex
	mpcBuffer[0] = (Status.Byte >> 4);
	if(mpcBuffer[0] < 0x0a) mpcBuffer[0] += '0'; else mpcBuffer[0] += 'A'; 
	
	mpcBuffer[1] = (Status.Byte & 0x0F);
	if(mpcBuffer[1] < 0x0a) mpcBuffer[1] += '0'; else mpcBuffer[1] += 'A'; 

	mpcBuffer[2] = '>';

	mpcBuffer[3] = Status.Bits.SRWD?'1':'0';
	mpcBuffer[4] = Status.Bits.ReserverdB6?'1':'0';
	mpcBuffer[5] = Status.Bits.ReserverdB5?'1':'0';
	mpcBuffer[6] = Status.Bits.ReserverdB4?'1':'0';

	mpcBuffer[7] = Status.Bits.BP1?'1':'0';
	mpcBuffer[8] = Status.Bits.BP0?'1':'0';
	mpcBuffer[9] = Status.Bits.WEL?'1':'0';
	mpcBuffer[10] = Status.Bits.WIP?'1':'0';

	mpcBuffer[11] = 0;

}


/*
#define DP 0xB9
#define RES 0xAB
*/