/*
 * Private header file for sunxi-rtc driver
 *
 * Copyright (C) 2018 Allwinner.
 *
 * Martin <wuyan@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _RTC_SUNXI_H_
#define _RTC_SUNXI_H_

#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/reset.h>

struct sunxi_rtc_data {
	u32 min_year;	/* Minimal real year allowed by hardware. eg: 1970. */
			/* For this IP, min_year can be set to any value that >= 1970, */
			/* for the valid value of (struct rtc_time).tm_year starts from 70 with an offset 1900. */
			/* See rtc_valid_tm() in drivers/rtc/lib.c */
	u32 max_year;	/* Maximum real year allowed by hardware. Can be caculated by HW_YEAR_MAX(min_year) */
	u32 year_mask;	/* bit mask  of YEAR field */
	u32 year_shift;	/* bit shift of YEAR field */
	u32 leap_shift;	/* bit shift of LEAP-YEAR field */
	u32 gpr_offset; /* Offset to General-Purpose-Register */
	u32 gpr_len;    /* Number of General-Purpose-Register */
};

struct sunxi_rtc_dev {
	struct rtc_device *rtc;
	struct device *dev;
	struct sunxi_rtc_data *data;
	struct clk *clk;
	struct clk *clk_bus;
	struct clk *clk_spi;
	struct reset_control *reset;
	void __iomem *base;
	int irq;
};

#endif /* end of _RTC_SUNXI_H_ */

