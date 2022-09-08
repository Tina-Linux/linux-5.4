/*
 * sound\soc\sunxi\snd_sun50iw9_codec.c
 * (C) Copyright 2021-2025
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_common.h"
#include "snd_sunxi_adapter.h"
#include "snd_sun50iw9_codec.h"

#define HLOG		"CODEC"

/* for sunxi_internal_codec_dev_probe */
static int sunxi_codec_regulator_init(struct regulator_cntlr *rglt,
				      struct sunxi_codec_info *codec_info)
{
	int ret;

	SND_LOG_DEBUG(HLOG, "\n");

	if (!rglt) {
		SND_LOG_ERR(HLOG, "regulator cntlr is NULL\n");
		return ADAPT_FAILURE;
        }

	/* regulator about */
	codec_info->avcc = rglt->ops->request(rglt, "avcc");
	if (!codec_info->avcc) {
		SND_LOG_ERR(HLOG, "request avcc failed\n");
		goto err;
        }
	codec_info->dvcc = rglt->ops->request(rglt, "dvcc");
	if (!codec_info->dvcc) {
		SND_LOG_ERR(HLOG, "request dvcc failed\n");
		goto err;
        }

	ret = rglt->ops->enable(rglt, codec_info->avcc);
	if (ret) {
		SND_LOG_ERR(HLOG, "avcc enable failed\n");
		goto err;
	}
	ret = rglt->ops->enable(rglt, codec_info->dvcc);
	if (ret) {
		SND_LOG_ERR(HLOG, "dvcc enable failed\n");
		goto err;
	}

	return ADAPT_SUCCESS;
err:
	rglt->ops->disable(rglt, codec_info->avcc);
	rglt->ops->release(rglt, codec_info->avcc);
	rglt->ops->disable(rglt, codec_info->dvcc);
	rglt->ops->release(rglt, codec_info->dvcc);

	return ADAPT_FAILURE;
}

static int sunxi_codec_clk_init(struct clk_cntlr *clk,
				struct sunxi_codec_info *codec_info)
{
	int ret;

	SND_LOG_DEBUG(HLOG, "\n");

	if (!clk) {
		SND_LOG_ERR(HLOG, "clk cntlr is NULL\n");
		return ADAPT_FAILURE;
	}

	/* deassert ret clk */
	codec_info->clk_rst = clk->ops->get_rst(clk, NULL);
	if (!codec_info->clk_rst) {
		SND_LOG_ERR(HLOG, "request clk rst failed\n");
		goto err;
	}
	ret = clk->ops->deassert_rst(clk, codec_info->clk_rst);
	if (ret) {
		SND_LOG_ERR(HLOG, "deassert clk rst failed\n");
		goto err;
	}

	/* clk about */
	codec_info->clk_bus_audio = clk->ops->request(clk, "clk_bus_audio");
	if (!codec_info->clk_bus_audio) {
		SND_LOG_ERR(HLOG, "request clk bus audio failed\n");
		goto err;
	}
	codec_info->clk_pll_audio = clk->ops->request(clk, "clk_pll_audio");
	if (!codec_info->clk_pll_audio) {
		SND_LOG_ERR(HLOG, "request clk pll audio failed\n");
		goto err;
	}
	codec_info->clk_audio = clk->ops->request(clk, "clk_audio");
	if (!codec_info->clk_audio) {
		SND_LOG_ERR(HLOG, "request clk audio failed\n");
		goto err;
	}

	ret = clk->ops->set_parent(clk,
				   codec_info->clk_audio,
				   codec_info->clk_pll_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "set clk audio parent failed\n");
		goto err;
	}

	ret = clk->ops->set_rate(clk, codec_info->clk_pll_audio, 24576000);
	if (ret) {
		SND_LOG_ERR(HLOG, "set rate clk pll audio failed\n");
		goto err;
	}

	ret = clk->ops->enable_prepare(clk, codec_info->clk_bus_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk bus audio failed\n");
		goto err;
	}

	ret = clk->ops->enable_prepare(clk, codec_info->clk_pll_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk pll audio failed\n");
		goto err;
	}
	ret = clk->ops->enable_prepare(clk, codec_info->clk_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk audio failed\n");
		goto err;
	}

	return ADAPT_SUCCESS;
