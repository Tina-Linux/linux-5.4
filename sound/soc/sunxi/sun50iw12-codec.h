/*
 * sound\soc\sunxi\sun50iw12-codec.h
 * (C) Copyright 2019-2021
 * allwinner Technology Co., Ltd. <www.allwinnertech.com>
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
#ifndef _SUN50IW12_CODEC_H
#define _SUN50IW12_CODEC_H

#include "sunxi-pcm.h"

/************** Audio Codec Register **************/
/* Base Address : 0x02030000 */
/* Digital Domain Register */
#define SUNXI_DAC_DPC		0x00
#define SUNXI_DAC_VOL_CTRL	0x04

#define SUNXI_DAC_FIFOC		0x10
#define SUNXI_DAC_FIFOS		0x14

#define SUNXI_DAC_TXDATA	0X20
#define SUNXI_DAC_CNT		0x24
#define SUNXI_DAC_DG		0x28

#define	SUNXI_ADC_FIFOC		0x30
#define	SUNXI_ADC_VOL_CTRL	0x34
#define SUNXI_ADC_FIFOS		0x38

#define SUNXI_ADC_RXDATA	0x40
#define SUNXI_ADC_CNT		0x44
#define SUNXI_ADC_DG		0x4C

#define SUNXI_DAC_DAP_CTL	0xF0
#define SUNXI_ADC_DAP_CTL	0xF8

#define SUNXI_DAC_DRC_HHPFC	0x100
#define SUNXI_DAC_DRC_LHPFC	0x104
#define SUNXI_DAC_DRC_CTRL	0x108
#define SUNXI_DAC_DRC_LPFHAT	0x10C
#define SUNXI_DAC_DRC_LPFLAT	0x110
#define SUNXI_DAC_DRC_RPFHAT	0x114
#define SUNXI_DAC_DRC_RPFLAT	0x118
#define SUNXI_DAC_DRC_LPFHRT	0x11C
#define SUNXI_DAC_DRC_LPFLRT	0x120
#define SUNXI_DAC_DRC_RPFHRT	0x124
#define SUNXI_DAC_DRC_RPFLRT	0x128
#define SUNXI_DAC_DRC_LRMSHAT	0x12C
#define SUNXI_DAC_DRC_LRMSLAT	0x130
#define SUNXI_DAC_DRC_RRMSHAT	0x134
#define SUNXI_DAC_DRC_RRMSLAT	0x138
#define SUNXI_DAC_DRC_HCT	0x13C
#define SUNXI_DAC_DRC_LCT	0x140
#define SUNXI_DAC_DRC_HKC	0x144
#define SUNXI_DAC_DRC_LKC	0x148
#define SUNXI_DAC_DRC_HOPC	0x14C
#define SUNXI_DAC_DRC_LOPC	0x150
#define SUNXI_DAC_DRC_HLT	0x154
#define SUNXI_DAC_DRC_LLT	0x158
#define SUNXI_DAC_DRC_HKI	0x15C
#define SUNXI_DAC_DRC_LKI	0x160
#define SUNXI_DAC_DRC_HOPL	0x164
#define SUNXI_DAC_DRC_LOPL	0x168
#define SUNXI_DAC_DRC_HET	0x16C
#define SUNXI_DAC_DRC_LET	0x170
#define SUNXI_DAC_DRC_HKE	0x174
#define SUNXI_DAC_DRC_LKE	0x178
#define SUNXI_DAC_DRC_HOPE	0x17C
#define SUNXI_DAC_DRC_LOPE	0x180
#define SUNXI_DAC_DRC_HKN	0x184
#define SUNXI_DAC_DRC_LKN	0x188
#define SUNXI_DAC_DRC_SFHAT	0x18C
#define SUNXI_DAC_DRC_SFLAT	0x190
#define SUNXI_DAC_DRC_SFHRT	0x194
#define SUNXI_DAC_DRC_SFLRT	0x198
#define SUNXI_DAC_DRC_MXGHS	0x19C
#define SUNXI_DAC_DRC_MXGLS	0x1A0
#define SUNXI_DAC_DRC_MNGHS	0x1A4
#define SUNXI_DAC_DRC_MNGLS	0x1A8
#define SUNXI_DAC_DRC_EPSHC	0x1AC
#define SUNXI_DAC_DRC_EPSLC	0x1B0
#define SUNXI_DAC_DRC_OPT	0x1B4
#define SUNXI_DAC_DRC_HPFHGAIN	0x1B8
#define SUNXI_DAC_DRC_HPFLGAIN	0x1BC

