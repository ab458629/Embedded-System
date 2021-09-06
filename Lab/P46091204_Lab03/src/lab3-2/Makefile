obj-m  = lab3-2-blink_freq.o
kernel-release = 5.4.0-1-generic
all:
	make -C /lib/modules/$(kernel-release)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(kernel-release)/build M=$(shell pwd) clean
