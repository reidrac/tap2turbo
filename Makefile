BIN=tap2turbo

all: $(BIN)

CC=gcc
CFLAGS=-s -O3 -Wall
#CFLAGS=-ggdb -Wall

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c

tap2turbo: tap2turbo.o
	$(CC) $(CFLAGS) $< -lspectrum -o $@

clean:
	rm -f $(BIN) *.o

