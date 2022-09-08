// SPDX-License-Identifier: SimPL-2.0
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (c) 2014
 *
 * ChangeLog
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/reset.h>
#include <linux/clk/sunxi.h>

#define REG_TP_KEY_CTL0			0X00
#define REG_TP_KEY_CTL1			0X04
#define REG_TP_KEY_CTL2			0X08
#define REG_TP_KEY_CTL3			0X0C
#define REG_TP_KEY_INT_CTL		0X10
#define REG_TP_KEY_INT_STS		0X14
#define REG_TP_KEY_COM_DAT		0X1C
#define REG_TP_KEY_ADC_DAT		0X24

#define TP_KEY_SHORT_DLY_CNT	2
/* voltage range 0~2.3v, unit is uv */
#define VOL_RANGE				(1800000UL)

struct sunxi_tpadc {
	struct platform_device	*pdev;
	struct input_dev *input_dev;
	struct device *dev;
	void __iomem	*reg_base;
	int				irq_num;
	int				key_num;
	unsigned int	*key_code;
	int				*key_val;
	struct clk *bus_clk;
	struct clk *mod_clk;
	struct reset_control *reset;
};

struct sunxi_tpadc *sunxi_tpadc;

static ssize_t
tpadc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 data, vol_data, reg_val;

	reg_val = readl((void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_INT_STS));
	data = readl((void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_ADC_DAT));
	writel(reg_val, (void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_INT_STS));

	data = ((VOL_RANGE / 4096) * data);	/* 12bits sample rate */
	vol_data = data / 1000;			//data to val_data

	return sprintf(buf, "%d\n", vol_data);
}

static ssize_t
tpadc_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	return 0;
}

static DEVICE_ATTR(tpadc, 0444, tpadc_show, tpadc_store);

static int sunxi_tp_clk_enable(struct sunxi_tpadc *ts)
{
	if (ts->reset)
		reset_control_deassert(ts->reset);

	if (ts->mod_clk)
		clk_prepare_enable(ts->mod_clk);

	if (ts->bus_clk)
		clk_prepare_enable(ts->bus_clk);

	return 0;
}

static int tp_key_which_key(struct sunxi_tpadc *sunxi_tpadc, u32 adc)
{
	int i = 0;

	if (!sunxi_tpadc) {
		pr_info("%s:invalid key data\n", __func__);
		return -1;
	}

	for (i = 0; i < sunxi_tpadc->key_num; i++) {
		if (adc < sunxi_tpadc->key_val[i])
			break;
	}

	if (i >= sunxi_tpadc->key_num)
		return -1;

	return i;
}

static irqreturn_t tp_key_isr(int irq, void *dummy)
{
	struct sunxi_tpadc *sunxi_tpadc = (struct sunxi_tpadc *)dummy;
	u32  reg_val = 0, reg_data = 0;
	int index = 0;
	static int key_cnt, prev_key = -1, key_down_flag;

	reg_val = readl((void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_INT_STS));
	reg_data = readl((void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_ADC_DAT));
	writel(reg_val, (void __iomem *)
		(sunxi_tpadc->reg_base + REG_TP_KEY_INT_STS));

	index = tp_key_which_key(sunxi_tpadc, reg_data);
	if (prev_key == -1 && index == -1)
		return IRQ_HANDLED;

	if (key_down_flag && prev_key >= 0 && prev_key != index) {
		/*if key down and current key is not equal to prev key, then the key up */
		input_report_key(sunxi_tpadc->input_dev, sunxi_tpadc->key_code[prev_key], 0);
		/* report key up event */
		input_sync(sunxi_tpadc->input_dev);
		key_down_flag = 0;
		prev_key = -1;
		key_cnt = 0;
		return IRQ_HANDLED;
	}

	if (prev_key == -1 && index >= 0)
		/* save first key index */
		prev_key = index;

	if (prev_key >= 0 && index >= 0 && prev_key != index) {
		pr_info("fatal: must clear all flag!\n");
		/* it is an error and clear all */
		prev_key = -1;
		key_cnt = 0;
		if (key_down_flag) {
			input_report_key(sunxi_tpadc->input_dev, sunxi_tpadc->key_code[index], 0);
			input_sync(sunxi_tpadc->input_dev);
		}
		key_down_flag = 0;
		return IRQ_HANDLED;
	}

	if (index >= 0) {
		if (!key_down_flag)
			key_cnt++;
	}
	if (key_cnt < TP_KEY_SHORT_DLY_CNT) {
		if (index == -1) { /* is a fake key */
			key_down_flag = 0;
			prev_key = -1;
			key_cnt = 0;
		}
		return IRQ_HANDLED;
	}

	if (!key_down_flag && prev_key == index) {
		/* if the current key equal prev key index, then the key down */
		input_report_key(sunxi_tpadc->input_dev, sunxi_tpadc->key_code[prev_key], 1);
		/* report key down event */
		input_sync(sunxi_tpadc->input_dev);
		key_down_flag = 1;
	}

	return IRQ_HANDLED;
}

