/*
 * sound\soc\sunxi\sun50iw12-codec.c
 * (C) Copyright 2019-2021
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * luguofang <luguofang@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/pinctrl-sunxi.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/reset.h>
#include <asm/dma.h>
#include <sound/pcm.h>
#include <sound/dmaengine_pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/core.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/sunxi-gpio.h>

#include "sun50iw12-codec.h"

static const struct sample_rate sample_rate_conv[] = {
	{8000,   5},
	{11025,  4},
	{12000,  4},
	{16000,  3},
	{22050,  2},
	{24000,  2},
	{32000,  1},
	{44100,  0},
	{48000,  0},
	{96000,  7},
	{192000, 6},
};

static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -11925, 75, 0);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -11925, 75, 0);
static const DECLARE_TLV_DB_SCALE(digital_tlv, -7424, 116, 0);
static const DECLARE_TLV_DB_SCALE(hpout_tlv, -4200, 600, 0);
static const unsigned int lineout_tlv[] = {
	TLV_DB_RANGE_HEAD(2),
	0, 1, TLV_DB_SCALE_ITEM(0, 0, 1),
	2, 31, TLV_DB_SCALE_ITEM(-4350, 150, 1),
};

static struct reg_label codec_reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_VOL_CTRL),
	REG_LABEL(SUNXI_DAC_FIFOC),
	REG_LABEL(SUNXI_DAC_FIFOS),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG),

	REG_LABEL(SUNXI_ADC_FIFOC),
	REG_LABEL(SUNXI_ADC_VOL_CTRL),
	REG_LABEL(SUNXI_ADC_FIFOS),
	REG_LABEL(SUNXI_ADC_CNT),
	REG_LABEL(SUNXI_ADC_DG),

	REG_LABEL(SUNXI_DAC_DAP_CTL),
	REG_LABEL(SUNXI_ADC_DAP_CTL),

	REG_LABEL(SUNXI_ADCL_REG),
	REG_LABEL(SUNXI_ADCR_REG),
	REG_LABEL(SUNXI_DAC_REG),
	REG_LABEL(SUNXI_MICBIAS_REG),
	REG_LABEL(SUNXI_BIAS_REG),
	REG_LABEL(SUNXI_HP_REG),
	REG_LABEL(SUNXI_HMIC_CTRL),
	REG_LABEL(SUNXI_HMIC_STS),
	REG_LABEL_END,
};

static struct reg_label i2s_reg_labels[] = {
	REG_LABEL(INTER_I2S_CTL),
	REG_LABEL(INTER_I2S_FMT0),
	REG_LABEL(INTER_I2S_FMT1),
	REG_LABEL(INTER_I2S_CLKDIV),
	REG_LABEL(INTER_I2S_CHCFG),

	REG_LABEL(INTER_I2S_TX0CHSEL),
	REG_LABEL(INTER_I2S_TX1CHSEL),
	REG_LABEL(INTER_I2S_TX2CHSEL),
	REG_LABEL(INTER_I2S_TX3CHSEL),
	REG_LABEL(INTER_I2S_TX0CHMAP0),
	REG_LABEL(INTER_I2S_TX0CHMAP1),
	REG_LABEL(INTER_I2S_TX1CHMAP0),
	REG_LABEL(INTER_I2S_TX1CHMAP1),
	REG_LABEL(INTER_I2S_TX2CHMAP0),
	REG_LABEL(INTER_I2S_TX2CHMAP1),
	REG_LABEL(INTER_I2S_TX3CHMAP0),
	REG_LABEL(INTER_I2S_TX3CHMAP1),

	REG_LABEL(INTER_I2S_RXCHSEL),
	REG_LABEL(INTER_I2S_RXCHMAP0),
	REG_LABEL(INTER_I2S_RXCHMAP1),
	REG_LABEL(INTER_I2S_RXCHMAP2),
	REG_LABEL(INTER_I2S_RXCHMAP3),
	REG_LABEL_END,
};

#if 0
static void adcdrc_config(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	/* Enable DRC gain Min and Max limit, detect noise, Using Peak Filter */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_ADC_DRC_CTRL,
			((0x1 << ADC_DRC_DELAY_BUF_EN) |
			(0x1 << ADC_DRC_GAIN_MAX_EN) |
			(0x1 << ADC_DRC_GAIN_MIN_EN) |
			(0x1 << ADC_DRC_NOISE_DET_EN) |
			(0x1 << ADC_DRC_SIGNAL_SEL) |
			(0x1 << ADC_DRC_LT_EN) |
			(0x1 << ADC_DRC_ET_EN)),
			((0x1 << ADC_DRC_DELAY_BUF_EN) |
			(0x1 << ADC_DRC_GAIN_MAX_EN) |
			(0x1 << ADC_DRC_GAIN_MIN_EN) |
			(0x1 << ADC_DRC_NOISE_DET_EN) |
			(0x1 << ADC_DRC_SIGNAL_SEL) |
			(0x1 << ADC_DRC_LT_EN) |
			(0x1 << ADC_DRC_ET_EN)));

	/* Left peak filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFHAT, (0x000B77F0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFLAT, 0x000B77F0 & 0xFFFF);
	/* Right peak filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFHAT, (0x000B77F0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFLAT, 0x000B77F0 & 0xFFFF);
	/* Left peak filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFHAT, (0x00012BB0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LPFLAT, 0x00012BB0 & 0xFFFF);
	/* Right RMS filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFHAT, (0x00012BB0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_RPFLAT, 0x00012BB0 & 0xFFFF);

	/* OPL */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HOPL, (0xFF641741 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LOPL, 0xFF641741 & 0xFFFF);
	/* OPC */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HOPC, (0xFC0380F3 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LOPC, 0xFC0380F3 & 0xFFFF);
	/* OPE */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HOPE, (0xF608C25F >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LOPE, 0xF608C25F & 0xFFFF);
	/* LT */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HLT, (0x035269E0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LLT, 0x035269E0 & 0xFFFF);
	/* CT */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HCT, (0x06B3002C >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LCT, 0x06B3002C & 0xFFFF);
	/* ET */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HET, (0x0A139682 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LET, 0x0A139682 & 0xFFFF);
	/* Ki */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HKI, (0x00222222 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LKI, 0x00222222 & 0xFFFF);
	/* Kc */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HKC, (0x01000000 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LKC, 0x01000000 & 0xFFFF);
	/* Kn */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HKN, (0x01C53EF0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LKN, 0x01C53EF0 & 0xFFFF);
	/* Ke */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HKE, (0x04234F68 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LKE, 0x04234F68 & 0xFFFF);

	/* smooth filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_SFHAT, (0x00017665 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_SFLAT, 0x00017665 & 0xFFFF);
	/* gain smooth filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* gain max setting */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_MXGHS, (0x69E0F95B >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_MXGLS, 0x69E0F95B & 0xFFFF);

	/* gain min setting */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_MNGHS, (0xF95B2C3F >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_MNGLS, 0xF95B2C3F & 0xFFFF);

	/* smooth filter release and attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_EPSHC, (0x00025600 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_EPSHC, 0x00025600 & 0xFFFF);
}

static void adcdrc_enable(struct snd_soc_component *component, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_dap *adc_dap = &sunxi_codec->adc_dap;

	if (on) {
		if (adc_dap->drc_enable++ == 0) {
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DRC0_EN), (0x1 << ADC_DRC0_EN));
			if (sunxi_codec->adc_dap_enable++ == 0) {
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
					(0x1 << ADC_DAP0_EN), (0x1 << ADC_DAP0_EN));
			}
		}
	} else {
		if (--adc_dap->drc_enable <= 0) {
			adc_dap->drc_enable = 0;
			if (--sunxi_codec->adc_dap_enable <= 0) {
				sunxi_codec->adc_dap_enable = 0;
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
					(0x1 << ADC_DAP0_EN), (0x0 << ADC_DAP0_EN));
			}
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DRC0_EN), (0x0 << ADC_DRC0_EN));
		}
	}
}

static void adchpf_config(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	/* HPF */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_ADC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void adchpf_enable(struct snd_soc_component *component, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_dap *adc_dap = &sunxi_codec->adc_dap;

	if (on) {
		if (adc_dap->hpf_enable++ == 0) {
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_HPF0_EN), (0x1 << ADC_HPF0_EN));
			if (sunxi_codec->adc_dap_enable++ == 0) {
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
					(0x1 << ADC_DAP0_EN), (0x1 << ADC_DAP0_EN));
			}
		}
	} else {
		if (--adc_dap->hpf_enable <= 0) {
			adc_dap->hpf_enable = 0;
			if (--sunxi_codec->adc_dap_enable <= 0) {
				sunxi_codec->adc_dap_enable = 0;
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
					(0x1 << ADC_DAP0_EN), (0x0 << ADC_DAP0_EN));
			}
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_HPF0_EN), (0x0 << ADC_HPF0_EN));
		}
	}
}

static void dacdrc_config(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	/* Left peak filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RPFHAT, (0x000B77F0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RPFLAT, 0x000B77F0 & 0xFFFF);

	/* Left peak filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LRMSHAT, (0x00012BB0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LRMSLAT, 0x00012BB0 & 0xFFFF);
	/* Right RMS filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RRMSHAT, (0x00012BB0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_RRMSLAT, 0x00012BB0 & 0xFFFF);

	/* smooth filter attack time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_SFHAT, (0x00017665 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_SFLAT, 0x00017665 & 0xFFFF);
	/* gain smooth filter release time */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HOPL, (0xFE56CB10 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LOPL, 0xFE56CB10 & 0xFFFF);
	/* OPC */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HOPC, (0xFB04612F >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LOPC, 0xFB04612F & 0xFFFF);
	/* OPE */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HOPE, (0xF608C25F >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LOPE, 0xF608C25F & 0xFFFF);
	/* LT */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HLT, (0x035269E0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LLT, 0x035269E0 & 0xFFFF);
	/* CT */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HCT, (0x06B3002C >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LCT, 0x06B3002C & 0xFFFF);
	/* ET */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HET, (0x0A139682 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LET, 0x0A139682 & 0xFFFF);
	/* Ki */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HKI, (0x00400000 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LKI, 0x00400000 & 0xFFFF);
	/* Kc */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HKC, (0x00FBCDA5 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LKC, 0x00FBCDA5 & 0xFFFF);
	/* Kn */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HKN, (0x0179B472 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LKN, 0x0179B472 & 0xFFFF);
	/* Ke */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HKE, (0x04234F68 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LKE, 0x04234F68 & 0xFFFF);
	/* MXG */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_MXGHS, (0x035269E0 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_MXGLS, 0x035269E0 & 0xFFFF);
	/* MNG */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_MNGHS, (0xF95B2C3F >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_MNGLS, 0xF95B2C3F & 0xFFFF);
	/* EPS */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_EPSHC, (0x00025600 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_EPSLC, 0x00025600 & 0xFFFF);
}

