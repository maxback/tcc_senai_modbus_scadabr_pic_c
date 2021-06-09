/***
**** Arquivo de cabeçalho do módulo de armazenamento de dados
***/
#ifndef DADOS_H
#define DADOS_H

//id unico do modulo para passar nos eventos previstos em base.h
#define DADOS_ID_MODULO 103


//////////////////////////////////// Definições, tipos e variaveis ///////////////////////////////////

// Tipos de registros a serem armazenados na memoria
// Estees valores possiveis são passados no parâmetro setado no item ucTipo da estrutura de dados dados_reg

// Registro de inicio de um periodo de captura dos AD - Deve conter data e hora do inicio das capturas
#define TR_CABECALHO_CAPTURA 0

// Registro de uam captura. Contem os valotes dos 3 ADs
#define TR_REGISTRO_CAPTURA 1

//Registro de fechamento de uma captura com data e hora e quantidade de registros da captura (2 bytes)
#define TR_FECHAMENTO_CAPTURA 2
#define TR_REGISTRO_MEDICAO 3

//registris de flaf de final do uso da memoria no CI o ude memória ainda não usada
#define TR_SEM_DADOS_NO_CI 4
#define TR_SEM_DADOS 0xFF


// modos de operacao para funcoes dados_alteraModo() e dados_ucRetornaModo()
#define DADOS_MODO_ESCRITA 0
#define DADOS_MODO_LEITURA 1




// definicao tipo para registro publico a ser preeenchido antes de chamar uam funcao de escrita
// e onde são armazenados os dados numa operacao de leitura
typedef struct 
{
  //tipo de registros armazenado na estrutura
  unsigned char ucTipo;
  
  //union para poder usar para medicao, regsitro de captura fechamento de captura com nomes mais adequados
  union
  {
    //Leituras de AD para registro de captura TR_REGISTRO_CAPTURA
    struct
    {
      //Leitura de AD de corrente
      unsigned int uiADcorrente;

      //Leituras de AD da parte positiva e negativa
      unsigned int uiADTensaoPositiva;
      unsigned int uiADTensaoNegativa;

    } Captura;

    //dados de qtd de registros em uma captura usado no fechamanto da captura, na abertura estes dados sao irelevantes
    struct
    { 
      //flag adefinir funcao
      unsigned int uiFlag;

      //valor log com a quantidade de capturas
      unsigned long int ulQtdCapturas;

    } CapturaFechamento;

    //Dados de carculo de petencia para registro de medicao TR_REGISTRO_MEDICAO
    struct
    {
      //Valor de potencia calculado
      unsigned long lPotencia;
   
      //Quantidade de capturas no periodo
      unsigned int uiQtdCapturas;

    } Medicao;

  } Dados;  

  // dados do RTC para o registros - Ficam no final facilitar gravar na flash a aprtir destra estrutura ignorando o RTC Iregsitros de captura)
  struct
  {
    unsigned char ucSegundo;
    unsigned char ucMinuto;
    unsigned char ucHora;
    unsigned char ucMes;
    unsigned char ucDia;
    unsigned char ucAno;

  } RTC; 
  
}TDadosRegistro;


// Definicao dos tamanhos da estrutura inteira e ignorando o RTC final (registro captura, sem os dados do RTC)
#define TAMANHO_REG_DADOS_COMPLETO sizeof(TDadosRegistro)
#define TAMANHO_REG_DADOS_CAPTURA (sizeof(TDadosRegistro) - 6)




// define varavel dados_reg
#ifndef DADOS_C
extern
#endif
TDadosRegistro dados_Reg;


//////////////////////////////////// Funções do módulo ///////////////////////////////////

//lista de funções do módulo


// Inicialzia o módulo
#ifndef DADOS_C
extern
#endif
void dados_iniciar(TBaseEventos *mpbeBaseEventos);

///////  Funções de operação sobre registros ////////////


// Escreve na próxima posicao os dados armazenados na estrutura passada por parametro
// retorna falso se não existe mais memória 
#ifndef DADOS_C
extern
#endif
unsigned char  dados_ucGravarRegistro(TDadosRegistro *mpdrRegistro);


// Le da memória para a variavel dados_Reg e retorna a quantidade de bytes a partir do nicio da estrutura ou 0, se jah nao tem dados
//apos ler os dados anda para o proximo registro.
#ifndef DADOS_C
extern
#endif
unsigned char dados_ucLerRegistro(void);


// Retorna a quantidade de memória com dados guardados, em registros
#ifndef DADOS_C
extern
#endif
unsigned long int dados_ulTamanhoRegistros(void);


///////  Funções de operação sobre a memória diretamente - Apenas para leitura ////////////



// Le da memória para a o buffer passado por ponteiro em mpucBuffer até mucTamanho bytes
// Se o parametro mucProximo for verdadeiro além de ler anda para frente para que na proxima leitura pegue outros bytes
// retorna o tamanaho lido ou 0 se já leu todos e avançou além.
#ifndef DADOS_C
extern
#endif
unsigned char dados_ucLerBuffer(unsigned char *mpucBuffer, unsigned char mucTamanho, unsigned char mucProximo);


// Retorna a quantidade de memória com dados guardados, em bytes
#ifndef DADOS_C
extern
#endif
unsigned long int dados_ulTamanhoBufferTotal;


// flag que indica se tem memoria para escrita ainda
#ifndef DADOS_C
extern
#endif
unsigned long int dados_ucTemMemoriaLivre;


/////////////////////// Funçoes gerais ////////////////////////////////
// Apaga a memória, limpando tdos os registros e reiniciando os ponteiros
#ifndef DADOS_C
extern
#endif
void dados_Limpar(void);

//Seta modo entre escrita e leitura
//se Escrita (DADOS_MODO_ESCRITA): posiciona na proxima area livre para escrita
//Se leitura (DADOS_MODO_LEITURA): posiciona no inicio da memoria para que possa ler dados e enviar a algum lugar, ou apenas inspecion'a-los
//alterar entre modos antes de iniciar escrita ou leitura. Inicia em modo ecrita
#ifndef DADOS_C
extern
#endif
void dados_alteraModo(unsigned char mucModo);

//retorna modo atual
#ifndef DADOS_C
extern
#endif
unsigned char dados_ucRetornaModo(void);



#endif