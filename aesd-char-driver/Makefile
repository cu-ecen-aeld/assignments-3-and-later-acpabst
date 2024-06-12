# See example Makefile from scull project
# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSCULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)

CC ?= gcc
CFLAGS ?= -g -pthread
LDFLAGS ?= -lm

ifneq ($(KERNELRELEASE),)
# call from kernel build system
obj-m	:= aesdchar.o
aesdchar-y := aesd-circular-buffer.o main.o
else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all: modules test

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

test:
	$(CC) $(CFLAGS) -c flush_test.c -o flush_test.o
	$(CC) $(CFLAGS) flush_test.o -o flush_test

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

