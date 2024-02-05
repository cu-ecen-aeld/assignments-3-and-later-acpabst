CC = gcc
CFLAGS = -g

ifeq ($(CROSS_COMPILE),)
 $(info Compiling natively.)
else
 CROSS_COMPILE_FULL = aarch-none-linux-gnu-$(CROSS_COMPILE)
 CC = $(CROSS_COMPILE_FULL)
 $(info Cross compiling: $(CROSS_COMPILE_FULL)) 
endif

writer:  writer.o
	$(CC) writer.o -o writer

writer.o: writer.c
	$(CC) -c writer.c -o writer.o

clean:
	rm -f writer writer.o