err:
	clk->ops->disable_unprepare(clk, codec_info->clk_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_pll_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_bus_audio);
	clk->ops->release(clk, codec_info->clk_audio);
	clk->ops->release(clk, codec_info->clk_pll_audio);
	clk->ops->release(clk, codec_info->clk_bus_audio);
	clk->ops->assert_rst(clk, codec_info->clk_rst);
	clk->ops->put_rst(clk, codec_info->clk_rst);

	return ADAPT_FAILURE;
}

static void sunxi_codec_parse_params(struct parse_cntlr *parse,
				     struct sunxi_codec_info *codec_info)
{
	int ret;
	u32 temp_val;

	SND_LOG_DEBUG(HLOG, "\n");

	codec_info->digital_vol = 0;
	codec_info->lineout_vol = 0;

	if (!parse) {
		SND_LOG_ERR(HLOG, "parse cntlr is NULL\n");
		return;
	}

	ret = parse->ops->read_u32(parse, "digital_vol", &temp_val);
	if (ret < 0) {
		SND_LOG_ERR(HLOG, "digital volume get failed, use default\n");
	} else {
		codec_info->digital_vol = temp_val;
	}

	ret = parse->ops->read_u32(parse, "lineout_vol", &temp_val);
	if (ret < 0) {
		SND_LOG_ERR(HLOG, "lineout volume get failed, use default\n");
	} else {
		codec_info->lineout_vol = temp_val;
	}

	SND_LOG_INFO(HLOG, "digital volume: %u\n", codec_info->digital_vol);
	SND_LOG_INFO(HLOG, "lineout volume: %u\n", codec_info->lineout_vol);
}

int internal_codec_probe(struct adapter_cntlr *cntlr,
			 struct sunxi_codec_info *codec_info)
{
	int ret;
	struct reg_cntlr *reg = NULL;

	SND_LOG_DEBUG(HLOG, "\n");

	if (!cntlr) {
		SND_LOG_ERR(HLOG, "adapter handle is NULL\n");
		return ADAPT_FAILURE;

	}
	reg = cntlr->reg;

	/* regulator init */
	ret = sunxi_codec_regulator_init(cntlr->regulator, codec_info);
	if (ret)
		return ADAPT_FAILURE;

	/* clk init */
	ret = sunxi_codec_clk_init(cntlr->clk, codec_info);
	if (ret)
		return ADAPT_FAILURE;

	/* regmap request */
	codec_info->regmap = reg->ops->request(reg, SUNXI_AUDIO_MAX_REG);
	if (!codec_info->regmap) {
		SND_LOG_ERR(HLOG, "regmap request failed\n");
		return ADAPT_FAILURE;
	}

	/* of params parse */
	sunxi_codec_parse_params(cntlr->parse, codec_info);

	/* gpio mute for peripheral circuit */
	codec_info->pa_cfg = pa_pin_init(cntlr, &codec_info->pa_pin_max);
	if (codec_info->pa_pin_max > 0) {
		SND_LOG_INFO(HLOG, "use pa pin, num is %u\n",
			     codec_info->pa_pin_max);
	}

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_dev_remove */
int internal_codec_remove(struct adapter_cntlr *cntlr,
			  struct sunxi_codec_info *codec_info)
{
	struct regulator_cntlr *rglt = NULL;
	struct clk_cntlr *clk = NULL;

	SND_LOG_DEBUG(HLOG, "\n");

	if (!cntlr) {
		SND_LOG_ERR(HLOG, "adapter handle is NULL\n");
		return ADAPT_FAILURE;

	}
	rglt = cntlr->regulator;
	clk = cntlr->clk;

	/* gpio mute for peripheral circuit */
	pa_pin_exit(cntlr, codec_info->pa_cfg, codec_info->pa_pin_max);

	/* clk about */
	clk->ops->disable_unprepare(clk, codec_info->clk_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_pll_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_bus_audio);
	clk->ops->release(clk, codec_info->clk_audio);
	clk->ops->release(clk, codec_info->clk_pll_audio);
	clk->ops->release(clk, codec_info->clk_bus_audio);
	clk->ops->assert_rst(clk, codec_info->clk_rst);
	clk->ops->put_rst(clk, codec_info->clk_rst);

	/* regulator about */
	rglt->ops->disable(rglt, codec_info->avcc);
	rglt->ops->release(rglt, codec_info->avcc);
	rglt->ops->disable(rglt, codec_info->dvcc);
	rglt->ops->release(rglt, codec_info->dvcc);

