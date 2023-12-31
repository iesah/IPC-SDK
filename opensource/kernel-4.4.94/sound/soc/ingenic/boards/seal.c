 /*
 * Copyright (C) 2017 Ingenic Semiconductor Co., Ltd.
 *	http://www.ingenic.com
 * Author: cli <chen.li@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include "../as-v2/as-baic.h"

static const struct snd_soc_dapm_route audio_map[] = {
	{ "BAIC0 playback", NULL, "LO0_MUX" },
	{ "BAIC1 playback", NULL, "LO1_MUX" },
	{ "SPDIF playback", NULL, "LO2_MUX" },
	{ "BAIC3 playback", NULL, "LO3_MUX" },
	{ "BAIC4 playback", NULL, "LO4_MUX" },

	{ "LI0", NULL, "DMIC capture" },
	{ "LI1", NULL, "SPDIF capture" },
	{ "LI2", NULL, "BAIC0 capture" },
	{ "LI3", NULL, "BAIC1 capture" },
	{ "LI4", NULL, "BAIC2 capture" },
	{ "LI6", NULL, "BAIC4 capture" },

	{ "LI8", NULL, "DMA0" },
	{ "LI9", NULL, "DMA1" },
	{ "LI10", NULL, "DMA2" },
	{ "LI11", NULL, "DMA3" },
	{ "LI12", NULL, "DMA4" },

	{ "DMA5", NULL, "LO5_MUX"},
	{ "DMA6", NULL, "LO6_MUX"},
	{ "DMA7", NULL, "LO7_MUX"},
	{ "DMA8", NULL, "LO8_MUX"},
	{ "DMA9", NULL, "LO9_MUX"},

	{ "MIX0", NULL, "LO10_MUX"},
	{ "MIX0", NULL, "LO11_MUX"},
	{ "LI7",  NULL, "MIX0"},

	{"SPDIF_OUT", NULL, "SPDIF playback"},
	{"SPDIF capture", NULL, "SPDIF_IN"},
	{"DMIC capture", NULL, "DMIC"},

	{"BAIC1 playback", NULL, "Virtual-FE0"},
	{"BAIC4 playback", NULL, "Virtual-FE1"},
	{"Virtual-FE2", NULL, "BAIC1 capture"},
	{"Virtual-FE3", NULL, "DMIC capture"},
};

static const struct snd_soc_dapm_widget dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("SPDIF_OUT"),
	SND_SOC_DAPM_INPUT("SPDIF_IN"),
	SND_SOC_DAPM_INPUT("DMIC"),
};

static int seal_baic_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params) {
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int id = cpu_dai->driver->id;
	bool is_playback = substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? true : false;
	int bclk_div = 0, sysclk = 0, slot_width = 0, slots = 0, dir = SND_SOC_CLOCK_OUT,
		sync_div = 0, sync_w_div = 1, tx_msk = 0, rx_msk = 0;
	int divid_dir = is_playback ? DIVID_DIR_T : 0;
	int clkid = CLKID_SYSCLK;
	int ret;

	switch (id) {
	case 0:
		clkid = CLKID_INNER_CODEC;
		break;
	case 1:
		sysclk = 24000000;
		sync_div = 64;
		bclk_div = (sysclk + ((params_rate(params) * sync_div) - 1))
			/ (params_rate(params) * sync_div);
		bclk_div &= ~0x1;
		break;
	case 2:
	case 3:
		sysclk = 24000000;
		slots = params_channels(params);
		slot_width = 32;
		sync_div = 256;
		//sync_div = 64;
		if (is_playback)
			tx_msk = (1 << slots) - 1;
		else
			rx_msk = (1 << slots) - 1;
		bclk_div = (sysclk + ((params_rate(params) * sync_div) - 1))
			/ (params_rate(params) * sync_div);
		bclk_div &= ~0x1;
		sync_w_div = 1;

		snd_soc_dai_set_tdm_slot(codec_dai, tx_msk, rx_msk, slots > 4 ? 8 : 4, slot_width);

		break;
	case 4:
		sysclk = 24000000;
		slots = params_channels(params);
		slot_width = 32;
		sync_div = 64;
		if (is_playback)
			tx_msk = (1 << slots) - 1;
		else
			rx_msk = (1 << slots) - 1;
		sync_w_div = 1;
		break;
	default:
		pr_err("x2000 sound hw params baic dai(%d.%s) not exist\n",\
				id, is_playback ? "p" : "c");
		return -EINVAL;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, clkid, sysclk, dir);
	if (ret)
		return ret;
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DIVID_SYNC_W|divid_dir, sync_w_div);
	if (ret)
		return ret;
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DIVID_SYNC|divid_dir, sync_div);
	if (ret)
		return ret;
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DIVID_BCLK|divid_dir, bclk_div);
	if (ret)
		return ret;
	ret = snd_soc_dai_set_tdm_slot(cpu_dai, tx_msk, rx_msk, slots, slot_width);
	if (ret)
		return ret;
	return 0;
};

static struct snd_soc_ops seal_baic_ops = {
	.hw_params = seal_baic_hw_params,
};

static int spdif_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	u32 sysclk = 0, spdif_out_clk_div = 0, spdif_in_clk_div = 0;
	int clk_ratio = substream->stream ? 1280 : 32 * 2 * params_channels(params);  //fixed 2channels
	int ret;

	sysclk = 24000000;

	spdif_out_clk_div = (sysclk + ((params_rate(params) * clk_ratio) - 1))
		/ (params_rate(params) * clk_ratio);
	spdif_out_clk_div &= ~0x1;

	spdif_in_clk_div = (sysclk + ((params_rate(params) * clk_ratio) - 1))
		/ (params_rate(params) * clk_ratio);
	spdif_in_clk_div &= ~0x1;

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, sysclk, 0);
	if (ret)
		return ret;
	ret = snd_soc_dai_set_clkdiv(cpu_dai, substream->stream,
			!(substream->stream) ? spdif_out_clk_div : spdif_in_clk_div);
	if (ret)
		return ret;

	return 0;
};

static struct snd_soc_ops seal_spdif_ops = {
	.hw_params  = spdif_hw_params,
};

static struct snd_soc_dai_link seal_dais[] = {
	/*FE DAIS*/
	[0] = {
		.name = "DMA0 playback",
		.stream_name = "DMA0 playback",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA0",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	[1] = {
		.name = "DMA1 playback",
		.stream_name = "DMA1 playback",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA1",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	[2] = {
		.name = "DMA2 playback",
		.stream_name = "DMA2 playback",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA2",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	[3] = {
		.name = "DMA3 playback",
		.stream_name = "DMA3 playback",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA3",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},

	[4] = {
		.name = "DMA4 playback",
		.stream_name = "DMA4 playback",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA4",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},

	[5] = {
		.name = "DMA5 capture",
		.stream_name = "DMA5 capture",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA5",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[6] = {
		.name = "DMA6 capture",
		.stream_name = "DMA6 capture",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA6",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[7] = {
		.name = "DMA7 capture",
		.stream_name = "DMA7 capture",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA7",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[8] = {
		.name = "DMA8 capture",
		.stream_name = "DMA8 capture",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA8",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[9] = {
		.name = "DMA9 capture",
		.stream_name = "DMA9 capture",
		.platform_name = "134d0000.as-platform",
		.cpu_dai_name = "DMA9",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[10] = {
		.name = "Dummy DMA0",
		.stream_name = "Dummy DMA0",
		.platform_name = "0.as-virtual-fe",
		.cpu_dai_name = "Virtual-FE0",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},

	[11] = {
		.name = "Dummy DMA1",
		.stream_name = "Dummy DMA1",
		.platform_name = "0.as-virtual-fe",
		.cpu_dai_name = "Virtual-FE1",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},

	[12] = {
		.name = "Dummy DMA2",
		.stream_name = "Dummy DMA2",
		.platform_name = "0.as-virtual-fe",
		.cpu_dai_name = "Virtual-FE2",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	[13] = {
		.name = "Dummy DMA3",
		.stream_name = "Dummy DMA3",
		.platform_name = "0.as-virtual-fe",
		.cpu_dai_name = "Virtual-FE3",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.trigger =  {SND_SOC_DPCM_TRIGGER_PRE, SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},

	/*BE DAIS*/
	[14] = {
		.name = "BAIC0",
		.stream_name = "BAIC0",
		.cpu_dai_name = "BAIC0",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ops = &seal_baic_ops,
		.be_hw_params_fixup = NULL,
		.dai_fmt = SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS,
		.no_pcm = 1,
		.dpcm_capture = 1,
		.dpcm_playback = 1,
	},
	[15] = {
		.name = "BAIC1",
		.stream_name = "BAIC1",
		.cpu_dai_name = "BAIC1",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ops = &seal_baic_ops,
		.be_hw_params_fixup = NULL,
		.dai_fmt = SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS,
		.no_pcm = 1,
		.dpcm_capture = 1,
		.dpcm_playback = 1,
	},
	[16] = {
		.name = "BAIC2",
		.stream_name = "BAIC2",
		.cpu_dai_name = "BAIC2",
		.codec_dai_name = "ak5558.4-0013", // ak5558.4-0013
		.codec_name = "ak5558.4-0013",	//ak5558.4-0013
		.ops = &seal_baic_ops,
		.be_hw_params_fixup = NULL,
		//.dai_fmt = SND_SOC_DAIFMT_DSP_A|SND_SOC_DAIFMT_NB_IF|SND_SOC_DAIFMT_CBS_CFS,
		.dai_fmt = SND_SOC_DAIFMT_DSP_B|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS,
		.dpcm_capture = 1,
		.no_pcm = 1,
	},
	[17] = {
		.name = "BAIC3",
		.stream_name = "BAIC3",
		.cpu_dai_name = "BAIC3",
		.codec_dai_name = "ak4458.4-0010",
		.codec_name = "ak4458.4-0010",
		.ops = &seal_baic_ops,
		.be_hw_params_fixup = NULL,
		//.dai_fmt = SND_SOC_DAIFMT_DSP_A|SND_SOC_DAIFMT_NB_IF|SND_SOC_DAIFMT_CBS_CFS,
		.dai_fmt = SND_SOC_DAIFMT_DSP_B|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
	[18] = {
		.name = "BAIC4",
		.stream_name = "BAIC4",
		.cpu_dai_name = "BAIC4",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ops = &seal_baic_ops,
		.be_hw_params_fixup = NULL,
		.dai_fmt = SND_SOC_DAIFMT_DSP_A|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS,
		.no_pcm = 1,
		.dpcm_capture = 1,
		.dpcm_playback = 1,
	},
	[19] = {
		.name = "SPDIF",
		.stream_name = "SPDIF",
		.cpu_dai_name = "SPDIF",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ops = &seal_spdif_ops,
		.no_pcm = 1,
		.dpcm_capture = 1,
		.dpcm_playback = 1,
	},
	[20] = {
		.name = "DMIC",
		.stream_name = "DMIC",
		.cpu_dai_name = "DMIC",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.no_pcm = 1,
		.capture_only = 1
	},
};

static struct snd_soc_aux_dev seal_x2000_aux_dev = {
	.name = "aux_mixer",
	.codec_name = "134dc000.as-mixer",
};


static int snd_seal_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card;
	int ret;

	card = (struct snd_soc_card *)devm_kzalloc(&pdev->dev,
			sizeof(struct snd_soc_card), GFP_KERNEL);
	if (!card)
		return -ENOMEM;

	card->dapm_widgets = dapm_widgets;
	card->num_dapm_widgets = ARRAY_SIZE(dapm_widgets);
	card->dapm_routes = audio_map;
	card->num_dapm_routes = ARRAY_SIZE(audio_map);
	card->dai_link = seal_dais;
	card->num_links = ARRAY_SIZE(seal_dais);
	card->owner = THIS_MODULE;
	card->dev = &pdev->dev;
	card->aux_dev = &seal_x2000_aux_dev;
	card->num_aux_devs = 1;

	ret = snd_soc_of_parse_card_name(card, "ingenic,model");
	if (ret)
		return ret;

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, card);
	dev_info(&pdev->dev, "Sound Card successed\n");

	return ret;
}

static int snd_seal_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id seal_match_table[] = {
	{ .compatible = "ingenic,seal-sound", },
	{ }
};
MODULE_DEVICE_TABLE(of, seal_match_table);

static struct platform_driver snd_seal_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "x2000-seal",
		.of_match_table = seal_match_table
	},
	.probe = snd_seal_probe,
	.remove = snd_seal_remove,
};
module_platform_driver(snd_seal_driver);

MODULE_AUTHOR("cli<chen.li@ingenic.com>");
MODULE_DESCRIPTION("ALSA SoC X2000 seal Snd Card");
MODULE_LICENSE("GPL");