#define SUNXI_ADC_DRC_HHPFC	0x200
#define SUNXI_ADC_DRC_LHPFC	0x204
#define SUNXI_ADC_DRC_CTRL	0x208
#define SUNXI_ADC_DRC_LPFHAT	0x20C
#define SUNXI_ADC_DRC_LPFLAT	0x210
#define SUNXI_ADC_DRC_RPFHAT	0x214
#define SUNXI_ADC_DRC_RPFLAT	0x218
#define SUNXI_ADC_DRC_LPFHRT	0x21C
#define SUNXI_ADC_DRC_LPFLRT	0x220
#define SUNXI_ADC_DRC_RPFHRT	0x224
#define SUNXI_ADC_DRC_RPFLRT	0x228
#define SUNXI_ADC_DRC_LRMSHAT	0x22C
#define SUNXI_ADC_DRC_LRMSLAT	0x230
#define SUNXI_ADC_DRC_HCT	0x23C
#define SUNXI_ADC_DRC_LCT	0x240
#define SUNXI_ADC_DRC_HKC	0x244
#define SUNXI_ADC_DRC_LKC	0x248
#define SUNXI_ADC_DRC_HOPC	0x24C
#define SUNXI_ADC_DRC_LOPC	0x250
#define SUNXI_ADC_DRC_HLT	0x254
#define SUNXI_ADC_DRC_LLT	0x258
#define SUNXI_ADC_DRC_HKI	0x25C
#define SUNXI_ADC_DRC_LKI	0x260
#define SUNXI_ADC_DRC_HOPL	0x264
#define SUNXI_ADC_DRC_LOPL	0x268
#define SUNXI_ADC_DRC_HET	0x26C
#define SUNXI_ADC_DRC_LET	0x270
#define SUNXI_ADC_DRC_HKE	0x274
#define SUNXI_ADC_DRC_LKE	0x278
#define SUNXI_ADC_DRC_HOPE	0x27C
#define SUNXI_ADC_DRC_LOPE	0x280
#define SUNXI_ADC_DRC_HKN	0x284
#define SUNXI_ADC_DRC_LKN	0x288
#define SUNXI_ADC_DRC_SFHAT	0x28C
#define SUNXI_ADC_DRC_SFLAT	0x290
#define SUNXI_ADC_DRC_SFHRT	0x294
#define SUNXI_ADC_DRC_SFLRT	0x298
#define SUNXI_ADC_DRC_MXGHS	0x29C
#define SUNXI_ADC_DRC_MXGLS	0x2A0
#define SUNXI_ADC_DRC_MNGHS	0x2A4
#define SUNXI_ADC_DRC_MNGLS	0x2A8
#define SUNXI_ADC_DRC_EPSHC	0x2AC
#define SUNXI_ADC_DRC_EPSLC	0x2B0
#define SUNXI_ADC_DRC_OPT	0x2B4
#define SUNXI_ADC_DRC_HPFHGAIN	0x2B8
#define SUNXI_ADC_DRC_HPFLGAIN	0x2BC

#define SUNXI_AC_VERSION	0x2C0

/* Analog Domain Register */
#define SUNXI_ADCL_REG		0x300
#define SUNXI_ADCR_REG		0x304
#define SUNXI_DAC_REG		0x310
#define SUNXI_MICBIAS_REG	0x318
#define SUNXI_BIAS_REG		0x320
#define SUNXI_HP_REG		0x324
#define SUNXI_HMIC_CTRL		0x328
#define SUNXI_HMIC_STS		0x32c

