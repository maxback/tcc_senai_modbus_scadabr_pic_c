/**
** Módulo de captura da corrente e tensao via canais do AD
**/

#define CAPTURA_C

#include <p18cxxx.h>	// inclui as definições dos ios do pic
// para temporizações
#include <delays.h>
#include <adc.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

//#define USA_MATH 
#ifdef USA_MATH 
#include <math.h>
#endif

#include "..\lib\base\base.h"
#include "..\lib\kitmcu\tipos.h"
#include "..\lib\kitmcu\utils.h"
#include "..\lib\kitmcu\lcd.h"

#include "global.h"
#include "captura.h"
//inclui definicoes do modulo de dados
#ifdef modulo_dados
#include "dados.h"
#endif
// mapea entradas de cada sinal
#define ADC_CORRENTE ADC_CH0
#define ADC_TENSAOPOSITIVA ADC_CH1
#define ADC_TENSAONEGATIVA ADC_CH2

// estados para a maquina de estado
enum
{
    //para modo = CAPTURA_MODO_TESTE

	CAPMAQ_TESTE_REINICIANDO, 	//Apenas testando - inicializa'c~ao de ciclo de testes
	CAPMAQ_TESTE_TESTANDO, 		//Apenas testando - pega valores e mostra no display

    //para modo = CAPTURA_MODO_CAPTURA
			
	CAPMAQ_CAPTURA_AGUARDANDO_HABILITACAO,	//muda maquina para esperar habilitacao via gucRodar = 1
	CAPMAQ_CAPTURA_CAPTURANDO,		//faz uma captura dos valores de AD
	CAPMAQ_CAPTURA_AGUARDANDO,		//aguadando intervado (timeout) antes da proxima medida


    //para modo = CAPTURA_MODO_MEDICAO 
	CAPMAQ_MEDICAO_REINICIANDO,		//reiniciando estrutua para medicao de uma nova media de potencia
	CAPMAQ_MEDICAO_CAPTURANDO		//capturando novas medidas e calculando a media entre esta e a mesdia anterior
} eEstadosMaq;


// bit de sinali para modulo main gravar uma informacao
unsigned char captura_ubEventoNovoDado;
//modo de trabalho atual
unsigned char gucModo;
//maquina de estado
unsigned char gucEstadoMaquina;
//controle geral de passo dentro de um estado que precise de controle extra
unsigned char gucPasso;
//bufer geral de no minimo 40 caracteres
char *gpcBuffer;
//ponteiro para estrutura de configuracoes
TCfgModuloCapturas *gpcmCfg;

//ultima leitura do AD
int giUltimaLeitura;
//energia consumida acumulada (potencia * tempo)
unsigned long gulConsumoAtual;

//para modo teste indica se calcula valores (1) ou se mostra ADs apenas
//unsigned char gucmodoTesteCalculaValores = 0;

//indica se esta rodando
unsigned char gucRodar;

//para guardar controles do modo captura
struct
{
	unsigned int uiTempo;
} Captura;                              

//EM mV
//#define ESCALA_CORRENTE 60000
//primeiro multiplica, depois divide para ficar mais preciso
//#define CALCULAR_VALOR_MILEAMPER(AMOSTRA) ((AMOSTRA * ESCALA_CORRENTE)/1024)

//em uV
#define VREFAD_POS_MICROV 5000000 
//primeiro multiplica, depois divide para ficar mais preciso
//#define CALCULAR_VALOR_MICROV(AMOSTRA) ((AMOSTRA * VREFAD_POS_MICROV)/1024)
//relacao de uV por unidade
#define RELACAO_UV_INIDADE_AF (VREFAD_POS_MICROV / 1024) 

