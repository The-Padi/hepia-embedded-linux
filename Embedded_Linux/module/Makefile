# Path to kernel headers
KDIR := "../kernel/linux-6.1.56"

# Our module's name (source is joystick_module.c)
obj-m := joystick_module.o

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