/* SUNXI_DAC_DPC:0x00 */
#define DAC_EN			31
#define I2S_CLK_SEL		30
#define DAC_SRC_SEL		29
#define DAC_MODQU		25
#define DAC_DWA_EN		24
#define DAC_HPF_EN		18
#define DAC_DVOL		12
#define DAC_HUB_EN		0

/* SUNXI_DAC_VOL_CTRL:0x04 */
#define DAC_VOL_SEL		16
#define DAC_VOL_L		8
#define DAC_VOL_R		0

/* SUNXI_DAC_FIFOC:0x10 */
#define DAC_FS			29
#define DAC_FIR_VER		28
#define DAC_SEND_LASAT		26
#define DAC_FIFO_MODE		24
#define DAC_DRQ_CLR_CNT		21
#define DAC_TRIG_LEVEL		8
#define DAC_MONO_EN		6
#define DAC_SAMPLE_BITS		5
#define DAC_DRQ_EN		4
#define DAC_IRQ_EN		3
#define DAC_UNDERRUN_IRQ_EN	2
#define DAC_OVERRUN_IRQ_EN	1
#define DAC_FIFO_FLUSH		0

/* SUNXI_DAC_FIFOS:0x14 */
#define	DAC_TX_EMPTY		23
#define	DAC_TXE_CNT		8
#define	DAC_TXE_INT		3
#define	DAC_TXU_INT		2
#define	DAC_TXO_INT		1

/* SUNXI_DAC_TXDATA:0x20 */
#define DAC_TX_DATA		0

/* SUNXI_DAC_CNT:0x24 */
#define DAC_TX_CNT		0

/* SUNXI_DAC_DG:0x28 */
#define	DAC_MODU_SEL		11
#define	DAC_PATTERN_SEL		9
#define	DAC_CODEC_CLK_SEL	8
#define	DAC_SWAP		6
#define	ADDA_LOOP_MODE		0

/* SUNXI_ADC_FIFOC:0x30 */
#define ADC_FS			29
#define ADC_EN			28
#define ADC_FDT			26
#define ADC_DFEN		25
#define ADC_FIFO_MODE		24
#define ADC_RX_SYNC_START	21
#define ADC_RX_SYNC_EN		20
#define ADC_VOL_SEL		17
#define ADC_SAMPLE_BITS		16
#define ADC_CHAN_EN		12
#define ADC_TRIG_LEVEL		4
#define ADC_DRQ_EN		3
#define ADC_IRQ_EN		2
#define ADC_OVERRUN_IRQ_EN	1
#define ADC_FIFO_FLUSH		0

/* SUNXI_ADC_VOL_CTRL:0x34 */
#define ADC_VOL_L		8
#define ADC_VOL_R		0

/* SUNXI_ADC_FIFOS:0x38 */
#define	ADC_RXA			23
#define	ADC_RXA_CNT		8
#define	ADC_RXA_INT		3
#define	ADC_RXO_INT		1

/* SUNXI_ADC_RXDATA:0x40 */
#define ADC_RX_DATA		0

/* SUNXI_ADC_CNT:0x44 */
#define ADC_RX_CNT		0

/* SUNXI_ADC_DG:0x4C */
#define	ADC_SWAP		24

/* SUNXI_DAC_DAP_CTL:0xF0 */
#define	DAC_DAP_EN		31
#define	DAC_DAP_DRC_EN		29
#define	DAC_DAP_HPF_EN		28

/* SUNXI_ADC_DAP_CTL:0xF8 */
#define	ADC_DAP0_EN		31
#define	ADC_DRC0_EN		29
#define	ADC_HPF0_EN		28

/* SUNXI_DAC_DRC_HHPFC : 0x100*/
#define DAC_HHPF_CONF		0

/* SUNXI_DAC_DRC_LHPFC : 0x104*/
#define DAC_LHPF_CONF		0