static int tp_key_dts_parse(struct device *pdev, struct sunxi_tpadc *sunxi_tpadc)
{
	struct device_node *np = pdev->of_node;
	u32 val[2] = {0, 0};
	int ret = 0, i = 0;
	char key_name[16];

	sunxi_tpadc->reg_base = of_iomap(np, 0);
	if (!sunxi_tpadc->reg_base) {
		pr_info("%s:Failed to ioremap io memory region.\n", __func__);
		ret = -ENODEV;
	}

	sunxi_tpadc->irq_num = irq_of_parse_and_map(np, 0);
	if (sunxi_tpadc->irq_num == 0) {
		pr_info("%s:Failed to map irq.\n", __func__);
		ret = -ENODEV;
	}

	sunxi_tpadc->reset = devm_reset_control_get(sunxi_tpadc->dev, NULL);
	if (sunxi_tpadc->reset) {
		reset_control_assert(sunxi_tpadc->reset);
		reset_control_deassert(sunxi_tpadc->reset);
	} else {
		pr_err("get tpadc reset failed\n");
	}

	sunxi_tpadc->mod_clk = devm_clk_get(sunxi_tpadc->dev, "mod");
	if (sunxi_tpadc->mod_clk)
		clk_prepare_enable(sunxi_tpadc->mod_clk);
	else
		pr_err("get tpadc mode clock failed.\n");

	sunxi_tpadc->bus_clk = devm_clk_get(sunxi_tpadc->dev, "bus");
	if (sunxi_tpadc->bus_clk)
		clk_prepare_enable(sunxi_tpadc->bus_clk);
	else
		pr_err("get tpadc bus clock failed\n");

	if (of_property_read_u32(np, "key_cnt", &sunxi_tpadc->key_num)) {
		pr_info("%s: get key count failed\n", __func__);
		return -ENODEV;
	}

	if (sunxi_tpadc->key_num <= 0) {
		pr_info("key num is not right\n");
		return -ENODEV;
	}

	sunxi_tpadc->key_code = devm_kzalloc(pdev, sizeof(int) * sunxi_tpadc->key_num, GFP_KERNEL);
	if (IS_ERR_OR_NULL(sunxi_tpadc->key_code)) {
		pr_info("sunxi_tpadc: not enough memory for key code array\n");
		return -ENOMEM;
	}

	sunxi_tpadc->key_val = devm_kzalloc(pdev, sizeof(int) * sunxi_tpadc->key_num, GFP_KERNEL);
	if (IS_ERR_OR_NULL(sunxi_tpadc->key_val)) {
		pr_info("sunxi_tpadc: not enough memory for key adc value array\n");
		return -ENOMEM;
	}

	for (i = 1; i <= sunxi_tpadc->key_num; i++) {
		sprintf(key_name, "key%d", i);
		if (of_property_read_u32_array(np, key_name, val, ARRAY_SIZE(val))) {
			pr_err("%s: get %s err!\n", __func__, key_name);
			return -EBUSY;
		}
		sunxi_tpadc->key_val[i - 1] = val[0];
		sunxi_tpadc->key_code[i - 1] = val[1];
		pr_info("key adc value:%d key code:%d\n",
			sunxi_tpadc->key_val[i - 1], sunxi_tpadc->key_code[i - 1]);
	}

	return 0;
}

