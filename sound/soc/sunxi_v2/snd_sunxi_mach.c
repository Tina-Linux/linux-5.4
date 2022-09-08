/*
 * sound\soc\sunxi\snd_sunxi_mach.c
 * (C) Copyright 2021-2025
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * based on ${LINUX}/sound/soc/generic/simple-card.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <sound/soc.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_mach.h"

#define HLOG	"MACH"
#define DAI	"sound-dai"
#define CELL	"#sound-dai-cells"
#define PREFIX	"soundcard-mach,"

#define DRV_NAME	"sunxi-snd-mach"

static int asoc_simple_parse_dai(struct device_node *node,
				 struct snd_soc_dai_link_component *dlc,
				 int *is_single_link)
{
	struct of_phandle_args args;
	int ret;

	if (!node)
		return 0;

	/*
	 * Get node via "sound-dai = <&phandle port>"
	 * it will be used as xxx_of_node on soc_bind_dai_link()
	 */
	ret = of_parse_phandle_with_args(node, DAI, CELL, 0, &args);
	if (ret)
		return ret;

	/*
	 * FIXME
	 *
	 * Here, dlc->dai_name is pointer to CPU/Codec DAI name.
	 * If user unbinded CPU or Codec driver, but not for Sound Card,
	 * dlc->dai_name is keeping unbinded CPU or Codec
	 * driver's pointer.
	 *
	 * If user re-bind CPU or Codec driver again, ALSA SoC will try
	 * to rebind Card via snd_soc_try_rebind_card(), but because of
	 * above reason, it might can't bind Sound Card.
	 * Because Sound Card is pointing to released dai_name pointer.
	 *
	 * To avoid this rebind Card issue,
	 * 1) It needs to alloc memory to keep dai_name eventhough
	 *    CPU or Codec driver was unbinded, or
	 * 2) user need to rebind Sound Card everytime
	 *    if he unbinded CPU or Codec.
	 */
	ret = snd_soc_of_get_dai_name(node, &dlc->dai_name);
	if (ret < 0)
		return ret;

	dlc->of_node = args.np;

	if (is_single_link)
		*is_single_link = !args.args_count;

	return 0;
}

static void asoc_simple_card_shutdown(struct snd_pcm_substream *substream)
{
}

static int asoc_simple_card_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

static int asoc_simple_card_hw_params(struct snd_pcm_substream *substream,
				      struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct asoc_simple_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, rtd->num);
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret, clk_div;
	unsigned int freq;

	switch (params_rate(params)) {
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
	case 192000:
		freq = 24576000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
	case 176400:
		freq = 22579200;
		break;
	default:
		SND_LOG_ERR(HLOG, "Invalid rate %d\n", params_rate(params));
		return -EINVAL;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, 0, freq, SND_SOC_CLOCK_IN);
	if (ret && ret != -ENOTSUPP) {
		SND_LOG_ERR(HLOG, "cadec_dai set sysclk failed.\n");
		return ret;
	}
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, freq, SND_SOC_CLOCK_OUT);
	if (ret && ret != -ENOTSUPP) {
		SND_LOG_ERR(HLOG, "cpu_dai set sysclk failed.\n");
		return ret;
	}

	ret = snd_soc_dai_set_pll(codec_dai, 0, 0, freq, freq);
	/* if (ret < 0) */
	/* 	SND_LOG_WARN(HLOG, "codec_dai set set_pll failed.\n"); */

	/* set codec dai fmt */
	ret = snd_soc_dai_set_fmt(codec_dai, dai_link->dai_fmt);
	if (ret && ret != -ENOTSUPP)
		SND_LOG_WARN(HLOG, "codec dai set fmt failed\n");

	/* set cpu dai fmt */
	ret = snd_soc_dai_set_fmt(cpu_dai, dai_link->dai_fmt);
	if (ret && ret != -ENOTSUPP)
		SND_LOG_WARN(HLOG, "cpu dai set fmt failed\n");

	clk_div = freq / params_rate(params);

	if (cpu_dai->driver->ops->set_clkdiv) {
		ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, clk_div);
		if (ret < 0) {
			SND_LOG_ERR(HLOG, "set clkdiv failed.\n");
			return ret;
		}
	}

	if (codec_dai->driver->ops->set_clkdiv) {
		ret = snd_soc_dai_set_clkdiv(codec_dai, 0, clk_div);
		if (ret < 0) {
			SND_LOG_ERR(HLOG, "codec_dai set clkdiv failed\n");
			return ret;
		}
	}

	return 0;
}

