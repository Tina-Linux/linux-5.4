/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, allwinnertech.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "sunxi_arisc_rpm.h"

#define MSG_BLOCK_CACHED_SIZE (32)

enum rpm_received_state {
	RPM_RECEIVED_IDLE,
	RPM_RECEIVED_HEADER,
	RPM_RECEIVED_PAYLOAD,
};

struct rpm_msg_block {
	struct list_head link;
	uint8_t type;
	uint8_t length;
	uint8_t payload[MAX_PAYLOAD_SIZE];
};

struct rpm_received_info {
	int state;
	int received_length;
	struct rpm_msg_block *msg_block;
};

struct sunxi_rpm_private {
	struct mutex mutex;
	struct rpmsg_device *sunxi_arisc_rpmsg_device;

	uint8_t seqnumber;
	sunxi_arisc_message_cb received_cb;
	struct rpm_received_info received_info;

	spinlock_t list_lock;
	struct list_head free_list_head;
	struct list_head ready_list;

	struct work_struct received_notify_work;
};

struct sunxi_rpm_private *rpm_private;

static void message_block_init(struct sunxi_rpm_private *rpm)
{
	int i = 0;
	struct rpm_msg_block *block;

	INIT_LIST_HEAD(&rpm->free_list_head);
	INIT_LIST_HEAD(&rpm->ready_list);

	while (i < MSG_BLOCK_CACHED_SIZE) {
		block = kmalloc(sizeof(struct rpm_msg_block), GFP_KERNEL | __GFP_ZERO);
		if (IS_ERR_OR_NULL(block)) {
			pr_err("kmalloc rpm_msg_block failed!\n");
			return;
		}
		list_add(&block->link, &rpm->free_list_head);
		i++;
	}
}

static void message_block_destroy(struct sunxi_rpm_private *rpm)
{
	struct rpm_msg_block *block, *next;

	list_for_each_entry_safe(block, next, &rpm_private->free_list_head, link) {
		list_del(&block->link);
		kfree(block);
	}
}

static inline struct rpm_msg_block *alloc_msg_block(void)
{
	unsigned long flags;
	struct rpm_msg_block *block = NULL;

	spin_lock_irqsave(&rpm_private->list_lock, flags);
	if (!list_empty(&rpm_private->free_list_head)) {
		block = list_first_entry(&rpm_private->free_list_head, struct rpm_msg_block, link);
		list_del_init(&block->link);
	}
	spin_unlock_irqrestore(&rpm_private->list_lock, flags);

	return block;
}

static inline void free_msg_block(struct rpm_msg_block *block)
{
	unsigned long flags;
	spin_lock_irqsave(&rpm_private->list_lock, flags);
	list_add(&block->link, &rpm_private->free_list_head);
	spin_unlock_irqrestore(&rpm_private->list_lock, flags);
}

void sunxi_arisc_rpm_register_message_callback(sunxi_arisc_message_cb cb)
{
	mutex_lock(&rpm_private->mutex);
	rpm_private->received_cb = cb;
	mutex_unlock(&rpm_private->mutex);
}
EXPORT_SYMBOL(sunxi_arisc_rpm_register_message_callback);

int sunxi_arisc_rpm_send(uint8_t type, uint8_t length, const uint8_t *pdata)
{
	int ret, i = 0;
	struct message_amp *msg;
	int size = sizeof(struct message_amp) + length;

	msg = kmalloc(size, GFP_KERNEL | __GFP_ZERO);
	if (IS_ERR_OR_NULL(msg)) {
		return -ENOMEM;
	}
	msg->magic = MESSAGE_AMP_MAGIC;
	msg->type  = type;
	msg->count = length;

	while (i < length) {
		msg->paras[i] = pdata[i];
		i++;
	}

	mutex_lock(&rpm_private->mutex);
	msg->seqnumber = rpm_private->seqnumber++;
	ret = rpmsg_send(rpm_private->sunxi_arisc_rpmsg_device->ept, msg, size);
	mutex_unlock(&rpm_private->mutex);

	return ret;
}
EXPORT_SYMBOL(sunxi_arisc_rpm_send);


static void notify_msg(struct rpm_msg_block *block)
{
	unsigned long flags;

	spin_lock_irqsave(&rpm_private->list_lock, flags);
	list_add_tail(&block->link, &rpm_private->ready_list);
	spin_unlock_irqrestore(&rpm_private->list_lock, flags);

	schedule_work(&rpm_private->received_notify_work);
}

