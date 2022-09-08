/*
 * sound\soc\sunxi\snd_sunxi_adapter.c
 * (C) Copyright 2021-2025
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/regmap.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_adapter.h"

#define HLOG		"ADAPTER"

/* regulator adapter */
static RGLT_HANDLE ada_regulator_request(struct regulator_cntlr *cntlr,
					 const char *id)
{
	RGLT_HANDLE handle = NULL;

	handle = regulator_get(cntlr->dev, id);
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regulator get failed\n");
		return NULL;
	}

	return handle;
}

static void ada_regulator_release(struct regulator_cntlr *cntlr,
				  RGLT_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regulator handle is err\n");
		return;
	}
	regulator_put((struct regulator *)handle);
}

static int ada_regulator_set_voltage(struct regulator_cntlr *cntlr,
				     RGLT_HANDLE handle,
				     int min_uv, int max_uv)
{
	int ret;

	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regulator handle is err\n");
		return ADAPT_FAILURE;
	}

	ret = regulator_set_voltage((struct regulator *)handle, min_uv, max_uv);
	if (ret) {
		SND_LOG_ERR(HLOG, "regulator set voltage failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static int ada_regulator_enable(struct regulator_cntlr *cntlr,
				RGLT_HANDLE handle)
{
	int ret;

	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regulator handle is err\n");
		return ADAPT_FAILURE;
	}

	ret = regulator_enable((struct regulator *)handle);
	if (ret) {
		SND_LOG_ERR(HLOG, "regulator enable failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static void ada_regulator_disable(struct regulator_cntlr *cntlr,
				  RGLT_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regulator handle is err\n");
		return;
	}

	regulator_disable((struct regulator *)handle);
}

static struct regulator_method g_rglt_method = {
	.request	= ada_regulator_request,
	.release	= ada_regulator_release,
	.set_voltage	= ada_regulator_set_voltage,
	.enable		= ada_regulator_enable,
	.disable	= ada_regulator_disable,
};

int regulator_adapter_probe(struct regulator_cntlr **cntlr, struct device *dev)
{
	struct regulator_cntlr *rglt = NULL;

	rglt = kzalloc(sizeof(struct regulator_cntlr), GFP_KERNEL);
	if (!rglt) {
		SND_LOG_ERR(HLOG, "regulator kmalloc faild\n");
		goto err;
	}

	rglt->dev = dev;
	rglt->ops = &g_rglt_method;
	*cntlr = rglt;

	return ADAPT_SUCCESS;
err:
	if (rglt)
		kfree(rglt);

	return ADAPT_FAILURE;
}

void regulator_adapter_remove(struct regulator_cntlr **cntlr)
{
	if (*cntlr) {
		(*cntlr)->ops = NULL;
		kfree(*cntlr);
	}
}

/* clk adapter */
static CLK_HANDLE ada_clk_get_rst(struct clk_cntlr *cntlr, const char *name)
{
	CLK_HANDLE handle = NULL;

	handle = of_reset_control_get(cntlr->np, name);
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk rst get failed\n");
		return NULL;
	}

	return handle;
}

static void ada_clk_put_rst(struct clk_cntlr *cntlr, CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk rst handle is err\n");
		return;
	}
	reset_control_put((struct reset_control *)handle);
}

