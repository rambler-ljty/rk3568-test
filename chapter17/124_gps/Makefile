##
CC=/usr/local/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc

## app
app:gps.o uart.o main.o

	$(CC) -o app gps.o uart.o main.o

## build gps
gps.o: gps.c gps.h
	$(CC) -c gps.c

## build uart

uart.o: uart.c
	$(CC) -c uart.c

## build main
main.o: uart.o gps.o gps.h
	$(CC) -c gps.c uart.c main.c

#clean
clean:
	rm -f app *.o