	/* others about */

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_probe() */
int internal_codec_init(struct adapter_cntlr *cntlr,
			struct sunxi_codec_info *codec_info)
{
	REG_HANDLE regmap = NULL;
	struct reg_cntlr *reg = NULL;

	SND_LOG_DEBUG(HLOG, "\n");

	if (!cntlr) {
		SND_LOG_ERR(HLOG, "adapter handle is NULL\n");
		return ADAPT_FAILURE;

	}
	reg = cntlr->reg;
	regmap = codec_info->regmap;

	/* Disable DRC function for playback */
	reg->ops->write(reg, regmap, SUNXI_DAC_DAP_CTL, 0);

	/* set digital vol */
	reg->ops->update_bits(reg, regmap, SUNXI_DAC_DPC,
			      0x3f << DVOL,
			      codec_info->digital_vol << DVOL);

	/* set lineout vol */
	reg->ops->update_bits(reg, regmap, AC_DAC_REG,
			      0x1f << LINEOUT_VOL,
			      codec_info->lineout_vol << LINEOUT_VOL);
	reg->ops->update_bits(reg, regmap, AC_DAC_REG,
			      0x1 << LMUTE, 0x1 << LMUTE);
	reg->ops->update_bits(reg, regmap, AC_DAC_REG,
			      0x1 << RMUTE, 0x1 << RMUTE);

	/* enable rampen to avoid pop */
	reg->ops->update_bits(reg, regmap, AC_RAMP_REG,
			      0x7 << RAMP_STEP, 0x1 << RAMP_STEP);
	reg->ops->update_bits(reg, regmap, AC_DAC_REG,
			      0x1 << RSWITCH, 0x0 << RSWITCH);
	reg->ops->update_bits(reg, regmap, AC_DAC_REG,
			      0x1 << RAMPEN, 0x1 << RAMPEN);

	return ADAPT_SUCCESS;
}

static int sunxi_codec_playback_event(struct snd_soc_dapm_widget *w,
				      struct snd_kcontrol *k, int event)
{
	REG_HANDLE regmap = NULL;
	struct reg_cntlr *reg = NULL;
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	regmap = codec_info->regmap;
	reg = codec_info->cntlr->reg;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_DPC,
				      0x1 << EN_DA, 0x1 << EN_DA);
		break;

	case SND_SOC_DAPM_POST_PMD:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_DPC,
				      0x1 << EN_DA, 0x0 << EN_DA);
		break;

	default:
		break;
	}
	return 0;
}

static int sunxi_lineout_event(struct snd_soc_dapm_widget *w,
			       struct snd_kcontrol *k, int event)
{
	REG_HANDLE regmap = NULL;
	struct reg_cntlr *reg = NULL;
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	regmap = codec_info->regmap;
	reg = codec_info->cntlr->reg;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		reg->ops->update_bits(reg, regmap, AC_DAC_REG,
				      (0x1 << LINEOUTL_EN) | (0x1 << LINEOUTR_EN),
				      (0x1 << LINEOUTL_EN) | (0x1 << LINEOUTR_EN));

		reg->ops->update_bits(reg, regmap, AC_RAMP_REG,
				      0x1 << RDEN, 0x1 << RDEN);

		pa_pin_enable(codec_info->cntlr, codec_info->pa_cfg, codec_info->pa_pin_max);

		break;

	case SND_SOC_DAPM_PRE_PMD:
		reg->ops->update_bits(reg, regmap, AC_RAMP_REG,
				      0x1 << RDEN, 0x0 << RDEN);

		reg->ops->update_bits(reg, regmap, AC_DAC_REG,
				      (0x1 << LINEOUTL_EN) | (0x1 << LINEOUTR_EN),
				      (0x0 << LINEOUTL_EN) | (0x0 << LINEOUTR_EN));

		pa_pin_disable(codec_info->cntlr, codec_info->pa_cfg, codec_info->pa_pin_max);

		break;

	default:
		break;
	}

	return 0;
}

static const DECLARE_TLV_DB_SCALE(digital_tlv, 0, -116, -7424);

static const unsigned int lineout_tlv[] = {
	TLV_DB_RANGE_HEAD(2),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 1),
	1, 31, TLV_DB_SCALE_ITEM(-4350, 150, 1),
};