static int tp_key_hw_init(void __iomem *reg_base)
{
	u32 val = 0;

	val = 0XF << 24;
	val |= 1 << 23;
	val &= ~(1 << 22); /*sellect HOSC(24MHz)*/
	val |= 0x3 << 20; /*00:CLK_IN/2,01:CLK_IN/3,10:CLK_IN/6,11:CLK_IN/1*/
	val |= 0x7f << 0; /*FS DIV*/
	writel(val, (void __iomem *)(reg_base + REG_TP_KEY_CTL0));

	val = 1 << 4 | 1 << 5; /* select adc mode and enable tp */
	writel(val, (void __iomem *)(reg_base + REG_TP_KEY_CTL1));

	val = 1 << 16; /* enable date irq */
	writel(val, (void __iomem *)(reg_base + REG_TP_KEY_INT_CTL));

	val = readl((void __iomem *)(reg_base + REG_TP_KEY_CTL1));
	val &= ~(0x0f);
	val |= 1 << 0;   /* select X1 */
	writel(val, (void __iomem *)(reg_base + REG_TP_KEY_CTL1));

	/* clear fifo */
	val = readl((void __iomem *)(reg_base + REG_TP_KEY_INT_CTL));
	val |= 1 << 4;
	writel(val, (void __iomem *)(reg_base + REG_TP_KEY_INT_CTL));

	return 0;
}

static int tp_key_probe(struct platform_device *pdev)
{
	int i;
	int ret = -1;
	struct input_dev *input_dev = NULL;

	pr_info("%s: start\n", __func__);
	sunxi_tpadc = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_tpadc), GFP_KERNEL);
	if (IS_ERR_OR_NULL(sunxi_tpadc)) {
		pr_info("sunxi_tpadc: not enough memory for key data\n");
		return -ENOMEM;
	}
	sunxi_tpadc->pdev = pdev;
	sunxi_tpadc->dev = &pdev->dev;

	ret = tp_key_dts_parse(&pdev->dev, sunxi_tpadc);
	if (ret != 0)
		return ret;

	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_info("%s:allocate input device fail\n", __func__);
		return -ENOMEM;
	}

	input_dev->name = "sunxi-tpadc";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;

	__set_bit(EV_REP, input_dev->evbit); /* support key repeat */
	__set_bit(EV_KEY, input_dev->evbit); /* support key repeat */

	for (i = 0; i < sunxi_tpadc->key_num; i++)
		__set_bit(sunxi_tpadc->key_code[i], input_dev->keybit);

	 sunxi_tpadc->input_dev = input_dev;

	/* hardware setting */
	if (sunxi_tp_clk_enable(sunxi_tpadc)) {
		pr_err("%s:request tpadc clk failed\n", __func__);
		return -EPROBE_DEFER;
	}
	tp_key_hw_init(sunxi_tpadc->reg_base);

	platform_set_drvdata(pdev, sunxi_tpadc);
	dev_set_drvdata(&input_dev->dev, sunxi_tpadc);

	ret = input_register_device(input_dev);
	if (ret) {
		pr_err("%s:register input device error\n", __func__);
		input_free_device(input_dev);
		return ret;
	}

	if (request_irq(sunxi_tpadc->irq_num, tp_key_isr,
			IRQF_TRIGGER_NONE, "sunxi_tpadc", sunxi_tpadc)) {
		input_unregister_device(input_dev);
		input_free_device(input_dev);
		pr_err("%s:sunxi_gpadc request irq failure\n", __func__);
		return -1;
	}

	if (device_create_file(&input_dev->dev, &dev_attr_tpadc))
		pr_err("%s: couldn't create device file for status\n", __func__);

	return 0;
}

static int tp_key_remove(struct platform_device *pdev)
{
	struct sunxi_tpadc *sunxi_tpadc = platform_get_drvdata(pdev);

	input_unregister_device(sunxi_tpadc->input_dev);
	input_free_device(sunxi_tpadc->input_dev);

	free_irq(sunxi_tpadc->irq_num, sunxi_tpadc);
	kfree(sunxi_tpadc);
	return 0;
}

static struct of_device_id tp_key_of_match[] = {
	{ .compatible = "allwinner,tp_key",},
	{ },
};
MODULE_DEVICE_TABLE(of, tp_key_of_match);

static struct platform_driver tp_key_driver = {
	.probe  = tp_key_probe,
	.remove = tp_key_remove,
	.driver = {
		.name   = "sunxi-tpadc",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(tp_key_of_match),
	},
};

module_platform_driver(tp_key_driver);
MODULE_AUTHOR("Edwin");
MODULE_DESCRIPTION("sunxi-tpadc AW1859 driver");
MODULE_LICENSE("GPL");