static void dacdrc_enable(struct snd_soc_component *component, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_dap *dac_dap = &sunxi_codec->dac_dap;

	if (on) {
		if (dac_dap->drc_enable++ == 0) {
			/* detect noise when ET enable */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_NOISE_DET_EN),
				(0x1 << DAC_DRC_NOISE_DET_EN));

			/* 0x0:RMS filter; 0x1:Peak filter */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_SIGNAL_SEL),
				(0x1 << DAC_DRC_SIGNAL_SEL));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_GAIN_MAX_EN),
				(0x1 << DAC_DRC_GAIN_MAX_EN));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_GAIN_MIN_EN),
				(0x1 << DAC_DRC_GAIN_MIN_EN));

			/* delay function enable */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_DELAY_BUF_EN),
				(0x1 << DAC_DRC_DELAY_BUF_EN));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_LT_EN),
				(0x1 << DAC_DRC_LT_EN));
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_ET_EN),
				(0x1 << DAC_DRC_ET_EN));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_DRC_EN),
				(0x1 << DDAP_DRC_EN));

			if (sunxi_codec->dac_dap_enable++ == 0)
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
					(0x1 << DDAP_EN), (0x1 << DDAP_EN));
		}
	} else {
		if (--dac_dap->drc_enable <= 0) {
			dac_dap->drc_enable = 0;
			if (--sunxi_codec->dac_dap_enable <= 0) {
				sunxi_codec->dac_dap_enable = 0;
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
					(0x1 << DDAP_EN), (0x0 << DDAP_EN));
			}

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_DRC_EN),
				(0x0 << DDAP_DRC_EN));

			/* detect noise when ET enable */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_NOISE_DET_EN),
				(0x0 << DAC_DRC_NOISE_DET_EN));

			/* 0x0:RMS filter; 0x1:Peak filter */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_SIGNAL_SEL),
				(0x0 << DAC_DRC_SIGNAL_SEL));

			/* delay function enable */
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_DELAY_BUF_EN),
				(0x0 << DAC_DRC_DELAY_BUF_EN));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_GAIN_MAX_EN),
				(0x0 << DAC_DRC_GAIN_MAX_EN));
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_GAIN_MIN_EN),
				(0x0 << DAC_DRC_GAIN_MIN_EN));

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_LT_EN),
				(0x0 << DAC_DRC_LT_EN));
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_CTRL,
				(0x1 << DAC_DRC_ET_EN),
				(0x0 << DAC_DRC_ET_EN));
		}
	}
}

static void dachpf_config(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	/* HPF */
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	regmap_write(sunxi_codec->codec_regmap, SUNXI_DAC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void dachpf_enable(struct snd_soc_component *component, bool on)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_dap *dac_dap = &sunxi_codec->dac_dap;

	if (on) {
		if (dac_dap->hpf_enable++ == 0) {
			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_HPF_EN), (0x1 << DDAP_HPF_EN));

			if (sunxi_codec->dac_dap_enable++ == 0)
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
					(0x1 << DDAP_EN), (0x1 << DDAP_EN));
		}
	} else {
		if (--dac_dap->hpf_enable <= 0) {
			dac_dap->hpf_enable = 0;
			if (--sunxi_codec->dac_dap_enable <= 0) {
				sunxi_codec->dac_dap_enable = 0;
				regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
					(0x1 << DDAP_EN), (0x0 << DDAP_EN));
			}

			regmap_update_bits(sunxi_codec->codec_regmap, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_HPF_EN),
				(0x0 << DDAP_HPF_EN));
		}
	}
}
#endif

/* sunxi codec hub mdoe select */
static int sunxi_codec_get_hub_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int reg_val = 0;

	regmap_read(sunxi_codec->codec_regmap, SUNXI_DAC_DPC, &reg_val);

	ucontrol->value.integer.value[0] =
				((reg_val & (0x1 << DAC_HUB_EN)) ? 1 : 0);

	return 0;
}

static int sunxi_codec_set_hub_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (ucontrol->value.integer.value[0]) {
	case	0:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_HUB_EN),
				(0x0 << DAC_HUB_EN));
		break;
	case	1:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_HUB_EN),
				(0x1 << DAC_HUB_EN));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static const char * const sunxi_codec_hub_function[] = {
				"hub_disable", "hub_enable"};

static const struct soc_enum sunxi_codec_hub_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(sunxi_codec_hub_function),
			sunxi_codec_hub_function),
};

/* sunxi codec dac swap func */
static int sunxi_codec_get_dacswap_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int reg_val = 0;

	regmap_read(sunxi_codec->codec_regmap, SUNXI_DAC_DG, &reg_val);

	ucontrol->value.integer.value[0] =
				((reg_val & (0x1 << DAC_SWAP)) ? 1 : 0);

	return 0;
}

static int sunxi_codec_set_dacswap_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (ucontrol->value.integer.value[0]) {
	case	0:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DG, (0x1 << DAC_SWAP),
				(0x0 << DAC_SWAP));
		break;
	case	1:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DG, (0x1 << DAC_SWAP),
				(0x1 << DAC_SWAP));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const char * const sunxi_codec_dacswap_function[] = {
				"Off", "On"};

static const struct soc_enum sunxi_codec_dacswap_func_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(sunxi_codec_dacswap_function),
			sunxi_codec_dacswap_function),
};

/* sunxi codec adc swap func */
static int sunxi_codec_get_adcswap_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int reg_val = 0;

	regmap_read(sunxi_codec->codec_regmap, SUNXI_ADC_DG, &reg_val);

	ucontrol->value.integer.value[0] =
				((reg_val & (0x1 << ADC_SWAP)) ? 1 : 0);

	return 0;
}

static int sunxi_codec_set_adcswap_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (ucontrol->value.integer.value[0]) {
	case	0:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_DG, (0x1 << ADC_SWAP),
				(0x0 << ADC_SWAP));
		break;
	case	1:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_DG, (0x1 << ADC_SWAP),
				(0x1 << ADC_SWAP));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const char * const sunxi_codec_adcswap_function[] = {
				"Off", "On"};

static const struct soc_enum sunxi_codec_adcswap_func_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(sunxi_codec_adcswap_function),
			sunxi_codec_adcswap_function),
};

/* sunxi codec dac src select */
static int sunxi_codec_get_dacsrc_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int reg_val = 0;

	regmap_read(sunxi_codec->codec_regmap, SUNXI_DAC_DPC, &reg_val);

	ucontrol->value.integer.value[0] =
				((reg_val & (0x1 << DAC_SRC_SEL)) ? 1 : 0);

	sunxi_codec->dac_data_src = ucontrol->value.integer.value[0];

	return 0;
}

static int sunxi_codec_set_dacsrc_mode(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (ucontrol->value.integer.value[0]) {
	case	0:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_SRC_SEL),
				(0x0 << DAC_SRC_SEL));

		sunxi_codec->dac_data_src = 0;
		break;
	case	1:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_SRC_SEL),
				(0x1 << DAC_SRC_SEL));

		sunxi_codec->dac_data_src = 1;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const char * const sunxi_codec_dacsrc_select[] = {
				"APB", "I2S"};

static const struct soc_enum sunxi_codec_dacsrc_sel_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(sunxi_codec_dacsrc_select),
			sunxi_codec_dacsrc_select),
};

/* SPK Output Control */
static int sunxi_codec_speaker_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_cfg);
//	struct codec_hw_config *hw_cfg = &(sunxi_codec->hw_config);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		if (spk_cfg->spk_used) {
			gpio_direction_output(spk_cfg->spk_gpio, 1);
			gpio_set_value(spk_cfg->spk_gpio, spk_cfg->pa_level);
			/* time delay to wait spk pa work fine */
			msleep(spk_cfg->pa_msleep);
		}
		break;
	case	SND_SOC_DAPM_PRE_PMD:
		if (spk_cfg->spk_used) {
			gpio_set_value(spk_cfg->spk_gpio, !(spk_cfg->pa_level));
		}
		break;
	default:
		break;
	}

	return 0;
}

/* HPOUT Output Control */
static int sunxi_codec_hpout_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k,	int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
//	struct codec_hw_config *hw_cfg = &(sunxi_codec->hw_config);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
#if 0
		if (hw_cfg->dacdrc_cfg & DAP_HP_EN)
			dacdrc_enable(component, 1);
		if (hw_cfg->dachpf_cfg & DAP_HP_EN)
			dachpf_enable(component, 1);
#endif
		/*open*/
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPINPUTEN),
				(0x1 << HPINPUTEN));
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPOUTPUTEN),
				(0x1 << HPOUTPUTEN));
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPPA_EN),
				(0x1 << HPPA_EN));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/*close*/
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPPA_EN),
				(0x0 << HPPA_EN));
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPOUTPUTEN),
				(0x0 << HPOUTPUTEN));
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_HP_REG, (0x1 << HPINPUTEN),
				(0x0 << HPINPUTEN));

#if 0
		if (hw_cfg->dacdrc_cfg & DAP_HP_EN)
			dacdrc_enable(component, 0);
		if (hw_cfg->dachpf_cfg & DAP_HP_EN)
			dachpf_enable(component, 0);
#endif
		break;
	}

	return 0;
}

/* LINEOUT Output Control */
static int sunxi_codec_lineout_event(struct snd_soc_dapm_widget *w,
				  struct snd_kcontrol *k, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
//	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_cfg);
//	struct codec_hw_config *hw_cfg = &(sunxi_codec->hw_config);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
#if 0
		if (hw_cfg->dacdrc_cfg & DAP_SPK_EN)
			dacdrc_enable(component, 1);
		if (hw_cfg->dachpf_cfg & DAP_SPK_EN)
			dachpf_enable(component, 1);
#endif
		/* DACL to left channel LINEOUT Mute control 0:mute 1: not mute */
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_REG, (0x1 << DAC_LMUTE),
				(0x1 << DAC_LMUTE));

		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_REG, (0x1 << DAC_LINEOUTLEN),
				(0x1 << DAC_LINEOUTLEN));
		break;
	case	SND_SOC_DAPM_PRE_PMD:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_REG, (0x1 << DAC_LINEOUTLEN),
				(0x0 << DAC_LINEOUTLEN));

		/* DACL to left channel LINEOUT Mute control 0:mute 1: not mute */
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_REG, (0x1 << DAC_LMUTE),
				(0x0 << DAC_LMUTE));

