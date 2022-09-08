// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 liujuan1@allwinnertech.com
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_nm.h"

#include "ccu-sun55iw3-r.h"

static const char * const ahbs_parents[] = { "dcxo24M", "ext-32k",
						   "rc-16m", "pll-peri0-div3",
						   "pll-audio1-4x" };

static SUNXI_CCU_M_WITH_MUX(r_ahb_clk, "r-ahb",
			     ahbs_parents, 0x000,
			     0, 5,
			     24, 3,
			     0);

static SUNXI_CCU_M_WITH_MUX(r_apbs0_clk, "r-apbs0",
			     ahbs_parents, 0x00c,
			     0, 5,
			     24, 3,
			     0);

static SUNXI_CCU_M_WITH_MUX(r_apbs1_clk, "r-apbs1",
			     ahbs_parents, 0x010,
			     0, 5,
			     24, 3,
			     0);

static const char * const r_timer_parents[] = { "dcxo24M", "ext-32k",
						   "rc-16m", "pll-peri-200m" };

static struct ccu_div r_timer0_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0100,
		.hw.init	= CLK_HW_INIT("r-timer0",
					      "r_timer_parents",
					      &ccu_div_ops, 0),
	},
};

static struct ccu_div r_timer1_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0104,
		.hw.init	= CLK_HW_INIT("r-timer1",
					      "r_timer_parents",
					      &ccu_div_ops, 0),
	},
};

static struct ccu_div r_timer2_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0108,
		.hw.init	= CLK_HW_INIT("r-timer2",
					      "r_timer_parents",
					      &ccu_div_ops, 0),
	},
};

static SUNXI_CCU_GATE(r_timer_gating_clk, "r-timer-gating",
                      "dcxo24M",
                      0x011c, BIT(0), 0);

static SUNXI_CCU_GATE(r_twd_gating_clk, "r-twd-gating",
                      "dcxo24M",
                      0x012c, BIT(0), 0);

static const char * const r_pwm_parents[] = { "dcxo24M", "ext-32k", "rc-16m" };

static SUNXI_CCU_MUX_WITH_GATE(r_pwm_clk, "r-pwm",
			r_pwm_parents, 0x0130,
			24, 2,
			BIT(31), 0 );

static SUNXI_CCU_GATE(r_pwm_gating_clk, "r-pwm-gating",
                      "dcxo24M",
                      0x013c, BIT(0), 0);

static SUNXI_CCU_GATE(r_can_gating_clk, "r-can-gating",
                      "dcxo24M",
                      0x014c, BIT(0), 0);

