/*
 * Based on drivers/input/keyboard/sunxi-keyboard.c
 *
 * Copyright (C) 2015 Allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//#define DEBUG
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/reset.h>

#if IS_ENABLED(CONFIG_PM)
#include <linux/pm.h>
#endif
#include "sunxi-keyboard.h"

#define	LRADC_BITS		6
#define LRADC_RESOLUTION	(1 << LRADC_BITS)

static unsigned char keypad_mapindex[LRADC_RESOLUTION];

#define INITIAL_VALUE (0xff)
#define VOL_NUM KEY_MAX_CNT

static u32 sunxi_keyboard_regs_offset[] = {
	LRADC_CTRL,
	LRADC_INTC,
};

struct sunxi_key_data {
	struct platform_device	*pdev;
	struct clk *mclk;
	struct clk *pclk;
	struct reset_control	*rst_clk;
	struct input_dev *input_dev;
	struct sunxi_adc_disc *disc;
	spinlock_t		lock; /* syn */
	void __iomem *reg_base;
	u32 scankeycodes[KEY_MAX_CNT];
	int irq_num;
	u32 key_val;
	unsigned char compare_later;
	unsigned char compare_before;
	u8 key_code;
	u8 last_key_code;
	char key_name[16];
	u8 key_cnt;
	int wakeup;
	u32 regs_backup[ARRAY_SIZE(sunxi_keyboard_regs_offset)];
};

static struct sunxi_adc_disc disc_1350 = {
	.measure = 1350,
	.resol = 21,
};

static struct sunxi_adc_disc disc_1200 = {
	.measure = 1200,
	.resol = 19,
};

static struct sunxi_adc_disc disc_2000 = {
	.measure = 2000,
	.resol = 31,
};

/*
 * Translate OpenFirmware node properties into platform_data
 */
static struct of_device_id const sunxi_keyboard_of_match[] = {
	{ .compatible = "allwinner,keyboard_1350mv",
		.data = &disc_1350 },
	{ .compatible = "allwinner,keyboard_1200mv",
		.data = &disc_1200 },
	{ .compatible = "allwinner,keyboard_2000mv",
		.data = &disc_2000 },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_keyboard_of_match);

static inline void sunxi_keyboard_save_regs(struct sunxi_key_data *key_data)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_keyboard_regs_offset); i++)
		key_data->regs_backup[i] = readl(key_data->reg_base + sunxi_keyboard_regs_offset[i]);
}

static inline void sunxi_keyboard_restore_regs(struct sunxi_key_data *key_data)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_keyboard_regs_offset); i++)
		writel(key_data->regs_backup[i], key_data->reg_base + sunxi_keyboard_regs_offset[i]);
}

#if IS_ENABLED(CONFIG_PM)
static int sunxi_keyboard_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_key_data *key_data = platform_get_drvdata(pdev);

	pr_debug("[%s] enter standby\n", __func__);

	/* Used to determine whether the device can be wakeup, and use
	 * this function */
	if (device_may_wakeup(dev)) {
		if (key_data->wakeup)
			enable_irq_wake(key_data->irq_num);
	} else {
		disable_irq_nosync(key_data->irq_num);

		sunxi_keyboard_save_regs(key_data);

		clk_disable_unprepare(key_data->mclk);

		reset_control_assert(key_data->rst_clk);
	}

	return 0;
}

static int sunxi_keyboard_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_key_data *key_data = platform_get_drvdata(pdev);

	pr_debug("[%s] return from standby\n", __func__);

	/* Used to determine whether the device can be wakeup, and use
	 * this function */
	if (device_may_wakeup(dev)) {
		if (key_data->wakeup)
			disable_irq_wake(key_data->irq_num);
	} else {
		reset_control_deassert(key_data->rst_clk);

		clk_prepare_enable(key_data->mclk);

		sunxi_keyboard_restore_regs(key_data);

		enable_irq(key_data->irq_num);
	}

	return 0;
}
#endif

