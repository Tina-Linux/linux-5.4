// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Allwinner sunxi resistive touchscreen controller driver
 *
 * Copyright (C) 2021 Allwinner Tech Co., Ltd
 * Author: Xu Minghui <xuminghuis@allwinnertech.com>
 *
 * Copyright (C) 2013 - 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * The hwmon parts are based on work by Corentin LABBE which is:
 * Copyright (C) 2013 Corentin LABBE <clabbe.montjoie@gmail.com>
 */

/*
 * The sunxi-ts controller is capable of detecting a second touch, but when a
 * second touch is present then the accuracy becomes so bad the reported touch
 * location is not useable.
 *
 * The original android driver contains some complicated heuristics using the
 * aprox. distance between the 2 touches to see if the user is making a pinch
 * open / close movement, and then reports emulated multi-touch events around
 * the last touch coordinate (as the dual-touch coordinates are worthless).
 *
 * These kinds of heuristics are just asking for trouble (and don't belong
 * in the kernel). So this driver offers straight forward, reliable single
 * touch functionality only.
 *
 */

//#define DEBUG /* Enable dev_dbg */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/err.h>
#include "sunxi-ts.h"

static int sunxi_ts_clk_enable(struct sunxi_ts *chip)
{
	int err;
	struct device *dev = chip->dev;

	err = reset_control_reset(chip->reset);
	if (err) {
		dev_err(dev, "reset_control_reset() failed\n");
		goto err0;
	}

	err = clk_prepare_enable(chip->mod_clk);
	if (err) {
		dev_err(dev, "Cannot enable ts->mod_clk\n");
		goto err1;
	}

	err = clk_prepare_enable(chip->bus_clk);
	if (err) {
		dev_err(dev, "Cannot enable ts->bus_clk\n");
		goto err2;
	}

	return 0;

err2:
	clk_disable_unprepare(chip->mod_clk);
err1:
	reset_control_assert(chip->reset);
err0:
	return err;
}

static void sunxi_ts_clk_disable(struct sunxi_ts *chip)
{
	if (chip->bus_clk)
		clk_disable_unprepare(chip->bus_clk);

	if (chip->mod_clk)
		clk_disable_unprepare(chip->mod_clk);

	if (chip->reset)
		reset_control_assert(chip->reset);
}

static int sunxi_ts_clk_get(struct sunxi_ts *chip)
{
	struct device *dev = chip->dev;

	chip->reset = devm_reset_control_get(chip->dev, NULL);
	if (IS_ERR(chip->reset)) {
		dev_err(dev, "get tpadc reset failed\n");
		return PTR_ERR(chip->reset);
	}

	chip->mod_clk = devm_clk_get(chip->dev, "mod");
	if (IS_ERR(chip->mod_clk)) {
		dev_err(dev, "get tpadc mode clock failed\n");
		return PTR_ERR(chip->mod_clk);
	}

	chip->bus_clk = devm_clk_get(chip->dev, "bus");
	if (IS_ERR(chip->bus_clk)) {
		dev_err(dev, "get tpadc bus clock failed\n");
		return PTR_ERR(chip->bus_clk);
	}

	return 0;
}

static void sunxi_ts_clk_put(struct sunxi_ts *chip)
{

}

static int sunxi_ts_clk_init(struct sunxi_ts *chip)
{
	int err;
	struct device *dev = chip->dev;

	err = sunxi_ts_clk_get(chip);
	if (err) {
		dev_err(dev, "sunxi_ts_clk_get() failed\n");
		return err;
	}

	err = sunxi_ts_clk_enable(chip);
	if (err) {
		dev_err(dev, "sunxi_ts_clk_enable() failed\n");
		return err;
	}

	return 0;
}

static void sunxi_ts_clk_deinit(struct sunxi_ts *chip)
{
	sunxi_ts_clk_disable(chip);
	sunxi_ts_clk_put(chip);
}

static irqreturn_t sunxi_ts_irq_handler(int irq, void *dev_id)
{
	struct sunxi_ts *chip = dev_id;
	struct device *dev = chip->dev;
	u32 x, y, reg_val;
	static bool ignore_first_packet = true;

	/* Read irq flags */
	reg_val = readl(chip->base + TP_INT_FIFOS);
	if (reg_val & FIFO_DATA_PENDING) {
		dev_dbg(dev, "sunxi-ts fifo data pending\n");

		/* The 1st location reported after an up event is unreliable */
		if (!ignore_first_packet) {
			dev_dbg(dev, "sunxi-ts report fifo data\n");
			x = readl(chip->base + TP_DATA);
			y = readl(chip->base + TP_DATA);
			input_report_abs(chip->input, ABS_X, x);
			input_report_abs(chip->input, ABS_Y, y);
			/*
			 * The hardware has a separate down status bit, but
			 * that gets set before we get the first location,
			 * resulting in reporting a click on the old location.
			 */
			input_report_key(chip->input, BTN_TOUCH, 1);	/* 1: report touch down event, 0: report touch up event*/
			input_sync(chip->input);
		} else {
			dev_dbg(dev, "sunxi-ts ignore first fifo data\n");
			ignore_first_packet = false;
		}
	}

	if (reg_val & TP_UP_PENDING) {
		dev_dbg(dev, "sunxi-ts up pending\n");
		ignore_first_packet = true;
		input_report_key(chip->input, BTN_TOUCH, 0);
		input_sync(chip->input);
	}

	/* Clear irq flags */
	writel(reg_val, chip->base + TP_INT_FIFOS);

	return IRQ_HANDLED;
}

