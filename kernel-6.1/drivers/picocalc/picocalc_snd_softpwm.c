/*
 * Copyright (C) 2017 starterkit.ru
 *
 * Based on simple driver for PWM (Pulse Width Modulator) controller
 * Sascha Hauer <s.hauer@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/control.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("hiro <hiro@hiro.com>");

#define MICRO_SEC 1000000
#define NANO_SEC  (MICRO_SEC*1000)
/*
#define DEFAULT_DUTY_CYCLE 0
#define DEFAULT_PERIOD (NANO_SEC / 10000) // = 125000 // 0.125ms
*/
#define DEFAULT_DUTY_CYCLE 0
#define DEFAULT_PERIOD (125000 / 1) //(NANO_SEC / 8000) // = 125000 // 0.125ms

/*
#define DEFAULT_DUTY_CYCLE 1500000 //1.5ms
#define DEFAULT_PERIOD  20000000 // 20ms
*/

static const struct of_device_id picocalc_snd_pwm_dt_ids[] = {
	{ .compatible = "fsl,picocalc-snd-pwm", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, picocalc_snd_pwm_dt_ids);

#define dbg(fmt, arg...) \
	printk("picocalc-snd-pwm: %s: " fmt, __func__, ## arg);

struct picocalc_snd_dev {
	struct platform_device *pdev;
	struct snd_card *snd_card;
	struct snd_pcm_substream *ss;
	spinlock_t lock;

	unsigned int data_ptr;
	unsigned int period_ptr;

	unsigned int is_on;
	struct gpio_desc *left_gpio;

    struct hrtimer tm1, tm2;
    ktime_t t1;
    unsigned long duty_cycle_ns;
    unsigned long period_ns;
    unsigned long cnt;
};

enum hrtimer_restart cb1(struct hrtimer *t) {
    struct picocalc_snd_dev *picocalc = container_of(t, struct picocalc_snd_dev, tm1);
    ktime_t now;
    int ovr;

    if (!picocalc->is_on)
    {
        return HRTIMER_NORESTART;
    }

    now = hrtimer_cb_get_time(t);
    ovr = hrtimer_forward(t, now, picocalc->t1);

    if (picocalc->duty_cycle_ns) {
        gpiod_set_raw_value(picocalc->left_gpio, 1);
        if (picocalc->duty_cycle_ns < picocalc->period_ns) {
            ktime_t t2 = ktime_set(0, picocalc->duty_cycle_ns);
            hrtimer_start(&picocalc->tm2, t2, HRTIMER_MODE_REL);
        }
    } else {
        //gpiod_set_raw_value(picocalc->left_gpio, 0);
    }

    return HRTIMER_RESTART;
}

/**
 * @brief callback for timer 2 - responsible for falling slope
 *
 * @param t timer 
 *
 * @return _NORESTART
 */
enum hrtimer_restart cb2(struct hrtimer *t) {
    struct picocalc_snd_dev *picocalc = container_of(t, struct picocalc_snd_dev, tm2);

	unsigned char *data;
	unsigned int buffer_size;
	unsigned int period_size;
	unsigned int period_elapsed = 0;

    if (!picocalc->is_on)
    {
        return HRTIMER_NORESTART;
    }

    gpiod_set_raw_value(picocalc->left_gpio, 0);

    picocalc->cnt++;
    if (picocalc->cnt < 1)
    {
        return HRTIMER_NORESTART;
    }
    picocalc->cnt = 0;

    data = picocalc->ss->runtime->dma_area;
    buffer_size = picocalc->ss->runtime->buffer_size;
    period_size = picocalc->ss->runtime->period_size;
    if (++picocalc->data_ptr >= buffer_size)
    {
    	picocalc->data_ptr = 0;
    }

    picocalc->duty_cycle_ns = (uint32_t)data[picocalc->data_ptr]  * picocalc->period_ns / 255;

    if (++picocalc->period_ptr >= period_size) {
    	picocalc->period_ptr = 0;
    	period_elapsed = 1;
    }

	if (period_elapsed)
		snd_pcm_period_elapsed(picocalc->ss);

    return HRTIMER_NORESTART;
}


static int picocalc_pwm_enable(struct picocalc_snd_dev *picocalc)
{
    hrtimer_init(&picocalc->tm1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hrtimer_init(&picocalc->tm2, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    picocalc->t1 = ktime_set(0, picocalc->period_ns);
    picocalc->tm1.function = &cb1;
    picocalc->tm2.function = &cb2;

    hrtimer_start(&picocalc->tm1, picocalc->t1, HRTIMER_MODE_REL);

	return 0;
}

static void picocalc_pwm_disable(struct picocalc_snd_dev *picocalc)
{
    picocalc->duty_cycle_ns = 0;
    if (hrtimer_active(&picocalc->tm1) || hrtimer_is_queued(&picocalc->tm1)) {
        hrtimer_cancel(&picocalc->tm1);
    }

    if (hrtimer_active(&picocalc->tm2) || hrtimer_is_queued(&picocalc->tm2)) {
        hrtimer_cancel(&picocalc->tm2);
    }
}

/*
static irqreturn_t picocalc_irq_handler(int irq, void *dev_id)
{
	struct picocalc_snd_dev *picocalc = dev_id;
	unsigned int period_elapsed = 0;
	unsigned long flags;

	spin_lock_irqsave(&picocalc->lock, flags);
	if (picocalc->is_on) {
		unsigned char *data = picocalc->ss->runtime->dma_area;
		unsigned int buffer_size = picocalc->ss->runtime->buffer_size;
		unsigned int period_size = picocalc->ss->runtime->period_size;
		int i = 0;

		for (i = 0; i < 3; i++) {
			if (++picocalc->data_ptr >= buffer_size)
				picocalc->data_ptr = 0;
			writel(data[picocalc->data_ptr], picocalc->mmio_base + MX3_PWMSAR);

			if (++picocalc->period_ptr >= period_size) {
				picocalc->period_ptr = 0;
				period_elapsed = 1;
			}
		}
		writel(readl(picocalc->mmio_base + MX3_PWMSR),
			picocalc->mmio_base + MX3_PWMSR);
	}
	spin_unlock_irqrestore(&picocalc->lock, flags);

	if (period_elapsed)
		snd_pcm_period_elapsed(picocalc->ss);

	return IRQ_HANDLED;
}
*/

/**
 * PCM Interface
 */
static int picocalc_pcm_hw_params(struct snd_pcm_substream *ss,
		struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_malloc_pages(ss, params_buffer_bytes(hw_params));
}

static int picocalc_pcm_hw_free(struct snd_pcm_substream *ss)
{
	return snd_pcm_lib_free_pages(ss);
}

static const struct snd_pcm_hardware picocalc_playback_hw = {
	.info			= (SNDRV_PCM_INFO_MMAP |
				   SNDRV_PCM_INFO_MMAP_VALID |
				   SNDRV_PCM_INFO_INTERLEAVED |
				   SNDRV_PCM_INFO_HALF_DUPLEX),
	.formats		= SNDRV_PCM_FMTBIT_U8,
	.rates			= SNDRV_PCM_RATE_8000,
	.rate_min		= 8000,
	.rate_max		= 8000,
	.channels_min		= 1,
	.channels_max		= 1,
	.buffer_bytes_max	= 8 * 1024,
	.period_bytes_min	= 4,
	.period_bytes_max	= 4 * 1024,
	.periods_min		= 4,
	.periods_max		= 1024, 
};

static int picocalc_pcm_open(struct snd_pcm_substream *ss)
{
	struct picocalc_snd_dev *picocalc = snd_pcm_substream_chip(ss);

	ss->runtime->hw = picocalc_playback_hw;
	picocalc->ss = ss;
	return 0;
}

static int picocalc_pcm_close(struct snd_pcm_substream *ss)
{
	return 0;
}

static int picocalc_pcm_prepare(struct snd_pcm_substream *ss)
{
	return 0;
}

static int picocalc_pcm_trigger(struct snd_pcm_substream *ss, int cmd)
{
	struct picocalc_snd_dev *picocalc = snd_pcm_substream_chip(ss);
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&picocalc->lock, flags);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		{
		    unsigned char *data = picocalc->ss->runtime->dma_area;
		    picocalc->duty_cycle_ns = (uint32_t)data[0] * picocalc->period_ns / 255; 
		    picocalc->data_ptr = 0; 
		    picocalc->period_ptr = 0; 
		    picocalc->cnt = 0;
		}
		if (picocalc_pwm_enable(picocalc) == 0)
			picocalc->is_on = 1;
		else
			ret = -EIO;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		if (!picocalc->is_on)
			break;
		picocalc->is_on = 0;
		picocalc->duty_cycle_ns = 0;
		break;
	default:
		ret = -EINVAL;
	}
	spin_unlock_irqrestore(&picocalc->lock, flags);

	return ret;
}

static snd_pcm_uframes_t picocalc_pcm_pointer(struct snd_pcm_substream *ss)
{
	struct picocalc_snd_dev *picocalc = snd_pcm_substream_chip(ss);

	return picocalc->data_ptr;
}

static struct snd_pcm_ops picocalc_pcm_ops = {
	.open = picocalc_pcm_open,
	.close = picocalc_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = picocalc_pcm_hw_params,
	.hw_free = picocalc_pcm_hw_free,
	.prepare = picocalc_pcm_prepare,
	.trigger = picocalc_pcm_trigger,
	.pointer = picocalc_pcm_pointer,
};

static int picocalc_snd_register(struct picocalc_snd_dev *picocalc)
{
	static struct snd_device_ops ops = { NULL };
	struct snd_card *card;
	struct snd_pcm *pcm;
	int ret;

	ret = snd_card_new(&picocalc->pdev->dev, SNDRV_DEFAULT_IDX1,
		SNDRV_DEFAULT_STR1,
		THIS_MODULE, 0, &card);
	if (ret < 0)
		return ret;

	picocalc->snd_card = card;
	strlcpy(card->driver, KBUILD_MODNAME, sizeof(card->driver));
	strlcpy(card->shortname, "picocalc-snd-pwm", sizeof(card->shortname));
	strlcpy(card->longname, "picocalc PWM audio", sizeof(card->longname));

	ret = snd_device_new(card, SNDRV_DEV_LOWLEVEL, picocalc, &ops);
	if (ret < 0)
		goto snd_error;

	ret = snd_pcm_new(card, card->driver, 0, 1, 0, &pcm);
	if (ret < 0)
		goto snd_error;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &picocalc_pcm_ops);
	snd_pcm_chip(pcm) = picocalc;
	pcm->info_flags = 0;
	snprintf(pcm->name, sizeof(pcm->name), "%s DAC", card->shortname);

	snd_pcm_lib_preallocate_pages_for_all(pcm,
				SNDRV_DMA_TYPE_CONTINUOUS,
				&picocalc->pdev->dev,
				8 * 1024, 8 * 1024);
	/*
	if (ret < 0)
		goto snd_error;
	*/

	ret = snd_card_register(card);
	if (ret == 0)
		return 0;

snd_error:
	snd_card_free(card);
	picocalc->snd_card = NULL;
	return ret;
}

static int picocalc_request_one_gpio(struct picocalc_snd_dev *picocalc,
                                    const char *name, int index,
                                    struct gpio_desc **gpiop)
{
    struct device *dev = &picocalc->pdev->dev;
    struct device_node *np = dev->of_node;
    int gpio, flags, rc = 0;
    enum of_gpio_flags of_flags;

    if (of_find_property(np, name, NULL)) {
        gpio = of_get_named_gpio_flags(np, name, index, &of_flags);
        if (gpio == -ENOENT)
            return 0;
        if (gpio == -EPROBE_DEFER)
            return gpio;
        if (gpio < 0) {
            dev_err(dev,
                    "failed to get '%s' from DT\n", name);
            return gpio;
        }

        flags = (of_flags & OF_GPIO_ACTIVE_LOW) ? GPIOF_OUT_INIT_LOW :
                GPIOF_OUT_INIT_HIGH;
        rc = devm_gpio_request_one(dev, gpio, flags,
                                   dev->driver->name);
        if (rc) {
            dev_err(dev,
                    "gpio_request_one('%s'=%d) failed with %d\n",
                    name, gpio, rc);
            return rc;
        }
        if (gpiop)
            *gpiop = gpio_to_desc(gpio);
    }

    return rc;
}

static int picocalc_probe(struct platform_device *pdev)
{
	struct picocalc_snd_dev *picocalc;
	int ret;
    //int i;

	printk("picocalc_probe snd\n");
	picocalc = devm_kzalloc(&pdev->dev, sizeof(*picocalc), GFP_KERNEL);
	if (picocalc == NULL)
		return -ENOMEM;

	picocalc->pdev = pdev;
	spin_lock_init(&picocalc->lock);

	ret = picocalc_request_one_gpio(picocalc, "leftpin", 0, &picocalc->left_gpio);
    if (ret) {
        dev_err(&picocalc->pdev->dev, "Request gpios failed!\n");
        return ret;
    }

    /*
    for(i = 0; i < 8000; i++)
    {
        gpiod_set_raw_value(picocalc->left_gpio, 1);
        ndelay(62500);
        gpiod_set_raw_value(picocalc->left_gpio, 0);
        ndelay(62500);
    }
    */

    picocalc->duty_cycle_ns = DEFAULT_DUTY_CYCLE;
    picocalc->period_ns = DEFAULT_PERIOD;

    if (0 != picocalc_pwm_enable(picocalc))
    {
	ret = -EIO;
        dev_err(&picocalc->pdev->dev, "picocalc_pwm_enable failed!\n");

    }
    
	platform_set_drvdata(pdev, picocalc);
	//return 0;

	ret = picocalc_snd_register(picocalc);
	if (ret)
	{
        dev_err(&picocalc->pdev->dev, "picocalc_snd_register failed!\n");
		return ret;
	}

	platform_set_drvdata(pdev, picocalc);
	return 0;
}

static int picocalc_remove(struct platform_device *pdev)
{
	struct picocalc_snd_dev *picocalc = platform_get_drvdata(pdev);

    	picocalc_pwm_disable(picocalc);

	snd_card_free(picocalc->snd_card);
	picocalc->snd_card = NULL;
	return 0;
}

static struct platform_driver picocalc_snd_pwm_driver = {
	.driver		= {
		.name	= "picocalc_snd",
		.of_match_table = picocalc_snd_pwm_dt_ids,
	},
	.probe		= picocalc_probe,
	.remove		= picocalc_remove,
};

module_platform_driver(picocalc_snd_pwm_driver);
