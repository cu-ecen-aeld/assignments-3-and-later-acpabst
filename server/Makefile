CC = gcc
CFLAGS = -g

ifeq ($(CROSS_COMPILE),)
 $(info Compiling natively.)
else
 CROSS_COMPILE_FULL = aarch64-none-linux-gnu-$(CROSS_COMPILE)
 CC = $(CROSS_COMPILE_FULL)
 $(info Cross compiling: $(CROSS_COMPILE_FULL))
endif

all: clean aesdsocket.o aesdsocket

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -o aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -c aesdsocket.c -o aesdsocket.o

clean:
	rm -f aesdsocket aesdsocket.o
