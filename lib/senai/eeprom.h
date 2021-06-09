#ifndef EEPROM_H
#define EEPROM_H

//id unico do modulo para passar nos eventos previstos em base.h
#define EEPROM_ID_MODULO 7


void eeprom_iniciar(TBaseEventos *mpbeBaseEventos);
unsigned char eeprom_ler(unsigned char endereco);
void eeprom_escrever(unsigned char endereco,unsigned char dado);
//void eeprom_enviar( char ch);
//void eeprom_print_num(unsigned int valor);
//void eeprom_printf(const char * texto);

//max
void eeprom_escreverBuffer(unsigned char endereco,unsigned char *mpucBuffer, unsigned char mucTamanho);
#endif
	