/* SUNXI_DAC_DRC_CTRL : 0x108*/
#define DAC_DRC_DELAY_OUT_STATE		15
#define DAC_DRC_SIGNAL_DELAY		8
#define DAC_DRC_DELAY_BUF_EN		7
#define DAC_DRC_GAIN_MAX_EN		6
#define DAC_DRC_GAIN_MIN_EN		5
#define DAC_DRC_NOISE_DET_EN		4
#define DAC_DRC_SIGNAL_SEL		3
#define DAC_DRC_DELAY_EN		2
#define DAC_DRC_LT_EN			1
#define DAC_DRC_ET_EN			0

/* SUNXI_ADC_DRC_HHPFC : 0x200*/
#define ADC_HHPF_CONF		0

/* SUNXI_ADC_DRC_LHPFC : 0x204*/
#define ADC_LHPF_CONF		0

/* SUNXI_ADC_DRC_CTRL : 0x208*/
#define ADC_DRC_DELAY_OUT_STATE		15
#define ADC_DRC_SIGNAL_DELAY		8
#define ADC_DRC_DELAY_BUF_EN		7
#define ADC_DRC_GAIN_MAX_EN		6
#define ADC_DRC_GAIN_MIN_EN		5
#define ADC_DRC_NOISE_DET_EN		4
#define ADC_DRC_SIGNAL_SEL		3
#define ADC_DRC_DELAY_EN		2
#define ADC_DRC_LT_EN			1
#define ADC_DRC_ET_EN			0

/* Anolog Register */
/* SUNXI_ADCL_REG : 0x300 */
#define ADCL_EN			31
#define ADCL_DITHER_RESET	29
#define ADCL_LINEINL_EN		19
#define ADCL_LINEINL_SEL	16
#define ADCL_IOPAAFL		6
#define ADCL_IOPSDML1		4
#define ADCL_IOPSDML2		2
#define ADCL_IOPMICL		0

/* SUNXI_ADCR_REG : 0x304 */
#define ADCR_EN			31
#define ADCR_DITHER_RESET	29
#define ADCR_LINEINR_EN		19
#define ADCR_LINEINR_SEL	16
#define ADCR_IOPAAFL		6
#define ADCR_IOPSDML1		4
#define ADCR_IOPSDML2		2
#define ADCR_IOPMICL		0

/* SUNXI_DAC_REG : 0x310 */
#define DAC_CUR_TEST_SELECT	31
#define DAC_HP_GAIN		28
#define DAC_CPLDO_EN		27
#define DAC_CPLDO_VOLTAGE	24
#define DAC_OPDRV_CUR		22
#define	DAC_VRA2_IOPVRS		20
#define	DAC_ILINEOUTAMPS	18
#define DAC_IOPDACS		16
#define DACL_EN			15
#define DACR_EN			14
#define DAC_LINEOUTLEN		13
#define DAC_LMUTE		12
#define DAC_LINEOUTREN		11
#define DAC_RMUTE		10
#define DAC_LINEOUTLDIFFEN	6
#define DAC_LINEOUTRDIFFEN	5
#define DAC_LINEOUT_VOL		0

/* SUNXI_MICBIAS_REG : 0x318 */
#define SELDETADCFS		28
#define SELDETADCDB		26
#define SELDETADCBF		24
#define JACKDETEN		23
#define SELDETADCDY		21
#define MICADCEN		20
#define POPFREE			19
#define DETMODE			18
#define AUTOPLEN		17
#define MICDETPL		16
#define HMICBIASEN		15
#define HBIASSEL		13
#define	HMICBIAS_CHOP_EN	12
#define HMICBIAS_CHOP_CLK_SEL	10
#define MMICBIASEN		7
#define	MBIASSEL		5
#define	MMICBIAS_CHOP_EN	4
#define MMICBIAS_CHOP_CLK_SEL	2

/* SUNXI_BIAS_REG : 0x320 */
#define AC_BIASDATA		0

