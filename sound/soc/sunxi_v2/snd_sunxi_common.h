/* sound\soc\sunxi\snd_sunxi_common.h
 * (C) Copyright 2021-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SND_SUNXI_COMMON_H
#define __SND_SUNXI_COMMON_H

#include "snd_sunxi_adapter.h"

/* for REG LABEL */
#define REG_LABEL(constant)	{#constant, constant, 0}
#define REG_LABEL_END		{NULL, 0, 0}

struct reg_label {
	const char *name;
	const unsigned int address;
	unsigned int value;
};

int save_audio_reg(struct reg_cntlr *reg, REG_HANDLE regmap,
		   struct reg_label *reg_labels);
int echo_audio_reg(struct reg_cntlr *reg, REG_HANDLE regmap,
		   struct reg_label *reg_labels);

/* for rate conversion */
#define RATE_CONV_END		{0, 0}

struct sample_rate {
	unsigned int samplerate;
	unsigned int rate_bit;
};

/* for pa config */
struct pa_config {
	u32 pin;
	u32 msleep;
	bool used;
	bool level;
};

struct pa_config* pa_pin_init(struct adapter_cntlr *cntlr, u32 *pa_pin_max);
void pa_pin_exit(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg, u32 pa_pin_max);
int pa_pin_enable(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg, u32 pa_pin_max);
int pa_pin_disable(struct adapter_cntlr *cntlr, struct pa_config *pa_cfg, u32 pa_pin_max);

#endif /* __SND_SUNXI_COMMON_H */
