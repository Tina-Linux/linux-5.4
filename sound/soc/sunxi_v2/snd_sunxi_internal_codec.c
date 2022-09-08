/*
 * sound\soc\sunxi\snd_sunxi_internal_codec.c
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
#include <sound/soc.h>
#include <sound/tlv.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_pcm.h"
#include "snd_sunxi_adapter.h"
#include "snd_sunxi_internal_codec.h"

#define HLOG		"CODECDAI"
#define DRV_NAME	"sunxi-snd-codec"

struct adapter_cntlr g_adapter_cntlr;

static int sunxi_internal_codec_dai_hw_params(struct snd_pcm_substream *substream,
					      struct snd_pcm_hw_params *params,
					      struct snd_soc_dai *dai)
{
	int ret;
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	ret = internal_codec_hw_params(&g_adapter_cntlr, codec_info,
				       substream->stream,
				       params_format(params),
				       params_rate(params),
				       params_channels(params));
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec set hw_params failed\n");
		return -1;
	}

	return 0;
}

static int sunxi_internal_codec_dai_set_sysclk(struct snd_soc_dai *dai,
					       int clk_id,
					       unsigned int freq,
					       int dir)
{
	int ret;
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	ret = internal_codec_set_sysclk(&g_adapter_cntlr, codec_info, freq);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec set sysclk failed\n");
		return -1;
	}

	return 0;
}


static int sunxi_internal_codec_dai_prepare(struct snd_pcm_substream *substream,
					    struct snd_soc_dai *dai)
{
	int ret;
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	ret = internal_codec_prepare(&g_adapter_cntlr, codec_info, substream->stream);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec set prepare failed\n");
		return -1;
	}

	return 0;
}

static int sunxi_internal_codec_dai_trigger(struct snd_pcm_substream *substream,
					    int cmd,
					    struct snd_soc_dai *dai)
{
	int ret, drq_en;
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "cmd -> %d\n", cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		drq_en = 1;
	break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		drq_en = 0;
	break;
	default:
		return -EINVAL;
	}

	ret = internal_codec_trigger(&g_adapter_cntlr, codec_info,
				     substream->stream, drq_en);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec set trigger failed\n");
		return -1;
	}

	return 0;
}

static const struct snd_soc_dai_ops sunxi_internal_codec_dai_ops = {
	.hw_params	= sunxi_internal_codec_dai_hw_params,
	.set_sysclk	= sunxi_internal_codec_dai_set_sysclk,
	.prepare	= sunxi_internal_codec_dai_prepare,
	.trigger	= sunxi_internal_codec_dai_trigger,
};

static struct snd_soc_dai_driver sunxi_internal_codec_dai = {
	.playback = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &sunxi_internal_codec_dai_ops,
};

static int sunxi_internal_codec_probe(struct snd_soc_component *component)
{
	int ret;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	internal_codec_init(&g_adapter_cntlr, codec_info);

	/* add controls */
	ret = internal_codec_controls_add(component);
	if (ret)
		SND_LOG_ERR(HLOG, "register codec controls failed\n");

	return 0;
}

static int sunxi_internal_codec_suspend(struct snd_soc_component *component)
{
	int ret;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	ret = internal_codec_suspend(&g_adapter_cntlr, codec_info);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec suspend failed\n");
		return -1;
	}

	return 0;
}

static int sunxi_internal_codec_resume(struct snd_soc_component *component)
{
	int ret;
	struct sunxi_codec_info *codec_info = snd_soc_component_get_drvdata(component);

	SND_LOG_DEBUG(HLOG, "\n");

	ret = internal_codec_resume(&g_adapter_cntlr, codec_info);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec resume failed\n");
		return -1;
	}

	return 0;
}

static struct snd_soc_component_driver sunxi_internal_codec_dev = {
	.probe		= sunxi_internal_codec_probe,
	.suspend	= sunxi_internal_codec_suspend,
	.resume		= sunxi_internal_codec_resume,
};