static void sunxi_report_key_down_event(struct sunxi_key_data *key_data)
{
	if (key_data->last_key_code == INITIAL_VALUE) {
		/* first time report key down event */
		input_report_key(key_data->input_dev,
				key_data->scankeycodes[key_data->key_code], 1);
		input_sync(key_data->input_dev);
		key_data->last_key_code = key_data->key_code;
		return;
	}
	if (key_data->scankeycodes[key_data->key_code] ==
		key_data->scankeycodes[key_data->last_key_code]) {
#ifdef REPORT_REPEAT_KEY_BY_INPUT_CORE
		/* report repeat key down event */
		input_report_key(key_data->input_dev,
				key_data->scankeycodes[key_data->key_code], 1);
		input_sync(key_data->input_dev);
#endif
	} else {
		/* report previous key up event
		 * and report current key down event
		 */
		input_report_key(key_data->input_dev,
				key_data->scankeycodes[key_data->last_key_code], 0);
		input_sync(key_data->input_dev);
		input_report_key(key_data->input_dev,
				key_data->scankeycodes[key_data->key_code], 1);
		input_sync(key_data->input_dev);
		key_data->last_key_code = key_data->key_code;
	}
}

static irqreturn_t sunxi_isr_key(int irq, void *dummy)
{
	struct sunxi_key_data *key_data = (struct sunxi_key_data *)dummy;
	u32 reg_val = 0;
	u32 key_val = 0;

	pr_debug("Key Interrupt\n");

	/* Clear interrupt register */
	reg_val = readl(key_data->reg_base + LRADC_INT_STA);
	writel(reg_val, key_data->reg_base + LRADC_INT_STA);

	if (reg_val & LRADC_ADC0_DOWNPEND)
		pr_debug("key down\n");

	if (reg_val & LRADC_ADC0_DATAPEND) {
		key_data->key_cnt++;
		key_val = readl(key_data->reg_base + LRADC_DATA0);
		key_data->compare_before = key_val & 0x3f;
		if (key_data->compare_before == key_data->compare_later) {
			key_data->key_code = keypad_mapindex[key_val & 0x3f];
			sunxi_report_key_down_event(key_data);
			key_data->key_cnt = 0;
		}
		if (key_data->key_cnt == 2) {
			key_data->compare_later = key_data->compare_before;
			key_data->key_cnt = 0;
		}
	}

	if (reg_val & LRADC_ADC0_UPPEND) {
		if (key_data->wakeup)
			pm_wakeup_event(key_data->input_dev->dev.parent, 0);
		pr_debug("report data:%8d key_val:%8d\n",
				key_data->scankeycodes[key_data->key_code],
				key_val);
		input_report_key(key_data->input_dev,
				key_data->scankeycodes[key_data->key_code], 0);
		input_sync(key_data->input_dev);
		pr_debug("key up\n");
		key_data->key_cnt = 0;
		key_data->compare_later = 0;
		key_data->last_key_code = INITIAL_VALUE;
	}

	return IRQ_HANDLED;
}

static int sunxi_keyboard_startup(struct sunxi_key_data *key_data,
				struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct device *dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Fail to get IORESOURCE_MEM\n");
		return -EINVAL;
	}

	key_data->reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(key_data->reg_base)) {
		dev_err(dev, "Fail to map IO resource\n");
		return PTR_ERR(key_data->reg_base);
	}

	key_data->irq_num = platform_get_irq(pdev, 0);
	if (key_data->irq_num < 0) {
		dev_err(dev, "No IRQ resource\n");
		return -EINVAL;
	}

	/* some IC will use clock gating while others HW use 24MHZ, So just try
	 * to get the clock, if it doesn't exist, give warning instead of error
	 */
	key_data->rst_clk = devm_reset_control_get(dev, NULL);
	if (IS_ERR(key_data->rst_clk)) {
		dev_err(dev, "reset_control_get() failed\n");
		goto err0;
	}

	if (reset_control_deassert(key_data->rst_clk)) {
		dev_err(dev, "enable apb1_keyadc clock failed!\n");
		goto err0;
	}

	key_data->mclk = devm_clk_get(dev, NULL);
	if (IS_ERR(key_data->mclk)) {
		dev_err(dev, "Failed to get clock 'mclk'\n");
		return PTR_ERR(key_data->mclk);
	}

	ret = clk_prepare_enable(key_data->mclk);
	if (ret) {
		dev_err(dev, "cannot enable clock 'mclk'");
		goto err1;
	}

	return ret;

