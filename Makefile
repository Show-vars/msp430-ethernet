CC=msp430-elf-gcc -mmcu=msp430g2553

SOURCES=$(wildcard *.c)
OUTPUT=out.elf

STRIPARGS=-Os -ffunction-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all

all: compile

compile:
	$(CC) $(SOURCES) -o $(OUTPUT)

compile-stripped:
	$(CC) $(SOURCES) $(STRIPARGS) -o $(OUTPUT)
