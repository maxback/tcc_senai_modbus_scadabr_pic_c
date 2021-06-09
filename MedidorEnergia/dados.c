/***
**** Arquivo de implementação do módulo de armazenamento de dados
***/
#include <p18cxxx.h>	// inclui as definições dos ios do pic

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

//definir para imprimir dados de 
//#define IMPRIME_DEBUG


#define DADOS_C

#include "..\lib\base\base.h"
#include "m25pxx.h"
#include "dados.h"
#include "global.h"
#include "..\lib\kitmcu\tipos.h"
#include "..\lib\kitmcu\utils.h"
#include "..\lib\kitmcu\lcd.h"

//indica o modo de trabalho entre escrita e leitura
unsigned char gucModoAcesso;

//buffer teste
char c[40];

//endereco atual
unsigned long int gulUltimoEndereco = 0;

//////////////////////////////////// funcoes privadas  ///////////////////////////////////

//anda a memoria de registro em registro at'e achar mem'oria livre para inserir o proximo registros
//retorna true se achou ou 0 se memoria cheia
unsigned char bProcuraMemoriaLivre(void)
{
	unsigned char i;
	unsigned char bEncontrouMemoriaLivre;
#ifdef IMPRIME_DEBUG
	unsigned char j = 0;
#endif
	//seta falso
	bEncontrouMemoriaLivre = 0;
	//zera total de bytes na memoria (acumulado entre os CIS de memoria)
	dados_ulTamanhoBufferTotal = 0;


	//percorre todos os CIs de memoria
	for(i=0; (i < M25PXX_QUANTIDADE_CI) && (!bEncontrouMemoriaLivre); i++)
	{
	
		//seleciona o indice do CI
		m25pxx_seleciona(i);
		//usa variavel de endereco do modulo zerada, iniciando no endereco 0
		gulUltimoEndereco = 0;

		
		do
		{
#ifdef IMPRIME_DEBUG
			
			//print do endereco atual
			if(!(j % 32))
			{	
			sprintf(c, "\f[%d]", (unsigned int)gulUltimoEndereco);
			lcd_puts(c);
			}
#endif
			m25pxx_leDadosEndereco3Bytes(gulUltimoEndereco, (unsigned char *)&dados_Reg, 1);
			//dados_Reg.ucTipo =0x55;

#ifdef IMPRIME_DEBUG
			if(!(j % 32))
			{	
			lcd_puts_const(":");
			//se eh tipo conhecido de registros ve quantos bytes pular (já andou um byte devido a leitura)
			//se na ofor um tipo conhecido nada anda

			//imprime o que leu - debug
			sprintf(c, "%0.2x:", dados_Reg.ucTipo);
			lcd_puts(c);
			}
#endif

			switch(dados_Reg.ucTipo)
			{
				//se esta livre, encontrou posicao de escrita
				case TR_SEM_DADOS:
					//seta que achou livre parea sair
					bEncontrouMemoriaLivre = 1;
#ifdef IMPRIME_DEBUG
					lcd_puts_const("-");
#endif
				break;
				// Registro de inicio de um periodo de captura dos AD - Deve conter data e hora do inicio das capturas
				case TR_CABECALHO_CAPTURA:
				//Registro de fechamento de uma captura com data e hora e quantidade de registros da captura (2 bytes)
				case TR_FECHAMENTO_CAPTURA:
				case TR_REGISTRO_MEDICAO:
					//auemnta o tamanho do dado do tipo 
					gulUltimoEndereco += TAMANHO_REG_DADOS_COMPLETO;

					//atualiza tamanho
					dados_ulTamanhoBufferTotal += TAMANHO_REG_DADOS_COMPLETO;
#ifdef IMPRIME_DEBUG
					lcd_puts_const("&");
#endif
				break;

				// Registro de uam captura. Contem os valotes dos 3 ADs
				case TR_REGISTRO_CAPTURA:
					//auemnta o tamanho do dado do tipo, jah descontando 
					gulUltimoEndereco += TAMANHO_REG_DADOS_CAPTURA;

					//atualiza tamanho
					dados_ulTamanhoBufferTotal += TAMANHO_REG_DADOS_CAPTURA;
#ifdef IMPRIME_DEBUG
					lcd_puts_const("*");
#endif
				break;


				//FLAG DE FIM DE DADOS NO CI, usado apra sinalizar que terminou no CI por não ter espaco exato parao registros
				case TR_SEM_DADOS_NO_CI:
#ifdef IMPRIME_DEBUG
				  //nada faz aqui	
				  lcd_puts_const("<");
#endif
				break;

				//nenhum dos bytes previstos, vai apra o proximo
				default:
#ifdef IMPRIME_DEBUG
					if(!(j % 32))
					{	

					lcd_puts_const("d");
					}
#endif
					//anda um endereco para a frente
					//auemnta o tamanho do dado do tipo, jah descontando 
					gulUltimoEndereco += 1;

					//atualiza tamanho
					dados_ulTamanhoBufferTotal += 1;

				break;
			}

#ifdef IMPRIME_DEBUG
			j++;
#endif
		} 
		//enquanto tiver um tipo no byte de tipo e ainda nao chegou no final da memoria
		while((!bEncontrouMemoriaLivre) && (gulUltimoEndereco < m25pxx_gulTamanhoMemoriaAtual()) && (dados_Reg.ucTipo != TR_SEM_DADOS_NO_CI)); 

	}

	//retorna o flag indicando se encontrou memória livre
	return bEncontrouMemoriaLivre;
}


