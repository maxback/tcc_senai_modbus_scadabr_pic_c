// Modulo de comunicação 3wire (3 fios)

#define M3WLIB_C

// Definições no Modulo
#ifdef PROJETO_PI
///// config para projeto PI 2009 usando KIT PIC 18F d MCU - Deve ser definido nas opções do arquivo no proceto //////////

#include <p18cxxx.h>	// inclui as definições dos ios do pic
#include "..\lib\base\base.h"
#include "m3wlib.h"
#include "bits.h" //

//TCC: alterada definica e permitido que seja definido 
// Define as portas usadas para comunição
#define data_tris TRISA
#define data_port PORTA
#define data_pin  3
#define rst_tris  TRISA
#define rst_port  PORTA
#define rst_pin   4
#define clk_tris  TRISA
#define clk_port  PORTA
#define clk_pin   5

#else

#include <htc.h>


#include "m3wlib.h"
#include "bits.h" //

#define data_tris TRISC
#define data_port PORTC
#define data_pin  1
#define rst_tris  TRISC
#define rst_port  PORTC
#define rst_pin   4
#define clk_tris  TRISC
#define clk_port  PORTC
#define clk_pin   0

#endif

// Inicia portas para comunicação 3w (3 fios)
void init_3w(TBaseEventos *mpbeBaseEventos)
{

    clear_bit(&data_tris, data_pin);
    clear_bit(&rst_tris, rst_pin);
    clear_bit(&clk_tris, clk_pin);

    clear_bit(&data_port, data_pin);
    clear_bit(&rst_port, rst_pin);
    clear_bit(&clk_port, clk_pin);

}

void reset_3w()
{

    clear_bit(&clk_port, clk_pin);
    clear_bit(&rst_port, rst_pin);
    set_bit(&rst_port, rst_pin);

}

/* ----------------------------------------------------------------------- */

void wbyte_3w(unsigned char W_Byte)
{
    unsigned char i;

    clear_bit(&data_tris, data_pin);

    for(i = 0; i < 8; ++i)
    {
        clear_bit(&data_port, data_pin);

        if(W_Byte & 0x01)
        {
            set_bit(&data_port, data_pin);
        }

        clear_bit(&clk_port, clk_pin);
        set_bit(&clk_port, clk_pin);
        W_Byte = (W_Byte>>1);
      }
}
/* ----------------------------------------------------------------------- */
unsigned char rbyte_3w()
{
    unsigned char i,R_Byte;

    set_bit(&data_tris, data_pin);

    R_Byte = 0x00;
    set_bit(&data_port, data_pin);

    for(i=0; i<8; ++i)
    {

        set_bit(&clk_port, clk_pin);
        clear_bit(&clk_port, clk_pin);

        R_Byte >>= 1;
        R_Byte = R_Byte | ((data_port >> data_pin) << 7);

    }
    return R_Byte;
}

