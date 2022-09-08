/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2020, allwinnertech.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/workqueue.h>
#include <linux/types.h>

#include "sunxi_cpu_comm.h"

#define CPU_COMM_MSG_LEN 4

static struct rpmsg_device *cpu_comm_rpmsg_device;
static cpu_comm_cb msgbox_received_cb;

void sunxi_cpu_comm_register_isr_cb(cpu_comm_cb cb)
{
	msgbox_received_cb = cb;
}
EXPORT_SYMBOL(sunxi_cpu_comm_register_isr_cb);

int sunxi_cpu_comm_send_intr_to_mips(communication_type type, msgbox_cpu_type target, int ack)
{
	uint32_t data = type;

	if (target != CPU_MIPS)
		return -EINVAL;

	return rpmsg_send(cpu_comm_rpmsg_device->ept, &data, CPU_COMM_MSG_LEN);
}
EXPORT_SYMBOL(sunxi_cpu_comm_send_intr_to_mips);

static int rpmsg_cpu_comm_cb(struct rpmsg_device *rpdev, void *data, int len,
		void *priv, u32 src)
{
	uint32_t comm_type = 0;

	if (len == CPU_COMM_MSG_LEN) {
		comm_type = *(uint32_t *)data;
		if (msgbox_received_cb != NULL)
			msgbox_received_cb(comm_type, CPU_MIPS);
	} else {
		dev_err(&rpdev->dev, "unknown message from addr: 0x%08x\n", rpdev->ept->addr);
		print_hex_dump_debug(__func__, DUMP_PREFIX_NONE, 16, 1, data, len,
				true);
	}
	return 0;
}

static int rpmsg_cpu_comm_probe(struct rpmsg_device *rpdev)
{
	cpu_comm_rpmsg_device = rpdev;
	dev_info(&rpdev->dev, "create rpmsg channel: 0x%x -> 0x%x!\n",
			rpdev->src, rpdev->dst);
	return 0;
}

static void rpmsg_cpu_comm_remove(struct rpmsg_device *rpdev)
{
	cpu_comm_rpmsg_device = NULL;
	msgbox_received_cb = NULL;
	dev_info(&rpdev->dev, "rpmsg sample client driver is removed\n");
}

static struct rpmsg_device_id rpmsg_driver_id_table[] = {
	{ .name = "sunxi,mips-msgbox" },
	{ },
};

MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_id_table);

static struct rpmsg_driver rpmsg_cpu_comm_client = {
	.drv.name = KBUILD_MODNAME,
	.id_table = rpmsg_driver_id_table,
	.probe	  = rpmsg_cpu_comm_probe,
	.callback = rpmsg_cpu_comm_cb,
	.remove   = rpmsg_cpu_comm_remove,
};

module_rpmsg_driver(rpmsg_cpu_comm_client);
MODULE_DESCRIPTION("sunxi remote processor messaging client driver");
MODULE_LICENSE("GPL v2");