//calcula a partir do endereco e CI atual se tem memoria livre ainda
unsigned char bChegouNoFinalDaMemoria(void)
{
	if(m25pxx_ucSelecionado() < M25PXX_QUANTIDADE_CI)
	{
		//ok, nem esta no ultimo CI
		return 0;
	}
	else
	{
		//esta no ultimo CI
		//tem espaco para mais um registro?
		if( (gulUltimoEndereco+TAMANHO_REG_DADOS_COMPLETO) < m25pxx_gulTamanhoMemoriaAtual())
		{
			//tem espaco no CI ainda
			return 0;
		}
		//encheu toda a memoria
		else
		{
			return 1;
		}
	}

}

//////////////////////////////////// Funções do módulo ///////////////////////////////////

// Inicialzia o módulo
void dados_iniciar(TBaseEventos *mpbeBaseEventos)
{
//	unsigned char i, j, iCI;
//	unsigned char ucBuffer[100];

	//inicializa modulo de memoria
	m25pxx_iniciar(mpbeBaseEventos);
	

	//Seta modo como escrita escrita e leitura
	dados_alteraModo(DADOS_MODO_ESCRITA);
/*********** Teste da memoria ***********************


	lcd_puts_const("\fmodo_ok...");

	//teste escrita e posteorior leitura
	for(iCI=0; iCI < 2; iCI++)
	{
	    m25pxx_seleciona(iCI);

		lcd_puts_const("\fApagando...");
		
		m25pxx_apagaMemoria(APAGA_TUDO);


		delay_ms(5000);

		gulUltimoEndereco = 0;

		for(j=0; j < 30; j++)
		{
		
			//escreve
			for(i=0; i < sizeof(ucBuffer); i++)
			{
				ucBuffer[i] = i;
			}
	
			
				//            0123456789abcdef
				sprintf(c, "\fEscrevendo: %d", (unsigned int)gulUltimoEndereco);
				lcd_puts(c);
				m25pxx_escreveDadosEndereco3Bytes(gulUltimoEndereco, ucBuffer, sizeof(ucBuffer));
	//		}
		
			//le
	
			lcd_puts_const("\fLendo...Ok");
	
			m25pxx_leDadosEndereco3Bytes(gulUltimoEndereco, ucBuffer, sizeof(ucBuffer));

	
			for(i=0; i < sizeof(ucBuffer); i++)
			{
				//se diferentes, mostra msg e trva
				if(i != ucBuffer[i])
				{
					//            0123456789abcdef  0123456789abcdef
					sprintf(c, "\fErro comparando \render.: %d.", (unsigned int)gulUltimoEndereco);
					lcd_puts(c);
					delay_ms(1000);
				}
			}

			//anda 
			gulUltimoEndereco += sizeof(ucBuffer); 

		}
	}
*/
}