const char * const left_lineoutl_text[] = {
	"LOMixer", "LROMixer",
};

static const struct soc_enum left_lineout_enum =
	SOC_ENUM_SINGLE(AC_DAC_REG, LINEOUTL_SEL,
			ARRAY_SIZE(left_lineoutl_text), left_lineoutl_text);
static const struct snd_kcontrol_new left_lineout_mux =
	SOC_DAPM_ENUM("Left LINEOUT Mux", left_lineout_enum);

const char * const right_lineoutr_text[] = {
	"ROMixer", "LROMixer",
};

static const struct soc_enum right_lineout_enum =
	SOC_ENUM_SINGLE(AC_DAC_REG, LINEOUTR_SEL,
			ARRAY_SIZE(right_lineoutr_text), right_lineoutr_text);

static const struct snd_kcontrol_new right_lineout_mux =
	SOC_DAPM_ENUM("Right LINEOUT Mux", right_lineout_enum);

struct snd_kcontrol_new sunxi_codec_controls[] = {
	SOC_SINGLE("hub mode switch", SUNXI_DAC_DPC, DAC_HUB_EN, 0x1, 0),
	SOC_SINGLE_TLV("digital volume", SUNXI_DAC_DPC, DVOL, 0x3F, 1, digital_tlv),
	SOC_SINGLE_TLV("lineout volume", AC_DAC_REG, LINEOUT_VOL, 0x1F, 0, lineout_tlv),
};

static const struct snd_kcontrol_new left_output_mixer[] = {
	SOC_DAPM_SINGLE("DACL Switch", AC_MIXER_REG, LMIX_LDAC, 1, 0),
	SOC_DAPM_SINGLE("DACR Switch", AC_MIXER_REG, LMIX_RDAC, 1, 0),
};

static const struct snd_kcontrol_new right_output_mixer[] = {
	SOC_DAPM_SINGLE("DACL Switch", AC_MIXER_REG, RMIX_LDAC, 1, 0),
	SOC_DAPM_SINGLE("DACR Switch", AC_MIXER_REG, RMIX_RDAC, 1, 0),
};

