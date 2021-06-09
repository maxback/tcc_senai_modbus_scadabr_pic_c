#ifndef BITS_H
#define BITS_H

//id unico do modulo para passar nos eventos previstos em base.h
#define BITS_ID_MODULO 5


void set_bit(unsigned char * REG,unsigned char BIT);
void clear_bit(unsigned char * REG,unsigned char BIT);
unsigned char test_bit(unsigned char * REG,unsigned char BIT);

#endif

