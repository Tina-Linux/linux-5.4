/* sound\soc\sunxi\snd_sun50iw9_codec.h
 * (C) Copyright 2021-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SND_SUN50IW9_CODEC_H
#define __SND_SUN50IW9_CODEC_H

#define SUNXI_DAC_DPC		0x00
#define SUNXI_DAC_FIFO_CTL	0x10
#define SUNXI_DAC_FIFO_STA	0x14

/* left blank */
#define SUNXI_DAC_TXDATA	0X20
#define SUNXI_DAC_CNT		0x24
#define SUNXI_DAC_DG_REG	0x28

#define	SUNXI_ADC_FIFO_CTL	0x30
#define SUNXI_ADC_FIFO_STA	0x34
#define SUNXI_ADC_RXDATA	0x40
#define SUNXI_ADC_CNT		0x44
#define SUNXI_ADC_DG_REG	0x4C

/*left blank */
#define SUNXI_DAC_DAP_CTL	0xf0
#define SUNXI_ADC_DAP_CTL	0xf8

/* DAC */
#define AC_DAC_REG		0x310
#define AC_MIXER_REG		0x314
#define AC_RAMP_REG		0x31c
#define SUNXI_AUDIO_MAX_REG	AC_RAMP_REG

/* SUNXI_DAC_DPC:0x00 */
#define EN_DA			31
#define MODQU			25
#define DWA_EN			24
#define HPF_EN			18
#define DVOL			12
#define DAC_HUB_EN		0

/* SUNXI_DAC_FIFO_CTL:0x10 */
#define DAC_FS			29
#define FIR_VER			28
#define SEND_LASAT		26
#define DAC_FIFO_MODE		24
#define DAC_DRQ_CLR_CNT		21
#define TX_TRIG_LEVEL		8
#define DAC_MONO_EN		6
#define TX_SAMPLE_BITS		5
#define DAC_DRQ_EN		4
#define DAC_IRQ_EN		3
#define DAC_FIFO_UNDERRUN_IRQ_EN	2
#define DAC_FIFO_OVERRUN_IRQ_EN	1
#define DAC_FIFO_FLUSH		0

/* SUNXI_DAC_FIFO_STA:0x14 */
#define	DAC_TX_EMPTY		23
#define	DAC_TXE_CNT			8
#define	DAC_TXE_INT			3
#define	DAC_TXU_INT			2
#define	DAC_TXO_INT			1

/* SUNXI_DAC_DG_REG:0x28 */
#define	DAC_MODU_SELECT		11
#define	DAC_PATTERN_SEL		9
#define	CODEC_CLK_SEL		8
#define	DA_SWP			6
#define	ADDA_LOOP_MODE		0

/* SUNXI_ADC_FIFO_CTL:0x30 */
#define ADC_FS			29
#define EN_AD			28
#define ADCFDT			26
#define ADCDFEN			25
#define RX_FIFO_MODE		24
#define RX_SAMPLE_BITS		16
#define ADCY_EN			15
#define ADCX_EN			14
#define ADCR_EN			13
#define ADCL_EN			12
#define ADC_CHAN_SEL		12
#define RX_FIFO_TRG_LEVEL	4
#define ADC_DRQ_EN		3
#define ADC_IRQ_EN		2
#define ADC_OVERRUN_IRQ_EN	1
#define ADC_FIFO_FLUSH		0

/* SUNXI_ADC_FIFO_STA:0x38 */
#define	ADC_RXA			23
#define	ADC_RXA_CNT		8
#define	ADC_RXA_INT		3
#define	ADC_RXO_INT		1

/* SUNXI_ADC_DG_REG:0x4c */
#define	ADXY_SWP		25
#define	ADLR_SWP		24

/* DAC */
/* AC_DAC_REG */
#define CURRENT_TEST_SEL	23
#define IOPVRS			20
#define ILINEOUTAMPS		18
#define IOPDACS			16
#define DACLEN			15
#define DACREN			14
#define LINEOUTL_EN		13
#define LMUTE			12
#define LINEOUTR_EN		11
#define RMUTE			10
#define RSWITCH			9
#define RAMPEN			8
#define LINEOUTL_SEL		6
#define LINEOUTR_SEL		5
#define LINEOUT_VOL		0

/* AC_MIXER_REG	*/
#define LMIX_LDAC		21
#define LMIX_RDAC		20
#define LMIXMUTE		20
#define RMIX_RDAC		17
#define RMIX_LDAC		16
#define RMIXMUTE		16
#define LMIXEN			11
#define RMIXEN			10
#define IOPMIXS			8

/* AC_RAMP_REG */
#define RAMP_STEP		4
#define RMDEN			3
#define RMUEN			2
#define RMCEN			1
#define RDEN			0

struct sunxi_codec_info {
	/* regulator about */
	RGLT_HANDLE avcc;
	RGLT_HANDLE dvcc;

	/* clk about */
	CLK_HANDLE clk_rst;
	CLK_HANDLE clk_bus_audio;
	CLK_HANDLE clk_pll_audio;
	CLK_HANDLE clk_audio;

	/* regmap about */
	REG_HANDLE regmap;

	/* gpio mute for peripheral circuit about */
	u32 pa_pin_max;
	struct pa_config *pa_cfg;

	/* parse params about */
	u32 digital_vol;
	u32 lineout_vol;

	/* others about */
	struct adapter_cntlr *cntlr;
};

int internal_codec_probe(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info);
int internal_codec_remove(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info);
int internal_codec_init(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info);

int internal_codec_controls_add(struct snd_soc_component *component);

int internal_codec_suspend(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info);
int internal_codec_resume(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info);

int internal_codec_hw_params(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info,
			     int stream, int format, unsigned int  rate, unsigned int channels);
int internal_codec_set_sysclk(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info,
			      unsigned int freq);
int internal_codec_prepare(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info,
			   int stream);
int internal_codec_trigger(struct adapter_cntlr *cntlr, struct sunxi_codec_info *codec_info,
			   int stream, int drq_en);

#endif /* __SND_SUN50IW9_CODEC_H */