static const struct snd_soc_dapm_widget sunxi_codec_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN_E("DACL", "Playback", 0, AC_DAC_REG, DACLEN, 0,
			      sunxi_codec_playback_event,
			      SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_AIF_IN_E("DACR", "Playback", 0, AC_DAC_REG, DACREN, 0,
			      sunxi_codec_playback_event,
			      SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER("Left Output Mixer", AC_MIXER_REG, LMIXEN, 0,
			   left_output_mixer, ARRAY_SIZE(left_output_mixer)),
	SND_SOC_DAPM_MIXER("Right Output Mixer", AC_MIXER_REG, RMIXEN, 0,
			   right_output_mixer, ARRAY_SIZE(right_output_mixer)),

	SND_SOC_DAPM_MUX("Left LINEOUT Mux", SND_SOC_NOPM,
			 0, 0, &left_lineout_mux),
	SND_SOC_DAPM_MUX("Right LINEOUT Mux", SND_SOC_NOPM,
			 0, 0, &right_lineout_mux),

	SND_SOC_DAPM_OUTPUT("LINEOUTL"),
	SND_SOC_DAPM_OUTPUT("LINEOUTR"),

	SND_SOC_DAPM_LINE("LINEOUT", sunxi_lineout_event),
};

static const struct snd_soc_dapm_route sunxi_codec_dapm_routes[] = {
	{"Left Output Mixer", "DACR Switch", "DACR"},
	{"Left Output Mixer", "DACL Switch", "DACL"},

	{"Right Output Mixer", "DACL Switch", "DACL"},
	{"Right Output Mixer", "DACR Switch", "DACR"},

	{"Left LINEOUT Mux", "LOMixer", "Left Output Mixer"},
	{"Left LINEOUT Mux", "LROMixer", "Right Output Mixer"},
	{"Right LINEOUT Mux", "ROMixer", "Right Output Mixer"},
	{"Right LINEOUT Mux", "LROMixer", "Left Output Mixer"},

	{"LINEOUTL", NULL, "Left LINEOUT Mux"},
	{"LINEOUTR", NULL, "Right LINEOUT Mux"},
};

/* for sunxi_internal_codec_probe() */
int internal_codec_controls_add(struct snd_soc_component *component)
{
	int ret;
	struct snd_soc_dapm_context *dapm = &component->dapm;

	ret = snd_soc_add_component_controls(component, sunxi_codec_controls,
					     ARRAY_SIZE(sunxi_codec_controls));
	if (ret) {
		SND_LOG_ERR(HLOG, "register codec kcontrols failed\n");
		return ADAPT_FAILURE;
	}

	ret = snd_soc_dapm_new_controls(dapm, sunxi_codec_dapm_widgets,
					ARRAY_SIZE(sunxi_codec_dapm_widgets));
	if (ret) {
		SND_LOG_ERR(HLOG, "register codec dapm_widgets failed\n");
		return ADAPT_FAILURE;
	}

	ret = snd_soc_dapm_add_routes(dapm, sunxi_codec_dapm_routes,
				      ARRAY_SIZE(sunxi_codec_dapm_routes));
	if (ret) {
		SND_LOG_ERR(HLOG, "register codec dapm_routes failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_suspend() */
static struct reg_label reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_FIFO_CTL),
	REG_LABEL(SUNXI_DAC_FIFO_STA),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG_REG),
	REG_LABEL(AC_DAC_REG),
	REG_LABEL(AC_MIXER_REG),
	REG_LABEL(AC_RAMP_REG),
	REG_LABEL_END,
};

int internal_codec_suspend(struct adapter_cntlr *cntlr,
			   struct sunxi_codec_info *codec_info)
{
	struct reg_cntlr *reg = cntlr->reg;
	struct regulator_cntlr *rglt = cntlr->regulator;
	struct clk_cntlr *clk =cntlr->clk;

	SND_LOG_DEBUG(HLOG, "\n");

	save_audio_reg(reg, codec_info->regmap, reg_labels);

	/* regulator about */
	rglt->ops->disable(rglt, codec_info->avcc);
	rglt->ops->disable(rglt, codec_info->dvcc);

	/* clk about */
	clk->ops->disable_unprepare(clk, codec_info->clk_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_pll_audio);
	clk->ops->disable_unprepare(clk, codec_info->clk_bus_audio);
	clk->ops->assert_rst(clk, codec_info->clk_rst);

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_resume() */
int internal_codec_resume(struct adapter_cntlr *cntlr,
			  struct sunxi_codec_info *codec_info)
{
	int ret;
	struct reg_cntlr *reg = cntlr->reg;
	struct regulator_cntlr *rglt = cntlr->regulator;
	struct clk_cntlr *clk =cntlr->clk;

	SND_LOG_DEBUG(HLOG, "\n");

	/* regulator about */
	rglt->ops->enable(rglt, codec_info->avcc);
	rglt->ops->enable(rglt, codec_info->dvcc);

	/* clk about */
	ret = clk->ops->deassert_rst(clk, codec_info->clk_rst);
	if (ret) {
		SND_LOG_ERR(HLOG, "deassert clk rst failed\n");
		return ADAPT_FAILURE;
	}
	ret = clk->ops->set_rate(clk, codec_info->clk_pll_audio, 24576000);
	if (ret) {
		SND_LOG_ERR(HLOG, "set rate clk pll audio failed\n");
		return ADAPT_FAILURE;
	}

	ret = clk->ops->enable_prepare(clk, codec_info->clk_bus_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk bus audio failed\n");
		return ADAPT_FAILURE;
	}

	ret = clk->ops->enable_prepare(clk, codec_info->clk_pll_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk pll audio failed\n");
		return ADAPT_FAILURE;
	}
	ret = clk->ops->enable_prepare(clk, codec_info->clk_audio);
	if (ret) {
		SND_LOG_ERR(HLOG, "enable clk audio failed\n");
		return ADAPT_FAILURE;
	}

	internal_codec_init(cntlr, codec_info);
	echo_audio_reg(reg, codec_info->regmap, reg_labels);

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_dai_hw_params() */
static const struct sample_rate sample_rate_conv[] = {
	{44100, 0},
	{48000, 0},
	{8000, 5},
	{32000, 1},
	{22050, 2},
	{24000, 2},
	{16000, 3},
	{11025, 4},
	{12000, 4},
	{192000, 6},
	{96000, 7},
	RATE_CONV_END,
};

int internal_codec_hw_params(struct adapter_cntlr *cntlr,
			     struct sunxi_codec_info *codec_info,
			     int stream, int format, unsigned int rate,
			     unsigned int channels)

{
	int i;
	REG_HANDLE regmap = codec_info->regmap;
	struct reg_cntlr *reg = cntlr->reg;

	SND_LOG_DEBUG(HLOG, "\n");

	if (stream == PCM_STREAM_PLAYBACK)
		goto playback;
	else
		goto capture;

playback:
	switch (format) {
	case PCM_FORMAT_S16_LE:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x3 << DAC_FIFO_MODE, 0x3 << DAC_FIFO_MODE);
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << TX_SAMPLE_BITS, 0x0 << TX_SAMPLE_BITS);
	break;
	case PCM_FORMAT_S24_LE:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x3 << DAC_FIFO_MODE, 0x0 << DAC_FIFO_MODE);
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << TX_SAMPLE_BITS, 0x1 << TX_SAMPLE_BITS);
	break;
	default:
		SND_LOG_ERR(HLOG, "unsupport format\n");
		break;
	}

	i = 0;
	while(sample_rate_conv[i].samplerate != 0) {
		if (sample_rate_conv[i].samplerate == rate)
			break;
		i++;
	}
	reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
			      0x7 << DAC_FS,
			      sample_rate_conv[i].rate_bit << DAC_FS);

	switch (channels) {
	case 1:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << DAC_MONO_EN, 0x1 << DAC_MONO_EN);
	break;
	case 2:
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << DAC_MONO_EN, 0x0 << DAC_MONO_EN);
	break;
	default:
		SND_LOG_ERR(HLOG, "unsupport channel\n");
		return -EINVAL;
	}

	return ADAPT_SUCCESS;