static int sunxi_ts_open(struct input_dev *dev)
{
	struct sunxi_ts *chip = input_get_drvdata(dev);
	u32 reg_val;

	/* Active input IRQs */
	reg_val = readl(chip->base + TP_INT_FIFOC);
	reg_val |= TEMP_IRQ_EN(1) | DATA_IRQ_EN(1) | FIFO_TRIG(1) | FIFO_FLUSH(1) | TP_UP_IRQ_EN(1) |
		OVERRUN_IRQ_EN(1) | DATA_DRQ_EN(1) | TP_DOWN_IRQ_EN(1);
	writel(reg_val, chip->base + TP_INT_FIFOC);

	return 0;
}

static void sunxi_ts_close(struct input_dev *dev)
{
	struct sunxi_ts *chip = input_get_drvdata(dev);

	/* Deactive input IRQs */
	writel(0, chip->base + TP_INT_FIFOC);
}

static void sunxi_ts_calibrate(struct sunxi_ts *chip)
{
	u32 reg_val;

	reg_val = readl(chip->base + TP_CTRL0);
	writel(reg_val | T_ACQ_DEFAULT, chip->base + TP_CTRL0);

	reg_val = readl(chip->base + TP_CTRL1);
	writel(reg_val | BIT(chip->hwdata->touch_pan_cali_en_bitofs), chip->base + TP_CTRL1);
}

static void sunxi_ts_hwinit(struct sunxi_ts *chip)
{
	u32 reg_val;
	struct device_node *np = chip->dev->of_node;
	struct sunxi_ts_config *ts_config = &chip->ts_config;
	ts_config->tp_sensitive_adjust = 15;
	ts_config->filter_type = 1;

	writel(ADC_FIRST_DLY(1) | ADC_FIRST_DLY_MODE(1) | ADC_CLK_DIV(2) |
			FS_DIV(2) | T_ACQ(5), chip->base + TP_CTRL0);

	/*
	 * tp_sensitive_adjust is an optional property
	 * tp_mode = 0 : only x and y coordinates, as we don't use dual touch
	 */
	of_property_read_u32(np, "allwinner,tp-sensitive-adjust",
			     &ts_config->tp_sensitive_adjust);
	writel(TP_SENSITIVE_ADJUST(ts_config->tp_sensitive_adjust) | TP_MODE_SELECT(0),
	       chip->base + TP_CTRL2);

	reg_val = readl(chip->base + TP_CTRL2);
	writel(reg_val | PRE_MEA_THRE_CNT(0xfff), chip->base + TP_CTRL2);

	/*
	 * Enable median and averaging filter, optional property for
	 * filter type.
	 */
	of_property_read_u32(np, "allwinner,filter-type", &ts_config->filter_type);
	writel(FILTER_EN(1) | FILTER_TYPE(ts_config->filter_type), chip->base + TP_CTRL3);

	/*
	 * Set stylus up debounce to aprox 10 ms, enable debounce, and
	 * finally enable tp mode.
	 */
	reg_val = STYLUS_UP_DEBOUN(5) | STYLUS_UP_DEBOUN_EN(1)
		| BIT(chip->hwdata->chopper_en_bitofs) | BIT(chip->hwdata->tp_mode_en_bitofs);

	writel(reg_val, chip->base + TP_CTRL1);
}

static void sunxi_ts_init_input(struct sunxi_ts *chip)
{
	chip->input->name = "sunxi-ts";
	chip->input->phys = "sunxi_ts/input0";
	chip->input->open = sunxi_ts_open;
	chip->input->close = sunxi_ts_close;
	chip->input->id.bustype = BUS_HOST;
	chip->input->id.vendor = 0x0001;
	chip->input->id.product = 0x0001;
	chip->input->id.version = 0x0100;
	chip->input->evbit[0] =  BIT(EV_SYN) | BIT(EV_KEY) | BIT(EV_ABS);
	__set_bit(BTN_TOUCH, chip->input->keybit);
	input_set_abs_params(chip->input, ABS_X, 0, 4095, 0, 0);
	input_set_abs_params(chip->input, ABS_Y, 0, 4095, 0, 0);
	input_set_drvdata(chip->input, chip);
}

static struct sunxi_ts_hwdata sun8iw20p1_ts_hwdata = {
	.chopper_en_bitofs        = 8,
	.touch_pan_cali_en_bitofs = 7,
	.tp_dual_en_bitofs        = 6,
	.tp_mode_en_bitofs        = 5,
	.tp_adc_select_bitofs     = 4,
};