#if 0
		if (hw_cfg->dacdrc_cfg & DAP_SPK_EN)
			dacdrc_enable(component, 0);
		if (hw_cfg->dachpf_cfg & DAP_SPK_EN)
			dachpf_enable(component, 0);
#endif
		break;
	default:
		break;
	}

	return 0;
}

/* Digital DAC Enable */
static int sunxi_codec_playback_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (event) {
	case	SND_SOC_DAPM_PRE_PMU:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_EN),
				(0x1 << DAC_EN));
		msleep(30);
		break;
	case	SND_SOC_DAPM_POST_PMD:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_DPC, (0x1 << DAC_EN),
				(0x0 << DAC_EN));
		break;
	default:
		break;
	}

	return 0;
}

/* Digital ADC Enable */
static int sunxi_codec_capture_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (event) {
	case	SND_SOC_DAPM_POST_PMU:
		/* delay 80ms to avoid the capture pop at the beginning */
		mdelay(80);
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_FIFOC, (0x1 << ADC_EN),
				(0x1 << ADC_EN));
		break;
	case	SND_SOC_DAPM_POST_PMD:
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_FIFOC, (0x1 << ADC_EN),
				(0x0 << ADC_EN));
		break;
	default:
		break;
	}

	return 0;
}

static const struct snd_kcontrol_new sunxi_codec_controls[] = {
	/* Audio Hub Output Enable */
	SOC_ENUM_EXT("Audio Hub Output", sunxi_codec_hub_mode_enum[0],
				sunxi_codec_get_hub_mode,
				sunxi_codec_set_hub_mode),
	/* Audio DAC Channel Output Swap */
	SOC_ENUM_EXT("DAC Chan Swap", sunxi_codec_dacswap_func_enum[0],
				sunxi_codec_get_dacswap_mode,
				sunxi_codec_set_dacswap_mode),
	/* Audio ADC Channel Input Swap */
	SOC_ENUM_EXT("ADC Chan Swap", sunxi_codec_adcswap_func_enum[0],
				sunxi_codec_get_adcswap_mode,
				sunxi_codec_set_adcswap_mode),
	/* Codec DAC Output Data Src Select */
	SOC_ENUM_EXT("DAC Src Select", sunxi_codec_dacsrc_sel_enum[0],
				sunxi_codec_get_dacsrc_mode,
				sunxi_codec_set_dacsrc_mode),
	/* Digital Volume */
	SOC_SINGLE_TLV("digital volume", SUNXI_DAC_DPC,
					DAC_DVOL, 0x3F, 1, digital_tlv),
	/* DAC Volume */
	SOC_DOUBLE_TLV("DAC Volume", SUNXI_DAC_VOL_CTRL, DAC_VOL_L, DAC_VOL_R,
		       0xFF, 0, dac_vol_tlv),
	/* ADC Volume */
	SOC_DOUBLE_TLV("ADC Volume", SUNXI_ADC_VOL_CTRL, ADC_VOL_L, ADC_VOL_R,
		       0xFF, 0, adc_vol_tlv),
	/* LINEOUT Volume */
	SOC_SINGLE_TLV("LINEOUT Volume", SUNXI_DAC_REG, DAC_LINEOUT_VOL,
			0x1F, 0, lineout_tlv),
	/* HPOUT Volume */
	SOC_SINGLE_TLV("HPOUT Volume", SUNXI_DAC_REG, DAC_HP_GAIN,
			0x7, 0, hpout_tlv),
};

/* lineinl controls */
static const char * const lineinl_src_text[] = {"NULL", "LINEIN2", "LINEIN1"};

static const struct soc_enum lineinl_src_enum =
	SOC_ENUM_SINGLE(SUNXI_ADCL_REG, ADCL_LINEINL_SEL,
			ARRAY_SIZE(lineinl_src_text), lineinl_src_text);

static const struct snd_kcontrol_new lineinl_src_mux =
	SOC_DAPM_ENUM("LINEINL", lineinl_src_enum);

/* lineinr controls */
static const char * const lineinr_src_text[] = {"NULL", "LINEIN2", "LINEIN1"};

static const struct soc_enum lineinr_src_enum =
	SOC_ENUM_SINGLE(SUNXI_ADCR_REG, ADCR_LINEINR_SEL,
			ARRAY_SIZE(lineinr_src_text), lineinr_src_text);

static const struct snd_kcontrol_new lineinr_src_mux =
	SOC_DAPM_ENUM("LINEINR", lineinr_src_enum);

/*audio dapm widget */
static const struct snd_soc_dapm_widget sunxi_codec_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN_E("DACL", "Playback", 0, SUNXI_DAC_REG,
				DACL_EN, 0,
				sunxi_codec_playback_event,
				SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_AIF_IN_E("DACR", "Playback", 0, SUNXI_DAC_REG,
				DACR_EN, 0,
				sunxi_codec_playback_event,
				SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_AIF_OUT_E("ADCL", "Capture", 0, SUNXI_ADCL_REG,
				ADCL_EN, 0,
				sunxi_codec_capture_event,
				SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_AIF_OUT_E("ADCR", "Capture", 0, SUNXI_ADCR_REG,
				ADCR_EN, 0,
				sunxi_codec_capture_event,
				SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MUX("LINEINL", SND_SOC_NOPM, 0, 0, &lineinl_src_mux),
	SND_SOC_DAPM_MUX("LINEINR", SND_SOC_NOPM, 0, 0, &lineinr_src_mux),

	SND_SOC_DAPM_PGA("LINEINL PGA", SUNXI_ADCL_REG, ADCL_LINEINL_EN, 0, NULL, 0),
	SND_SOC_DAPM_PGA("LINEINR PGA", SUNXI_ADCR_REG, ADCR_LINEINR_EN, 0, NULL, 0),

//	SND_SOC_DAPM_MICBIAS("MainMic Bias", SUNXI_MICBIAS_REG, MMICBIASEN, 0),

	SND_SOC_DAPM_LINE("LINEIN1L", NULL),
	SND_SOC_DAPM_LINE("LINEIN1R", NULL),
	SND_SOC_DAPM_LINE("LINEIN2L", NULL),
	SND_SOC_DAPM_LINE("LINEIN2R", NULL),

	SND_SOC_DAPM_OUTPUT("HPOUTL"),
	SND_SOC_DAPM_OUTPUT("HPOUTR"),

	SND_SOC_DAPM_OUTPUT("LINEOUTL"),
	SND_SOC_DAPM_OUTPUT("LINEOUTR"),

	SND_SOC_DAPM_HP("HPOUT", sunxi_codec_hpout_event),
	SND_SOC_DAPM_LINE("LINEOUT", sunxi_codec_lineout_event),
	SND_SOC_DAPM_SPK("Speaker", sunxi_codec_speaker_event),
};

static const struct snd_soc_dapm_route sunxi_codec_dapm_routes[] = {
	/* LINEIN Input Route */
	{"LINEINL", "LINEIN1", "LINEIN1L"},
	{"LINEINL", "LINEIN2", "LINEIN2L"},
	{"LINEINR", "LINEIN1", "LINEIN1R"},
	{"LINEINR", "LINEIN2", "LINEIN2R"},

	{"LINEINL PGA", NULL, "LINEINL"},
	{"LINEINR PGA", NULL, "LINEINR"},

	{"ADCL", NULL, "LINEINL PGA"},
	{"ADCR", NULL, "LINEINR PGA"},

	/* LINEOUT Output Route */
	{"LINEOUTL", NULL, "DACL"},
	{"LINEOUTR", NULL, "DACR"},

	{"LINEOUT", NULL, "LINEOUTL"},
	{"LINEOUT", NULL, "LINEOUTR"},

	{"Speaker", NULL, "LINEOUT"},

	/* HPOUT Output Route */
	{"HPOUTL", NULL, "DACL"},
	{"HPOUTR", NULL, "DACR"},

	{"HPOUT", NULL, "HPOUTL"},
	{"HPOUT", NULL, "HPOUTR"},

	{"Speaker", NULL, "HPOUT"},
};

static void sunxi_codec_init(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	/* DAC_VOL_SEL default disabled */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_VOL_CTRL, (0x1 << DAC_VOL_SEL),
			(0x1 << DAC_VOL_SEL));

	/* ADC_VOL_SEL default disabled */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_ADC_VOL_CTRL, (0x1 << ADC_VOL_SEL),
			(0x1 << ADC_VOL_SEL));

	/* Enable ADCFDT to overcome niose at the beginning */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_ADC_FIFOC, (0x7 << ADC_DFEN),
			(0x7 << ADC_DFEN));

	/* Digital VOL defeult setting */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_DPC, 0x3F << DAC_DVOL,
			sunxi_codec->digital_vol << DAC_DVOL);

	/* LINEOUT VOL defeult setting */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_REG, 0x1F << DAC_LINEOUT_VOL,
			sunxi_codec->lineout_vol << DAC_LINEOUT_VOL);

	/* Headphone Gain defeult setting */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_REG, 0x7 << DAC_HP_GAIN,
			sunxi_codec->hpout_vol << DAC_HP_GAIN);

	/* ADCL/R IOP params default setting */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_ADCL_REG, 0xFF << ADCL_IOPMICL,
			0x55 << ADCL_IOPMICL);
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_ADCR_REG, 0xFF << ADCR_IOPMICL,
			0x55 << ADCR_IOPMICL);

	/* For improve performance of THD+N about HP */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_HP_REG, (0x3 << CP_CLKS),
			(0x2 << CP_CLKS));

	/* LINEOUT Output ways default setting: differential output */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_REG, 0x1 << DAC_LINEOUTLDIFFEN,
			0x1 << DAC_LINEOUTLDIFFEN);
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_REG, 0x1 << DAC_LINEOUTRDIFFEN,
			0x1 << DAC_LINEOUTRDIFFEN);

	/* To fix some pop noise */
	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_HP_REG, (0x1 << HPCALIFIRST),
			(0x1 << HPCALIFIRST));

	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_HP_REG, (0x3 << HPPA_DEL),
			(0x3 << HPPA_DEL));

	regmap_update_bits(sunxi_codec->codec_regmap,
			SUNXI_DAC_REG, (0x3 << DAC_CPLDO_VOLTAGE),
			(0x1 << DAC_CPLDO_VOLTAGE));

