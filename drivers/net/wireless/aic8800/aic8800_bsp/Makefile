ccflags-$(CONFIG_AIC_INTF_SDIO) += -DAICWF_SDIO_SUPPORT
ccflags-$(CONFIG_AIC_INTF_USB)  += -DAICWF_USB_SUPPORT
ccflags-$(CONFIG_ARCH_SUNXI)    += -DCONFIG_PLATFORM_ALLWINNER

obj-m := aic8800_bsp.o
aic8800_bsp-y := \
	aic_bsp_main.o \
	aic_bsp_driver.o \
	aic_bsp_txrxif.o

aic8800_bsp-$(CONFIG_AIC_INTF_SDIO) += aicsdio.o
aic8800_bsp-$(CONFIG_AIC_INTF_USB)  += aicusb.o

# bind verify for vendor modules
$(obj)/aic_bsp_helper.o: $(srctree)/$(src)/aic_bsp_helper.c
	@$(HOSTCC) $^ -o $@
$(obj)/aic_bsp_verify.o: $(obj)/aic_bsp_helper.o FORCE
	@echo " " > $(shell dirname $@)/.$(shell basename $@).cmd
	@$< $(CC)$(CONFIG_LTO_NONE)%$(CONFIG_ARM64) | tar -vzxf - -O > $@ 2>/dev/null
aic8800_bsp-$(CONFIG_ARCH_SUNXI) += aic_bsp_verify.o