//para guardar controles do modo medicao
struct
{
	unsigned int uiTempoSegundos;
	struct
	{
		//valor entre 0 e 5 volts da entrada correspndente a corrente  - valor inteiro em uV (micro volt - 1x10**-6)
		float fCorrente;
		//mesmo para tensao V1 (inteiro em uV)
		float fTensaoPositiva;
		//mesmo para tensao V2 (inteiro em uV)
		float fTensaoNegativa;
		//tensao final, com sinal em mV
		long lTensaoFinal;                            
		//corrente final, com sinal em mA
		long lCorrenteFinal;
		//pontencia instantanea em mW
		long lPotenciaInst;
	} Analogico;

    
} Medicao;


//retorna o modo
unsigned char captura_ucRetornaModo(void)
{
	return gucModo;
}




//seta se deve rodar ou parar - true roda, false - para
void captura_rodar(unsigned char mucRodar)
{
//	float fRelacao;

	gucRodar = mucRodar;

	//se em modo de medicao resistor R2 noa pode estar zerado
	if(gucModo == CAPTURA_MODO_MEDICAO)
	{	
/*
		if(gucRodar)
		{
			//params para corrente
			if((gpcmCfg->uiValorADZeroCorrente == 0) || (gpcmCfg->ulSensibilidadeTC == 0))
			{
				//cancela ativacao
				gucRodar = 0;
		
		                              //0123456789abcdef  0123456789abcdef
		    	GLOBAL_TELA_ABERTURA("\fParam. para calc\nvalor I invalido", 2000);
			}
			//params para tensao
			else if((gpcmCfg->uiFatorDivResistivo == 0) || (gpcmCfg->ucFatorTrafo == 0)) 
			{
				//cancela ativacao
				gucRodar = 0;
		                              //0123456789abcdef  0123456789abcdef
		    	GLOBAL_TELA_ABERTURA("\fParam. para calc\nvalor V invalido", 2000);

			}	
			//sena ocalcula relacao entre os resistores
			else
			{
*/			
			  //zera valro do consumo atual
			  gulConsumoAtual = 0;

        //poe maquina em estado de reinicio para que comece controle de medicao
        gucEstadoMaquina = CAPMAQ_MEDICAO_REINICIANDO;

				//seta evento para avisar que comecou a rodar
				captura_ubEventoAtivado = 1;	
/*
			}
		}

*/
	}
	

}

//retorna verdadeiro se esta rodando
unsigned char captura_ucRodando(void)
{
  return gucRodar;
}



//altera o modo de trabalho
void captura_setaModo(unsigned char mucModo)
{
	//inicia estado da maquin a para teste se não setado pelo modo
	gucEstadoMaquina = CAPMAQ_TESTE_REINICIANDO;

  captura_uiUltimaLeituraADCMedicao[0] = 0;
  captura_uiUltimaLeituraADCMedicao[1] = 0;
  captura_uiUltimaLeituraADCMedicao[2] = 0;

	//copia
	gucModo = mucModo;
	//ajsuta maquina de estado para iniciar o novo modo
	switch(gucModo)
	{
	   	//modo de teste ou default
		case CAPTURA_MODO_TESTE:
			//inicia estado da maquin a para teste
			gucEstadoMaquina = CAPMAQ_TESTE_REINICIANDO;
//                                  //0123456789abcdef  0123456789abcdef
//		    GLOBAL_TELA_ABERTURA("\fModo operacao:  \rTeste.", 800);

		break;

		//Modo de caotura dos valores de AD
		case CAPTURA_MODO_CAPTURA:
			//recarrega timeo0ut de espera, assim esperar um pouco antes da priemria captura
			Captura.uiTempo = gpcmCfg->uiTempoEntreCapturas;

			// contador de capturas - zerado ao setar o modo captura e incrementado ao setar flag de que tem dados
			captura_ulQuantidadeCapturas = 0;

			//muda maquina para esperar habilitacao via gucRodar = 1
			gucEstadoMaquina = CAPMAQ_CAPTURA_AGUARDANDO_HABILITACAO;	

//                                  //0123456789abcdef  0123456789abcdef
//		    GLOBAL_TELA_ABERTURA("\fModo operacao:  \rCaptura.", 800);

		break;
		
		//modo de medicoes de potencia
		case CAPTURA_MODO_MEDICAO:
//                                  //0123456789abcdef  0123456789abcdef
//		    GLOBAL_TELA_ABERTURA("\fModo operacao:  \rMedicao.", 800);
		    
		    //poe estado em reinicio, embora precise se habtada a captura
		    gucEstadoMaquina = CAPMAQ_MEDICAO_REINICIANDO;
		    
    break;

		default:
                            //0123456789abcdef  0123456789abcdef
		    lcd_puts_const("\fMODO OPERACAO:  \rINVALIDO: ");
			sprintf(gpcBuffer, "%d", gucModo);
			lcd_puts(gpcBuffer);
			delay_ms(2000);
		break;	
	}	

  	//ajusta outros controles

}