#if 0
	if (sunxi_codec->hw_config.adcdrc_cfg)
		adcdrc_config(component);
	if (sunxi_codec->hw_config.adchpf_cfg)
		adchpf_config(component);

	if (sunxi_codec->hw_config.dacdrc_cfg)
		dacdrc_config(component);
	if (sunxi_codec->hw_config.dachpf_cfg)
		dachpf_config(component);
#endif
}

static int sunxi_codec_i2s_clk_setting(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct sunxi_codec_i2s_params *i2s_params = &sunxi_codec->i2s_params;
	unsigned int mclk_div;

	/* inter i2s mclk div setting */
	if (i2s_params->mclk_div) {
		switch (i2s_params->mclk_div) {
		case	1:
			mclk_div = INTER_I2S_MCLK_DIV_1;
			break;
		case	2:
			mclk_div = INTER_I2S_MCLK_DIV_2;
			break;
		case	4:
			mclk_div = INTER_I2S_MCLK_DIV_3;
			break;
		case	6:
			mclk_div = INTER_I2S_MCLK_DIV_4;
			break;
		case	8:
			mclk_div = INTER_I2S_MCLK_DIV_5;
			break;
		case	12:
			mclk_div = INTER_I2S_MCLK_DIV_6;
			break;
		case	16:
			mclk_div = INTER_I2S_MCLK_DIV_7;
			break;
		case	24:
			mclk_div = INTER_I2S_MCLK_DIV_8;
			break;
		case	32:
			mclk_div = INTER_I2S_MCLK_DIV_9;
			break;
		case	48:
			mclk_div = INTER_I2S_MCLK_DIV_10;
			break;
		case	64:
			mclk_div = INTER_I2S_MCLK_DIV_11;
			break;
		case	96:
			mclk_div = INTER_I2S_MCLK_DIV_12;
			break;
		case	128:
			mclk_div = INTER_I2S_MCLK_DIV_13;
			break;
		case	176:
			mclk_div = INTER_I2S_MCLK_DIV_14;
			break;
		case	192:
			mclk_div = INTER_I2S_MCLK_DIV_15;
			break;
		default:
			LOG_ERR("unsupport mclk_div\n");
			return -EINVAL;
		}

		/* setting Mclk output as external codec input clk */
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CLKDIV, (0xF << I2S_MCLKDIV),
				(mclk_div << I2S_MCLKDIV));

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CLKDIV, (0x1 << I2S_MCLKOUT_EN),
				(0x1 << I2S_MCLKOUT_EN));
	} else {
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CLKDIV, (0x1 << I2S_MCLKOUT_EN),
				(0x0 << I2S_MCLKOUT_EN));
	}

	return 0;
}

static int sunxi_codec_i2s_fmt_setting(struct snd_soc_component *component,
							unsigned int fmt)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int lrck_polarity, brck_polarity;
	unsigned int mode;

	/* internal i2s part fmt setting */
	/* internal i2s master / slave mode setting */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case	SND_SOC_DAIFMT_CBM_CFM:
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_LRCK_OUT),
				(0x0 << I2S_LRCK_OUT));

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_BCLK_OUT),
				(0x0 << I2S_BCLK_OUT));
		break;
	case	SND_SOC_DAIFMT_CBS_CFS:
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_LRCK_OUT),
				(0x1 << I2S_LRCK_OUT));

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_BCLK_OUT),
				(0x1 << I2S_BCLK_OUT));
		break;
	default:
		LOG_ERR("inter i2s part unknown master/slave format\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case	SND_SOC_DAIFMT_LEFT_J:
		mode = INTER_I2S_MODE_CTL_LEFT;
		break;
	case	SND_SOC_DAIFMT_RIGHT_J:
		mode = INTER_I2S_MODE_CTL_RIGHT;
		break;
	default:
		LOG_ERR("inter i2s format setting failed, maybe doesn't support it\n");
		return -EINVAL;
	}

	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_CTL, (0x3 << I2S_MODE_SEL),
			(mode << I2S_MODE_SEL));

	/* linux-4.9 kernel: SND_SOC_DAIFMT_NB_NF: 1 */
	/* linux-5.4 kernel: SND_SOC_DAIFMT_NB_NF: 0 */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case	SND_SOC_DAIFMT_NB_NF:
		lrck_polarity = INTER_I2S_LRCK_POLARITY_NOR;
		brck_polarity = INTER_I2S_BCLK_POLARITY_NOR;
		break;
	case	SND_SOC_DAIFMT_NB_IF:
		lrck_polarity = INTER_I2S_LRCK_POLARITY_INV;
		brck_polarity = INTER_I2S_BCLK_POLARITY_NOR;
		break;
	case	SND_SOC_DAIFMT_IB_NF:
		lrck_polarity = INTER_I2S_LRCK_POLARITY_NOR;
		brck_polarity = INTER_I2S_BCLK_POLARITY_INV;
		break;
	case	SND_SOC_DAIFMT_IB_IF:
		lrck_polarity = INTER_I2S_LRCK_POLARITY_INV;
		brck_polarity = INTER_I2S_BCLK_POLARITY_INV;
		break;
	default:
		LOG_ERR("inter i2s part: invert clk setting failed!\n");
		return -EINVAL;
	}

	if (((fmt & SND_SOC_DAIFMT_FORMAT_MASK) == SND_SOC_DAIFMT_DSP_A) ||
		((fmt & SND_SOC_DAIFMT_FORMAT_MASK) == SND_SOC_DAIFMT_DSP_B)) {
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_FMT0, (0x1 << I2S_LRCK_POLARITY),
				((lrck_polarity^1) << I2S_LRCK_POLARITY));
	} else {
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_FMT0, (0x1 << I2S_LRCK_POLARITY),
				(lrck_polarity << I2S_LRCK_POLARITY));
	}

	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT0, (0x1 << I2S_BCLK_POLARITY),
			(brck_polarity << I2S_BCLK_POLARITY));

	return 0;
}

static int sunxi_codec_i2s_clkdiv_setting(struct snd_soc_component *component,
								int clk_div)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct sunxi_codec_i2s_params *i2s_params = &sunxi_codec->i2s_params;
	unsigned int bclk_div, div_ratio;

	/* internal i2s part clkdiv setting */
	if (i2s_params->tdm_config) {
		/* I2S/TDM two channel mode */
		div_ratio = clk_div / (i2s_params->pcm_lrck_period * 2);
	} else {
		/* PCM mode */
		div_ratio = clk_div / i2s_params->pcm_lrck_period;
	}

	switch (div_ratio) {
	case	1:
		bclk_div = INTER_I2S_BCLK_DIV_1;
		break;
	case	2:
		bclk_div = INTER_I2S_BCLK_DIV_2;
		break;
	case	4:
		bclk_div = INTER_I2S_BCLK_DIV_3;
		break;
	case	6:
		bclk_div = INTER_I2S_BCLK_DIV_4;
		break;
	case	8:
		bclk_div = INTER_I2S_BCLK_DIV_5;
		break;
	case	12:
		bclk_div = INTER_I2S_BCLK_DIV_6;
		break;
	case	16:
		bclk_div = INTER_I2S_BCLK_DIV_7;
		break;
	case	24:
		bclk_div = INTER_I2S_BCLK_DIV_8;
		break;
	case	32:
		bclk_div = INTER_I2S_BCLK_DIV_9;
		break;
	case	48:
		bclk_div = INTER_I2S_BCLK_DIV_10;
		break;
	case	64:
		bclk_div = INTER_I2S_BCLK_DIV_11;
		break;
	case	96:
		bclk_div = INTER_I2S_BCLK_DIV_12;
		break;
	case	128:
		bclk_div = INTER_I2S_BCLK_DIV_13;
		break;
	case	176:
		bclk_div = INTER_I2S_BCLK_DIV_14;
		break;
	case	192:
		bclk_div = INTER_I2S_BCLK_DIV_15;
		break;
	default:
		LOG_ERR("inter i2s unsupport clk_div\n");
		return -EINVAL;
	}

	LOG_ERR("clk_div: %d, pcm_lrck_period: %d, div_ratio: %d, bclk_div: %d\n",
		clk_div, i2s_params->pcm_lrck_period, div_ratio, bclk_div);

	/* setting bclk div */
	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_CLKDIV, (0xF << I2S_BCLKDIV),
			(bclk_div << I2S_BCLKDIV));

	return 0;
}

static int sunxi_codec_i2s_init(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct sunxi_codec_i2s_params *i2s_params = &sunxi_codec->i2s_params;
	unsigned int ret = 0;

	i2s_params->i2s_global_enable = 0;

	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT0, (0x1 << I2S_LRCK_WIDTH),
			(i2s_params->frame_type << I2S_LRCK_WIDTH));

	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT0, (0x3FF << I2S_LRCK_PERIOD),
			((i2s_params->pcm_lrck_period - 1) << I2S_LRCK_PERIOD));

	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT0, (0x7 << I2S_SLOT_WIDTH),
			(((i2s_params->slot_width_select >> 2) - 1) << I2S_SLOT_WIDTH));

	/*
	 * MSB on the transmit format, always be first.
	 * default using Linear-PCM, without no companding.
	 * A-law<Eourpean standard> or U-law<US-Japan> not working ok.
	 */
	/* TX setting MSB first default */
	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT1, (0x1 << I2S_TXMLS),
			(0x0 << I2S_TXMLS));

	/* RX setting MSB first default */
	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT1, (0x1 << I2S_RXMLS),
			(0x0 << I2S_RXMLS));

	/* sign extend setting default */
	regmap_update_bits(sunxi_codec->i2s_regmap,
			INTER_I2S_FMT1, (0x3 << I2S_SEXT),
			(0x3 << I2S_SEXT));

	ret = sunxi_codec_i2s_clk_setting(component);
	if (ret) {
		LOG_ERR("inter i2s clk settine faild!\n");
	}

	ret = sunxi_codec_i2s_fmt_setting(component, (i2s_params->audio_format
		| (i2s_params->signal_inversion << 8)
		| (i2s_params->daudio_master << 12)));
	if (ret) {
		LOG_ERR("inter i2s clk settine faild!\n");
	}

	return ret;
}


static int sunxi_codec_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
#if 0
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (sunxi_codec->hw_config.adchpf_cfg)
			adchpf_enable(component, 1);
	}
#endif

	return 0;
}