err1:
	reset_control_assert(key_data->rst_clk);
err0:
	return ret;

}

static int sunxikbd_key_init(struct sunxi_key_data *key_data,
			struct platform_device *pdev)
{
	struct device_node *np;
	const struct of_device_id *match;
	struct sunxi_adc_disc *disc;
	int i;
	int j = 0;
	u32 val[2] = {0, 0};
	u32 key_num ;
	u32 key_vol[VOL_NUM];

	np = pdev->dev.of_node;
	match = of_match_node(sunxi_keyboard_of_match, np);
	disc = (struct sunxi_adc_disc *)match->data;
	key_data->disc = disc;
	if (of_property_read_u32(np, "key_cnt", &key_num)) {
		pr_err("%s: get key count failed", __func__);
		return -EBUSY;
	}
	pr_debug("%s key number = %d.\n", __func__, key_num);
	if (key_num < 1 || key_num >= VOL_NUM) {
		pr_err("incorrect key number.\n");
		return -1;
	}
	for (i = 0; i < key_num; i++) {
		sprintf(key_data->key_name, "key%d", i);
		if (of_property_read_u32_array(np, key_data->key_name,
						val, ARRAY_SIZE(val))) {
			pr_err("%s:get%s err!\n", __func__, key_data->key_name);
			return -EBUSY;
		}
		key_vol[i] = val[0];
		key_data->scankeycodes[i] = val[1];
		pr_debug("%s: key%d vol= %d code= %d\n", __func__, i,
				key_vol[i], key_data->scankeycodes[i]);
	}

	/* set the key judgment threshold */
	key_vol[key_num] = disc->measure;
	for (i = 0; i < key_num; i++)
		key_vol[i] += (key_vol[i+1] - key_vol[i])/2;

	for (i = 0; i < 64; i++) {
		if (i * disc->resol > key_vol[j])
			j++;
		keypad_mapindex[i] = j;
	}

	key_data->wakeup = of_property_read_bool(np, "wakeup-source");
	device_init_wakeup(&pdev->dev, key_data->wakeup);

	key_data->last_key_code = INITIAL_VALUE;

	return 0;
}

#if IS_ENABLED(CONFIG_IIO)
struct sunxi_lradc_iio {
	struct sunxi_key_data *key_data;
};

static const struct iio_chan_spec sunxi_lradc_channels[] = {
	{
		.indexed = 1,
		.type = IIO_VOLTAGE,
		.channel = 0,
		.datasheet_name = "LRADC",
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
};

/* default maps used by iio consumer (axp charger driver) */
static struct iio_map sunxi_lradc_default_iio_maps[] = {
	{
		.consumer_dev_name = "axp-charger",
		.consumer_channel = "axp-battery-lradc",
		.adc_channel_label = "LRADC",
	},
	{ }
};

static int sunxi_lradc_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan,
			int *val, int *val2, long mask)
{
	int ret = 0;
	int key_val, id_vol;
	struct sunxi_lradc_iio *info = iio_priv(indio_dev);
	struct sunxi_key_data *key_data = info->key_data;
	struct sunxi_adc_disc *disc = key_data->disc;

	mutex_lock(&indio_dev->mlock);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		key_val = readl(key_data->reg_base + LRADC_DATA0) & 0x3f;
		id_vol = key_val * disc->resol;
		*val = id_vol;
		break;
	default:
		ret = -EINVAL;
	}
	mutex_unlock(&indio_dev->mlock);

	return ret;
}

