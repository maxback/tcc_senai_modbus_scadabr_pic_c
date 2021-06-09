# MPLAB IDE generated this makefile for use with GNU make.
# Project: MedidorEnergia.mcp
# Date: Sun Nov 28 10:58:07 2010

AS = mpasmwin.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

MedidorEnergia.cof : lcd.o utils.o bits.o eeprom.o m3wlib.o RTC_1302.o Serial.o captura.o dados.o m25pxx.o main.o protocolo.o
	$(LD) /l"C:\MCC18\lib" "rm18f4550.lkr" "lcd.o" "utils.o" "bits.o" "eeprom.o" "m3wlib.o" "RTC_1302.o" "Serial.o" "captura.o" "dados.o" "m25pxx.o" "main.o" "protocolo.o" /u_CRUNTIME /u_DEBUG /z__MPLAB_BUILD=1 /z__MPLAB_DEBUG=1 /m"MedidorEnergia.map" /w /o"MedidorEnergia.cof"

lcd.o : ../lib/kitmcu/lcd.c ../lib/kitmcu/utils.h ../../../../../../../../../../MCC18/h/pwm.h ../lib/kitmcu/lcd.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../lib/kitmcu/tipos.h ../../../../../../../../../../MCC18/h/pconfig.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\kitmcu\lcd.c" -fo="lcd.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

utils.o : ../lib/kitmcu/utils.c ../../../../../../../../../../MCC18/h/timers.h ../../../../../../../../../../MCC18/h/delays.h ../lib/kitmcu/utils.h ../lib/kitmcu/utils.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../../../../../../../../../../MCC18/h/pconfig.h ../lib/kitmcu/tipos.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\kitmcu\utils.c" -fo="utils.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

bits.o : ../lib/senai/bits.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\senai\bits.c" -fo="bits.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

eeprom.o : ../lib/senai/eeprom.c ../lib/senai/eeprom.h ../lib/senai/eeprom.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\senai\eeprom.c" -fo="eeprom.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

m3wlib.o : ../lib/senai/m3wlib.c ../lib/senai/m3wlib.h ../lib/senai/bits.h ../lib/senai/m3wlib.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\senai\m3wlib.c" -fo="m3wlib.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

RTC_1302.o : ../lib/senai/RTC_1302.C ../lib/senai/m3wlib.h ../lib/senai/RTC_1302.C ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\senai\RTC_1302.C" -fo="RTC_1302.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

Serial.o : ../lib/senai/Serial.c ../lib/senai/serial.h ../lib/senai/Serial.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h
	$(CC) -p=18F4550 "C:\Users\maxback\Senai\2010\TCC\Projeto\Programacao\C\PIC\lib\senai\Serial.c" -fo="Serial.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

captura.o : captura.c ../../../../../../../../../../MCC18/h/delays.h ../../../../../../../../../../MCC18/h/adc.h ../../../../../../../../../../MCC18/h/stdio.h ../../../../../../../../../../MCC18/h/stdlib.h ../../../../../../../../../../MCC18/h/string.h ../lib/kitmcu/utils.h ../lib/kitmcu/lcd.h captura.h dados.h captura.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../../../../../../../../../../MCC18/h/pconfig.h ../../../../../../../../../../MCC18/h/stdarg.h ../../../../../../../../../../MCC18/h/stddef.h ../lib/kitmcu/tipos.h global.h
	$(CC) -p=18F4550 "captura.c" -fo="captura.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

dados.o : dados.c ../../../../../../../../../../MCC18/h/stdio.h ../../../../../../../../../../MCC18/h/stdlib.h ../../../../../../../../../../MCC18/h/string.h m25pxx.h dados.h ../lib/kitmcu/utils.h ../lib/kitmcu/lcd.h dados.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../../../../../../../../../../MCC18/h/stdarg.h ../../../../../../../../../../MCC18/h/stddef.h global.h ../lib/kitmcu/tipos.h
	$(CC) -p=18F4550 "dados.c" -fo="dados.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

m25pxx.o : m25pxx.c ../../../../../../../../../../MCC18/h/delays.h ../lib/kitmcu/utils.h ../lib/kitmcu/lcd.h m25pxx.h m25pxx.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../lib/kitmcu/tipos.h global.h
	$(CC) -p=18F4550 "m25pxx.c" -fo="m25pxx.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

main.o : main.c ../../../../../../../../../../MCC18/h/delays.h ../../../../../../../../../../MCC18/h/stdio.h ../../../../../../../../../../MCC18/h/stdlib.h ../../../../../../../../../../MCC18/h/string.h ../lib/kitmcu/utils.h ../lib/kitmcu/lcd.h ../lib/senai/serial.h ../lib/senai/eeprom.h captura.h dados.h protocolo.h m25pxx.h main.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../../../../../../../../../../MCC18/h/stdarg.h ../../../../../../../../../../MCC18/h/stddef.h ../lib/kitmcu/tipos.h ../lib/senai/rtc_1302.h global.h
	$(CC) -p=18F4550 "main.c" -fo="main.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

protocolo.o : protocolo.c ../../../../../../../../../../MCC18/h/stdio.h ../../../../../../../../../../MCC18/h/stdlib.h ../../../../../../../../../../MCC18/h/string.h ../lib/senai/serial.h protocolo.h protocolo.c ../../../../../../../../../../MCC18/h/p18cxxx.h ../../../../../../../../../../MCC18/h/p18f4550.h ../../../../../../../../../../MCC18/h/stdarg.h ../../../../../../../../../../MCC18/h/stddef.h global.h
	$(CC) -p=18F4550 "protocolo.c" -fo="protocolo.o" -D__DEBUG -DPROJETO_PI -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

clean : 
	$(RM) "lcd.o" "utils.o" "bits.o" "eeprom.o" "m3wlib.o" "RTC_1302.o" "Serial.o" "captura.o" "dados.o" "m25pxx.o" "main.o" "protocolo.o" "MedidorEnergia.cof" "MedidorEnergia.hex" "MedidorEnergia.map"

