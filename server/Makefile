CC ?= gcc
CFLAGS ?= -g -pthread
LDFLAGS ?= -lm

ifeq ($(CROSS_COMPILE),)
 $(info Compiling natively.)
else
 CROSS_COMPILE_FULL = aarch64-none-linux-gnu-$(CROSS_COMPILE)
 CC = $(CROSS_COMPILE_FULL)
 $(info Cross compiling: $(CROSS_COMPILE_FULL))
endif

all: clean aesdsocket.o aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) aesdsocket.o -o aesdsocket $(LDFLAGS)

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o $(LDFLAGS)

clean:
	rm -f aesdsocket aesdsocket.o