static const struct of_device_id sunxi_ts_of_match[] = {
	{ .compatible = "allwinner,sun8i-ts", .data = &sun8iw20p1_ts_hwdata},
	{ /* sentinel */ }
};

static int sunxi_ts_probe(struct platform_device *pdev)
{
	struct sunxi_ts *chip;
	struct device *dev = &pdev->dev;
	const struct of_device_id *of_id;
	int error;

	dev_dbg(dev, "%s(): BEGIN\n", __func__);

	chip = devm_kzalloc(dev, sizeof(struct sunxi_ts), GFP_KERNEL);
	if (!chip) {
		dev_err(dev, "devm_kzalloc() failed\n");
		error = -ENOMEM;
		goto err0;
	}

	platform_set_drvdata(pdev, chip);

	of_id = of_match_device(sunxi_ts_of_match, &pdev->dev);
	if (!of_id) {
		dev_err(dev, "of_match_device() failed\n");
		error = -EINVAL;
		goto err0;
	}

	chip->dev = dev;
	chip->hwdata = (struct sunxi_ts_hwdata *)(of_id->data);

	error = sunxi_ts_clk_init(chip);
	if (error) {
		dev_err(dev, "sunxi_ts_clk_init() failed\n");
		goto err0;
	}

	chip->input = devm_input_allocate_device(dev);
	if (!chip->input) {
		dev_err(dev, "devm_input_allocate_device() failed\n");
		error = -ENOMEM;
		goto err1;
	}
	sunxi_ts_init_input(chip);
	error = input_register_device(chip->input);
	if (error) {
		dev_err(dev, "sunxi ts register failed\n");
		goto err2;
	}

	chip->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(chip->base)) {
		dev_err(dev, "Fail to map IO resource\n");
		error = PTR_ERR(chip->base);
		goto err3;
	}

	sunxi_ts_calibrate(chip);
	sunxi_ts_hwinit(chip);

	chip->irq = platform_get_irq(pdev, 0);
	error = devm_request_irq(dev, chip->irq, sunxi_ts_irq_handler, 0, "sunxi-ts", chip);
	if (error) {
		dev_err(dev, "tpadc request irq failed\n");
		goto err3;
	}

	dev_dbg(dev, "%s(): END\n", __func__);

	return 0;

err3:
	input_unregister_device(chip->input);
err2:
	writel(0, chip->base + TP_INT_FIFOC);	/* clear interrupt regs */
err1:
	sunxi_ts_clk_deinit(chip);
err0:
	return error;
}

static int sunxi_ts_remove(struct platform_device *pdev)
{
	struct sunxi_ts *chip = platform_get_drvdata(pdev);

	/* Explicit unregister to avoid open/close changing the imask later */
	input_unregister_device(chip->input);

	/* Deactivate all IRQs */
	writel(0, chip->base + TP_INT_FIFOC);

	/* Disable all clks */
	sunxi_ts_clk_deinit(chip);

	return 0;
}

#ifdef CONFIG_PM
static inline void sunxi_ts_save_regs(struct sunxi_ts *chip)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_ts_regs_offset); i++)
		chip->regs_backup[i] = readl(chip->base + sunxi_ts_regs_offset[i]);
}

static inline void sunxi_ts_restore_regs(struct sunxi_ts *chip)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_ts_regs_offset); i++)
		writel(chip->regs_backup[i], chip->base + sunxi_ts_regs_offset[i]);
}

static int sunxi_ts_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_ts *chip = platform_get_drvdata(pdev);

	sunxi_ts_save_regs(chip);
	sunxi_ts_clk_disable(chip);

	return 0;
}

static int sunxi_ts_resume_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_ts *chip = platform_get_drvdata(pdev);

	sunxi_ts_clk_enable(chip);
	sunxi_ts_restore_regs(chip);

	return 0;
}

static const struct dev_pm_ops sunxi_ts_dev_pm_ops = {
	.suspend_noirq = sunxi_ts_suspend_noirq,
	.resume_noirq = sunxi_ts_resume_noirq,
};
#define SUNXI_TS_DEV_PM_OPS (&sunxi_ts_dev_pm_ops)
#else
#define SUNXI_TS_DEV_PM_OPS NULL
#endif


MODULE_DEVICE_TABLE(of, sunxi_ts_of_match);

static struct platform_driver sunxi_ts_driver = {
	.driver = {
		.name	= "sunxi-ts",
		.of_match_table = of_match_ptr(sunxi_ts_of_match),
		.pm = SUNXI_TS_DEV_PM_OPS,
	},
	.probe	= sunxi_ts_probe,
	.remove	= sunxi_ts_remove,
};

module_platform_driver(sunxi_ts_driver);

MODULE_DESCRIPTION("Allwinner sunxi resistive touchscreen controller driver");
MODULE_AUTHOR("Xu Minghui<Xuminghuis@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.0");
