obj-m := newmodule.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all: default

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions