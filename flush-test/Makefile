#CC ?= gcc
CFLAGS ?= -g -pthread
LDFLAGS ?= -lm

ifeq ($(CROSS_COMPILE),)
 $(info Compiling natively.)
else
 CROSS_COMPILE_FULL = aarch64-none-linux-gnu-$(CROSS_COMPILE)
 CC = $(CROSS_COMPILE_FULL)
 $(info Cross compiling: $(CROSS_COMPILE_FULL))
endif

all: clean flush_test.o flush_test

aesdsocket: flush_test.o
	$(CC) $(CFLAGS) flush_test.o -o flush_test $(LDFLAGS)

aesdsocket.o: flush_test.c
	$(CC) $(CFLAGS) -c flush_test.c -o flush_test.o $(LDFLAGS)

clean:
	rm -f flush_test flush_test.o
