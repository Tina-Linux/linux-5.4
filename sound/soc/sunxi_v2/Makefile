# common -> platform of dma
snd_soc_sunxi_pcm-objs				+= snd_sunxi_pcm.o
obj-$(CONFIG_SND_SOC_SUNXI_PCM)			+= snd_soc_sunxi_pcm.o

# common -> machine
snd_soc_sunxi_machine-objs			+= snd_sunxi_mach.o
snd_soc_sunxi_machine-objs			+= snd_sunxi_mach_utils.o
obj-$(CONFIG_SND_SOC_SUNXI_MACH)		+= snd_soc_sunxi_machine.o

# internal codec -> codec
ifneq ($(CONFIG_SND_SOC_SUNXI_SUN50IW9_CODEC), n)
snd_soc_sunxi_internal_codec-objs		+= snd_sun50iw9_codec.o
# e.g.
# else ifneq ($(CONFIG_SND_SOC_SUNXI_SUNxxIWxx_CODEC), n)
# snd_soc_sunxi_internal_codec-objs		+= snd_sunxxiwxx_codec.o
endif
snd_soc_sunxi_internal_codec-objs		+= snd_sunxi_common.o
snd_soc_sunxi_internal_codec-objs		+= snd_sunxi_adapter.o
snd_soc_sunxi_internal_codec-objs		+= snd_sunxi_internal_codec.o
obj-$(CONFIG_SND_SOC_SUNXI_INTERNALCODEC)	+= snd_soc_sunxi_internal_codec.o

# dummy codec -> codec
snd_soc_sunxi_dummy_codec-objs			+= snd_sunxi_dummy_codec.o
obj-$(CONFIG_SND_SOC_SUNXI_DUMMYCODEC)		+= snd_soc_sunxi_dummy_codec.o

# aaudio/spdif/dmic/daudio/ahub -> platform
snd_soc_sunxi_aaudio-objs			+= snd_sunxi_aaudio.o
obj-$(CONFIG_SND_SOC_SUNXI_AAUDIO)		+= snd_soc_sunxi_aaudio.o

snd_soc_sunxi_spdif-objs			+= snd_sunxi_spdif.o
obj-$(CONFIG_SND_SOC_SUNXI_SPDIF)		+= snd_soc_sunxi_spdif.o

snd_soc_sunxi_dmic-objs				+= snd_sunxi_dmic.o
obj-$(CONFIG_SND_SOC_SUNXI_DMIC)		+= snd_soc_sunxi_dmic.o

snd_soc_sunxi_daudio-objs			+= snd_sunxi_daudio.o
obj-$(CONFIG_SND_SOC_SUNXI_DAUDIO)		+= snd_soc_sunxi_daudio.o

#snd_soc_sunxi_ahub-objs			+= snd_sunxi_ahub.o
#obj-$(CONFIG_SND_SOC_SUNXI_AHUB)		+= snd_soc_sunxi_ahub.o

# ahub tmp
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_TMP)		+= ahub/
