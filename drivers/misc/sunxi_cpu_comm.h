/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, allwinnertech.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _SUNXI_CPU_COMM_H_
#define _SUNXI_CPU_COMM_H_

typedef enum _tag_communication_type {
	SUNXI_TYPE_CALL,
	SUNXI_TYPE_RETURN,
	SUNXI_TYPE_CALL_ACK,
	SUNXI_TYPE_RETURN_ACK,
	SUNXI_TYPE_COMM_ALL,
} communication_type;

typedef enum _tag_msgbox_cpu_type {
	CPU_ARM  = 0,
	CPU_RISC = 1,
	CPU_MIPS = 2
} msgbox_cpu_type;

typedef void (*cpu_comm_cb)(communication_type, msgbox_cpu_type);

void sunxi_cpu_comm_register_isr_cb(cpu_comm_cb cb);
int sunxi_cpu_comm_send_intr_to_mips(communication_type type, msgbox_cpu_type target, int ack);

#endif
