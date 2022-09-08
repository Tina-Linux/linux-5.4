/*
 * sound\soc\sunxi\snd_sunxi_common.c
 * (C) Copyright 2021-2025
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <stddef.h>
#include <linux/slab.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_common.h"
#include "snd_sunxi_adapter.h"

#define HLOG		"COMMON"

/* for REG LABEL */
int save_audio_reg(struct reg_cntlr *reg, REG_HANDLE regmap,
		   struct reg_label *reg_labels)
{
	int i = 0;

	while (reg_labels[i].name != NULL) {
		reg->ops->read(reg, regmap,
			       reg_labels[i].address, &(reg_labels[i].value));
		i++;
	}

	return i;
}

int echo_audio_reg(struct reg_cntlr *reg, REG_HANDLE regmap,
		   struct reg_label *reg_labels)
{
	int i = 0;

	while (reg_labels[i].name != NULL) {
		reg->ops->write(reg, regmap,
			        reg_labels[i].address, reg_labels[i].value);
		i++;
	}

	return i;
}

/* for pa config */
struct pa_config* pa_pin_init(struct adapter_cntlr *cntlr, u32 *pa_pin_max)
{
	int ret, i;
	u32 pin_max;
	u32 temp_val;
	char str[20] = {0};
	struct pa_config *pa_cfg = NULL;
	struct gpio_cntlr *gpio = NULL;
	struct parse_cntlr *parse = NULL;

	if (!cntlr) {
		SND_LOG_ERR(HLOG, "handles or pa_cfg is NULL\n");
		return NULL;
	}
	if (!cntlr->gpio || !cntlr->parse) {
		SND_LOG_ERR(HLOG, "handle NULL\n");
		return NULL;
	}
	gpio = cntlr->gpio;
	parse = cntlr->parse;

	ret = parse->ops->read_u32(parse, "pa_pin_max", &temp_val);
	if (ret < 0) {
		SND_LOG_WARN(HLOG, "pa_pin_max get failed, default 0\n");
		*pa_pin_max = 0;
		return NULL;
	} else {
		pin_max = temp_val;
	}
	*pa_pin_max = pin_max;

	pa_cfg = kzalloc(sizeof(struct pa_config) * pin_max, GFP_KERNEL);
	if (!pa_cfg) {
		SND_LOG_ERR(HLOG, "can't pa_config memory\n");
		return NULL;
	}

	for (i = 0; i < pin_max; i++) {
		sprintf(str, "pa_pin_%d", i);
		ret = gpio->ops->request(gpio, str, &temp_val);
		if (ret < 0) {
			SND_LOG_WARN(HLOG, "pa_pin_%u request failed\n", i);
			pa_cfg[i].pin = 0;
			pa_cfg[i].used = 0;
			continue;
		} else {
			pa_cfg[i].pin = temp_val;
			pa_cfg[i].used = 1;
		}
		sprintf(str, "pa_pin_level_%d", i);
		ret = parse->ops->read_u32(parse, str, &temp_val);
		if (ret < 0) {
			SND_LOG_WARN(HLOG, "%s get failed, default low\n", str);
			pa_cfg[i].level = 0;
		} else {
			if (temp_val > 0)
				pa_cfg[i].level = 1;
		}
		sprintf(str, "pa_pin_msleep_%d", i);
		ret = parse->ops->read_u32(parse, str, &temp_val);
		if (ret < 0) {
			SND_LOG_WARN(HLOG, "%s get failed, default 0\n", str);
			pa_cfg[i].msleep = 0;
		} else {
			pa_cfg[i].msleep = temp_val;
		}
	}

	pa_pin_disable(cntlr, pa_cfg, pin_max);

	return pa_cfg;
}

void pa_pin_exit(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg,
		 u32 pa_pin_max)
{
	int i;
	struct gpio_cntlr *gpio = NULL;

	if (pa_pin_max < 1) {
		SND_LOG_DEBUG(HLOG, "no pa pin config\n");
		return;
	}
	if (!cntlr) {
		SND_LOG_ERR(HLOG, "handles or pa_cfg is NULL\n");
		return;
	}
	if (!cntlr->gpio) {
		SND_LOG_ERR(HLOG, "handle NULL\n");
		return;
	}
	gpio = cntlr->gpio;

	pa_pin_disable(cntlr, pa_cfg, pa_pin_max);

	for (i = 0; i < pa_pin_max; i++)
		gpio->ops->release(gpio, pa_cfg[i].pin);

	if (pa_cfg)
		kfree(pa_cfg);
}

int pa_pin_enable(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg,
		  u32 pa_pin_max)
{
	int i;
	struct gpio_cntlr *gpio = NULL;

	if (pa_pin_max < 1) {
		SND_LOG_DEBUG(HLOG, "no pa pin config\n");
		return 0;
	}
	if (!cntlr) {
		SND_LOG_ERR(HLOG, "handles or pa_cfg is NULL\n");
		return -1;
	}
	if (!cntlr->gpio) {
		SND_LOG_ERR(HLOG, "handle NULL\n");
		return -1;
	}
	gpio = cntlr->gpio;

	for (i = 0; i < pa_pin_max; i++) {
		if (!pa_cfg[i].used)
			continue;

		gpio->ops->setdir(gpio, pa_cfg[i].pin, 1);
		gpio->ops->write(gpio, pa_cfg[i].pin, pa_cfg[i].level);
	}

	return 0;
}

int pa_pin_disable(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg,
		   u32 pa_pin_max)
{
	int i;
	struct gpio_cntlr *gpio = NULL;

	if (pa_pin_max < 1) {
		SND_LOG_DEBUG(HLOG, "no pa pin config\n");
		return 0;
	}
	if (!cntlr) {
		SND_LOG_ERR(HLOG, "handles or pa_cfg is NULL\n");
		return -1;
	}
	if (!cntlr->gpio) {
		SND_LOG_ERR(HLOG, "handle NULL\n");
		return -1;
	}
	gpio = cntlr->gpio;

	for (i = 0; i < pa_pin_max; i++) {
		if (!pa_cfg[i].used)
			continue;

		gpio->ops->setdir(gpio, pa_cfg[i].pin, 1);
		gpio->ops->write(gpio, pa_cfg[i].pin, !pa_cfg[i].level);
	}

	return 0;
}