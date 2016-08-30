

all:
	msp430-elf-gcc -mmcu=msp430g2553 enc28j60.c spi.c main.c -o out.elf
