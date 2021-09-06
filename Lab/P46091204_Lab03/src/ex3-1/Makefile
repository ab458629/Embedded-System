obj-m  = ex3-1-on_off.o
kernel-release = 5.4.0-1-generic
all:
	make -C /lib/modules/$(kernel-release)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(kernel-release)/build M=$(shell pwd) clean