static int sunxi_codec_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	int i = 0;

	switch (params_format(params)) {
	case	SNDRV_PCM_FORMAT_S16_LE:
		/* internal i2s part 16bit setting */
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_FMT0, (0x7 << I2S_SR),
				(0x3 << I2S_SR));

		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			/* codec part playback bit setting */
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x3 << DAC_FIFO_MODE),
					(0x3 << DAC_FIFO_MODE));
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x1 << DAC_SAMPLE_BITS),
					(0x0 << DAC_SAMPLE_BITS));
		} else {
			/* codec part capture bit setting */
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x1 << ADC_FIFO_MODE),
					(0x1 << ADC_FIFO_MODE));
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x1 << ADC_SAMPLE_BITS),
					(0x0 << ADC_SAMPLE_BITS));
		}
		break;
	case	SNDRV_PCM_FORMAT_S24_LE:
		/* internal i2s part 20bit setting */
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_FMT0, (0x7 << I2S_SR),
				(0x4 << I2S_SR));

		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			/* codec part playback bit setting */
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x3 << DAC_FIFO_MODE),
					(0x0 << DAC_FIFO_MODE));
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x1 << DAC_SAMPLE_BITS),
					(0x1 << DAC_SAMPLE_BITS));
		} else {
			/* codec part capture bit setting */
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x1 << ADC_FIFO_MODE),
					(0x0 << ADC_FIFO_MODE));
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x1 << ADC_SAMPLE_BITS),
					(0x1 << ADC_SAMPLE_BITS));
		}
		break;
	default:
		LOG_ERR("unsupprt format!\n");
		break;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_conv); i++) {
		if (sample_rate_conv[i].samplerate == params_rate(params)) {
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x7 << DAC_FS),
					(sample_rate_conv[i].rate_bit << DAC_FS));
			} else {
				if (sample_rate_conv[i].samplerate > 48000) {
					return -EINVAL;
				}
				regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x7 << ADC_FS),
					(sample_rate_conv[i].rate_bit<<ADC_FS));
			}
		}
	}

#if 0
	/* reset the adchpf func setting for different sampling */
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (sunxi_codec->hw_config.adchpf_cfg) {
			if (params_rate(params) == 16000) {
				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_HHPFC,
						(0x00F623A5 >> 16) & 0xFFFF);

				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_LHPFC,
						0x00F623A5 & 0xFFFF);
			} else if (params_rate(params) == 44100) {
				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_HHPFC,
						(0x00FC60DB >> 16) & 0xFFFF);
				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_LHPFC,
						0x00FC60DB & 0xFFFF);
			} else {
				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_HHPFC,
						(0x00FCABB3 >> 16) & 0xFFFF);
				regmap_write(sunxi_codec->codec_regmap,
						SUNXI_ADC_DRC_LHPFC,
						0x00FCABB3 & 0xFFFF);
			}
		}
	}
#endif

	/* internal codec part channels setting and mapping */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* default setting TX0 */
		regmap_write(sunxi_codec->i2s_regmap,
				INTER_I2S_TX0CHMAP0,
				0xFEDCBA98);

		regmap_write(sunxi_codec->i2s_regmap,
				INTER_I2S_TX0CHMAP1,
				0x76543210);

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_TX0CHSEL, (0xF << I2S_TX0_CHSEL),
				((params_channels(params)-1) << I2S_TX0_CHSEL));

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_TX0CHSEL, (0xFFFF << I2S_TX0_CHEN),
				((0x1 << params_channels(params))-1) << I2S_TX0_CHEN);

	} else {
		/* default setting RX */
		regmap_write(sunxi_codec->i2s_regmap,
				INTER_I2S_RXCHMAP3,
				0x03020100);

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_RXCHSEL, (0xF << I2S_RX_CHSEL),
				((params_channels(params)-1) << I2S_RX_CHSEL));
	}

	/* codec part channels setting */
	switch (params_channels(params)) {
	case 1:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x1 << DAC_MONO_EN),
					0x1 << DAC_MONO_EN);
		} else {
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x3 << ADC_CHAN_EN),
					(0x1 << ADC_CHAN_EN));
		}
		break;
	case 2:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_DAC_FIFOC, (0x1 << DAC_MONO_EN),
					(0x0 << DAC_MONO_EN));
		} else {
			regmap_update_bits(sunxi_codec->codec_regmap,
					SUNXI_ADC_FIFOC, (0x3 << ADC_CHAN_EN),
					(0x3 << ADC_CHAN_EN));
		}
		break;
	default:
		LOG_ERR("[%s] only support mono or stereo mode.\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int sunxi_codec_set_sysclk(struct snd_soc_dai *dai,
			int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	switch (clk_id) {
	case	0:
		/* For setting the clk source to 90.3168M to surpport playback */
#if 0
		if (clk_set_parent(sunxi_codec->dacclk, sunxi_codec->pllcomdiv5)) {
			LOG_ERR("set parent of dacclk to pllcomdiv5 failed\n");
			return -EINVAL;
		}
#endif
		if (clk_set_rate(sunxi_codec->dacclk, freq)) {
			LOG_ERR("codec set dac clk rate failed\n");
			return -EINVAL;
		}
		break;
	case	1:
#if 0
		/* For setting the clk source to 90.3168M to surpport capture */
		if (clk_set_parent(sunxi_codec->adcclk, sunxi_codec->pllcomdiv5)) {
			LOG_ERR("set parent of adcclk to pllcomdiv5 failed\n");
			return -EINVAL;
		}
#endif
		if (clk_set_rate(sunxi_codec->adcclk, freq)) {
			LOG_ERR("codec set adc clk rate failed\n");
			return -EINVAL;
		}
		break;
	case	2:
		/* For setting the clk source to 98.304M to surpport playback */
		if (clk_set_parent(sunxi_codec->dacclk, sunxi_codec->pllclk)) {
			LOG_ERR("set parent of dacclk to pllclk failed\n");
			return -EINVAL;
		}

		if (clk_set_rate(sunxi_codec->dacclk, freq)) {
			LOG_ERR("codec set dac clk rate failed\n");
			return -EINVAL;
		}
		break;
	case	3:
		/* For setting the clk source to 98.304M to surpport capture */
		if (clk_set_parent(sunxi_codec->adcclk, sunxi_codec->pllclk)) {
			LOG_ERR("set parent of adcclk to pllclk failed\n");
			return -EINVAL;
		}

		if (clk_set_rate(sunxi_codec->adcclk, freq)) {
			LOG_ERR("codec set adc clk rate failed\n");
			return -EINVAL;
		}
		break;
	default:
		LOG_ERR("Bad clk params input!\n");
		return -EINVAL;
	}

	return 0;
}

static int sunxi_codec_set_clkdiv(struct snd_soc_dai *dai,
						int clk_id, int clk_div)
{
	struct snd_soc_component *component = dai->component;
	unsigned int ret = 0;

	/* inter i2s clk div setting */
	ret = sunxi_codec_i2s_clkdiv_setting(component, clk_div);

	return ret;
}

static void sunxi_codec_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
#if 0
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (sunxi_codec->hw_config.adchpf_cfg)
			adchpf_enable(component, 0);
	}
#endif
}

static int sunxi_codec_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_FIFOC, (0x1 << DAC_FIFO_FLUSH),
				(0x1 << DAC_FIFO_FLUSH));
		regmap_write(sunxi_codec->codec_regmap,
				SUNXI_DAC_FIFOS,
				(0x1 << DAC_TXE_INT | 0x1 << DAC_TXU_INT | 0x1 << DAC_TXO_INT));
		regmap_write(sunxi_codec->codec_regmap,
				SUNXI_DAC_CNT,
				0x0);
	} else {
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_FIFOC, (0x1 << ADC_FIFO_FLUSH),
				(0x1 << ADC_FIFO_FLUSH));
		regmap_write(sunxi_codec->codec_regmap,
				SUNXI_ADC_FIFOS,
				(0x1 << ADC_RXA_INT | 0x1 << ADC_RXO_INT));
		regmap_write(sunxi_codec->codec_regmap,
				SUNXI_ADC_CNT,
				0x0);
	}

	return 0;
}

static void sunxi_codec_enable(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai, bool enable)
{
	struct sunxi_codec_info *sunxi_codec =
				snd_soc_component_get_drvdata(dai->component);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_DAC_FIFOC, (0x1 << DAC_DRQ_EN),
				(enable << DAC_DRQ_EN));
	} else {
		regmap_update_bits(sunxi_codec->codec_regmap,
				SUNXI_ADC_FIFOC, (0x1 << ADC_DRQ_EN),
				(enable << ADC_DRQ_EN));
	}
}

static void sunxi_codec_i2s_enable(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai, bool enable)
{
	struct sunxi_codec_info *sunxi_codec =
				snd_soc_component_get_drvdata(dai->component);
	struct sunxi_codec_i2s_params *i2s_params = &sunxi_codec->i2s_params;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* internal i2s tx enable */
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_DOUT0_EN),
				(enable << I2S_DOUT0_EN));

		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_TXEN),
				(enable << I2S_TXEN));
	} else {
		/* internal i2s rx enable */
		regmap_update_bits(sunxi_codec->i2s_regmap,
				INTER_I2S_CTL, (0x1 << I2S_RXEN),
				(enable << I2S_RXEN));
	}

	/* internal i2s global enable */
	if (enable) {
		if (i2s_params->i2s_global_enable++ == 0) {
			regmap_update_bits(sunxi_codec->i2s_regmap,
					INTER_I2S_CTL, (0x1 << I2S_GEN),
					(0x1 << I2S_GEN));
		}
	} else {
		if (--i2s_params->i2s_global_enable <= 0) {
			i2s_params->i2s_global_enable = 0;
			regmap_update_bits(sunxi_codec->i2s_regmap,
					INTER_I2S_CTL, (0x1 << I2S_GEN),
					(0x0 << I2S_GEN));
		}
	}
}