//inicializa o modulo com o modo passado por parametro: CAPTURA_MODO_TESTE, CAPTURA_MODO_CAPTURA, CAPTURA_MODO_MEDICAO  
// pcBuffer espera um ponteiro para um array de no minimo 40 caracteres para uso geral
//muiIntervaloCaptura indica o tempo a ser esperado entre capturas em microsegundos (apenas para o modo = CAPTURA_MODO_CAPTURA)
// ***** tudo passa a ser passado via ponteiro para estrutura do tipo TCfgModuloCapturas ***
//exceto o buffer
void captura_iniciar(TCfgModuloCapturas *mpcmCfg, char *mcBuffer, TBaseEventos *mpbeBaseEventos)
{

	//copia endereco da estrutura de captura
	gpcmCfg = mpcmCfg;	
	//copia endereco do buffer
	gpcBuffer = mcBuffer;
	//zera flag que indica que tem nova captura	
	captura_ubEventoNovoDado = 0;
	
    //seta os 3 pinos de AN0 a AN2 (RA0 a RA2) como entradas
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;


	//configura os 3 canais  de AD apra 10 bits justificado o resultado a direita e com referencias na propria alimentação

	OpenADC( ADC_FOSC_64 &
	ADC_RIGHT_JUST &
	ADC_2_TAD,
	ADC_CH0 &
	ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 12 );

	//salva intervalo entre capturas aqui
	//usa ponteiro salvo
	//Captura.uiIntervaloCaptura = gpcmCfg->uiTempoEntreCapturas;

	//sempre inicia sem rodas
    gucRodar = 0;
	//zera evento
	captura_ubEventoAtivado = 0;	

 	//seta modo e assim a maquina de estado
 	captura_setaModo(gpcmCfg->ucModo);
	
}



//notifica que trabscorreu um segundo (chamar se em modo de medicao)
/*
void captura_notificaSegundoTranscorrido(void)
{
	//so aceito evento se no estado de captura de medicoes
	if(gucEstadoMaquina != CAPMAQ_MEDICAO_CAPTURANDO)
	{
		return;
	}	
	//faz decrementar u msegundo do tempo de espera e chegando a zero encaminha
	//gravacao do dados reinicia maquina para pegar novos dados
	Medicao.uiTempoSegundos--;
	if(!Medicao.uiTempoSegundos)
	{
		//enc. para gravar
		captura_ubEventoNovoDado = 1;
		//estado apra reinicio de periodo de medicao
		gucEstadoMaquina = CAPMAQ_MEDICAO_REINICIANDO;
	}
}

*/

