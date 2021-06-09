#ifndef M3WLIB_H
#define M3WLIB_H

//id unico do modulo para passar nos eventos previstos em base.h
#define M3WLIB_ID_MODULO 3


#ifndef M3WLIB_C
extern
#endif
void init_3w();

#ifndef M3WLIB_C
extern
#endif
void reset_3w();

#ifndef M3WLIB_C
extern
#endif
void wbyte_3w(unsigned char W_Byte);

#ifndef M3WLIB_C
extern
#endif
unsigned char rbyte_3w();


#endif