/* SUNXI_HP_REG : 0x324 */
#define HPRCALIVERIFY		24
#define HPLCALIVERIFY		16
#define HPPA_EN			15
#define HPINPUTEN		11
#define HPOUTPUTEN		10
#define HPPA_DEL		8
#define CP_CLKS			6
#define HPCALIMODE		5
#define HPCALIVERIFY		4
#define HPCALIFIRST		3
#define HPCALICKS		0

/* SUNXI_HMIC_CTRL : 0x328 */
#define HMIC_SAMPLE_SEL		21
#define MDATA_THRESHOLD		16
#define HMIC_SF			14
#define HMIC_M			10
#define HMIC_N			6
#define MDATA_THRESHOLD_DB	3
#define JACK_OUT_IRQ_EN		2
#define JACK_IN_IRQ_EN		1
#define MIC_DET_IRQ_EN		0

/* SUNXI_HMIC_STS : 0x32c */
#define MDATA_DISCARD		13
#define	HMIC_DATA		8
#define JACK_DET_OUT_ST		4
#define JACK_DET_OIRQ		4
#define JACK_DET_IIN_ST 	3
#define JACK_DET_IIRQ		3
#define MIC_DET_ST		0



/************** Internal I2S Register **************/
/* Base Address : 0x02031000 */
#define INTER_I2S_CTL		0x00
#define INTER_I2S_FMT0		0x04
#define INTER_I2S_FMT1		0x08
#define INTER_I2S_ISTA		0x0C
#define INTER_I2S_RXFIFO	0x10
#define INTER_I2S_FCTL		0x14
#define INTER_I2S_FSTA		0X18
#define	INTER_I2S_INT		0X1C
#define INTER_I2S_TXFIFIO	0X20
#define INTER_I2S_CLKDIV	0X24
#define INTER_I2S_TXCNT		0X28
#define INTER_I2S_RXCNT		0X2C
#define INTER_I2S_CHCFG		0X30
#define INTER_I2S_TX0CHSEL	0X34
#define INTER_I2S_TX1CHSEL	0X38
#define INTER_I2S_TX2CHSEL	0X3C
#define INTER_I2S_TX3CHSEL	0X40
#define INTER_I2S_TX0CHMAP0	0X44
#define INTER_I2S_TX0CHMAP1	0X48
#define INTER_I2S_TX1CHMAP0	0X4C
#define INTER_I2S_TX1CHMAP1	0X50
#define INTER_I2S_TX2CHMAP0	0X54
#define INTER_I2S_TX2CHMAP1	0X58
#define INTER_I2S_TX3CHMAP0	0X5C
#define INTER_I2S_TX3CHMAP1	0X60
#define INTER_I2S_RXCHSEL	0X64
#define INTER_I2S_RXCHMAP0	0X68
#define INTER_I2S_RXCHMAP1	0X6C
#define INTER_I2S_RXCHMAP2	0X70
#define INTER_I2S_RXCHMAP3	0X74
#define INTER_I2S_DBG		0x78
#define INTER_I2S_REV		0x7C

/* INTER_I2S_CTL : 0x00 */
#define I2S_BCLK_OUT		18
#define I2S_LRCK_OUT		17
#define I2S_DOUT3_EN		11
#define I2S_DOUT2_EN		10
#define I2S_DOUT1_EN		9
#define I2S_DOUT0_EN		8
#define I2S_OUT_MUTE		6
#define I2S_MODE_SEL		4
#define I2S_LOOP		3
#define I2S_TXEN		2
#define I2S_RXEN		1
#define I2S_GEN			0

/* INTER_I2S_FMT0 : 0x04 */
#define I2S_LRCK_WIDTH		30
#define I2S_LRCK_POLARITY	19
#define I2S_LRCK_PERIOD		8
#define I2S_BCLK_POLARITY	7
#define I2S_SR			4
#define I2S_EDGE_TRANSFER	3
#define I2S_SLOT_WIDTH		0

/* INTER_I2S_FMT1 : 0x08 */
#define I2S_RXMLS		7
#define I2S_TXMLS		6
#define I2S_SEXT		4

