/*
 * sound\soc\sunxi\snd_sunxi_mach_utils.c
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
#include <linux/of.h>
#include <sound/soc.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_mach_utils.h"

int asoc_simple_clean_reference(struct snd_soc_card *card)
{
	struct snd_soc_dai_link *dai_link;
	int i;

	for_each_card_prelinks(card, i, dai_link) {
		of_node_put(dai_link->cpus->of_node);
		of_node_put(dai_link->codecs->of_node);
	}
	return 0;
}

int asoc_simple_init_priv(struct asoc_simple_priv *priv,
			  struct link_info *li)
{
	struct snd_soc_card *card = simple_priv_to_card(priv);
	struct device *dev = simple_priv_to_dev(priv);
	struct snd_soc_dai_link *dai_link;
	struct simple_dai_props *dai_props;
	struct asoc_simple_dai *dais;
	struct snd_soc_codec_conf *cconf = NULL;
	int i;

	dai_props = devm_kcalloc(dev, li->link, sizeof(*dai_props), GFP_KERNEL);
	dai_link  = devm_kcalloc(dev, li->link, sizeof(*dai_link),  GFP_KERNEL);
	dais      = devm_kcalloc(dev, li->dais, sizeof(*dais),      GFP_KERNEL);
	if (!dai_props || !dai_link || !dais)
		return -ENOMEM;

	if (li->conf) {
		cconf = devm_kcalloc(dev, li->conf, sizeof(*cconf), GFP_KERNEL);
		if (!cconf)
			return -ENOMEM;
	}

	/*
	 * Use snd_soc_dai_link_component instead of legacy style
	 * It is codec only. but cpu/platform will be supported in the future.
	 * see
	 *	soc-core.c :: snd_soc_init_multicodec()
	 *
	 * "platform" might be removed
	 * see
	 *	simple-card-utils.c :: asoc_simple_canonicalize_platform()
	 */
	for (i = 0; i < li->link; i++) {
		dai_link[i].cpus		= &dai_props[i].cpus;
		dai_link[i].num_cpus		= 1;
		dai_link[i].codecs		= &dai_props[i].codecs;
		dai_link[i].num_codecs		= 1;
		dai_link[i].platforms		= &dai_props[i].platforms;
		dai_link[i].num_platforms	= 1;
	}

	priv->dai_props		= dai_props;
	priv->dai_link		= dai_link;
	priv->dais		= dais;
	priv->codec_conf	= cconf;

	card->dai_link		= priv->dai_link;
	card->num_links		= li->link;
	card->codec_conf	= cconf;
	card->num_configs	= li->conf;

	return 0;
}

int asoc_simple_parse_widgets(struct snd_soc_card *card,
			      char *prefix)
{
	struct device_node *node = card->dev->of_node;
	char prop[128];

	if (!prefix)
		prefix = "";

	snprintf(prop, sizeof(prop), "%s%s", prefix, "widgets");

	if (of_property_read_bool(node, prop))
		return snd_soc_of_parse_audio_simple_widgets(card, prop);

	/* no widgets is not error */
	return 0;
}

int asoc_simple_parse_routing(struct snd_soc_card *card,
			      char *prefix)
{
	struct device_node *node = card->dev->of_node;
	char prop[128];

	if (!prefix)
		prefix = "";

	snprintf(prop, sizeof(prop), "%s%s", prefix, "routing");

	if (!of_property_read_bool(node, prop))
		return 0;

	return snd_soc_of_parse_audio_routing(card, prop);
}

int asoc_simple_parse_pin_switches(struct snd_soc_card *card,
				   char *prefix)
{
	const unsigned int nb_controls_max = 16;
	const char **strings, *control_name;
	struct snd_kcontrol_new *controls;
	struct device *dev = card->dev;
	unsigned int i, nb_controls;
	char prop[128];
	int ret;

	if (!prefix)
		prefix = "";

	snprintf(prop, sizeof(prop), "%s%s", prefix, "pin-switches");

	if (!of_property_read_bool(dev->of_node, prop))
		return 0;

	strings = devm_kcalloc(dev, nb_controls_max,
			       sizeof(*strings), GFP_KERNEL);
	if (!strings)
		return -ENOMEM;

	ret = of_property_read_string_array(dev->of_node, prop,
					    strings, nb_controls_max);
	if (ret < 0)
		return ret;

	nb_controls = (unsigned int)ret;

	controls = devm_kcalloc(dev, nb_controls,
				sizeof(*controls), GFP_KERNEL);
	if (!controls)
		return -ENOMEM;

	for (i = 0; i < nb_controls; i++) {
		control_name = devm_kasprintf(dev, GFP_KERNEL,
					      "%s Switch", strings[i]);
		if (!control_name)
			return -ENOMEM;

		controls[i].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
		controls[i].name = control_name;
		controls[i].info = snd_soc_dapm_info_pin_switch;
		controls[i].get = snd_soc_dapm_get_pin_switch;
		controls[i].put = snd_soc_dapm_put_pin_switch;
		controls[i].private_value = (unsigned long)strings[i];
	}

	card->controls = controls;
	card->num_controls = nb_controls;

	return 0;
}

