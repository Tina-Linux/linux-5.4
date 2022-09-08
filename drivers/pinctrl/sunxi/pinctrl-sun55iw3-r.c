// SPDX-License-Identifier: (GPL-2.0+ or MIT)
/*
 * Copyright (c) 2021 liujuan1@allwinnertech.com
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include "pinctrl-sunxi.h"

static const struct sunxi_desc_pin sun55iw3_r_pins[] = {
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x3, "s_twi0")),		/* SCK */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x3, "s_twi0")),		/* SDA */
};

static const struct sunxi_pinctrl_desc sun55iw3_r_pinctrl_data = {
	.pins = sun55iw3_r_pins,
	.npins = ARRAY_SIZE(sun55iw3_r_pins),
	.pin_base = SUNXI_PIN_BASE('L'),
	.hw_type = SUNXI_PCTL_HW_TYPE_0,
};

static int sun55iw3_r_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_pinctrl_init(pdev, &sun55iw3_r_pinctrl_data);
}

static struct of_device_id sun55iw3_r_pinctrl_match[] = {
	{ .compatible = "allwinner,sun55iw3-r-pinctrl", },
	{}
};
MODULE_DEVICE_TABLE(of, sun55iw3_r_pinctrl_match);

static struct platform_driver sun55iw3_r_pinctrl_driver = {
	.probe	= sun55iw3_r_pinctrl_probe,
	.driver	= {
		.name		= "sun55iw3-r-pinctrl",
		.of_match_table	= sun55iw3_r_pinctrl_match,
	},
};

static int __init sun55iw3_r_pio_init(void)
{
	return platform_driver_register(&sun55iw3_r_pinctrl_driver);
}
postcore_initcall(sun55iw3_r_pio_init);

MODULE_DESCRIPTION("Allwinner sun55iw3 R_PIO pinctrl driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
