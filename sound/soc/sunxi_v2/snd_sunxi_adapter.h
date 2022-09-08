/* sound\soc\sunxi\snd_sunxi_adapter.h
 * (C) Copyright 2021-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SND_SUNXI_ADAPTER_H
#define __SND_SUNXI_ADAPTER_H

typedef unsigned int	u32;

typedef enum {
	ADAPT_SUCCESS	= 0,
	ADAPT_FAILURE	= -1,
} ADAPT_STA;

/* linux platform api adapter */
struct regulator_method;
struct regulator_cntlr;

struct clk_method;
struct clk_cntlr;

struct parse_method;
struct parse_cntlr;

struct reg_method;
struct reg_cntlr;

struct gpio_method;
struct gpio_cntlr;

struct adapter_cntlr {
	struct regulator_cntlr *regulator;
	struct clk_cntlr *clk;
	struct parse_cntlr *parse;
	struct reg_cntlr *reg;
	struct gpio_cntlr *gpio;
};

/* regulator adapter */
typedef void* RGLT_HANDLE;

struct regulator_cntlr {
	struct device *dev;
	struct regulator_method *ops;
};

struct regulator_method {
	RGLT_HANDLE (*request)(struct regulator_cntlr*, const char*);
	void (*release)(struct regulator_cntlr*, RGLT_HANDLE);
	int (*set_voltage)(struct regulator_cntlr*, RGLT_HANDLE, int, int);
	int (*enable)(struct regulator_cntlr*, RGLT_HANDLE);
	void (*disable)(struct regulator_cntlr*, RGLT_HANDLE);
};

int regulator_adapter_probe(struct regulator_cntlr **cntlr, struct device *dev);
void regulator_adapter_remove(struct regulator_cntlr **cntlr);

/* clk adapter */
typedef void* CLK_HANDLE;

struct clk_cntlr {
	struct device_node *np;
	struct clk_method *ops;
};

struct clk_method {
	CLK_HANDLE (*get_rst)(struct clk_cntlr*, const char*);
	void (*put_rst)(struct clk_cntlr*, CLK_HANDLE);
	int (*deassert_rst)(struct clk_cntlr*, CLK_HANDLE);
	int (*assert_rst)(struct clk_cntlr*, CLK_HANDLE);

	CLK_HANDLE (*request)(struct clk_cntlr*, const char*);
	void (*release)(struct clk_cntlr*, CLK_HANDLE);
	int (*set_parent)(struct clk_cntlr*, CLK_HANDLE, CLK_HANDLE);
	int (*set_rate)(struct clk_cntlr*, CLK_HANDLE, unsigned long);
	int (*enable_prepare)(struct clk_cntlr*, CLK_HANDLE);
	void (*disable_unprepare)(struct clk_cntlr*, CLK_HANDLE);
};

int clk_adapter_probe(struct clk_cntlr **cntlr, struct device_node *np);
void clk_adapter_remove(struct clk_cntlr **cntlr);

/* regmap adapter */
typedef void* REG_HANDLE;

struct reg_cntlr {
	struct device *dev;
	const char *name;
	struct reg_method *ops;
};

struct reg_method {
	REG_HANDLE (*request)(struct reg_cntlr*, unsigned int);
	void (*release)(struct reg_cntlr*, REG_HANDLE);
	int (*write)(struct reg_cntlr*, REG_HANDLE, unsigned int, unsigned int);
	int (*read)(struct reg_cntlr*, REG_HANDLE, unsigned int, unsigned int*);
	int (*update_bits)(struct reg_cntlr*, REG_HANDLE, unsigned int,
			   unsigned int, unsigned int);
};

int reg_adapter_probe(struct reg_cntlr **cntlr, struct device *dev);
void reg_adapter_remove(struct reg_cntlr **cntlr);

/* parse params adapter */
struct parse_cntlr {
	struct device_node *np;
	struct parse_method *ops;
};

struct parse_method {
	int (*read_u32)(struct parse_cntlr*, char*, u32*);
};

int parse_adapter_probe(struct parse_cntlr **cntlr, struct device_node *np);
void parse_adapter_remove(struct parse_cntlr **cntlr);

/* gpio adapter */
struct gpio_cntlr {
	struct device *dev;
	struct gpio_method *ops;
};

struct gpio_method {
	int (*request)(struct gpio_cntlr*, char*, u32*);
	void (*release)(struct gpio_cntlr*, u32);
	int (*setdir)(struct gpio_cntlr*, u32, int);
	int (*write)(struct gpio_cntlr*, u32, int);
	int (*read)(struct gpio_cntlr*, u32, int*);
};

int gpio_adapter_probe(struct gpio_cntlr **cntlr, struct device *dev);
void gpio_adapter_remove(struct gpio_cntlr **cntlr);

/* ALSA api adapter */
enum {
	PCM_STREAM_PLAYBACK = 0,
	PCM_STREAM_CAPTURE,
	PCM_STREAM_LAST = PCM_STREAM_CAPTURE,
};

#define	PCM_FORMAT_S8		0
#define	PCM_FORMAT_U8		1
#define	PCM_FORMAT_S16_LE	2
#define	PCM_FORMAT_S16_BE	3
#define	PCM_FORMAT_U16_LE	4
#define	PCM_FORMAT_U16_BE	5
#define	PCM_FORMAT_S24_LE	6
#define	PCM_FORMAT_S24_BE	7
#define	PCM_FORMAT_U24_LE	8
#define	PCM_FORMAT_U24_BE	9
#define	PCM_FORMAT_S32_LE	10
#define	PCM_FORMAT_S32_BE	11
#define	PCM_FORMAT_U32_LE	12
#define	PCM_FORMAT_U32_BE	13

#endif /* __SND_SUNXI_ADAPTER_H */