static struct snd_soc_ops asoc_simple_card_ops = {
	.startup = asoc_simple_card_startup,
	.shutdown = asoc_simple_card_shutdown,
	.hw_params = asoc_simple_card_hw_params,
};

static int asoc_simple_card_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int asoc_simple_card_dai_link_of(struct device_node *node,
					struct asoc_simple_priv *priv,
					int idx,
					bool is_top_level_node)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, idx);
	struct device_node *cpu = NULL;
	struct device_node *plat = NULL;
	struct device_node *codec = NULL;
	char prop[128];
	char *prefix = "";
	int ret, single_cpu;

	/* For single DAI link & old style of DT node */
	if (is_top_level_node) {
		prefix = PREFIX;
	}

	snprintf(prop, sizeof(prop), "%scpu", prefix);
	cpu = of_get_child_by_name(node, prop);
	if (!cpu) {
		ret = -EINVAL;
		SND_LOG_ERR(HLOG, "Can't find %s DT node\n", prop);
		goto dai_link_of_err;
	}

	snprintf(prop, sizeof(prop), "%splat", prefix);
	plat = of_get_child_by_name(node, prop);

	snprintf(prop, sizeof(prop), "%scodec", prefix);
	codec = of_get_child_by_name(node, prop);
	if (!codec) {
		ret = -EINVAL;
		SND_LOG_ERR(HLOG, "Can't find %s DT node\n", prop);
		goto dai_link_of_err;
	}

	ret = asoc_simple_parse_daistream(dev, node, prefix, dai_link);
	if (ret < 0)
		goto dai_link_of_err;

	ret = asoc_simple_parse_daifmt(dev, node, codec,
				       prefix, &dai_link->dai_fmt);
	if (ret < 0)
		goto dai_link_of_err;

	ret = asoc_simple_parse_cpu(cpu, dai_link, &single_cpu);
	if (ret < 0)
		goto dai_link_of_err;

	ret = asoc_simple_parse_codec(codec, dai_link);
	if (ret < 0) {
		if (ret == -EPROBE_DEFER)
			goto dai_link_of_err;
		/*
		dai_link->codecs->name = "snd-soc-dummy";
		dai_link->codec_dai_name = "snd-soc-dummy-dai";
		*/
		dai_link->codecs->name = "sunxi-dummy-codec";
		dai_link->codecs->dai_name = "sunxi-dummy-codec-dai";
		SND_LOG_INFO(HLOG, "use dummy codec for simple card.\n");
	}

	ret = asoc_simple_parse_platform(plat, dai_link);
	if (ret < 0)
		goto dai_link_of_err;

	ret = asoc_simple_set_dailink_name(dev, dai_link,
					   "%s-%s",
					   dai_link->cpus->dai_name,
					   dai_link->codecs->dai_name);
	if (ret < 0)
		goto dai_link_of_err;

	dai_link->ops = &asoc_simple_card_ops;
	dai_link->init = asoc_simple_card_dai_init;

	SND_LOG_INFO(HLOG, "name   : %s\n", dai_link->stream_name);
	SND_LOG_INFO(HLOG, "format : %x\n", dai_link->dai_fmt);
	SND_LOG_INFO(HLOG, "cpu    : %s\n", dai_link->cpus->name);
	SND_LOG_INFO(HLOG, "codec  : %s\n", dai_link->codecs->name);

	asoc_simple_canonicalize_cpu(dai_link, single_cpu);
	asoc_simple_canonicalize_platform(dai_link);

