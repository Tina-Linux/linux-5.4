/*
 * Private header file for sunxi-ts driver
 *
 * Copyright (C) 2021 Allwinner.
 *
 * Xu Minghui <xuminghuis@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _SUNXI_TS_H_
#define _SUNXI_TS_H

#include <linux/clk.h>
#include <linux/reset.h>

#define TP_CTRL0		0x00
#define TP_CTRL1		0x04
#define TP_CTRL2		0x08
#define TP_CTRL3		0x0c
#define TP_INT_FIFOC		0x10
#define TP_INT_FIFOS		0x14
#define TP_TPR			0x18
#define TP_CDAT			0x1c
#define TEMP_DATA		0x20
#define TP_DATA			0x24

/* TP_CTRL0 bits */
#define ADC_FIRST_DLY(x)	((x) << 24) /* 8 bits */
#define ADC_FIRST_DLY_MODE(x)	((x) << 23)
#define ADC_CLK_SEL(x)		((x) << 22)
#define ADC_CLK_DIV(x)		((x) << 20) /* 3 bits */
#define FS_DIV(x)		((x) << 16) /* 4 bits */
#define T_ACQ(x)		((x) << 0) /* 16 bits */
#define T_ACQ_DEFAULT		0xffff

/* TP_CTRL1 bits */
#define STYLUS_UP_DEBOUN(x)	((x) << 12) /* 8 bits */
#define STYLUS_UP_DEBOUN_EN(x)	((x) << 9)
/* the other bits is written in compatible .data */

/* TP_CTRL2 bits */
#define TP_SENSITIVE_ADJUST(x)	((x) << 28) /* 4 bits */
#define TP_MODE_SELECT(x)	((x) << 26) /* 2 bits */
#define PRE_MEA_EN(x)		((x) << 24)
#define PRE_MEA_THRE_CNT(x)	((x) << 0) /* 24 bits */

/* TP_CTRL3 bits */
#define FILTER_EN(x)		((x) << 2)
#define FILTER_TYPE(x)		((x) << 0)  /* 2 bits */

/* TP_INT_FIFOC irq and fifo mask / control bits */
#define TEMP_IRQ_EN(x)		((x) << 18)
#define OVERRUN_IRQ_EN(x)	((x) << 17)
#define DATA_IRQ_EN(x)		((x) << 16)
#define TP_DATA_XY_CHANGE(x)	((x) << 13)
#define FIFO_TRIG(x)		((x) << 8)  /* 5 bits */
#define DATA_DRQ_EN(x)		((x) << 7)
#define FIFO_FLUSH(x)		((x) << 4)
#define TP_UP_IRQ_EN(x)		((x) << 1)
#define TP_DOWN_IRQ_EN(x)	((x) << 0)

/* TP_INT_FIFOS irq and fifo status bits */
#define TEMP_DATA_PENDING	BIT(18)
#define FIFO_OVERRUN_PENDING	BIT(17)
#define FIFO_DATA_PENDING	BIT(16)
#define TP_IDLE_FLG		BIT(2)
#define TP_UP_PENDING		BIT(1)
#define TP_DOWN_PENDING		BIT(0)

/* Registers which needs to be saved and restored before and after sleeping */
u32 sunxi_ts_regs_offset[] = {
	TP_CTRL0,
	TP_CTRL1,
	TP_CTRL2,
	TP_CTRL3,
	TP_INT_FIFOC,
	TP_TPR,
};

struct sunxi_ts_hwdata {
	u32 chopper_en_bitofs;
	u32 touch_pan_cali_en_bitofs;
	u32 tp_dual_en_bitofs;
	u32 tp_mode_en_bitofs;
	u32 tp_adc_select_bitofs;
};

struct sunxi_ts_config {
	u32 tp_sensitive_adjust;	/* tpadc sensitive parameter, from 0000(least sensitive) to 1111(most sensitive) */
	u32 filter_type;		/* tpadc filter type, eg:(Median Filter Size/Averaging Filter Size)
					   00(4/2), 01(5/3), 10(8/4), 11(16/8)*/
};

struct sunxi_ts {
	struct device *dev;
	struct input_dev *input;
	void __iomem *base;
	int irq;
	struct clk *bus_clk;
	struct clk *mod_clk;
	struct reset_control *reset;
	struct sunxi_ts_hwdata *hwdata;  /* to distinguish platform own register */
	struct sunxi_ts_config ts_config;
	u32 regs_backup[ARRAY_SIZE(sunxi_ts_regs_offset)];
};

#endif /* end of _SUNXI_TS_H */
