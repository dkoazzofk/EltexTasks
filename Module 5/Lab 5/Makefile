obj-m += kernalspace.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc ./userspace.c -o userspace

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm ./userspace