static int sunxi_internal_codec_dev_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct sunxi_codec_info *codec_info;

	SND_LOG_DEBUG(HLOG, "\n");

	/* sunxi codec info */
	codec_info = kzalloc(sizeof(struct sunxi_codec_info), GFP_KERNEL);
	if (!codec_info) {
		SND_LOG_ERR(HLOG, "can't allocate sunxi codec memory\n");
		ret = -ENOMEM;
		goto err;
	}
	dev_set_drvdata(dev, codec_info);
	codec_info->cntlr = &g_adapter_cntlr;

	/* sunxi adapter probe */
	ret = regulator_adapter_probe(&g_adapter_cntlr.regulator, dev);
	if(ret) {
		SND_LOG_ERR(HLOG, "regulator adapter failed\n");
		goto err;
	}
	ret = clk_adapter_probe(&g_adapter_cntlr.clk, np);
	if(ret) {
		SND_LOG_ERR(HLOG, "clk adapter failed\n");
		goto err;
	}
	ret = reg_adapter_probe(&g_adapter_cntlr.reg, dev);
	if(ret) {
		SND_LOG_ERR(HLOG, "regmap adapter failed\n");
		goto err;
	}
	ret = parse_adapter_probe(&g_adapter_cntlr.parse, np);
	if(ret) {
		SND_LOG_ERR(HLOG, "parse adapter failed\n");
		goto err;
	}
	ret = gpio_adapter_probe(&g_adapter_cntlr.gpio, dev);
	if(ret) {
		SND_LOG_ERR(HLOG, "gpio adapter failed\n");
		goto err;
	}

	/* sunxi internal codec probe */
	ret = internal_codec_probe(&g_adapter_cntlr, codec_info);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal codec probe failed\n");
		ret = -1;
		goto err;
	}

	/* alsa component register */
	ret = snd_soc_register_component(dev,
					 &sunxi_internal_codec_dev,
					 &sunxi_internal_codec_dai, 1);
	if (ret) {
		SND_LOG_ERR(HLOG, "internal-codec component register failed\n");
		ret = -ENOMEM;
		goto err;
	}

	SND_LOG_INFO(HLOG, "register internal-codec codec success\n");

	return 0;

err:
	regulator_adapter_remove(&g_adapter_cntlr.regulator);
	clk_adapter_remove(&g_adapter_cntlr.clk);
	reg_adapter_remove(&g_adapter_cntlr.reg);
	parse_adapter_remove(&g_adapter_cntlr.parse);
	if (codec_info)
		kfree(codec_info);
	of_node_put(np);

	return ret;
}

static int sunxi_internal_codec_dev_remove(struct platform_device *pdev)
{
	struct sunxi_codec_info *codec_info = dev_get_drvdata(&pdev->dev);

	SND_LOG_DEBUG(HLOG, "\n");

	/* alsa component unregister */
	snd_soc_unregister_component(&pdev->dev);

	/* sunxi internal codec remove */
	internal_codec_remove(&g_adapter_cntlr, codec_info);

	/* sunxi adapter remove */
	regulator_adapter_remove(&g_adapter_cntlr.regulator);
	clk_adapter_remove(&g_adapter_cntlr.clk);
	reg_adapter_remove(&g_adapter_cntlr.reg);
	parse_adapter_remove(&g_adapter_cntlr.parse);
	gpio_adapter_remove(&g_adapter_cntlr.gpio);

	/* sunxi codec custom info free */
	if (codec_info)
		kfree(codec_info);
	of_node_put(pdev->dev.of_node);

	SND_LOG_INFO(HLOG, "unregister internal-codec codec success\n");

	return 0;
}

static const struct of_device_id sunxi_internal_codec_of_match[] = {
	{ .compatible = "allwinner," DRV_NAME, },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_internal_codec_of_match);

static struct platform_driver sunxi_internal_codec_driver = {
	.driver	= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= sunxi_internal_codec_of_match,
	},
	.probe	= sunxi_internal_codec_dev_probe,
	.remove	= sunxi_internal_codec_dev_remove,
};

module_platform_driver(sunxi_internal_codec_driver);

MODULE_AUTHOR("Dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sunxi soundcard codec of internal-codec");