static int sunxi_codec_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(dai->component);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		/* if dac data src is form i2s, then should enable it first */
		if (sunxi_codec->dac_data_src == 1) {
			/* internal i2s part enable */
			sunxi_codec_i2s_enable(substream, dai, true);
		} else {
			/* internal codec part enable */
			sunxi_codec_enable(substream, dai, true);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		/* if dac data src is form i2s, then should disable it first */
		if (sunxi_codec->dac_data_src == 1) {
			/* internal i2s part disable */
			sunxi_codec_i2s_enable(substream, dai, false);
		} else {
			/* internal codec part disable */
			sunxi_codec_enable(substream, dai, false);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct snd_soc_dai_ops sunxi_codec_dai_ops = {
	.startup	= sunxi_codec_startup,
	.hw_params	= sunxi_codec_hw_params,
	.shutdown	= sunxi_codec_shutdown,
	.set_sysclk	= sunxi_codec_set_sysclk,
	.set_clkdiv	= sunxi_codec_set_clkdiv,
	.trigger	= sunxi_codec_trigger,
	.prepare	= sunxi_codec_prepare,
};

static struct snd_soc_dai_driver sunxi_codec_dai[] = {
	{
		.name	= "sun50iw12codec",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates	= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
			.formats = SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE,
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
			.formats = SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE,
		},
		.ops = &sunxi_codec_dai_ops,
	},
};

static int sunxi_codec_probe(struct snd_soc_component *component)
{
	struct snd_soc_dapm_context *dapm = &component->dapm;
	int ret;

	ret = snd_soc_add_component_controls(component, sunxi_codec_controls,
					ARRAY_SIZE(sunxi_codec_controls));
	if (ret) {
		LOG_ERR("failed to register codec controls!\n");
		return -EINVAL;
	}

	snd_soc_dapm_new_controls(dapm, sunxi_codec_dapm_widgets,
				ARRAY_SIZE(sunxi_codec_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, sunxi_codec_dapm_routes,
				ARRAY_SIZE(sunxi_codec_dapm_routes));

	sunxi_codec_init(component);

	sunxi_codec_i2s_init(component);

	return 0;
}

static void sunxi_codec_remove(struct snd_soc_component *component)
{

}

static int save_audio_reg(struct sunxi_codec_info *sunxi_codec)
{
	int i = 0;

	while (codec_reg_labels[i].name != NULL) {
		regmap_read(sunxi_codec->codec_regmap,
				codec_reg_labels[i].address,
				&codec_reg_labels[i].value);
		i++;
	}

	while (i2s_reg_labels[i].name != NULL) {
		regmap_read(sunxi_codec->i2s_regmap,
				i2s_reg_labels[i].address,
				&i2s_reg_labels[i].value);
		i++;
	}

	return 0;
}

static int echo_audio_reg(struct sunxi_codec_info *sunxi_codec)
{
	int i = 0;

	while (codec_reg_labels[i].name != NULL) {
		regmap_write(sunxi_codec->codec_regmap,
				codec_reg_labels[i].address,
				codec_reg_labels[i].value);
		i++;
	}

	while (i2s_reg_labels[i].name != NULL) {
		regmap_write(sunxi_codec->i2s_regmap,
				i2s_reg_labels[i].address,
				i2s_reg_labels[i].value);
		i++;
	}

	return 0;
}

static int sunxi_codec_suspend(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_cfg);

	pr_debug("Enter %s\n", __func__);
	save_audio_reg(sunxi_codec);

	if (spk_cfg->spk_used)
		gpio_set_value(spk_cfg->spk_gpio, !(spk_cfg->pa_level));

	if (sunxi_codec->vol_supply.avcc)
		regulator_disable(sunxi_codec->vol_supply.avcc);

	if (sunxi_codec->vol_supply.cpvin)
		regulator_disable(sunxi_codec->vol_supply.cpvin);

	clk_disable_unprepare(sunxi_codec->dacclk);
	clk_disable_unprepare(sunxi_codec->adcclk);
	clk_disable_unprepare(sunxi_codec->pllclk);
	clk_disable_unprepare(sunxi_codec->codec_clk_bus);
	reset_control_assert(sunxi_codec->codec_clk_rst);

	pr_debug("End %s\n", __func__);

	return 0;
}

static int sunxi_codec_resume(struct snd_soc_component *component)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_cfg);
	unsigned int ret;

	pr_debug("Enter %s\n", __func__);

	if (sunxi_codec->vol_supply.avcc) {
		ret = regulator_enable(sunxi_codec->vol_supply.avcc);
		if (ret)
			LOG_ERR("[%s]: resume avcc enable failed!\n", __func__);
	}

	if (sunxi_codec->vol_supply.cpvin) {
		ret = regulator_enable(sunxi_codec->vol_supply.cpvin);
		if (ret)
			LOG_ERR("[%s]: resume cpvin enable failed!\n", __func__);
	}

	if (clk_set_rate(sunxi_codec->pllclk, 98304000)) {
		LOG_ERR("audiocodec: resume codec source set pllclk rate failed\n");
		return -EBUSY;
	}

	if (reset_control_deassert(sunxi_codec->codec_clk_rst)) {
		LOG_ERR("audiocodec: resume deassert the codec reset failed\n");
		return -EBUSY;
	}

	if (clk_prepare_enable(sunxi_codec->codec_clk_bus)) {
		LOG_ERR("enable codec bus clk failed, resume exit\n");
		return -EBUSY;
	}

	if (clk_prepare_enable(sunxi_codec->pllclk)) {
		LOG_ERR("enable pllclk failed, resume exit\n");
		return -EBUSY;
	}

	if (clk_prepare_enable(sunxi_codec->dacclk)) {
		LOG_ERR("enable dacclk failed, resume exit\n");
		return -EBUSY;
	}

	if (clk_prepare_enable(sunxi_codec->adcclk)) {
		LOG_ERR("enable  adcclk failed, resume exit\n");
		clk_disable_unprepare(sunxi_codec->adcclk);
		return -EBUSY;
	}

	if (spk_cfg->spk_used) {
		gpio_direction_output(spk_cfg->spk_gpio, 1);
		gpio_set_value(spk_cfg->spk_gpio, !(spk_cfg->pa_level));
	}

	sunxi_codec_init(component);
	echo_audio_reg(sunxi_codec);

	pr_debug("End %s\n", __func__);

	return 0;
}

static unsigned int sunxi_codec_read(struct snd_soc_component *component,
					unsigned int reg)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);
	unsigned int reg_val = 0;

	regmap_read(sunxi_codec->codec_regmap, reg, &reg_val);

	return reg_val;
}

static int sunxi_codec_write(struct snd_soc_component *component,
				unsigned int reg, unsigned int val)
{
	struct sunxi_codec_info *sunxi_codec = snd_soc_component_get_drvdata(component);

	regmap_write(sunxi_codec->codec_regmap, reg, val);

	return 0;
};

static struct snd_soc_component_driver soc_codec_dev_sunxi = {
	.probe = sunxi_codec_probe,
	.remove = sunxi_codec_remove,
	.suspend = sunxi_codec_suspend,
	.resume = sunxi_codec_resume,
	.read = sunxi_codec_read,
	.write = sunxi_codec_write,
};

/* audiocodec reg dump about */
static ssize_t show_audio_reg(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(dev);
	unsigned int size1 = ARRAY_SIZE(codec_reg_labels);
	unsigned int size2 = ARRAY_SIZE(i2s_reg_labels);
	unsigned int reg_val = 0;
	int count = 0, i = 0;

	count += sprintf(buf, "dump audiocodec reg --> codec part:\n");

	while ((i < size1) && (codec_reg_labels[i].name != NULL)) {
		regmap_read(sunxi_codec->codec_regmap,
				codec_reg_labels[i].address, &reg_val);
		count += sprintf(buf + count, "%-20s [0x%03x]: 0x%-10x save_val:0x%x\n",
			codec_reg_labels[i].name, (codec_reg_labels[i].address),
			reg_val, codec_reg_labels[i].value);
		i++;
	}

	count += sprintf(buf, "dump audiocodec reg --> internal i2s part:\n");

	while ((i < size2) && (i2s_reg_labels[i].name != NULL)) {
		regmap_read(sunxi_codec->i2s_regmap,
				i2s_reg_labels[i].address, &reg_val);
		count += sprintf(buf + count, "%-20s [0x%03x]: 0x%-10x save_val:0x%x\n",
			i2s_reg_labels[i].name, (i2s_reg_labels[i].address),
			reg_val, i2s_reg_labels[i].value);
		i++;
	}

	return count;
}

/* Note:
 * param 1: 0 -> codec part;	1 -> internal i2s part;
 * param 2: 0 -> to read;		1 -> to write;
 * param 3: the reg value what you want to read or write;
 * param 4: the write value what you want to write in;
 * For Examples:
READ:
*	read the codec part reg's value:
		echo 0,0,0x00 > audiocodec_reg
*	read the internal i2s part reg's value:
		echo 1,0,0x00 > audiocodec_reg
WRITE:
	write the value:0x0a to the codec part reg: 0x00:
		echo 0,1,0x00,0x0a > audiocodec_reg
	write the value:0x0a to the internal i2s part reg: 0x00:
		echo 1,1,0x00,0x0a > audiocodec_reg
*/
static ssize_t store_audio_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(dev);
	int input_reg_val = 0;
	int input_reg_offset = 0;
	int reg_flag;
	int rw_flag;
	int ret;

	ret = sscanf(buf, "%d,%d,0x%x,0x%x",
			&reg_flag, &rw_flag, &input_reg_offset, &input_reg_val);

	LOG_INFO("reg flag:%d, rw_flag:%d, reg_offset:%d, reg_val:0x%x\n",
			reg_flag, rw_flag, input_reg_offset, input_reg_val);

	if (!(reg_flag == 1 || reg_flag == 0)) {
		LOG_ERR("the reg_flag input false! -> 0: codec part; 1: internal i2s part;\n");
		ret = count;
		goto out;
	}

	if (!(rw_flag == 1 || rw_flag == 0)) {
		LOG_ERR("the rw_flag input false! -> 0: to read; 1: to write;\n");
		ret = count;
		goto out;
	}

	if (reg_flag == 0) {
		if (input_reg_offset > SUNXI_HMIC_STS) {
			LOG_ERR("ERROR: the reg offset[0x%03x] > THE MAX REG[0x%03x]\n",
				input_reg_offset, SUNXI_HMIC_STS);
			ret = count;
			goto out;
		}

		if (rw_flag) {
			regmap_write(sunxi_codec->codec_regmap,
					input_reg_offset, input_reg_val);
		} else {
			regmap_read(sunxi_codec->codec_regmap,
					input_reg_offset, &input_reg_val);
			LOG_INFO("\n\n Reg[0x%x] : 0x%08x\n\n",
					input_reg_offset, input_reg_val);
		}
	}

	if (reg_flag == 1) {
		if (input_reg_offset > INTER_I2S_REV) {
			LOG_ERR("ERROR: the reg offset[0x%03x] > THE MAX REG[0x%03x]\n",
				input_reg_offset, SUNXI_BIAS_REG);
			ret = count;
			goto out;
		}

		if (rw_flag) {
			regmap_write(sunxi_codec->i2s_regmap,
					input_reg_offset, input_reg_val);
		} else {
			regmap_read(sunxi_codec->i2s_regmap,
					input_reg_offset, &input_reg_val);
			LOG_INFO("\n\n Reg[0x%x] : 0x%08x\n\n",
					input_reg_offset, input_reg_val);
		}
	}

	ret = count;
