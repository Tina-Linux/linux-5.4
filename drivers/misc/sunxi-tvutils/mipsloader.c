/*
 * Copyright(c) 2017-2018 Allwinnertech Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <asm-generic/cacheflush.h>
#include <linux/device.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("allwinnertech.com");
MODULE_LICENSE("Dual BSD/GPL");

#define MIPS_MEMORY_OFFSET 0x44000000
#define MIPS_MEMORY_SIZE   0x01400000

#define MIPS_SRAM_MEMORY_OFFSET 0x104000
#define MIPS_SRAM_MEMORY_SIZE	0x001000

#define MIPS_LOADER_IOC_RW 0x0648
#define MIPS_FIRMWARE_R    0x01
#define MIPS_FIRMWARE_W    0x00

struct firmware_info {
	uint32_t type;
	uint32_t offset;
	uint32_t size;
	uint64_t buffer;
};

static void *mips_firmware_mem_va;
static void *mips_sram_mem_va;

static void *mips_memory_map(uint32_t offset, uint32_t size)
{
	void *va = NULL;

	pr_info("mipsloader: memremap offset 0x%08x size 0x%08x\n", offset, size);
	va = memremap(offset, size, MEMREMAP_WB);
	if (!va) {
		pr_info("unable to map memory region: 0x%08x+0x%08x\n", offset, size);
		return NULL;
	}
	pr_info("mipsloader: memremap pa 0x%08x --> %px\n", offset, va);
	memset(va, 0, size);
	flush_cache_all();
	return va;
}

static void mips_memory_unmap(void)
{
	if (mips_firmware_mem_va)
		memunmap(mips_firmware_mem_va);
	if (mips_sram_mem_va)
		memunmap(mips_sram_mem_va);
}

static int mipsloader_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mipsloader_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mipsloader_firmware_rw(struct firmware_info *info)
{
	uint32_t offset;
	void *base = 0;

	if (info->offset >= MIPS_MEMORY_OFFSET
			&& info->offset + info->size <= MIPS_MEMORY_OFFSET + MIPS_MEMORY_SIZE) {
		pr_info("mipsloader: r/w DRAM space\n");
		offset = info->offset - MIPS_MEMORY_OFFSET;
		base = mips_firmware_mem_va + offset;
	} else if (info->offset >= MIPS_SRAM_MEMORY_OFFSET
			&& info->offset + info->size < MIPS_SRAM_MEMORY_OFFSET + MIPS_SRAM_MEMORY_SIZE) {
		pr_info("mipsloader: r/w SRAM space\n");
		offset = info->offset - MIPS_SRAM_MEMORY_OFFSET;
		base = mips_sram_mem_va + offset;
	} else {
		pr_err("mipsloader: out of mips firmware memory range\n");
		return -EINVAL;
	}

	if (info->type == MIPS_FIRMWARE_R) {
		if (copy_to_user(u64_to_user_ptr(info->buffer), base, info->size)) {
			pr_err("mipsloader: read firmware failed\n");
			return -1;
		}
	} else {
		if (copy_from_user(base, (void __user *)(info->buffer), info->size)) {
			pr_err("mipsloader: write firmware failed\n");
			return -1;
		}
		flush_cache_all();
	}
	return 0;
}

static long mipsloader_ioctl(struct file *file, unsigned int cmd,
		unsigned long param)
{
	struct firmware_info info;

	pr_info("mipsloader: cmd: 0x%08x param: %#lx\n", cmd, param);
	switch (cmd) {
	case MIPS_LOADER_IOC_RW:
		if (copy_from_user(&info, (void __user *)param,
					sizeof(struct firmware_info))) {
			pr_err("mipsloader: copy_from_user fail\n");
			return	-EFAULT;
		}
		return mipsloader_firmware_rw(&info);
	default:
		pr_err("mipsloader: unknown cmd %08x\n", cmd);
		return -EFAULT;
	}
	return 0;
}

static int mipsloader_mmap(struct file *filp, struct vm_area_struct *vma)
{
	pr_info("mipsloader: phy %016lx size %lu\n",
			(vma->vm_pgoff << PAGE_SHIFT), (vma->vm_end - vma->vm_start));

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static const struct file_operations mipsloader_ops = {
	.owner	 = THIS_MODULE,
	.open	 = mipsloader_open,
	.release = mipsloader_release,
	.mmap	 = mipsloader_mmap,
	.unlocked_ioctl = mipsloader_ioctl,
	.compat_ioctl   = mipsloader_ioctl,
};

struct miscdevice mipsloader_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "mipsloader",
	.fops  = &mipsloader_ops,
};

static int mipsloader_init(void)
{
	int result = 0;

	result = misc_register(&mipsloader_device);
	if (result)
		pr_err("Error %d adding mipsloader", result);

	mips_firmware_mem_va = mips_memory_map(MIPS_MEMORY_OFFSET, MIPS_MEMORY_SIZE);
	mips_sram_mem_va = mips_memory_map(MIPS_SRAM_MEMORY_OFFSET, MIPS_SRAM_MEMORY_SIZE);
	return 0;
}

static void mipsloader_exit(void)
{
	mips_memory_unmap();
	misc_deregister(&mipsloader_device);
}

module_init(mipsloader_init);
module_exit(mipsloader_exit);