///////  Funções de operação sobre registros ////////////


// Escreve na próxima posicao os dados armazenados na estrutura passada por parametro
// retorna falso se não existe mais memória 
unsigned char  dados_ucGravarRegistro(TDadosRegistro *mpdrRegistro)
{
	//tamanho a ser gravado
	unsigned int uiTam;
	//um byte
	unsigned char ucFlagSemDAdosNoCI = TR_SEM_DADOS_NO_CI;
	//flag que indica sucesso
	unsigned char ucFlagSucesso;

	//1 se gravou registro
	ucFlagSucesso = 0;
	
	//se tem memoria, aceita coamndo
	if(dados_ucTemMemoriaLivre)
	{
		//se tipo for registros de captura eh um tamanho
		if(mpdrRegistro->ucTipo == TR_REGISTRO_CAPTURA)
		{
			uiTam = TAMANHO_REG_DADOS_CAPTURA;
		}
		//senao eh outro (para os outros tipos)
		else
		{
			uiTam = TAMANHO_REG_DADOS_COMPLETO;
		}
		

		//se nao cabe mais no CI atual
		if((gulUltimoEndereco+uiTam) > m25pxx_gulTamanhoMemoriaAtual())
		{
			//no entanto se sobraram alguns bytes grava flag de que paro ude usar memoria no CI
			if(gulUltimoEndereco < m25pxx_gulTamanhoMemoriaAtual())
			{
				//grava apenas o byte com o flag
				m25pxx_escreveDadosEndereco3Bytes(gulUltimoEndereco, &ucFlagSemDAdosNoCI, 1);
			}


			//eh o ultimo CI, indica sem memoria?
			if(m25pxx_ucSelecionado() == M25PXX_QUANTIDADE_CI)
			{
				//acabou a memória
				dados_ucTemMemoriaLivre = 0;
			}
			//senao muda de CI, indo para o primeiro endereco
			else
			{
				//px CI
				m25pxx_seleciona(m25pxx_ucSelecionado()+1);
				//endereco inicial
				gulUltimoEndereco = 0;
			}
		}

		//ainda tem memoria?
		if(dados_ucTemMemoriaLivre)
		{
			//manda gravar
			m25pxx_escreveDadosEndereco3Bytes(gulUltimoEndereco, (unsigned char *)mpdrRegistro, uiTam);
			//anda
			gulUltimoEndereco += uiTam;

			//incrementa variavel que totaliza a qtd de bytes co mdados
			dados_ulTamanhoBufferTotal += uiTam;

			//ve se sobrou espaco apos gravar
			dados_ucTemMemoriaLivre = !bChegouNoFinalDaMemoria();

			//indica sucesso
			ucFlagSucesso = 1;

		}


	}

	//retorna resultado
	return ucFlagSucesso;
	
}




