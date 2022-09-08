/*
 *  DSP standby support for Allwinner SoCs
 *
 *  Copyright (C) 2021 Allwinner Ltd.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rpmsg.h>
#include <linux/delay.h>

#include "linux/types.h"

#define  PM_DSP_POWER_SUSPEND 0xf3f30102
#define  PM_DSP_POWER_RESUME  0xf3f30201

#define  PM_DSP_WAITI_STATUS_REG  (0x01700000+0x10)
#define  PM_DSP_WAITI_STATUS_BIT  (0x1<<5)

#define  SUSPEND_TIMEOUT 10
#define  RESUME_TIMEOUT  10

struct sunxi_dsp_power {
	struct completion	complete;
	wait_queue_head_t	wait;
	/* get dsp status*/
	void __iomem  *addr;
};

static struct sunxi_dsp_power *dsp_power;

static int rpmsg_dsp_power_cb(struct rpmsg_device *rpdev, void *data, int len,
			      void *priv, u32 src)
{
	u32 tmp = *(u32 *)data;

	dev_info(&rpdev->dev, "rev: 0x%08x\n", tmp);

	switch (tmp) {
	case PM_DSP_POWER_SUSPEND:
	case PM_DSP_POWER_RESUME:
		/*wakeup standby task*/
		complete(&dsp_power->complete);
		break;
	default:
		break;
	}

	return 0;
}

static int rpmsg_dsp_power_probe(struct rpmsg_device *rpdev)
{
	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n", rpdev->src,
		 rpdev->dst);

	dsp_power = devm_kmalloc(&rpdev->dev, sizeof(struct sunxi_dsp_power), GFP_KERNEL);
	if (NULL == dsp_power) {
		dev_err(&rpdev->dev, "kmalloc failed.\n");
		return -ENOMEM;
	}

	memset(dsp_power, 0, sizeof(struct sunxi_dsp_power));

	dsp_power->addr = ioremap(PM_DSP_WAITI_STATUS_REG, 4);
	if (!dsp_power->addr) {
		dev_err(&rpdev->dev, "ioremap failed.\n");
		return -EPERM;
	}

	init_completion(&dsp_power->complete);
	init_waitqueue_head(&dsp_power->wait);

	dev_set_drvdata(&rpdev->dev, rpdev);

	return 0;
}

static void rpmsg_dsp_power_remove(struct rpmsg_device *rpdev)
{
	dev_info(&rpdev->dev, "rpmsg dsp_power client driver is removed\n");

	iounmap(dsp_power->addr);
	reinit_completion(&dsp_power->complete);
}

#ifdef CONFIG_PM_SLEEP
static int rpmsg_dsp_power_suspend(struct device *dev)
{
	u32 data = PM_DSP_POWER_SUSPEND;
	struct rpmsg_device *rd = dev_get_drvdata(dev);

	if (!rd) {
		dev_err(dev, "%s: get drvdata failed.\n", __func__);
		return -EINVAL;
	}

	rpmsg_send(rd->ept, &data, sizeof(data));

	/*we must wait dsp return msg.*/
	if (!wait_for_completion_timeout(&dsp_power->complete, SUSPEND_TIMEOUT)) {
		dev_err(dev, "timout wait dsp return msg.\n");
		return -EBUSY;
	}

	return 0;
}

static int rpmsg_dsp_power_resume(struct device *dev)
{
	u32 data = PM_DSP_POWER_RESUME;
	struct rpmsg_device *rd = dev_get_drvdata(dev);

	if (!rd) {
		dev_err(dev, "%s: get drvdata failed.\n", __func__);
		return -EINVAL;
	}

	rpmsg_send(rd->ept, &data, sizeof(data));

	/*we must wait dsp return msg.*/
	if (!wait_for_completion_timeout(&dsp_power->complete, RESUME_TIMEOUT)) {
		dev_err(dev, "timout wait dsp return msg.\n");
		return -EBUSY;
	}

	return 0;
}

/*
 * check dsp has been enter waiti mode.
 */
static int check_dsp_mode(void *p)
{
	u32 val;

	val = readl(dsp_power->addr);

	return !!(val & PM_DSP_WAITI_STATUS_BIT);
}


static int rpmsg_dsp_power_suspend_noirq(struct device *dev)
{
	if (!wait_event_timeout(dsp_power->wait, check_dsp_mode(dev), SUSPEND_TIMEOUT))
		return -EBUSY;

	return 0;
}


static struct dev_pm_ops rpmsg_dsp_power_pm_ops = {
	.suspend = rpmsg_dsp_power_suspend,
	.suspend_noirq = rpmsg_dsp_power_suspend_noirq,
	.resume  = rpmsg_dsp_power_resume,
};
#endif

static struct rpmsg_device_id rpmsg_driver_dsp_power_id_table[] = {
	{ .name = "sunxi,dsp-power-msgbox" },
	{},
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_dsp_power_id_table);

static struct rpmsg_driver rpmsg_dsp_power_client = {
	.drv = {
		.name = KBUILD_MODNAME,
#ifdef CONFIG_PM_SLEEP
		.pm = &rpmsg_dsp_power_pm_ops,
#endif
	},
	.id_table = rpmsg_driver_dsp_power_id_table,
	.probe = rpmsg_dsp_power_probe,
	.callback = rpmsg_dsp_power_cb,
	.remove = rpmsg_dsp_power_remove,
};
module_rpmsg_driver(rpmsg_dsp_power_client);

MODULE_DESCRIPTION("Remote processor messaging dsp_power client driver");
MODULE_LICENSE("GPL v2");