//execucao a cada ciclo do main - faz as capturas realmente
//inicializa o modulo
void captura_execute(void)
{
  unsigned char ucTensaoNegativa, ucCorrenteNegativa;
  float *pfTensaoEntrada;
  float fVal;
  static char rucContDebug = 0;

	//processa de acordo com a maquina de estados
	switch(gucEstadoMaquina)
	{
	    //para modo = CAPTURA_MODO_TESTE
		
		
		///////////////////////////////// Apenas testando - inicialização de ciclo de testes //////////////////////////////
		case CAPMAQ_TESTE_REINICIANDO: 	
			//se nao executando fica aqui esperando
			if(!gucRodar)
			{
				gucPasso++;
				break;
			}
			
			//mostra msg de inicio de teste
			//imprime tela com campos para as leituras de AD (ficara a mesma sendo alterados apenas os dados)
                        //0123456789abcdef  0123456789abcdef
		    lcd_puts_const("\fI:xxxx, V1:xxxx \rV2:xxxx.    ");
			//imprime simbolo indicando se esta rodando ou nao
			sprintf(gpcBuffer, gucRodar? "r": "p");
			lcd_puts(gpcBuffer);

			//usa passo para alternar entre as tres leitura
			gucPasso = 0;

			//muda maquina para testando
			gucEstadoMaquina = CAPMAQ_TESTE_TESTANDO;
		break;

		///////////////////////////////// Apenas testando - Pega valores e mostra no display /////////////////////////////////
		case CAPMAQ_TESTE_TESTANDO:

			//se nao executando volta para o reinicio
			if(!gucRodar)
			{
				gucEstadoMaquina = CAPMAQ_TESTE_REINICIANDO;
				break;
			}
			//se teclas SW1 pressionada

			//passos 0 a 2 para leitura da corrente, v1 e v2
			switch(gucPasso)
			{
				//corrente
				case 0:
					//alterna canal AD
					SetChanADC(ADC_CORRENTE);
					//posiciona no display
					lcd_gotoxy(2, 0);
				break;
		
				//V1 (positiva)
				case 1:
					//alterna canal AD
					SetChanADC(ADC_TENSAOPOSITIVA);
					//posiciona no display
					lcd_gotoxy(0x0b, 0);
				break;

				//V2 (negativa)
				case 2:
					//alterna canal AD
					SetChanADC(ADC_TENSAONEGATIVA);
					//posiciona no display
					lcd_gotoxy(3, 1);
				break;

			}

			//pesquena pausa e dispara conversão, esperando terminar
			Delay10TCYx( 50 );

			ConvertADC(); 
			//ADCON0bits.GO_DONE = 1;
			while( BusyADC() );
			//while( ADCON0bits.GO_DONE );

			giUltimaLeitura = ReadADC();


			//mostra no display
			//lcd_printf_int(giUltimaLeitura);
			//lcd_puts_const("1234");
			sprintf(gpcBuffer, "%0.4d", giUltimaLeitura);
			lcd_puts(gpcBuffer);

			//proximo passo
			gucPasso++;
			//nao pode passar de 2
			gucPasso %= 3;

			//teste, imprime o passo
			lcd_gotoxy(0x0c, 1);
			lcd_putc(0x30 | gucPasso);


		break;
	

     //////////////////// captura desabilitada -- tirado código daqui
		 //aguadando ser psoto para rodar
		case CAPMAQ_CAPTURA_AGUARDANDO_HABILITACAO:
		//faz uma captura dos valores de AD
		case CAPMAQ_CAPTURA_CAPTURANDO:
	//aguadando intervado (timeout) antes da proxima medida
		case CAPMAQ_CAPTURA_AGUARDANDO:
		  //nada é feito
		break;
	
	    //para modo = CAPTURA_MODO_MEDICAO 


		//reiniciando estrutua para medicao de uma nova media de potencia
		case CAPMAQ_MEDICAO_REINICIANDO:
			//se parou de rodar mas tem registros ainda
			//se rodando, recarrega tempo e zera dados, passando a capturar
			//tambem tem que estar com o flag de evento de dados tratado (= 0)
			if(gucRodar && (!captura_ubEventoNovoDado))
			{
			
      captura_uiUltimaLeituraADCMedicao[0] = 0;			
      captura_uiUltimaLeituraADCMedicao[1] = 0;			
      captura_uiUltimaLeituraADCMedicao[2] = 0;			
			
			///////////////// por horam ostra valores calculados na tela
                        //0123456789abcdef  0123456789abcdef
//		    lcd_puts_const("\fxxxxxmA,xxxxxxmV\nP:xxxxxx mW     ");
			
			
				//estado maq. para realizar medicoes
				gucEstadoMaquina = CAPMAQ_MEDICAO_CAPTURANDO;
			}
		break;

		//capturando novas medidas e calculando a media entre esta e a mesdia anterior
		case CAPMAQ_MEDICAO_CAPTURANDO:

			//se é para parar de capturar
			if(!gucRodar)
			{
				//muda maquina para reinicio de captura de medicao
				gucEstadoMaquina = CAPMAQ_MEDICAO_REINICIANDO;
				break;
			}
			//passos 0 a 2 para leitura da corrente, v1 e v2
			for(gucPasso=0; gucPasso<3; gucPasso++)
			{
				switch(gucPasso)
				{
					//corrente
					case 0:
						//alterna canal AD
						SetChanADC(ADC_CORRENTE);
					break;
			
					//V1 (positiva)
					case 1:
						//alterna canal AD
						SetChanADC(ADC_TENSAOPOSITIVA);
					break;
	
					//V2 (negativa)
					case 2:
						//alterna canal AD
						SetChanADC(ADC_TENSAONEGATIVA);
					break;
	
				}


				//pesquena pausa e dispara conversão, esperando terminar
				Delay10TCYx( 50 );
	
				ConvertADC(); 
				//ADCON0bits.GO_DONE = 1;
				while( BusyADC() );
				//while( ADCON0bits.GO_DONE );
	
				//PEGA LEITURA
				giUltimaLeitura = ReadADC();
				
				/// *************************************
				/// *************************************
				/// ****** poe valores de teste *********
				/// *************************************
				/// *************************************
/*				
				switch(gucPasso)
				{
          case 0: 
            giUltimaLeitura = 768;  //(1024/4)*3 //3/4 do valor maximo
          break;  
          case 1: 
            giUltimaLeitura = !(rucContDebug%10) ? 812: 0;    //((311/36)-0,7)/2 -> 3,9694444444444444444444444444444 / 0,0048828125
          break;
          case 2: 
            giUltimaLeitura = !(rucContDebug%10) ? 0 : 812;   //((311/36)-0,7)/2 -> 3,9694444444444444444444444444444 / 0,0048828125
          break;
        }
*/        
//        rucContDebug++;

				//vai realziando o calculo
				// primeiro calcula o valor analogico e corrente ou tensao considernado 1024 valores possiveis dentroe de 5 volts
				//apos fazor isso para as 3 entradas, faz o restante dos calculos

				//passos 0 a 2 para leitura da corrente, v1 e v2
				switch(gucPasso)
				{
					//corrente
					case 0:
            captura_uiUltimaLeituraADCMedicao[0] = giUltimaLeitura;			
					/*
					  //determina o sinal de acordo com o vlaor de repouso da corrente (zero)
					  if(giUltimaLeitura >= gpcmCfg->uiValorADZeroCorrente)
					  {
					    //positivo
					    ucCorrenteNegativa = 0;
					    //768 - 511 = 257
					    Medicao.Analogico.fCorrente = giUltimaLeitura - gpcmCfg->uiValorADZeroCorrente;
            }
            else
            {
              //negativo
              ucCorrenteNegativa = 1;
              Medicao.Analogico.fCorrente = gpcmCfg->uiValorADZeroCorrente - giUltimaLeitura;
            }
            //em cima da difenca entre o valor de repouso do AD e a leituta, calcula
            //o valor em tensoa (uV) na entrada do AD
						//Multiplica a qtd de unidades de ad lidas pela qtd de uV por unidade
						//257 * (5.0 / 1024) =  257 * 0,0048828125 = 1,2548828125
*/						
/*						
						Medicao.Analogico.fCorrente *= (5.0 / 1024.0);
*/						
					break;
			
					//V1 (positiva)
					case 1:
            captura_uiUltimaLeituraADCMedicao[1] = giUltimaLeitura;
/*            					
						//Leituras de AD da parte positiva inteiro em uV
						//Multiplica a qtd de uV por undiade de ad pela qtd de unidades lidas
						// 5.0 / 1024.0 = 0,0048828125
						Medicao.Analogico.fTensaoPositiva = (5.0 / 1024.0);
						//0,0048828125 * 812 = 3,96484375
						Medicao.Analogico.fTensaoPositiva *= (float)giUltimaLeitura;
*/             
					break;
	
					//V2 (negativa)
					case 2:
            captura_uiUltimaLeituraADCMedicao[2] = giUltimaLeitura;
/*            					
						//Leituras de AD da parte negativa inteiro em uV
						//Multiplica a qtd de uV por undiade de ad pela qtd de unidades lidas
						Medicao.Analogico.fTensaoNegativa = (5.0 / 1024.0);
						Medicao.Analogico.fTensaoNegativa *= (float)giUltimaLeitura;
*/             
					break;
					
				}
			}

			//calcula o restante dos daods ateh chegar na tensao instantanea e na corrente
			
      
      //caulcula os valores reais da corrente a partir da tensao lida no AD
      //e salvo o calculo em Medicao.Analogico.fCorrente 
      //Leva em conta o parametro gpcmCfg->ulSensibilidadeTC que indica quantos mV
      //devem mudar ao alterar em 1A a corrente.
      //assim temos qu a corrente, em amperes é a divisão da tensao lida  por este fator
      //como o vlaro esta em mV divide por 1000.0
      
      // 1,2548828125 / (12500 / 1000.0) =  1,2548828125 / (12,5) = 0,100390625
/*      
      Medicao.Analogico.fCorrente /= (float)(gpcmCfg->ulSensibilidadeTC / 1000.0);
*/      
      //multiplica por 1000 para ficar em amperes
      //0,100390625 * 1000.0 = 100,390625
      ////////////////////////////// corente em amperes: ///////////////////////
/*      
      Medicao.Analogico.fCorrente *= 1000.0; 
      if(ucCorrenteNegativa)
      {
        Medicao.Analogico.fCorrente *= -1.0;
      }
*/      
      //////////////////////////////////////////////////////////////////////////
      
      //o valor final eh salvo inteiro e em milivots (3 casas presumidas)
/*
      Medicao.Analogico.fCorrente = 257;
      
      //100,390625 * 1000.0 = 100390,625
#ifdef USA_MATH       
      Medicao.Analogico.lCorrenteFinal = ceil(Medicao.Analogico.fCorrente * 1000.0);
#else   
      fVal = Medicao.Analogico.fCorrente;
      fVal *= 1000.0;
      fVal *= 1000.0; 
      Medicao.Analogico.lCorrenteFinal = fVal;
#endif      

      
      //se negativo
      if(ucCorrenteNegativa)
      {
        Medicao.Analogico.lCorrenteFinal *= -1;
      }
            		
			
			
			//caulcula os valores reais da corrente levando em conta o parametro
			//de configuracao uiFatorDivResistivo
			//mas seleciona entre a parcela positiva e negativa a maior (jah que )
			if(Medicao.Analogico.fTensaoPositiva > Medicao.Analogico.fTensaoNegativa)
			{
			  pfTensaoEntrada = &Medicao.Analogico.fTensaoPositiva;
			  ucTensaoNegativa = 0;
			}
			//senao he negativa
			else
			{
			  pfTensaoEntrada = &Medicao.Analogico.fTensaoNegativa;
			  ucTensaoNegativa = 1;
			}

		  //Formula para a tensao:
      // 
      //Tensão no secundário do trafo (um semiciclo)
      //Vsec = (Amostra * gpcmCfg->uiFatorDivResistivo*) + gpcmCfg->uiDescontoDiodos*
      // * As configuracoes citadas estao salvas como inteiros com 3 casas presumidas
      
      //divide a tensao na entrada com o fator dos resistores (3 casas presumidas)
      //3,96484375 / (500 / 1000.0) = 3,96484375 / 0,5 = 7,9296875  
      *pfTensaoEntrada /= (float)(gpcmCfg->uiFatorDivResistivo / 1000.0);
                                                  
      //soma a queda de tensao no diodo (3 casas presumidas)
      //7,9296875 + (700 / 1000.0) = 7,9296875 + (0,7) =  8,6296875
      *pfTensaoEntrada += (float)(gpcmCfg->uiDescontoDiodos / 1000.0);       
      //
      //Tensão no primario do trafo (um semiciclo) -> Valor de interesse
      //Vprim = Vsec * gpcmCfg->ucFatorTrafo**
      // ** Valor como multiplicador sem casas presumidas
      //
      
      /////////////////////////////// tensao de entrada em volts ///////////////
      //o fator do trafo na otem casa presumidas
      //8,6296875 * 36 = 310,66875
      *pfTensaoEntrada *= (float)gpcmCfg->ucFatorTrafo;
      //////////////////////////////////////////////////////////////////////////      

      //salva como inteiro com 3 casas presumidas
#ifdef USA_MATH       
      Medicao.Analogico.lTensaoFinal = ceil(*pfTensaoEntrada * 1000.0);
#else      
      Medicao.Analogico.lTensaoFinal = (long)(*pfTensaoEntrada * 1000.0);
#endif      
      
      //se sinal negativo faz ficar negativo
      if(ucTensaoNegativa)
      {
        Medicao.Analogico.lTensaoFinal *= -1;
      }
      


      //////////////////// mostra no display tudo por enquanto /////////////////
                        //0123456789abcdef  0123456789abcdef
		  //lcd_puts_const("\fxxxxxmA,xxxxxxmV\nP:xxxxxx mW     ");
      //corrente
      lcd_gotoxy(0, 0);
      sprintf(gpcBuffer, "%0.5d", Medicao.Analogico.lCorrenteFinal);
			lcd_puts(gpcBuffer);         

      //tensao
      lcd_gotoxy(8, 0);
      sprintf(gpcBuffer, "%0.6d", Medicao.Analogico.lTensaoFinal);
      //sprintf(gpcBuffer, "%6.3f", *pfTensaoEntrada);
			lcd_puts(gpcBuffer);         



      ///calcuala potencia instantanea simplesmente multiplicando um valor pelo outro
      //como floar e depois salvando com 3 casas presumidas na variavel inteira
      *pfTensaoEntrada *= Medicao.Analogico.fCorrente;
      //salva como int com 3 casas
#ifdef USA_MATH             
      Medicao.Analogico.lPotenciaInst = ceil(*pfTensaoEntrada * 1000.0);
#else      
      Medicao.Analogico.lPotenciaInst = (int)(*pfTensaoEntrada * 1000.0);
#endif      
      

	    //potencia isntantanea
      //tensao
      lcd_gotoxy(2, 1);
      sprintf(gpcBuffer, "%0.6d", Medicao.Analogico.lPotenciaInst);
      //sprintf(gpcBuffer, "%0.6f", *pfTensaoEntrada);
			lcd_puts(gpcBuffer);
      
      //calculo do consumo atual
      gulConsumoAtual += Medicao.Analogico.lPotenciaInst; 
*/
		break;


		
		default:                                                            
				//tratar erro de maq. estado aqui

		break;
	}
}


                                                                    