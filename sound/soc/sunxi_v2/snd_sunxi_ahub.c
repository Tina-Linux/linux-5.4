/*
 * sound\soc\sunxi\snd_sunxi_ahub.c
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
#include <sound/soc.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_pcm.h"
#include "snd_sunxi_ahub.h"

static int sunxi_ahub_dai_hw_params(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *params,
				    struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_ahub_dai_set_sysclk(struct snd_soc_dai *dai,
				     int clk_id,
				     unsigned int freq,
				     int dir)
{
	return 0;
}

static int sunxi_ahub_dai_trigger(struct snd_pcm_substream *substream,
				  int cmd,
				  struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_ahub_dai_prepare(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops sunxi_ahub_dai_ops = {
	.hw_params	= sunxi_ahub_dai_hw_params,
	.set_sysclk	= sunxi_ahub_dai_set_sysclk,
	.trigger	= sunxi_ahub_dai_trigger,
	.prepare	= sunxi_ahub_dai_prepare,
};

static struct snd_soc_dai_driver sunxi_ahub_dai[] = {
	{
		.name = "ahub_daudio0",
		.playback = {
			.stream_name	= "Playback",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.capture = {
			.stream_name	= "Capture",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &sunxi_ahub_dai_ops,
	},
	{
		.name = "ahub_daudio1",
		.playback = {
			.stream_name	= "Playback",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.capture = {
			.stream_name	= "Capture",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &sunxi_ahub_dai_ops,
	},
	{
		.name = "ahub_daudio2",
		.playback = {
			.stream_name	= "Playback",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.capture = {
			.stream_name	= "Capture",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &sunxi_ahub_dai_ops,
	},
	{
		.name = "ahub_daudio3",
		.playback = {
			.stream_name	= "Playback",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.capture = {
			.stream_name	= "Capture",
			.channels_min	= 1,
			.channels_max	= 16,
			.rates		= SNDRV_PCM_RATE_8000_192000
					| SNDRV_PCM_RATE_KNOT,
			.formats	= SNDRV_PCM_FMTBIT_S16_LE
					| SNDRV_PCM_FMTBIT_S24_LE
					| SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &sunxi_ahub_dai_ops,
	},
};

static int sunxi_ahub_probe(struct snd_soc_component *component)
{
	return 0;
}

static int sunxi_ahub_suspend(struct snd_soc_component *component)
{
	return 0;
}

static int sunxi_ahub_resume(struct snd_soc_component *component)
{
	return 0;
}

static struct snd_soc_component_driver sunxi_ahub_dev = {
	.probe		= sunxi_ahub_probe,
	.suspend	= sunxi_ahub_suspend,
	.resume		= sunxi_ahub_resume,
};

static int sunxi_ahub_dev_probe(struct platform_device *pdev)
{
	int ret;

	ret = snd_soc_register_component(&pdev->dev,
					 &sunxi_ahub_dev,
					 sunxi_ahub_dai,
					 ARRAY_SIZE(sunxi_ahub_dai));
	if (ret) {
		SND_LOG_ERR("component register failed\n");
		ret = -ENOMEM;
		goto err_snd_soc_register_component;
	}

	ret = snd_soc_sunxi_dma_platform_register(&pdev->dev);
	if (ret) {
		SND_LOG_ERR("register ASoC platform failed\n");
		ret = -ENOMEM;
		goto err_snd_soc_sunxi_dma_platform_register;
	}

	SND_LOG_INFO("register ahub platform success\n");

	return 0;

err_snd_soc_sunxi_dma_platform_register:
	snd_soc_unregister_component(&pdev->dev);
err_snd_soc_register_component:
	return ret;
}

static int sunxi_ahub_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);

	SND_LOG_INFO("unregister ahub platform success\n");

	return 0;
}

static const struct of_device_id sunxi_ahub_of_match[] = {
	{ .compatible = "allwinner,sunxi-snd-plat-ahub", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_ahub_of_match);

static struct platform_driver sunxi_ahub_driver = {
	.driver	= {
		.name		= "sunxi-snd-plat-ahub",
		.owner		= THIS_MODULE,
		.of_match_table	= sunxi_ahub_of_match,
	},
	.probe	= sunxi_ahub_dev_probe,
	.remove	= sunxi_ahub_dev_remove,
};

module_platform_driver(sunxi_ahub_driver);

MODULE_AUTHOR("Dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sunxi soundcard platform of ahub");