/* INTER_I2S_CLKDIV : 0x24 */
#define I2S_MCLKOUT_EN		8
#define I2S_BCLKDIV		4
#define I2S_MCLKDIV		0

/* INTER_I2S_CHCFG : 0x30 */
#define I2S_TX_SLOT_HIZ		9
#define I2S_TX_STATE		8

/* INTER_I2S_TX0CHSEL : 0x34 */
#define I2S_TX0_CHSEL		16
#define I2S_TX0_CHEN		0

/* INTER_I2S_TX1CHSEL : 0x38 */
#define I2S_TX1_CHSEL		16
#define I2S_TX1_CHEN		0

/* INTER_I2S_TX2CHSEL : 0x3C */
#define I2S_TX2_CHSEL		16
#define I2S_TX2_CHEN		0

/* INTER_I2S_TX3CHSEL : 0x40 */
#define I2S_TX3_CHSEL		16
#define I2S_TX3_CHEN		0

/* INTER_I2S_TX0CHMAP0 : 0x44 */
#define I2S_TX0_CH15_MAP	28
#define I2S_TX0_CH14_MAP	24
#define I2S_TX0_CH13_MAP	20
#define I2S_TX0_CH12_MAP	16
#define I2S_TX0_CH11_MAP	12
#define I2S_TX0_CH10_MAP	8
#define I2S_TX0_CH9_MAP		4
#define I2S_TX0_CH8_MAP		0

/* INTER_I2S_TX0CHMAP1 : 0x48 */
#define I2S_TX0_CH7_MAP		28
#define I2S_TX0_CH6_MAP		24
#define I2S_TX0_CH5_MAP		20
#define I2S_TX0_CH4_MAP		16
#define I2S_TX0_CH3_MAP		12
#define I2S_TX0_CH2_MAP		8
#define I2S_TX0_CH1_MAP		4
#define I2S_TX0_CH0_MAP		0

/* INTER_I2S_TX1CHMAP0 : 0x4C */
#define I2S_TX1_CH15_MAP	28
#define I2S_TX1_CH14_MAP	24
#define I2S_TX1_CH13_MAP	20
#define I2S_TX1_CH12_MAP	16
#define I2S_TX1_CH11_MAP	12
#define I2S_TX1_CH10_MAP	8
#define I2S_TX1_CH9_MAP		4
#define I2S_TX1_CH8_MAP		0

/* INTER_I2S_TX1CHMAP1 : 0x50 */
#define I2S_TX1_CH7_MAP		28
#define I2S_TX1_CH6_MAP		24
#define I2S_TX1_CH5_MAP		20
#define I2S_TX1_CH4_MAP		16
#define I2S_TX1_CH3_MAP		12
#define I2S_TX1_CH2_MAP		8
#define I2S_TX1_CH1_MAP		4
#define I2S_TX1_CH0_MAP		0

/* INTER_I2S_TX2CHMAP0 : 0x54 */
#define I2S_TX2_CH15_MAP	28
#define I2S_TX2_CH14_MAP	24
#define I2S_TX2_CH13_MAP	20
#define I2S_TX2_CH12_MAP	16
#define I2S_TX2_CH11_MAP	12
#define I2S_TX2_CH10_MAP	8
#define I2S_TX2_CH9_MAP		4
#define I2S_TX2_CH8_MAP		0

/* INTER_I2S_TX2CHMAP1 : 0x58 */
#define I2S_TX2_CH7_MAP		28
#define I2S_TX2_CH6_MAP		24
#define I2S_TX2_CH5_MAP		20
#define I2S_TX2_CH4_MAP		16
#define I2S_TX2_CH3_MAP		12
#define I2S_TX2_CH2_MAP		8
#define I2S_TX2_CH1_MAP		4
#define I2S_TX2_CH0_MAP		0