/* call from interrupt context!!! */
static int sunxi_arisc_rpm_cb(struct rpmsg_device *rpdev, void *data, int len,
		void *priv, u32 src)
{
	struct rpm_received_info *info;
	uint32_t rx, i;
	uint8_t magic;

	if (len != 4) {
		dev_err(&rpdev->dev, "invalid rpm length from endpoint addr: 0x%08x\n", rpdev->ept->addr);
		return -1;
	}

	rx = *(uint32_t *)data;
	info = &rpm_private->received_info;

	switch (info->state) {
	case RPM_RECEIVED_IDLE:
		magic = rx & 0xff;
		if (magic == MESSAGE_AMP_MAGIC) {
			info->msg_block = alloc_msg_block();
			if (IS_ERR_OR_NULL(info->msg_block)) {
				pr_err("alloc rpm msg block failed!\n");
				break;
			}
			info->received_length = 0;
			info->state = RPM_RECEIVED_HEADER;

			info->msg_block->type	= (uint8_t)((rx >> 16) & 0xff);
			info->msg_block->length = (uint8_t)((rx >> 24) & 0xff);
		}
		pr_debug("rpm header: %08x\n", rx);
		break;
	case RPM_RECEIVED_HEADER:
		if (info->msg_block->length == 0) {
			// zero length payload message.
			notify_msg(info->msg_block);
			info->msg_block = NULL;
			info->state = RPM_RECEIVED_IDLE;
		} else
			info->state = RPM_RECEIVED_PAYLOAD;

		break;
	case RPM_RECEIVED_PAYLOAD:
		i = 0;
		while (i < 4) {
			info->msg_block->payload[info->received_length++] = (rx >> (i * 8)) & 0xFF;
			if (info->received_length >= info->msg_block->length)
				break;
			i++;
		}
		if (info->received_length == info->msg_block->length) {
			notify_msg(info->msg_block);
			info->msg_block = NULL;
			info->state = RPM_RECEIVED_IDLE;
		}
		break;
	default:
		break;
	}

	return 0;
}

static void sunxi_arisc_rpm_received_worker(struct work_struct *work)
{
	unsigned long flags;
	struct list_head tmp;
	struct rpm_msg_block *block, *next;

	spin_lock_irqsave(&rpm_private->list_lock, flags);
	list_replace_init(&rpm_private->ready_list, &tmp);
	spin_unlock_irqrestore(&rpm_private->list_lock, flags);

	list_for_each_entry_safe(block, next, &tmp, link) {
		list_del(&block->link);
		if (rpm_private->received_cb) {
			rpm_private->received_cb(block->type, block->length, block->payload);
		}
		print_hex_dump_debug("	", DUMP_PREFIX_OFFSET, 16, 1, block->payload, block->length, false);
		free_msg_block(block);
	}
}

static int sunxi_arisc_rpm_probe(struct rpmsg_device *rpdev)
{
	dev_info(&rpdev->dev, "create rpmsg channel: 0x%x -> 0x%x!\n",
			rpdev->src, rpdev->dst);

	rpm_private = kmalloc(sizeof(struct sunxi_rpm_private), GFP_KERNEL | __GFP_ZERO);
	if (IS_ERR_OR_NULL(rpm_private)) {
		return -ENOMEM;
	}

	rpm_private->sunxi_arisc_rpmsg_device = rpdev;
	rpm_private->seqnumber = 0;
	rpm_private->received_cb = NULL;

	rpm_private->received_info.state = RPM_RECEIVED_IDLE;
	rpm_private->received_info.received_length = 0;
	rpm_private->received_info.msg_block = NULL;

	message_block_init(rpm_private);

	INIT_WORK(&rpm_private->received_notify_work, sunxi_arisc_rpm_received_worker);

	spin_lock_init(&rpm_private->list_lock);
	mutex_init(&rpm_private->mutex);

	return 0;
}

static void sunxi_arisc_rpm_remove(struct rpmsg_device *rpdev)
{
	if (rpm_private) {
		flush_work(&rpm_private->received_notify_work);
		cancel_work_sync(&rpm_private->received_notify_work);

		message_block_destroy(rpm_private);
		kfree(rpm_private);
	}

	dev_info(&rpdev->dev, "rpmsg sample client driver is removed\n");
}

static struct rpmsg_device_id rpmsg_driver_id_table[] = {
	{ .name = "sunxi,cpus-msgbox" },
	{ },
};

MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_id_table);

static struct rpmsg_driver sunxi_arisc_rpm_client = {
	.drv.name = KBUILD_MODNAME,
	.id_table = rpmsg_driver_id_table,
	.probe	  = sunxi_arisc_rpm_probe,
	.callback = sunxi_arisc_rpm_cb,
	.remove   = sunxi_arisc_rpm_remove,
};

module_rpmsg_driver(sunxi_arisc_rpm_client);
MODULE_DESCRIPTION("sunxi arisc remote processor messaging client driver");
MODULE_LICENSE("GPL v2");