out:
	return ret;
}

static DEVICE_ATTR(audiocodec_reg, 0644, show_audio_reg, store_audio_reg);

static struct attribute *audio_debug_attrs[] = {
	&dev_attr_audiocodec_reg.attr,
	NULL,
};

static struct attribute_group audio_debug_attr_group = {
	.name   = "audiocodec_reg_dump",
	.attrs  = audio_debug_attrs,
};

/* codec regmap configuration */
static const struct regmap_config sunxi_codec_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = SUNXI_HMIC_STS,
	.cache_type = REGCACHE_NONE,
};

/* internal i2s regmap configuration */
static const struct regmap_config sunxi_i2s_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = INTER_I2S_REV,
	.cache_type = REGCACHE_NONE,
};

static int sunxi_codec_regulator_init(struct platform_device *pdev,
				struct sunxi_codec_info *sunxi_codec)
{
	int ret = 0;

	sunxi_codec->vol_supply.avcc = regulator_get(&pdev->dev, "avcc");
	if (IS_ERR(sunxi_codec->vol_supply.avcc)) {
		LOG_ERR("get audio avcc failed\n");
	} else {
		ret = regulator_set_voltage(sunxi_codec->vol_supply.avcc,
							1800000, 1800000);
		if (ret) {
			LOG_ERR("avcc set vol failed\n");
			goto err_regulator_avcc;
		}

		ret = regulator_enable(sunxi_codec->vol_supply.avcc);
		if (ret) {
			LOG_ERR("avcc enable failed!\n");
			goto err_regulator_avcc;
		}
	}

	sunxi_codec->vol_supply.cpvin = regulator_get(&pdev->dev, "cpvin");
	if (IS_ERR(sunxi_codec->vol_supply.cpvin)) {
		LOG_ERR("get audio cpvin failed\n");
		goto err_regulator_avcc;
	} else {
		ret = regulator_set_voltage(sunxi_codec->vol_supply.cpvin,
							1800000, 1800000);
		if (ret) {
			LOG_ERR("cpvin set vol failed\n");
			goto err_regulator_cpvin;
		}

		ret = regulator_enable(sunxi_codec->vol_supply.cpvin);
		if (ret) {
			LOG_ERR("cpvin enable failed!\n");
			goto err_regulator_cpvin;
		}
	}

	return 0;

err_regulator_cpvin:
	regulator_put(sunxi_codec->vol_supply.cpvin);
err_regulator_avcc:
	regulator_put(sunxi_codec->vol_supply.avcc);
	return ret;
}

static int sunxi_codec_clk_init(struct device_node *np,
				struct platform_device *pdev,
				struct sunxi_codec_info *sunxi_codec)
{
	int ret = 0;

	/* get the parent clk and the module clk */
	sunxi_codec->pllclk = of_clk_get_by_name(np, "pll_audio");
	sunxi_codec->dacclk = of_clk_get_by_name(np, "codec_dac");
	sunxi_codec->adcclk = of_clk_get_by_name(np, "codec_adc");
	sunxi_codec->codec_clk_bus = of_clk_get_by_name(np, "codec_bus");
	sunxi_codec->codec_clk_rst = devm_reset_control_get(&pdev->dev, NULL);

	if (reset_control_deassert(sunxi_codec->codec_clk_rst)) {
		LOG_ERR("deassert the codec reset failed\n");
		goto err_devm_kfree;
	}

	if (clk_set_parent(sunxi_codec->dacclk, sunxi_codec->pllclk)) {
		LOG_ERR("set parent of dacclk to pllclk failed\n");
		goto err_devm_kfree;
	}

	if (clk_set_parent(sunxi_codec->adcclk, sunxi_codec->pllclk)) {
		LOG_ERR("set parent of adcclk to pllclk failed\n");
		goto err_devm_kfree;
	}

	if (clk_set_rate(sunxi_codec->pllclk, 98304000)) {
		LOG_ERR("codec source set pllclk rate failed\n");
		goto err_devm_kfree;
	}

	if (clk_prepare_enable(sunxi_codec->codec_clk_bus)) {
		LOG_ERR("codec clk bus enable failed\n");
		goto err_devm_kfree;
	}

	if (clk_prepare_enable(sunxi_codec->pllclk)) {
		LOG_ERR("pllclk enable failed\n");
		goto err_bus_kfree;
	}

	if (clk_prepare_enable(sunxi_codec->dacclk)) {
		LOG_ERR("dacclk enable failed\n");
		goto err_pllclk_kfree;
	}

	if (clk_prepare_enable(sunxi_codec->adcclk)) {
		LOG_ERR("moduleclk enable failed\n");
		goto err_dacclk_kfree;
	}

	return 0;

err_dacclk_kfree:
	clk_disable_unprepare(sunxi_codec->dacclk);
err_pllclk_kfree:
	clk_disable_unprepare(sunxi_codec->pllclk);
err_bus_kfree:
	clk_disable_unprepare(sunxi_codec->codec_clk_bus);
err_devm_kfree:
	return ret;
}

static int sunxi_codec_parse_params(struct device_node *np,
				struct platform_device *pdev,
				struct sunxi_codec_info *sunxi_codec)
{
	struct sunxi_codec_i2s_params *i2s_params = &sunxi_codec->i2s_params;
	unsigned int temp_val = 0;
	int ret = 0;

	/* get the special property form the board.dts */
	ret = of_property_read_u32(np, "digital_vol", &temp_val);
	if (ret < 0) {
		LOG_WARN("digital volume get failed, use default value: 0\n");
		sunxi_codec->digital_vol = 0;
	} else {
		sunxi_codec->digital_vol = temp_val;
	}

	/* lineout volume */
	ret = of_property_read_u32(np, "lineout_vol", &temp_val);
	if (ret < 0) {
		LOG_WARN("lineout volume get failed, use default value: 0\n");
		sunxi_codec->lineout_vol = 0;
	} else {
		sunxi_codec->lineout_vol = temp_val;
	}

	/* hpout volume */
	ret = of_property_read_u32(np, "hpout_vol", &temp_val);
	if (ret < 0) {
		LOG_WARN("hpout volume get failed, use default value: 0\n");
		sunxi_codec->hpout_vol = 0;
	} else {
		sunxi_codec->hpout_vol = temp_val;
	}

	/* PA/SPK enable property */
	ret = of_property_read_u32(np, "spk_used", &temp_val);
	if (ret < 0) {
		LOG_WARN("spk_used get failed, use default value: 0\n");
		sunxi_codec->spk_cfg.spk_used = 0;
	} else {
		sunxi_codec->spk_cfg.spk_used = temp_val;
	}

	/* Pa's delay time(ms) to work fine */
	ret = of_property_read_u32(np, "pa_msleep_time", &temp_val);
	if (ret < 0) {
		LOG_WARN("pa_msleep get failed, use default value: 160\n");
		sunxi_codec->spk_cfg.pa_msleep = 160;
	} else {
		sunxi_codec->spk_cfg.pa_msleep = temp_val;
	}

	/* PA/SPK enable property */
	ret = of_property_read_u32(np, "pa_level", &temp_val);
	if (ret < 0) {
		LOG_WARN("pa_level get failed, use default value: 1\n");
		sunxi_codec->spk_cfg.pa_level = 1;
	} else {
		sunxi_codec->spk_cfg.pa_level = temp_val;
	}

	LOG_INFO("digital_vol:%d, lineout_vol:%d, hpout_vol:%d, spk_used:%d, pa_msleep_time:%d, pa_level:%d\n",
		sunxi_codec->digital_vol,
		sunxi_codec->lineout_vol,
		sunxi_codec->hpout_vol,
		sunxi_codec->spk_cfg.spk_used,
		sunxi_codec->spk_cfg.pa_msleep,
		sunxi_codec->spk_cfg.pa_level);

	/* get the inter i2s params */
	ret = of_property_read_u32(np, "daudio_master", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s daudio_master config missing or invalid\n");
		i2s_params->daudio_master = 4;
	} else {
		i2s_params->daudio_master = temp_val;
	}

	ret = of_property_read_u32(np, "audio_format", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s audio_format config missing or invalid\n");
		i2s_params->audio_format = 1;
	} else {
		i2s_params->audio_format = temp_val;
	}

	ret = of_property_read_u32(np, "signal_inversion", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s signal_inversion config missing or invalid\n");
		i2s_params->signal_inversion = 1;
	} else {
		i2s_params->signal_inversion = temp_val;
	}

	ret = of_property_read_u32(np, "pcm_lrck_period", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s pcm_lrck_period config missing or invalid\n");
		i2s_params->pcm_lrck_period = 0;
	} else {
		i2s_params->pcm_lrck_period = temp_val;
	}

	ret = of_property_read_u32(np, "slot_width_select", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s slot_width_select config missing or invalid\n");
		i2s_params->slot_width_select = 0;
	} else {
		i2s_params->slot_width_select = temp_val;
	}

	ret = of_property_read_u32(np, "frametype", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s frametype config missing or invalid\n");
		i2s_params->frame_type = 0;
	} else {
		i2s_params->frame_type = temp_val;
	}

	ret = of_property_read_u32(np, "mclk_div", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s mclk_div config missing or invalid\n");
		i2s_params->mclk_div = 0;
	} else {
		i2s_params->mclk_div = temp_val;
	}

	ret = of_property_read_u32(np, "tdm_config", &temp_val);
	if (ret < 0) {
		LOG_WARN("inter i2s tdm_config config missing or invalid\n");
		i2s_params->tdm_config = 1;
	} else {
		i2s_params->tdm_config = temp_val;
	}