/* INTER_I2S_TX3CHMAP0 : 0x5C */
#define I2S_TX3_CH15_MAP	28
#define I2S_TX3_CH14_MAP	24
#define I2S_TX3_CH13_MAP	20
#define I2S_TX3_CH12_MAP	16
#define I2S_TX3_CH11_MAP	12
#define I2S_TX3_CH10_MAP	8
#define I2S_TX3_CH9_MAP		4
#define I2S_TX3_CH8_MAP		0

/* INTER_I2S_TX3CHMAP1 : 0x60 */
#define I2S_TX3_CH7_MAP		28
#define I2S_TX3_CH6_MAP		24
#define I2S_TX3_CH5_MAP		20
#define I2S_TX3_CH4_MAP		16
#define I2S_TX3_CH3_MAP		12
#define I2S_TX3_CH2_MAP		8
#define I2S_TX3_CH1_MAP		4
#define I2S_TX3_CH0_MAP		0

/* INTER_I2S_RXCHSEL : 0x64 */
#define I2S_RX_CHSEL		16

/* INTER_I2S_RXCHMAP0 : 0x68 */
#define I2S_RX_CH15_SEL		28
#define I2S_RX_CH15_MAP		24
#define I2S_RX_CH14_SEL		20
#define I2S_RX_CH14_MAP		16
#define I2S_RX_CH13_SEL		12
#define I2S_RX_CH13_MAP		8
#define I2S_RX_CH12_SEL		4
#define I2S_RX_CH12_MAP		0

/* INTER_I2S_RXCHMAP1 : 0x6C */
#define I2S_RX_CH11_SEL		28
#define I2S_RX_CH11_MAP		24
#define I2S_RX_CH10_SEL		20
#define I2S_RX_CH10_MAP		16
#define I2S_RX_CH9_SEL		12
#define I2S_RX_CH9_MAP		8
#define I2S_RX_CH8_SEL		4
#define I2S_RX_CH8_MAP		0

/* INTER_I2S_RXCHMAP2 : 0x70 */
#define I2S_RX_CH7_SEL		28
#define I2S_RX_CH7_MAP		24
#define I2S_RX_CH6_SEL		20
#define I2S_RX_CH6_MAP		16
#define I2S_RX_CH5_SEL		12
#define I2S_RX_CH5_MAP		8
#define I2S_RX_CH4_SEL		4
#define I2S_RX_CH4_MAP		0

/* INTER_I2S_RXCHMAP3 : 0x74 */
#define I2S_RX_CH3_SEL		28
#define I2S_RX_CH3_MAP		24
#define I2S_RX_CH2_SEL		20
#define I2S_RX_CH2_MAP		16
#define I2S_RX_CH1_SEL		12
#define I2S_RX_CH1_MAP		8
#define I2S_RX_CH0_SEL		4
#define I2S_RX_CH0_MAP		0

/* INTER_I2S_REV : 0x7C */
#define I2S_REV

/************** Other Definitions **************/
/* INTER_I2S_CTL:0x00 */
#define	INTER_I2S_MODE_CTL_LEFT		1
#define	INTER_I2S_MODE_CTL_RIGHT	2

#define	INTER_I2S_LRCK_POLARITY_NOR	0
#define	INTER_I2S_LRCK_POLARITY_INV	1
#define	INTER_I2S_BCLK_POLARITY_NOR	0
#define	INTER_I2S_BCLK_POLARITY_INV	1