static const char * const r_spi_parents[] = { "dcxo24M", "pll-peri0-div3",
					      "pll-peri0-300m", "pll-peri1-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_spi_clk, "r-spi",
                                 r_spi_parents, 0x0150,
			         0, 5,	/* M */
			         24, 2,	/* mux */
				 BIT(31),	/* gate */
				 CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(r_spi_gating_clk, "r-spi-gating",
                      "dcxo24M",
                      0x015c, BIT(0), 0);

static SUNXI_CCU_GATE(r_splock_gating_clk, "r-splock-gating",
                      "dcxo24M",
                      0x016c, BIT(0), 0);

static SUNXI_CCU_GATE(r_mbox_gating_clk, "r-mbox-gating",
                      "dcxo24M",
                      0x017c, BIT(0), 0);

static SUNXI_CCU_GATE(r_uart1_gating_clk, "r-uart1-gating",
                      "dcxo24M",
                      0x018c, BIT(1), 0);

static SUNXI_CCU_GATE(r_uart0_gating_clk, "r-uart0-gating",
                      "dcxo24M",
                      0x018c, BIT(1), 0);

static SUNXI_CCU_GATE(r_twi1_gating_clk, "r-twi1-gating",
                      "dcxo24M",
                      0x019c, BIT(1), 0);

static SUNXI_CCU_GATE(r_twi0_gating_clk, "r-twi0-gating",
                      "dcxo24M",
                      0x019c, BIT(0), 0);

static SUNXI_CCU_GATE(r_ppu1_gating_clk, "r-ppu1-gating",
                      "dcxo24M",
                      0x01ac, BIT(1), 0);

static SUNXI_CCU_GATE(r_ppu_gating_clk, "r-ppu-gating",
                      "dcxo24M",
                      0x01ac, BIT(0), 0);

static SUNXI_CCU_GATE(r_tzma_gating_clk, "r-tzma-gating",
                      "dcxo24M",
                      0x01b0, BIT(0), 0);

static SUNXI_CCU_GATE(r_cpus_bist_gating_clk, "r-cpus-bist-gating",
                      "dcxo24M",
                      0x01bc, BIT(0), 0);

static const char * const r_irrx_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_irrx_clk, "r-irrx",
                                 r_spi_parents, 0x01c0,
			         0, 5,	/* M */
			         24, 2,	/* mux */
				 BIT(31),	/* gate */
				 CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(r_irrx_gating_clk, "r-irrx-gating",
                      "dcxo24M",
                      0x01cc, BIT(0), 0);

static SUNXI_CCU_GATE(dma_clken_sw_clk, "dma-clken-sw",
                      "dcxo24M",
                      0x01dc, BIT(0), 0);

static SUNXI_CCU_GATE(r_rtc_gating_clk, "r-rtc-gating",
                      "dcxo24M",
                      0x020c, BIT(0), 0);

static SUNXI_CCU_GATE(r_cpucfg_gating_clk, "r-cpucfg-gating",
                      "dcxo24M",
                      0x022c, BIT(0), 0);

static struct ccu_common *sun55iw3_r_ccu_clks[] = {
	&r_ahb_clk.common,
	&r_apbs0_clk.common,
	&r_apbs1_clk.common,
	&r_timer0_clk.common,
	&r_timer1_clk.common,
	&r_timer2_clk.common,
};

static struct clk_hw_onecell_data sun55iw3_r_hw_clks = {
	.hws	= {
		[CLK_R_TIMER0]	= &r_timer0_clk.common.hw,
		[CLK_R_TIMER1]	= &r_timer1_clk.common.hw,
		[CLK_R_TIMER2]	= &r_timer2_clk.common.hw,
	},
	.num	= CLK_NUMBER,
};

static struct ccu_reset_map sun55iw3_r_ccu_resets[] = {
	[RST_R_TIMER]		=  { 0x11b, BIT(16) },
	[RST_R_PWM]		=  { 0x13c, BIT(16) },
	[RST_R_CAN]		=  { 0x14c, BIT(16) },
	[RST_R_SPI]		=  { 0x15c, BIT(16) },
	[RST_R_SPLOCK]		=  { 0x16c, BIT(16) },
	[RST_R_MBOX]		=  { 0x17c, BIT(16) },
	[RST_R_UART1]		=  { 0x18c, BIT(17) },
	[RST_R_UART0]		=  { 0x18c, BIT(16) },
	[RST_R_TWI1]		=  { 0x19c, BIT(17) },
	[RST_R_TWI0]		=  { 0x19c, BIT(16) },
	[RST_R_PPU1]		=  { 0x1ac, BIT(17) },
	[RST_R_PPU]		=  { 0x1ac, BIT(16) },
	[RST_R_IRRX]		=  { 0x1cc, BIT(16) },
	[RST_R_RTC]		=  { 0x20c, BIT(16) },
	[RST_R_CPUCFG]		=  { 0x22c, BIT(16) },
};

static const struct sunxi_ccu_desc sun55iw3_r_ccu_desc = {
	.ccu_clks	= sun55iw3_r_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun55iw3_r_ccu_clks),

	.hw_clks	= &sun55iw3_r_hw_clks,

	.resets		= sun55iw3_r_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun55iw3_r_ccu_resets),
};

static int sun55iw3_r_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun55iw3_r_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun55iw3_r_ccu_clks,
			     ARRAY_SIZE(sun55iw3_r_ccu_clks),
			     NULL, 0);

	return 0;
}

static const struct of_device_id sun55iw3_r_ccu_ids[] = {
	{ .compatible = "allwinner,sun55iw3-r-ccu" },
	{ .compatible = "allwinner,sun20iw1-r-ccu" },
	{ }
};

static struct platform_driver sun55iw3_r_ccu_driver = {
	.probe	= sun55iw3_r_ccu_probe,
	.driver	= {
		.name	= "sun55iw3-r-ccu",
		.of_match_table	= sun55iw3_r_ccu_ids,
	},
};

static int __init sunxi_r_ccu_sun55iw3_init(void)
{
	int ret;

	ret = platform_driver_register(&sun55iw3_r_ccu_driver);
	if (ret)
		pr_err("register ccu sun55iw3 failed\n");

	return ret;
}
core_initcall(sunxi_r_ccu_sun55iw3_init);

static void __exit sunxi_r_ccu_sun55iw3_exit(void)
{
	return platform_driver_unregister(&sun55iw3_r_ccu_driver);
}
module_exit(sunxi_r_ccu_sun55iw3_exit);

MODULE_VERSION("1.0.0");