// Le da memória para a variavel dados_Reg e retorna a quantidade de bytes a partir do nicio da estrutura ou 0, se jah nao tem dados
//apos ler os dados anda para o proximo registro.
unsigned char dados_ucLerRegistro(void)
{
	//tamanhoa a ser lido, se 0 indica que nao tem mais registros a ler
	unsigned int uiTam;
	//flag de tipo do registros
	unsigned char ucFlagTipoReg;

	//retorna 0 por default
	uiTam = 0;
	 
	do
	{

		//le o primeiro byte
		m25pxx_leDadosEndereco3Bytes(gulUltimoEndereco, &ucFlagTipoReg, 1);
	
		//verifica de acordo com o tipo lido o que fazer
		switch(ucFlagTipoReg)
		{
			//sem dados, area livre
			case TR_SEM_DADOS:
				//retorna false, indicando que n~ao tem mais dados
				return 0;
			break;
			// Registro de inicio de um periodo de captura dos AD - Deve conter data e hora do inicio das capturas
			case TR_CABECALHO_CAPTURA:
			//Registro de fechamento de uma captura com data e hora e quantidade de registros da captura (2 bytes)
			case TR_FECHAMENTO_CAPTURA:
			case TR_REGISTRO_MEDICAO:
				//seta tamanho 
				uiTam = TAMANHO_REG_DADOS_COMPLETO;
			break;
	
			// Registro de uam captura. Contem os valotes dos 3 ADs
			case TR_REGISTRO_CAPTURA:
				//seta tamanho
				uiTam = TAMANHO_REG_DADOS_CAPTURA;
			break;
	
			//Acabou so dados no CI atual, ve se tem mais dados alem deste ponto e pula apra o proximo CI se for o caso
			case TR_SEM_DADOS_NO_CI:
				//se esta no ultimo CI j'a varreu toda a memoria
				if(m25pxx_ucSelecionado() >= M25PXX_QUANTIDADE_CI)
				{
					//retorna false, indicando que chegou no final
					return 0;
				}
				else
				{
					//muda de CI, zera endereco e ve se tem dados
					m25pxx_seleciona(m25pxx_ucSelecionado()+1);
					//zera endereco
					gulUltimoEndereco = 0;
	
				}
				
			break;
		}
	//executa denovo a pesquisa se mudou de CI
	} while(ucFlagTipoReg == TR_SEM_DADOS_NO_CI);

	//se setou tamanho indica que tem que ler o registros
	if(uiTam)
	{
		//le o registros na estrutura publica
		m25pxx_leDadosEndereco3Bytes(gulUltimoEndereco, (unsigned char *)&dados_Reg, uiTam);
		//anda
		gulUltimoEndereco += uiTam;


	}

	//retorna o tamanho lido
	return uiTam;

}



// Retorna a quantidade de memória com dados guardados, em registros

unsigned long int dados_ulTamanhoRegistros(void)
{


}



///////  Funções de operação sobre a memória diretamente - Apenas para leitura ////////////



// Le da memória para a o buffer passado por ponteiro em mpucBuffer até mucTamanho bytes
// Se o parametro mucProximo for verdadeiro além de ler anda para frente para que na proxima leitura pegue outros bytes
// retorna o tamanaho lido ou 0 se já leu todos e avançou além.

unsigned char dados_ucLerBuffer(unsigned char *mpucBuffer, unsigned char mucTamanho, unsigned char mucProximo)
{


}






/////////////////////// Funçoes gerais ////////////////////////////////
// Apaga a memória, limpando tdos os registros e reiniciando os ponteiros
void dados_Limpar(void)
{
	unsigned char i;

	//percorre todos os CIs de memoria
	for(i=0; i < M25PXX_QUANTIDADE_CI; i++)
	{
	
		//seleciona o indice do CI
		m25pxx_seleciona(i);

		//manda apagar
		m25pxx_apagaMemoria(APAGA_TUDO);
	}

	//posiciona no primeiro CI
	m25pxx_seleciona(0);

	//posiciona no primeiro endereco
	gulUltimoEndereco = 0;
}


//Seta modo entre escrita e leitura
//se Escrita (DADOS_MODO_ESCRITA): posiciona na proxima area livre para escrita
//Se leitura (DADOS_MODO_LEITURA): posiciona no inicio da memoria para que possa ler dados e enviar a algum lugar, ou apenas inspecion'a-los
//alterar entre modos antes de iniciar escrita ou leitura. Inicia em modo ecrita
//alterar entre modos antes de iniciar escrita ou leitura. Inicia em modo ecrita
void dados_alteraModo(unsigned char mucModo)
{
	//salva o modo
	gucModoAcesso = mucModo;
	//se escrita acha ponto de insert livre
	if(gucModoAcesso == DADOS_MODO_ESCRITA)
	{
		//localiza proximo ponto de gravacao

		dados_ucTemMemoriaLivre = bProcuraMemoriaLivre();
		lcd_puts_const("z");
	}
	//senao poe endereco no inicio do primeiro CI
	else
	{
		//primeiro CI
		m25pxx_seleciona(SEL_CI0);
		//endereco inicial
		gulUltimoEndereco = 0;
	}

}

//retorna modo atual
unsigned char dados_ucRetornaModo(void)
{
	return gucModoAcesso;
}