static const struct iio_info sunxi_lradc_iio_info = {
	.read_raw = &sunxi_lradc_read_raw,
};

static void sunxi_lradc_remove_iio(void *_data)
{
	struct iio_dev *indio_dev = _data;

	if (IS_ERR_OR_NULL(indio_dev)) {
		pr_err("indio_dev is null\n");
	} else {
		iio_device_unregister(indio_dev);
		iio_map_array_unregister(indio_dev);
	}
}

static int sunxi_keyboard_iio_init(struct platform_device *pdev)
{
	int ret;
	struct iio_dev *indio_dev;
	struct sunxi_lradc_iio *info;
	struct sunxi_key_data *key_data = platform_get_drvdata(pdev);

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*info));
	if (!indio_dev)
		return -ENOMEM;

	info = iio_priv(indio_dev);
	info->key_data = key_data;

	indio_dev->dev.parent = &pdev->dev;
	indio_dev->name = pdev->name;
	indio_dev->channels = sunxi_lradc_channels;
	indio_dev->num_channels = ARRAY_SIZE(sunxi_lradc_channels);
	indio_dev->info = &sunxi_lradc_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	ret = iio_map_array_register(indio_dev, sunxi_lradc_default_iio_maps);
	if (ret < 0)
		return ret;

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "unable to register iio device\n");
		goto err_array_unregister;
	}

	ret = devm_add_action(&pdev->dev,
				sunxi_lradc_remove_iio, indio_dev);
	if (ret) {
		dev_err(&pdev->dev, "unable to add iio cleanup action\n");
		goto err_iio_unregister;
	}

	return 0;

err_iio_unregister:
	iio_device_unregister(indio_dev);

err_array_unregister:
	iio_map_array_unregister(indio_dev);

	return ret;
}
#else
static inline int sunxi_keyboard_iio_init(struct platform_device *pdev)
{
	return -ENODEV;
}
#endif

static int sunxi_keyboard_probe(struct platform_device *pdev)
{
	static struct input_dev *sunxikbd_dev;
	struct sunxi_key_data *key_data;
	unsigned long mask, para;
	u32 reg_val;
	int i;
	int err;

	key_data = kzalloc(sizeof(*key_data), GFP_KERNEL);
	if (!key_data) {
		pr_err("key_data: not enough memory for key data\n");
		return -ENOMEM;
	}

	pr_debug("sunxikbd_init\n");

	err = sunxi_keyboard_startup(key_data, pdev);
	if (err < 0)
		goto fail1;

	if (sunxikbd_key_init(key_data, pdev)) {
		err = -EFAULT;
		goto fail2;
	}

	sunxikbd_dev = input_allocate_device();
	if (!sunxikbd_dev) {
		pr_err("sunxikbd: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail3;
	}

	sunxikbd_dev->name = INPUT_DEV_NAME;
	sunxikbd_dev->phys = "sunxikbd/input0";
	sunxikbd_dev->id.bustype = BUS_HOST;
	sunxikbd_dev->id.vendor = 0x0001;
	sunxikbd_dev->id.product = 0x0001;
	sunxikbd_dev->id.version = 0x0100;

#ifdef REPORT_REPEAT_KEY_BY_INPUT_CORE
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY)|BIT_MASK(EV_REP);
	pr_info("support report repeat key value.\n");
#else
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY);
#endif

	for (i = 0; i < KEY_MAX_CNT; i++) {
		if (key_data->scankeycodes[i] < KEY_MAX)
			set_bit(key_data->scankeycodes[i], sunxikbd_dev->keybit);
	}
	key_data->input_dev = sunxikbd_dev;
	platform_set_drvdata(pdev, key_data);