static int ada_clk_deassert_rst(struct clk_cntlr *cntlr, CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return ADAPT_FAILURE;
	}

	if (reset_control_deassert((struct reset_control *)handle)) {
		SND_LOG_ERR(HLOG, "deassert the reset clk failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static int ada_clk_assert_rst(struct clk_cntlr *cntlr, CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return ADAPT_FAILURE;
	}

	if (reset_control_assert((struct reset_control *)handle)) {
		SND_LOG_ERR(HLOG, "assert the reset clk failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static CLK_HANDLE ada_clk_request(struct clk_cntlr *cntlr, const char *name)
{
	CLK_HANDLE handle = NULL;

	handle = of_clk_get_by_name(cntlr->np, name);
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk get failed\n");
		return NULL;
	}

	return handle;
}

static void ada_clk_release(struct clk_cntlr *cntlr, CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return;
	}
	clk_put((struct clk *)handle);
}

static int ada_clk_set_parent(struct clk_cntlr *cntlr,
			      CLK_HANDLE handle1, CLK_HANDLE handle2)
{
	if (IS_ERR_OR_NULL(handle1) || IS_ERR_OR_NULL(handle2)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return ADAPT_FAILURE;
	}

	if (clk_set_parent((struct clk *)handle1, (struct clk *)handle2)) {
		SND_LOG_ERR(HLOG, "clk set parent failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static int ada_clk_set_rate(struct clk_cntlr *cntlr, CLK_HANDLE handle,
			    unsigned long rate)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return ADAPT_FAILURE;
	}

	if (clk_set_rate((struct clk *)handle, rate)) {
		SND_LOG_ERR(HLOG, "clk set rate failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static int ada_clk_enable_prepare(struct clk_cntlr *cntlr, CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return ADAPT_FAILURE;
	}

	if (clk_prepare_enable((struct clk *)handle)) {
		SND_LOG_ERR(HLOG, "clk prepare enable failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static void ada_clk_disable_unprepare(struct clk_cntlr *cntlr,
				      CLK_HANDLE handle)
{
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "clk handle is err\n");
		return;
	}

	clk_disable_unprepare((struct clk *)handle);
}

static struct clk_method g_clk_method = {
	.get_rst		= ada_clk_get_rst,
	.put_rst		= ada_clk_put_rst,
	.deassert_rst		= ada_clk_deassert_rst,
	.assert_rst		= ada_clk_assert_rst,

	.request		= ada_clk_request,
	.release		= ada_clk_release,
	.set_parent		= ada_clk_set_parent,
	.set_rate		= ada_clk_set_rate,
	.enable_prepare		= ada_clk_enable_prepare,
	.disable_unprepare	= ada_clk_disable_unprepare,
};

int clk_adapter_probe(struct clk_cntlr **cntlr, struct device_node *np)
{
	struct clk_cntlr *clk = NULL;

	clk = kzalloc(sizeof(struct clk_cntlr), GFP_KERNEL);
	if (!clk) {
		SND_LOG_ERR(HLOG, "clk kmalloc faild\n");
		goto err;
	}

	clk->np = np;
	clk->ops = &g_clk_method;
	*cntlr = clk;

	return ADAPT_SUCCESS;

err:
	if (clk)
		kfree(clk);

	return ADAPT_FAILURE;
}

void clk_adapter_remove(struct clk_cntlr **cntlr)
{
	if (*cntlr) {
		(*cntlr)->ops = NULL;
		kfree(*cntlr);
	}

	return;
}

/* regmap adapter */
static REG_HANDLE ada_regmap_request(struct reg_cntlr *cntlr,
				     unsigned int max_reg)
{
	int ret;
	REG_HANDLE handle = NULL;

	struct resource res;
	struct resource *memregion = NULL;
	void __iomem *digital_base = NULL;
	struct regmap_config sunxi_regmap_config = {
		.reg_bits = 32,
		.reg_stride = 4,
		.val_bits = 32,
		.max_register = max_reg,
		.cache_type = REGCACHE_NONE,
	};

	ret = of_address_to_resource(cntlr->dev->of_node, 0, &res);
	if (ret) {
		SND_LOG_ERR(HLOG, "get resource failed\n");
		return NULL;
	}

	memregion = devm_request_mem_region(cntlr->dev, res.start,
					    resource_size(&res),
					    cntlr->name);
	if (!memregion) {
		SND_LOG_ERR(HLOG, "memory region already claimed\n");
		return NULL;
	}

	digital_base = devm_ioremap(cntlr->dev, res.start, resource_size(&res));
	if (!digital_base) {
		SND_LOG_ERR(HLOG, "digital_base ioremap failed\n");
		return NULL;
	}

	handle = devm_regmap_init_mmio(cntlr->dev, digital_base,
				       &sunxi_regmap_config);
	if (IS_ERR_OR_NULL(handle)) {
		SND_LOG_ERR(HLOG, "regmap init failed\n");
		return NULL;
	}

	return handle;
}

static void ada_regmap_release(struct reg_cntlr *cntlr, REG_HANDLE handle)
{
	return;
}

static int ada_regmap_write(struct reg_cntlr *cntlr, REG_HANDLE handle,
			    unsigned int reg, unsigned int val)
{
	regmap_write((struct regmap *)handle, reg, val);

	return ADAPT_SUCCESS;
}

static int ada_regmap_read(struct reg_cntlr *cntlr, REG_HANDLE handle,
			   unsigned int reg, unsigned int *val)
{
	regmap_read((struct regmap *)handle, reg, val);

	return ADAPT_SUCCESS;
}

static int ada_regmap_update_bits(struct reg_cntlr *cntlr, REG_HANDLE handle,
				  unsigned int reg, unsigned int mask,
				  unsigned int val)
{
	regmap_update_bits((struct regmap *)handle, reg, mask, val);

	return ADAPT_SUCCESS;
}

static struct reg_method g_regmap_method = {
	.request		= ada_regmap_request,
	.release		= ada_regmap_release,
	.write			= ada_regmap_write,
	.read			= ada_regmap_read,
	.update_bits		= ada_regmap_update_bits,
};

int reg_adapter_probe(struct reg_cntlr **cntlr, struct device *dev)
{
	struct reg_cntlr *regmap = NULL;

	regmap = kzalloc(sizeof(struct reg_cntlr), GFP_KERNEL);
	if (!regmap) {
		SND_LOG_ERR(HLOG, "regmap kmalloc faild\n");
		goto err;
	}

	regmap->dev = dev;
	regmap->ops = &g_regmap_method;
	*cntlr = regmap;

	return ADAPT_SUCCESS;

err:
	if (regmap)
		kfree(regmap);

	return ADAPT_FAILURE;
}

void reg_adapter_remove(struct reg_cntlr **cntlr)
{
	if (*cntlr) {
		(*cntlr)->ops = NULL;
		kfree(*cntlr);
	}

	return;
}

/* parse params adapter */
static int ada_parse_read_u32(struct parse_cntlr *cntlr, char *name, u32 *value)
{
	int ret;

	ret = of_property_read_u32(cntlr->np, name, value);
	if (ret < 0) {
		SND_LOG_ERR(HLOG, "params read faild\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

static struct parse_method g_parse_method = {
	.read_u32		= ada_parse_read_u32,
};

int parse_adapter_probe(struct parse_cntlr **cntlr, struct device_node *np)
{
	struct parse_cntlr *parse = NULL;

	parse = kzalloc(sizeof(struct parse_cntlr), GFP_KERNEL);
	if (!parse) {
		SND_LOG_ERR(HLOG, "parse kmalloc faild\n");
		goto err;
	}

	parse->np = np;
	parse->ops = &g_parse_method;
	*cntlr = parse;

	return ADAPT_SUCCESS;

err:
	if (parse)
		kfree(parse);

	return ADAPT_FAILURE;
}

void parse_adapter_remove(struct parse_cntlr **cntlr)
{
	if (*cntlr) {
		(*cntlr)->ops = NULL;
		kfree(*cntlr);
	}

	return;
}

/* gpio adapter */
static int ada_gpio_request(struct gpio_cntlr *cntlr, char *name, u32 *gpio)
{
	int ret;
	u32 gpio_tmp;

	if (!name) {
		SND_LOG_ERR(HLOG, "gpio name is NULL\n");
		return ADAPT_FAILURE;
	}

	ret = of_get_named_gpio(cntlr->dev->of_node, name, 0);
	if (ret < 0) {
		SND_LOG_ERR(HLOG, "gpio get failed\n");
		return ADAPT_FAILURE;
	}

	gpio_tmp = ret;
	if (!gpio_is_valid(gpio_tmp)) {
		SND_LOG_ERR(HLOG, "gpio %u is invalid\n", gpio_tmp);
		return ADAPT_FAILURE;
	}

	ret = devm_gpio_request(cntlr->dev, gpio_tmp, name);
	if (ret) {
		SND_LOG_ERR(HLOG, "gpio %u request failed\n", gpio_tmp);
		return ADAPT_FAILURE;
	}

	*gpio = gpio_tmp;

	return ADAPT_SUCCESS;
}

static void ada_gpio_release(struct gpio_cntlr *cntlr, u32 gpio)
{
	return;
}

static int ada_gpio_setdir(struct gpio_cntlr *cntlr, u32 gpio, int value)
{
	if (value > 0)
		gpio_direction_output(gpio, 1);
	else
		gpio_direction_input(gpio);

	return ADAPT_SUCCESS;
}

static int ada_gpio_write(struct gpio_cntlr *cntlr, u32 gpio, int value)
{
	gpio_set_value(gpio, value);

	return ADAPT_SUCCESS;
}

static int ada_gpio_read(struct gpio_cntlr *cntlr, u32 gpio, int *value)
{
	*value = gpio_get_value(gpio);

	return ADAPT_SUCCESS;
}

static struct gpio_method g_gpio_method = {
	.request		= ada_gpio_request,
	.release		= ada_gpio_release,
	.setdir			= ada_gpio_setdir,
	.write			= ada_gpio_write,
	.read			= ada_gpio_read,
};

int gpio_adapter_probe(struct gpio_cntlr **cntlr, struct device *dev)
{
	struct gpio_cntlr *gpio = NULL;

	gpio = kzalloc(sizeof(struct gpio_cntlr), GFP_KERNEL);
	if (!gpio) {
		SND_LOG_ERR(HLOG, "gpio kmalloc faild\n");
		goto err;
	}

	gpio->dev = dev;
	gpio->ops = &g_gpio_method;
	*cntlr = gpio;

	return ADAPT_SUCCESS;

err:
	if (gpio)
		kfree(gpio);

	return ADAPT_FAILURE;
}

void gpio_adapter_remove(struct gpio_cntlr **cntlr)
{
	if (*cntlr) {
		(*cntlr)->ops = NULL;
		kfree(*cntlr);
	}

	return;
}

/* ALSA api adapter */

/* others adapter */

MODULE_AUTHOR("Dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sunxi soundcard platform of ahub");
