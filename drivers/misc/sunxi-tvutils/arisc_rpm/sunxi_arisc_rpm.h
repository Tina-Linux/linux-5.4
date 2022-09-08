/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, allwinnertech.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _SUNXI_ARISC_RPM_H_
#define _SUNXI_ARISC_RPM_H_

#define MAX_PAYLOAD_SIZE 128
#define MESSAGE_AMP_MAGIC 0xA5

#define MESSAGE_TYPE_BASE 0x00
#define MESSAGE_TYPE_TEST (MESSAGE_TYPE_BASE + 0x01)

typedef struct message_amp {
	unsigned char magic;		/* identify the begin of message frame */
	unsigned char seqnumber;
	unsigned char type;			/* message type define by user */
	unsigned char count;		/* message paras count unit of byte */
	unsigned char reserved[4];
	unsigned char paras[0];		/* the point of message parameters */
} message_amp_t;

typedef void (*sunxi_arisc_message_cb)(uint8_t, uint8_t, uint8_t *);

int sunxi_arisc_rpm_send(uint8_t type, uint8_t length, const uint8_t *pdata);
void sunxi_arisc_rpm_register_message_callback(sunxi_arisc_message_cb cb);

#endif
