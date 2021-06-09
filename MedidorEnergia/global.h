/*
** Definições e macros globais para o projeto usar
*/
#ifndef GLOBAL_H
#define GLOBAL_H

#define GLOBAL_TELA_ABERTURA(MSG, TEMPO) { lcd_puts_const(MSG); delay_ms(TEMPO); }

//indices de cfg na eeprom do equipe.

//indica de configuracao de momo e funcionamento
											//Valores(tamanho): 
#define GLOBAL_INDEX_CFG_MODO 0				//0-modoOperacao(1)
#define GLOBAL_INDEX_CFG_TEMPOCAPTURAS 1	//1-TempoEntreCapturas(2),
#define GLOBAL_INDEX_CFG_VALORADZEROCOR 3   //2-ValorADZeroCorrente(2)
#define GLOBAL_INDEX_CFG_FATORTRAFO 5 		//3-FatorTrafo(1)
#define GLOBAL_INDEX_CFG_DESCONTODIODOS 6 	//4-DescontoDiodos(2)*
#define GLOBAL_INDEX_CFG_FATORDIVRESISTIVO 8 		//5-Fator div Resistor1(2)**
#define GLOBAL_INDEX_CFG_SENSIBILIDADE_TC 10  //sensibilidae do TC (4)
//variavel do endereco
#define GLOBAL_INDEX_CFG_ENDERECO_REDE 14  //endereço de comunicação(1)


#define GLOBAL_INDEX_CFG_MIN	0
#define GLOBAL_INDEX_CFG_MAX	14

//indice para ovalor acumulado de energia consumida
#define GLOBAL_INDEX_CFG_CONSUMO_ACUMULADO 15  //4 bytes

#define GLOBAL_INDEX_CFG_SAIDA_ASSOCIADA_OPER 19


//indices de entradas e saidas no barramento para esta aplicacao
//ENTRADAS
//entradas na interface de automação
#define GLOBAL_SEL_ENT_IA 0
//botoes da interface de usuário
#define GLOBAL_SEL_ENT_IU_BOTOES 1

//SAIDAS
//saida spi 0(nao usada)
#define GLOBAL_SEL_SAI_CS0 0
//saida na interface de usuário para display
#define GLOBAL_SEL_SAI_IU_DISPLAY 1
//saida na interface de automação
#define GLOBAL_SEL_SAI_IA 2
//said na interface de usuário para os LEDs
#define GLOBAL_SEL_SAI_IU_LEDS 3
//saida spi 4 (nao usada)
#define GLOBAL_SEL_SAI_CS4 4
//saida spi 5 (nao usada)
#define GLOBAL_SEL_SAI_CS5 5
//saida spi 6 (nao usada)
#define GLOBAL_SEL_SAI_CS6 6
//saida 7 (nao usada)
#define GLOBAL_SEL_SAI_7 7
//SAIDA A SER USADA PARA MUDAR PORTA SE AFETAR SAIDAS IMPORTANTES
#define GLOBAL_SEL_NENHUM GLOBAL_SEL_SAI_CS0



//mascara para os bits de entrada no barramento pro tecla (se sel. entrada interface usuário)
#define GLOBAL_MASCARA_BTN_ENTER 0x01
#define GLOBAL_MASCARA_BTN_UP 0x02
#define GLOBAL_MASCARA_BTN_DOWN 0x04
#define GLOBAL_MASCARA_BTN_CANCEL 0x08
#define GLOBAL_MASCARA_BTN_F1 0x10
#define GLOBAL_MASCARA_BTN_F2 0x20

//mascara para os leds indicadores e macros para ler e altear seus estados
#define GLOBAL_MASCARA_LED1 0x01
#define GLOBAL_MASCARA_LED2 0x02
#define GLOBAL_MASCARA_LED3 0x04
#define GLOBAL_MASCARA_LED4 0x08

//mascaras para funcao dos leds jah
#define GLOBAL_MASCARA_LED_RODANDO GLOBAL_MASCARA_LED1
#define GLOBAL_MASCARA_LED_MEDINDO GLOBAL_MASCARA_LED2
#define GLOBAL_MASCARA_LED_DEBUG GLOBAL_MASCARA_LED3
#define GLOBAL_MASCARA_LED_COM GLOBAL_MASCARA_LED4


#endif