int asoc_simple_parse_daistream(struct device *dev,
				struct device_node *node,
				char *prefix,
				struct snd_soc_dai_link *dai_link)
{
	char prop[128];
	unsigned int dai_stream = 0;
	unsigned int playback_only = BIT(0);
	unsigned int capture_only = BIT(1);

	if (!prefix)
		prefix = "";

	/* check "[prefix]playback_only" */
	snprintf(prop, sizeof(prop), "%splayback_only", prefix);
	if (of_property_read_bool(node, prop))
		dai_stream |= playback_only;

	/* check "[prefix]capture_only" */
	snprintf(prop, sizeof(prop), "%scapture_only", prefix);
	if (of_property_read_bool(node, prop))
		dai_stream |= capture_only;

	if (dai_stream == (playback_only | capture_only)) {
		pr_err("unsupport stream\n");
		dai_link->playback_only = 0;
		dai_link->capture_only = 0;
	} else if (dai_stream == playback_only) {
		dai_link->playback_only = 1;
	} else if (dai_stream == capture_only) {
		dai_link->capture_only = 1;
	} else {
		dai_link->playback_only = 0;
		dai_link->capture_only = 0;
	}

	return 0;
}

int asoc_simple_parse_daifmt(struct device *dev,
			     struct device_node *node,
			     struct device_node *codec,
			     char *prefix,
			     unsigned int *retfmt)
{
	struct device_node *bitclkmaster = NULL;
	struct device_node *framemaster = NULL;
	unsigned int daifmt;

	daifmt = snd_soc_of_parse_daifmt(node, prefix,
					 &bitclkmaster, &framemaster);
	daifmt &= ~SND_SOC_DAIFMT_MASTER_MASK;

	if (!bitclkmaster && !framemaster) {
		/*
		 * No dai-link level and master setting was not found from
		 * sound node level, revert back to legacy DT parsing and
		 * take the settings from codec node.
		 */
		dev_dbg(dev, "Revert to legacy daifmt parsing\n");

		daifmt = snd_soc_of_parse_daifmt(codec, NULL, NULL, NULL) |
			(daifmt & ~SND_SOC_DAIFMT_CLOCK_MASK);
	} else {
		if (codec == bitclkmaster)
			daifmt |= (codec == framemaster) ?
				SND_SOC_DAIFMT_CBM_CFM : SND_SOC_DAIFMT_CBM_CFS;
		else
			daifmt |= (codec == framemaster) ?
				SND_SOC_DAIFMT_CBS_CFM : SND_SOC_DAIFMT_CBS_CFS;
	}

	of_node_put(bitclkmaster);
	of_node_put(framemaster);

	*retfmt = daifmt;

	return 0;
}

int asoc_simple_parse_card_name(struct snd_soc_card *card,
				char *prefix)
{
	int ret;

	if (!prefix)
		prefix = "";

	/* Parse the card name from DT */
	ret = snd_soc_of_parse_card_name(card, "label");
	if (ret < 0 || !card->name) {
		char prop[128];

		snprintf(prop, sizeof(prop), "%sname", prefix);
		ret = snd_soc_of_parse_card_name(card, prop);
		if (ret < 0)
			return ret;
	}

	if (!card->name && card->dai_link)
		card->name = card->dai_link->name;

	return 0;
}

int asoc_simple_set_dailink_name(struct device *dev,
				 struct snd_soc_dai_link *dai_link,
				 const char *fmt, ...)
{
	va_list ap;
	char *name = NULL;
	int ret = -ENOMEM;

	va_start(ap, fmt);
	name = devm_kvasprintf(dev, GFP_KERNEL, fmt, ap);
	va_end(ap);

	if (name) {
		ret = 0;

		dai_link->name		= name;
		dai_link->stream_name	= name;
	}

	return ret;
}

void asoc_simple_canonicalize_platform(struct snd_soc_dai_link *dai_link)
{
	/* Assumes platform == cpu */
	if (!dai_link->platforms->of_node)
		dai_link->platforms->of_node = dai_link->cpus->of_node;

	/*
	 * DPCM BE can be no platform.
	 * Alloced memory will be waste, but not leak.
	 */
	if (!dai_link->platforms->of_node)
		dai_link->num_platforms = 0;
}

void asoc_simple_canonicalize_cpu(struct snd_soc_dai_link *dai_link,
				  int is_single_links)
{
	/*
	 * In soc_bind_dai_link() will check cpu name after
	 * of_node matching if dai_link has cpu_dai_name.
	 * but, it will never match if name was created by
	 * fmt_single_name() remove cpu_dai_name if cpu_args
	 * was 0. See:
	 *	fmt_single_name()
	 *	fmt_multiple_name()
	 */
	if (is_single_links)
		dai_link->cpus->dai_name = NULL;
}

MODULE_AUTHOR("Dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sunxi soundcard machine utils");
