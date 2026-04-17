INSTALLDIR = /lib/modules/`uname -r`/kernel/drivers/usb/kromek

TARGET = "kromekusb.ko"
obj-m := kromekusb.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install: all
	install -D $(TARGET) $(INSTALLDIR)/$(TARGET)
	depmod -a

clean:
	rm -f kromekusb.o kromekusb.ko kromekusb.mod.o .cache.mk