/* INTER_I2S_CLKDIV : 0x24 */
/* Inter i2s bclk div level */
#define	INTER_I2S_BCLK_DIV_1		1
#define	INTER_I2S_BCLK_DIV_2		2
#define	INTER_I2S_BCLK_DIV_3		3
#define	INTER_I2S_BCLK_DIV_4		4
#define	INTER_I2S_BCLK_DIV_5		5
#define	INTER_I2S_BCLK_DIV_6		6
#define	INTER_I2S_BCLK_DIV_7		7
#define	INTER_I2S_BCLK_DIV_8		8
#define	INTER_I2S_BCLK_DIV_9		9
#define	INTER_I2S_BCLK_DIV_10		10
#define	INTER_I2S_BCLK_DIV_11		11
#define	INTER_I2S_BCLK_DIV_12		12
#define	INTER_I2S_BCLK_DIV_13		13
#define	INTER_I2S_BCLK_DIV_14		14
#define	INTER_I2S_BCLK_DIV_15		15
/* Inter i2s mclk div level */
#define	INTER_I2S_MCLK_DIV_1		1
#define	INTER_I2S_MCLK_DIV_2		2
#define	INTER_I2S_MCLK_DIV_3		3
#define	INTER_I2S_MCLK_DIV_4		4
#define	INTER_I2S_MCLK_DIV_5		5
#define	INTER_I2S_MCLK_DIV_6		6
#define	INTER_I2S_MCLK_DIV_7		7
#define	INTER_I2S_MCLK_DIV_8		8
#define	INTER_I2S_MCLK_DIV_9		9
#define	INTER_I2S_MCLK_DIV_10		10
#define	INTER_I2S_MCLK_DIV_11		11
#define	INTER_I2S_MCLK_DIV_12		12
#define	INTER_I2S_MCLK_DIV_13		13
#define	INTER_I2S_MCLK_DIV_14		14
#define	INTER_I2S_MCLK_DIV_15		15

/*125ms * (HP_DEBOUCE_TIME+1)*/
#define HP_DEBOUCE_TIME		0x1

#define REG_LABEL(constant)	{#constant, constant, 0}
#define REG_LABEL_END           {NULL, 0, 0}

/* debug printk */
#define LOG_ERR(fmt, arg...)	pr_err("[AUDIOCODEC][%s][%d]:" fmt "\n", __func__, __LINE__, ##arg)
#define LOG_WARN(fmt, arg...)	pr_warn("[AUDIOCODEC][%s][%d]:" fmt "\n", __func__, __LINE__, ##arg)
#define LOG_INFO(fmt, arg...)	pr_info("[AUDIOCODEC][%s][%d]:" fmt "\n", __func__, __LINE__, ##arg)

struct reg_label {
	const char *name;
	const unsigned int address;
	unsigned int value;
};

struct sample_rate {
	unsigned int samplerate;
	unsigned int rate_bit;
};

struct voltage_supply {
	struct regulator *avcc;
	struct regulator *cpvin;
};

struct codec_spk_config {
	u32 spk_gpio;
	u32 pa_msleep;
	bool spk_used;
	bool pa_level;
};

struct codec_hw_algo_config {
	bool adcdrc_used;
	bool dacdrc_used;
	bool adchpf_used;
	bool dachpf_used;
};

struct sunxi_codec_i2s_params {
	unsigned int i2s_global_enable;
	unsigned int daudio_master;
	unsigned int audio_format;
	unsigned int signal_inversion;
	unsigned int pcm_lrck_period;
	unsigned int slot_width_select;
	unsigned int frame_type;
	unsigned int tdm_config;
	unsigned int mclk_div;
};

struct sunxi_codec_info {
	struct device *dev;
	/* module reg resource about */
	void __iomem *codec_addr;
	void __iomem *i2s_addr;
	struct regmap *codec_regmap;
	struct regmap *i2s_regmap;

	/* module clk about */
	struct clk *pllclk;
	struct clk *dacclk;
	struct clk *adcclk;
	struct clk *codec_clk_bus;
	struct reset_control *codec_clk_rst;

	/* module regulator about */
	struct voltage_supply vol_supply;

	/* module private data about */
	u32 digital_vol;
	u32 lineout_vol;
	u32 hpout_vol;

	/* module speaker output about */
	struct codec_spk_config spk_cfg;

	/* module hardware algo about */
	struct codec_hw_algo_config algo_cfg;

	/* codec output data src select */
	int dac_data_src; /* 0 -> APB; 1 -> I2S */

	/* internal i2s part params about */
	struct sunxi_codec_i2s_params i2s_params;
};

#endif /* __SUN50IW12_CODEC_H */