#ifdef ONE_CHANNEL

	reg_val = readl(key_data->reg_base + LRADC_INTC);
	para = LRADC_ADC0_UP_EN | LRADC_ADC0_DOWN_EN | LRADC_ADC0_DATA_EN;
	mask = LRADC_ADC0_UP_EN_MASK | LRADC_ADC0_DOWN_EN_MASK | LRADC_ADC0_DATA_EN_MASK;
	reg_val &= ~mask;
	reg_val |= para;
	writel(reg_val, key_data->reg_base + LRADC_INTC);

	reg_val = readl(key_data->reg_base + LRADC_CTRL);
	para = FIRST_CONVERT_DLY | LEVELB_VOL | KEY_MODE_SELECT
		| LRADC_HOLD_EN | ADC_CHAN_SELECT | LEVELB_CNT
		| LRADC_SAMPLE_250HZ | LRADC_EN;
	mask = FIRST_CONVERT_DLY_MASK | LEVELB_VOL_MASK | KEY_MODE_SELECT_MASK
		| LRADC_HOLD_EN_MASK | LEVELB_CNT_MASK
		| LRADC_SAMPLE_250HZ_MASK | LRADC_EN_MASK;
	reg_val &= ~mask;
	reg_val |= para;
	writel(reg_val, key_data->reg_base + LRADC_CTRL);

#endif

	err = input_register_device(key_data->input_dev);
	if (err)
		goto fail4;

	if (request_irq(key_data->irq_num, sunxi_isr_key, 0,
					"sunxikbd", key_data)) {
		err = -EBUSY;
		pr_err("request irq failure.\n");
		goto fail5;
	}

	/* Clear interrupt register */
	reg_val = readl(key_data->reg_base + LRADC_INT_STA);
	writel(reg_val, key_data->reg_base + LRADC_INT_STA);

	pr_debug("sunxikbd_init end\n");
	return 0;
fail5:
	input_unregister_device(key_data->input_dev);
fail4:
	input_free_device(key_data->input_dev);
fail3:
	device_init_wakeup(&pdev->dev, 0);
fail2:
	clk_disable_unprepare(key_data->mclk);
	reset_control_assert(key_data->rst_clk);
fail1:
	kfree(key_data);
	pr_err("sunxikbd_init failed.\n");

	return err;
}

static int sunxi_keyboard_remove(struct platform_device *pdev)
{
	struct sunxi_key_data *key_data = platform_get_drvdata(pdev);

	free_irq(key_data->irq_num, key_data);
	input_unregister_device(key_data->input_dev);
	input_free_device(key_data->input_dev);
	device_init_wakeup(&pdev->dev, 0);
	clk_disable_unprepare(key_data->mclk);
	reset_control_assert(key_data->rst_clk);
	kfree(key_data);
	return 0;
}

#if IS_ENABLED(CONFIG_PM)
static const struct dev_pm_ops sunxi_keyboard_pm_ops = {
	.suspend = sunxi_keyboard_suspend,
	.resume = sunxi_keyboard_resume,
};

#define SUNXI_KEYBOARD_PM_OPS (&sunxi_keyboard_pm_ops)
#endif

static struct platform_driver sunxi_keyboard_driver = {
	.probe  = sunxi_keyboard_probe,
	.remove = sunxi_keyboard_remove,
	.driver = {
		.name   = "sunxi-keyboard",
		.owner  = THIS_MODULE,
#if IS_ENABLED(CONFIG_PM)
		.pm	= SUNXI_KEYBOARD_PM_OPS,
#endif
		.of_match_table = of_match_ptr(sunxi_keyboard_of_match),
	},
};

static int __init sunxi_keyboard_init(void)
{
	int ret;

	ret = platform_driver_register(&sunxi_keyboard_driver);

	return ret;
}

static void __exit sunxi_keyboard_exit(void)
{
	platform_driver_unregister(&sunxi_keyboard_driver);
}

subsys_initcall_sync(sunxi_keyboard_init);
module_exit(sunxi_keyboard_exit);

MODULE_AUTHOR(" Qin");
MODULE_DESCRIPTION("sunxi-keyboard driver");
MODULE_LICENSE("GPL");