capture:
	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_dai_set_sysclk() */
int internal_codec_set_sysclk(struct adapter_cntlr *cntlr,
			      struct sunxi_codec_info *codec_info,
			      unsigned int freq)

{
	int ret;
	struct clk_cntlr *clk = cntlr->clk;

	SND_LOG_DEBUG(HLOG, "\n");

	ret = clk->ops->set_rate(clk, codec_info->clk_pll_audio, freq);
	if (ret) {
		SND_LOG_ERR(HLOG, "set rate clk pll audio failed\n");
		return ADAPT_FAILURE;
	}
	ret = clk->ops->set_rate(clk, codec_info->clk_audio, freq);
	if (ret) {
		SND_LOG_ERR(HLOG, "set rate clk audio failed\n");
		return ADAPT_FAILURE;
	}

	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_dai_prepare() */
int internal_codec_prepare(struct adapter_cntlr *cntlr,
			   struct sunxi_codec_info *codec_info,
			   int stream)

{
	REG_HANDLE regmap = codec_info->regmap;
	struct reg_cntlr *reg = cntlr->reg;

	SND_LOG_DEBUG(HLOG, "\n");

	if (stream == PCM_STREAM_PLAYBACK)
		goto playback;
	else
		goto capture;

playback:
	reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
			      0x1 << DAC_FIFO_FLUSH,
			      0x1 << DAC_FIFO_FLUSH);
	reg->ops->write(reg, regmap, SUNXI_DAC_FIFO_STA,
			0x1 << DAC_TXE_INT | 1 << DAC_TXU_INT | 0x1 << DAC_TXO_INT);
	reg->ops->write(reg, regmap, SUNXI_DAC_CNT, 0);

	return ADAPT_SUCCESS;

capture:
	return ADAPT_SUCCESS;
}

/* for sunxi_internal_codec_dai_trigger() */
int internal_codec_trigger(struct adapter_cntlr *cntlr,
			   struct sunxi_codec_info *codec_info,
			   int stream, int drq_en)
{
	REG_HANDLE regmap = codec_info->regmap;
	struct reg_cntlr *reg = cntlr->reg;

	SND_LOG_DEBUG(HLOG, "\n");

	if (stream == PCM_STREAM_PLAYBACK)
		goto playback;
	else
		goto capture;

playback:
	if (drq_en) {
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << DAC_DRQ_EN,
				      0x1 << DAC_DRQ_EN);
	} else {
		reg->ops->update_bits(reg, regmap, SUNXI_DAC_FIFO_CTL,
				      0x1 << DAC_DRQ_EN,
				      0x0 << DAC_DRQ_EN);
	}

	return ADAPT_SUCCESS;

capture:
	return ADAPT_SUCCESS;
}