dai_link_of_err:
	of_node_put(cpu);
	of_node_put(codec);

	return ret;
}

static int asoc_simple_card_parse_of(struct asoc_simple_priv *priv)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct device_node *dai_link;
	struct device_node *node = dev->of_node;
	int ret;

	if (!node)
		return -EINVAL;

	/* The off-codec widgets */
	ret = asoc_simple_parse_widgets(&priv->snd_card, PREFIX);
	if (ret < 0)
		return ret;

	/* DAPM routes */
	ret = asoc_simple_parse_routing(&priv->snd_card, PREFIX);
	if (ret < 0)
		return ret;

	ret = asoc_simple_parse_pin_switches(&priv->snd_card, PREFIX);
	if (ret < 0)
		return ret;

	dai_link = of_get_child_by_name(node, PREFIX "dai-link");
	/* Single/Muti DAI link(s) & New style of DT node */
	if (dai_link) {
		struct device_node *np = NULL;
		int i = 0;

		for_each_child_of_node(node, np) {
			SND_LOG_DEBUG(HLOG, "\tlink %d:\n", i);
			ret = asoc_simple_card_dai_link_of(np, priv,
							   i, false);
			if (ret < 0) {
				of_node_put(np);
				goto card_parse_end;
			}
			i++;
		}
	} else {
		/* For single DAI link & old style of DT node */
		ret = asoc_simple_card_dai_link_of(node, priv, 0, true);
		if (ret < 0)
			goto card_parse_end;
	}

	ret = asoc_simple_parse_card_name(&priv->snd_card, PREFIX);

card_parse_end:
	of_node_put(dai_link);

	return ret;
}

static int simple_soc_probe(struct snd_soc_card *card)
{
	return 0;
}

static int asoc_simple_card_probe(struct platform_device *pdev)
{
	struct asoc_simple_priv *priv;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct snd_soc_card *card;
	struct link_info li;
	int ret;

	/* Allocate the private data and the DAI link array */
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	card = simple_priv_to_card(priv);
	card->owner		= THIS_MODULE;
	card->dev		= dev;
	card->probe		= simple_soc_probe;

	memset(&li, 0, sizeof(li));

	/* Get the number of DAI links */
	if (np && of_get_child_by_name(np, PREFIX "dai-link"))
		li.link = of_get_child_count(np);
	else
		li.link = 1;

	ret = asoc_simple_init_priv(priv, &li);
	if (ret < 0)
		return ret;

	if (np && of_device_is_available(np)) {

		ret = asoc_simple_card_parse_of(priv);
		if (ret < 0) {
			if (ret != -EPROBE_DEFER)
				SND_LOG_ERR(HLOG, "parse error %d\n", ret);
			goto err;
		}

	} else {
		SND_LOG_ERR(HLOG, "simple card dts available\n");
	}

	snd_soc_card_set_drvdata(&priv->snd_card, priv);

	/* asoc_simple_debug_info(priv); */
	ret = devm_snd_soc_register_card(&pdev->dev, &priv->snd_card);
	if (ret >= 0)
		return ret;
err:
	asoc_simple_clean_reference(&priv->snd_card);

	return ret;
}

static int asoc_simple_card_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	return asoc_simple_clean_reference(card);
}

static const struct of_device_id snd_soc_sunxi_of_match[] = {
	{ .compatible = "allwinner," DRV_NAME, },
	{},
};
MODULE_DEVICE_TABLE(of, snd_soc_sunxi_of_match);

static struct platform_driver sunxi_soundcard_machine_driver = {
	.driver	= {
		.name		= DRV_NAME,
		.pm		= &snd_soc_pm_ops,
		.of_match_table	= snd_soc_sunxi_of_match,
	},
	.probe	= asoc_simple_card_probe,
	.remove	= asoc_simple_card_remove,
};

module_platform_driver(sunxi_soundcard_machine_driver);

MODULE_AUTHOR("Dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sunxi soundcard machine");