	LOG_INFO("inter i2s params: daudio_master:%d, audio_format:%d, signal_inversion:%d,"
		"pcm_lrck_period:0x%x, slot_width_select:0x%x, frametype:0x%x,"
		"mclk_div:0x%x, tdm_config:0x%x\n",
		i2s_params->daudio_master,
		i2s_params->audio_format,
		i2s_params->signal_inversion,
		i2s_params->pcm_lrck_period,
		i2s_params->slot_width_select,
		i2s_params->frame_type,
		i2s_params->mclk_div,
		i2s_params->tdm_config);

#if 0
	/* ADC/DAC DRC/HPF func enable property */
	ret = of_property_read_u32(np, "adcdrc_cfg", &temp_val);
	if (ret < 0) {
		LOG_ERR("[%s] adcdrc_cfg configs missing or invalid.\n", __func__);
	} else {
		sunxi_codec->hw_config.adcdrc_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "adchpf_cfg", &temp_val);
	if (ret < 0) {
		LOG_ERR("[%s] adchpf_cfg configs missing or invalid.\n", __func__);
	} else {
		sunxi_codec->hw_config.adchpf_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "dacdrc_cfg", &temp_val);
	if (ret < 0) {
		LOG_ERR("[%s] dacdrc_cfg configs missing or invalid.\n", __func__);
	} else {
		sunxi_codec->hw_config.dacdrc_cfg = temp_val;
	}

	ret = of_property_read_u32(np, "dachpf_cfg", &temp_val);
	if (ret < 0) {
		LOG_ERR("[%s] dachpf_cfg configs missing or invalid.\n", __func__);
	} else {
		sunxi_codec->hw_config.dachpf_cfg = temp_val;
	}

	pr_debug("adcdrc_cfg:%d, adchpf_cfg:%d, dacdrc_cfg:%d, dachpf:%d\n",
		sunxi_codec->hw_config.adcdrc_cfg,
		sunxi_codec->hw_config.adchpf_cfg,
		sunxi_codec->hw_config.dacdrc_cfg,
		sunxi_codec->hw_config.dachpf_cfg);
#endif

	/* get the gpio number to control external speaker */
	if (sunxi_codec->spk_cfg.spk_used) {
		ret = of_get_named_gpio(np, "gpio-spk", 0);
		if (ret >= 0) {
			sunxi_codec->spk_cfg.spk_used = 1;
			sunxi_codec->spk_cfg.spk_gpio = ret;
			if (!gpio_is_valid(sunxi_codec->spk_cfg.spk_gpio)) {
				LOG_ERR("gpio-spk is invalid\n");
				return -EINVAL;
			} else {
				pr_debug("gpio-spk:%d\n",
					sunxi_codec->spk_cfg.spk_gpio);
				ret = devm_gpio_request(&pdev->dev,
					sunxi_codec->spk_cfg.spk_gpio, "SPK");
				if (ret) {
					LOG_ERR("failed to request gpio-spk gpio\n");
					return -EBUSY;
				}
			}
		} else {
			sunxi_codec->spk_cfg.spk_used = 0;
			LOG_ERR("gpio-spk get faild!\n");
		}
	}

	return ret;
}

static int sunxi_codec_mem_init(struct device_node *np,
				struct platform_device *pdev,
				struct sunxi_codec_info *sunxi_codec)
{
	struct resource res;
	struct resource *memregion = NULL;
	int ret = 0;

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		LOG_ERR("Failed to get sunxi codec resource\n");
		return -EINVAL;
	}

	memregion = devm_request_mem_region(&pdev->dev, res.start,
				resource_size(&res), "sunxi-internal-codec");
	if (!memregion) {
		LOG_ERR("sunxi memory region already claimed\n");
		return -EINVAL;
	}

	sunxi_codec->codec_addr = devm_ioremap(&pdev->dev,
					res.start, resource_size(&res));
	if (!sunxi_codec->codec_addr) {
		LOG_ERR("sunxi codec_addr ioremap failed\n");
		return -EINVAL;
	}

	sunxi_codec->codec_regmap = devm_regmap_init_mmio(&pdev->dev,
				sunxi_codec->codec_addr,
				&sunxi_codec_regmap_config);
	if (IS_ERR_OR_NULL(sunxi_codec->codec_regmap)) {
		LOG_ERR("codec regmap init failed\n");
		return -EINVAL;
	}

	return ret;
}

static int sunxi_codec_i2s_mem_init(struct device_node *np,
				struct platform_device *pdev,
				struct sunxi_codec_info *sunxi_codec)
{
	struct resource res;
	struct resource *memregion = NULL;
	int ret = 0;

	ret = of_address_to_resource(np, 1, &res);
	if (ret) {
		LOG_ERR("Failed to get sunxi codec internal i2s resource\n");
		return -EINVAL;
	}

	memregion = devm_request_mem_region(&pdev->dev, res.start,
				resource_size(&res), "sunxi-internal-codec");
	if (!memregion) {
		LOG_ERR("sunxi memory region already claimed\n");
		return -EINVAL;
	}

	sunxi_codec->i2s_addr = devm_ioremap(&pdev->dev,
					res.start, resource_size(&res));
	if (!sunxi_codec->i2s_addr) {
		LOG_ERR("sunxi i2s_addr ioremap failed\n");
		return -EINVAL;
	}

	sunxi_codec->i2s_regmap = devm_regmap_init_mmio(&pdev->dev,
				sunxi_codec->i2s_addr,
				&sunxi_i2s_regmap_config);
	if (IS_ERR_OR_NULL(sunxi_codec->i2s_regmap)) {
		LOG_ERR("regmap init failed\n");
		return -EINVAL;
	}

	return ret;
}

static int sunxi_internal_codec_probe(struct platform_device *pdev)
{
	struct sunxi_codec_info *sunxi_codec;
	struct device_node *np = pdev->dev.of_node;
	int ret;

	if (IS_ERR_OR_NULL(np)) {
		LOG_ERR("pdev->dev.of_node is err.\n");
		ret = -EFAULT;
		goto err_node_put;
	}

	sunxi_codec = devm_kzalloc(&pdev->dev,
				sizeof(struct sunxi_codec_info), GFP_KERNEL);
	if (!sunxi_codec) {
		LOG_ERR("Can't allocate sunxi codec memory\n");
		ret = -ENOMEM;
		goto err_node_put;
	}
	dev_set_drvdata(&pdev->dev, sunxi_codec);
	sunxi_codec->dev = &pdev->dev;

	/* module regulator init */
	if (sunxi_codec_regulator_init(pdev, sunxi_codec) != 0) {
		LOG_ERR("Failed to init sunxi audio regulator\n");
		ret = -EINVAL;
		goto err_devm_kfree;
	}

	/* module params init */
	if (sunxi_codec_parse_params(np, pdev, sunxi_codec) != 0) {
		LOG_ERR("Failed to parse sunxi audio params\n");
		ret = -EINVAL;
		goto err_devm_kfree;
	}

	/* module clk init */
	if (sunxi_codec_clk_init(np, pdev, sunxi_codec) != 0) {
		LOG_ERR("Failed to init sunxi audio clk\n");
		ret = -EFAULT;
		goto err_devm_kfree;
	}

	/* module codec reg_base */
	if (sunxi_codec_mem_init(np, pdev, sunxi_codec) != 0) {
		LOG_ERR("Failed to init sunxi codec reg mem\n");
		ret = -EFAULT;
		goto err_devm_kfree;
	}

	/* module internal i2s reg_base */
	if (sunxi_codec_i2s_mem_init(np, pdev, sunxi_codec) != 0) {
		LOG_ERR("Failed to init sunxi codec reg mem\n");
		ret = -EFAULT;
		goto err_devm_kfree;
	}

	/* CODEC DAI cfg and register */
	ret = devm_snd_soc_register_component(&pdev->dev, &soc_codec_dev_sunxi,
				sunxi_codec_dai, ARRAY_SIZE(sunxi_codec_dai));
	if (ret < 0) {
		LOG_ERR("register codec failed\n");
		goto err_devm_kfree;
	}

	ret  = sysfs_create_group(&pdev->dev.kobj, &audio_debug_attr_group);
	if (ret) {
		LOG_WARN("failed to create attr group\n");
		goto err_devm_kfree;
	}

	LOG_WARN("codec probe finished.\n");

	return 0;

err_devm_kfree:
	devm_kfree(&pdev->dev, sunxi_codec);
err_node_put:
	of_node_put(np);
	return ret;
}

static int  __exit sunxi_internal_codec_remove(struct platform_device *pdev)
{
	struct sunxi_codec_info *sunxi_codec = dev_get_drvdata(&pdev->dev);
	struct codec_spk_config *spk_cfg = &(sunxi_codec->spk_cfg);

	if (spk_cfg->spk_used) {
		devm_gpio_free(&pdev->dev,
					sunxi_codec->spk_cfg.spk_gpio);
	}

	if (sunxi_codec->vol_supply.avcc) {
		regulator_disable(sunxi_codec->vol_supply.avcc);
		regulator_put(sunxi_codec->vol_supply.avcc);
	}

	if (sunxi_codec->vol_supply.cpvin) {
		regulator_disable(sunxi_codec->vol_supply.cpvin);
		regulator_put(sunxi_codec->vol_supply.cpvin);
	}

	sysfs_remove_group(&pdev->dev.kobj, &audio_debug_attr_group);

	snd_soc_unregister_component(&pdev->dev);

	clk_disable_unprepare(sunxi_codec->dacclk);
	clk_put(sunxi_codec->dacclk);
	clk_disable_unprepare(sunxi_codec->adcclk);
	clk_put(sunxi_codec->adcclk);
	clk_disable_unprepare(sunxi_codec->pllclk);
	clk_put(sunxi_codec->pllclk);
	clk_disable_unprepare(sunxi_codec->codec_clk_bus);
	clk_put(sunxi_codec->codec_clk_bus);
	reset_control_assert(sunxi_codec->codec_clk_rst);

	devm_iounmap(&pdev->dev, sunxi_codec->codec_addr);
	devm_iounmap(&pdev->dev, sunxi_codec->i2s_addr);

	devm_kfree(&pdev->dev, sunxi_codec);
	platform_set_drvdata(pdev, NULL);

	LOG_WARN("codec remove finished.\n");

	return 0;
}

static const struct of_device_id sunxi_internal_codec_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-codec", },
	{},
};

static struct platform_driver sunxi_internal_codec_driver = {
	.driver = {
		.name = "sunxi-internal-codec",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_internal_codec_of_match,
	},
	.probe = sunxi_internal_codec_probe,
	.remove = __exit_p(sunxi_internal_codec_remove),
};
module_platform_driver(sunxi_internal_codec_driver);

MODULE_DESCRIPTION("SUNXI Codec ASoC driver");
MODULE_AUTHOR("luguofang <luguofang@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-internal-codec");
