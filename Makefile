# Makefile for building in native and cross compile mode.
# Set environment variable KERNEL_SRC to point to top of the kernel source
# And set CCPREFIX to point to compiler location with cross compile prefix

TARGET = snesconio
obj-m += $(TARGET).o
$(TARGET)-objs := snescon.o snescon_base.o

KERNEL_SRC_LOCATION = /lib/modules/$(KVERSION)/build
CROSS_COMPILE_SETTINGS =

ifneq ($(strip $(CCPREFIX)),)
KERNEL_SRC_LOCATION = ${KERNEL_SRC}
CROSS_COMPILE_SETTINGS = ARCH=arm CROSS_COMPILE=${CCPREFIX}
endif

all:
	@$(MAKE) $(CROSS_COMPILE_SETTINGS) -C $(KERNEL_SRC_LOCATION) M=$(PWD) modules

clean: 
	@$(MAKE) -C ${KERNEL_SRC_LOCATION} M=$(PWD) clean
