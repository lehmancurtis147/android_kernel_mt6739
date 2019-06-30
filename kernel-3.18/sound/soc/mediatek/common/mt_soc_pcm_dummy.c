




#include <linux/dma-mapping.h>
#include "AudDrv_Common.h"
#include "AudDrv_Def.h"
#include "AudDrv_Afe.h"
#include "AudDrv_Ana.h"
#include "AudDrv_Clk.h"
#include "mt_soc_pcm_common.h"


static int mtk_dummy_probe(struct platform_device *pdev);
static int mtk_dummypcm_close(struct snd_pcm_substream *substream);
static int mtk_asoc_dummypcm_new(struct snd_soc_pcm_runtime *rtd);
static int mtk_afe_dummy_probe(struct snd_soc_platform *platform);


static struct snd_soc_pcm_runtime *pruntimepcm;

static struct snd_pcm_hardware mtk_dummy_hardware = {
	.info = (SNDRV_PCM_INFO_MMAP |
	SNDRV_PCM_INFO_INTERLEAVED |
	SNDRV_PCM_INFO_RESUME |
	SNDRV_PCM_INFO_MMAP_VALID),
	.formats =   SND_SOC_ADV_MT_FMTS,
	.rates =        SOC_HIGH_USE_RATE,
	.rate_min =     SOC_HIGH_USE_RATE_MIN,
	.rate_max =     SOC_HIGH_USE_RATE_MAX,
	.channels_min =     SOC_NORMAL_USE_CHANNELS_MIN,
	.channels_max =     SOC_NORMAL_USE_CHANNELS_MAX,
	.buffer_bytes_max = SOC_NORMAL_USE_BUFFERSIZE_MAX,
	.period_bytes_max = SOC_NORMAL_USE_BUFFERSIZE_MAX,
	.periods_min =      SOC_NORMAL_USE_PERIODS_MIN,
	.periods_max =     SOC_NORMAL_USE_PERIODS_MAX,
	.fifo_size =        0,
};
static int mtk_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = mtk_dummy_hardware;
	return 0;
}

static int mtk_dummypcm_close(struct snd_pcm_substream *substream)
{
	return 0;
}

static int mtk_dummypcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	/*pr_warn("dummy_pcm_trigger cmd = %d\n", cmd);*/

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		break;
	}
	return -EINVAL;
}

static int mtk_pcm_copy(struct snd_pcm_substream *substream,
			int channel, snd_pcm_uframes_t pos,
			void __user *dst, snd_pcm_uframes_t count)
{
	/*pr_warn("dummy_pcm_copy pos = %lu count = %lu\n ", pos, count);*/
	return 0;
}

static int mtk_pcm_silence(struct snd_pcm_substream *substream,
			   int channel, snd_pcm_uframes_t pos,
			   snd_pcm_uframes_t count)
{
	/*pr_warn("dummy_pcm_silence\n");*/
	return 0; /* do nothing */
}


static void *dummy_page[2];

static struct page *mtk_pcm_page(struct snd_pcm_substream *substream,
				 unsigned long offset)
{
	/*pr_warn("dummy_pcm_page\n");*/
	return virt_to_page(dummy_page[substream->stream]); /* the same page */
}

static int mtk_pcm_prepare(struct snd_pcm_substream *substream)
{
	/*pr_warn("mtk_alsa_prepare\n");*/
	return 0;
}

static int mtk_pcm_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *hw_params)
{
	int ret = 0;

	/*PRINTK_AUDDRV("mtk_pcm_hw_params\n");*/
	return ret;
}

static int mtk_dummy_pcm_hw_free(struct snd_pcm_substream *substream)
{
	/*PRINTK_AUDDRV("mtk_dummy_pcm_hw_free\n");*/
	return snd_pcm_lib_free_pages(substream);
}

static struct snd_pcm_ops mtk_afe_ops = {
	.open =     mtk_pcm_open,
	.close =    mtk_dummypcm_close,
	.ioctl =    snd_pcm_lib_ioctl,
	.hw_params =    mtk_pcm_hw_params,
	.hw_free =  mtk_dummy_pcm_hw_free,
	.prepare =  mtk_pcm_prepare,
	.trigger =  mtk_dummypcm_trigger,
	.copy =     mtk_pcm_copy,
	.silence =  mtk_pcm_silence,
	.page =     mtk_pcm_page,
};

static struct snd_soc_platform_driver mtk_soc_dummy_platform = {
	.ops        = &mtk_afe_ops,
	.pcm_new    = mtk_asoc_dummypcm_new,
	.probe      = mtk_afe_dummy_probe,
};

static int mtk_dummy_probe(struct platform_device *pdev)
{
	pr_warn("mtk_dummy_probe\n");

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	if (pdev->dev.of_node)
		dev_set_name(&pdev->dev, "%s", MT_SOC_DUMMY_PCM);

	pr_warn("%s: dev name %s\n", __func__, dev_name(&pdev->dev));
	return snd_soc_register_platform(&pdev->dev,
					 &mtk_soc_dummy_platform);
}

static int mtk_asoc_dummypcm_new(struct snd_soc_pcm_runtime *rtd)
{
	int ret = 0;

	pruntimepcm  = rtd;
	pr_warn("%s\n", __func__);
	return ret;
}

static int mtk_afe_dummy_probe(struct snd_soc_platform *platform)
{
	pr_warn("mtk_afe_dummy_probe\n");
	return 0;
}


static int mtk_afedummy_remove(struct platform_device *pdev)
{
	pr_debug("%s\n", __func__);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mt_soc_pcm_dummy_of_ids[] = {
	{ .compatible = "mediatek,mt_soc_pcm_dummy", },
	{}
};
#endif

static struct platform_driver mtk_afedummy_driver = {
	.driver = {
		.name = MT_SOC_DUMMY_PCM,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = mt_soc_pcm_dummy_of_ids,
#endif
	},
	.probe = mtk_dummy_probe,
	.remove = mtk_afedummy_remove,
};

#ifndef CONFIG_OF
static struct platform_device *soc_mtkafe_dummy_dev;
#endif

static int __init mtk_soc_dummy_platform_init(void)
{
	int ret = 0;

	pr_warn("%s\n", __func__);
#ifndef CONFIG_OF
	soc_mtkafe_dummy_dev = platform_device_alloc(MT_SOC_DUMMY_PCM , -1);
	if (!soc_mtkafe_dummy_dev)
		return -ENOMEM;

	ret = platform_device_add(soc_mtkafe_dummy_dev);
	if (ret != 0) {
		platform_device_put(soc_mtkafe_dummy_dev);
		return ret;
	}
#endif
	ret = platform_driver_register(&mtk_afedummy_driver);

	return ret;

}
module_init(mtk_soc_dummy_platform_init);

static void __exit mtk_soc_dummy_platform_exit(void)
{

	pr_warn("%s\n", __func__);
	platform_driver_unregister(&mtk_afedummy_driver);
}
module_exit(mtk_soc_dummy_platform_exit);

MODULE_DESCRIPTION("AFE PCM module platform driver");
MODULE_LICENSE("GPL");