
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <asm-generic/cacheflush.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>

MODULE_AUTHOR("allwinnertech.com");
MODULE_LICENSE("Dual BSD/GPL");

#define IMAGE_LOADER_IOC_ADD   0x064800
#define IMAGE_LOADER_IOC_CLEAR 0x064801

struct dma_buf_attach_info {
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	dma_addr_t dma_addr;
};

struct image_block {
	struct dma_buf *buf;
	uint32_t size;

	struct dma_buf_attach_info info;
	struct list_head link;
};

struct image_data {
	int dmafd;
	uint32_t buffer_size;
};

static LIST_HEAD(image_list_head);

static struct device *get_imagedevice(void);

static int tvutils_open (struct inode *inode, struct file *filp)
{
	return 0;
}

static int tvutils_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int __attach_buf(struct dma_buf *buf, struct dma_buf_attach_info *info)
{
	info->attachment = dma_buf_attach(buf, get_imagedevice());
	if (IS_ERR(info->attachment)) {
		pr_err("dma_buf_attach failed\n");
		return -1;
	}
	info->sgt = dma_buf_map_attachment(info->attachment, DMA_TO_DEVICE);
	if (IS_ERR_OR_NULL(info->sgt)) {
		pr_err("dma_buf_map_attachment failed\n");
		dma_buf_detach(buf, info->attachment);
		return -1;
	}
	info->dma_addr = sg_dma_address(info->sgt->sgl);
	return 0;
}

static int __detach_buf(struct dma_buf *buf, struct dma_buf_attach_info *info)
{
	dma_buf_unmap_attachment(info->attachment, info->sgt, DMA_TO_DEVICE);
	dma_buf_detach(buf, info->attachment);
	return 0;
}

static int tvutils_add(struct image_data *data)
{
	struct image_block *block;
	struct dma_buf *buf = dma_buf_get(data->dmafd);
	if (IS_ERR(buf)) {
		pr_err("dma_buf_get failed, fd=%d\n", data->dmafd);
		return -EINVAL;
	}

	block = kmalloc(
			sizeof(struct image_block), GFP_KERNEL | __GFP_ZERO);
	if (block == NULL) {
		pr_err("kmalloc for image_block failed\n");
		dma_buf_put(buf);
		return -ENOMEM;
	}

	if (__attach_buf(buf, &block->info) != 0) {
		dma_buf_put(buf);
		kfree(block);
		return -EINVAL;
	}

	block->buf = buf;
	block->size = data->buffer_size;
	INIT_LIST_HEAD(&block->link);

	list_add_tail(&block->link, &image_list_head);

	return 0;
}

static int tvutils_clear(void)
{
	struct image_block *block, *next;

	list_for_each_entry_safe(block, next, &image_list_head, link) {
		list_del(&block->link);
		__detach_buf(block->buf, &block->info);
		dma_buf_put(block->buf);
		kfree(block);
	}
	return 0;
}

static long tvutils_ioctl(struct file *file, unsigned int cmd,
		unsigned long param)
{
	struct image_data data;

	switch (cmd) {
	case IMAGE_LOADER_IOC_ADD:
		if (copy_from_user(&data, (void __user *)param, sizeof(struct image_data))) {
			pr_err("tvutils: copy_from_user fail\n");
			return	-EFAULT;
		}
		return tvutils_add(&data);
	case IMAGE_LOADER_IOC_CLEAR:
		return tvutils_clear();
	default:
		pr_err("tvutils: unknown cmd %08x\n", cmd);
		return -EFAULT;
	}
	return 0;
}

static ssize_t imgdev_imgbuf_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	struct image_block *block, *next;
	int printed = 0;
	printed += sprintf(buf, "active image buffers: \n");

	list_for_each_entry_safe(block, next, &image_list_head, link) {
		printed += sprintf(buf + printed, " physical offset: %016llx size: %d\n",
				block->info.dma_addr, block->size);
	}

	return printed;
}

#define CCMU_REG_BASE			0x02001000
#define CCMU_REG_LENGTH			0x1000
#define UART_BUS_GATING_RST_REG 0x090C

static ssize_t tvutils_tvdisp_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	void __iomem *va = NULL;

	if (strncasecmp(buf, "1", 1) == 0) {
		va = ioremap(CCMU_REG_BASE, CCMU_REG_LENGTH);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap ccmu error: %pK\n", va);
			goto __out;
		}
		// tvdisp subsystem gating and reset
		writel(0x00010001, va + 0x0dd8); // tvdisp
		iounmap(va);

		// tvdisp top
		va = ioremap(0x05700000, 0x100);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap tvdisp_top error: %pK\n", va);
			goto __out;
		}
		writel(0xFFFFFFFF, va + 0x0000);
		writel(0xFFFFFFFF, va + 0x0004);
		writel(0xFFFFFFFF, va + 0x0040);
		writel(0xFFFFFFFF, va + 0x0044);
		writel(0xFFFFFFFF, va + 0x0080);
		writel(0xFFFFFFFF, va + 0x0084);
		writel(0xFFFFFFFF, va + 0x0088);
		iounmap(va);
	}

__out:
	return count;
}

static ssize_t tvutils_tvcap_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	void __iomem *va = NULL;

	if (strncasecmp(buf, "1", 1) == 0) {
		va = ioremap(CCMU_REG_BASE, CCMU_REG_LENGTH);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap ccmu error: %pK\n", va);
			goto __out;
		}
		// tvcap subsystem gating and reset
		writel(0x00010001, va + 0x0d88); // tvcap
		iounmap(va);

		// tvcap top
		va = ioremap(0x06E00000, 0x100);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap tvcap_top error: %pK\n", va);
			goto __out;

		}
		writel(0xFFFFFFFF, va + 0x0000);
		writel(0xFFFFFFFF, va + 0x0004);
		writel(0xFFFFFFFF, va + 0x0008);
		iounmap(va);
	}

__out:
	return count;
}

static ssize_t tvutils_tvfe_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	void __iomem *va = NULL;

	if (strncasecmp(buf, "1", 1) == 0) {
		va = ioremap(CCMU_REG_BASE, CCMU_REG_LENGTH);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap ccmu error: %pK\n", va);
			goto __out;
		}
		// tvfe subsystem gating and reset
		writel(0x00010001, va + 0x0d64); // tvfe
		iounmap(va);

		// tvfe top
		va = ioremap(0x06700000, 0x100);
		if (IS_ERR_OR_NULL(va)) {
			pr_err("ioremap tvdisp_top error: %pK\n", va);
			goto __out;
		}
		writel(0x003003FF, va + 0x0000);
		writel(0x00000003, va + 0x0004);
		iounmap(va);
	}

__out:
	return count;
}

static DEVICE_ATTR(imgbuf, 0444, imgdev_imgbuf_show, NULL);
static DEVICE_ATTR(tvdisp, 0444, NULL, tvutils_tvdisp_debug_store);
static DEVICE_ATTR(tvcap,  0444, NULL, tvutils_tvcap_debug_store);
static DEVICE_ATTR(tvfe,   0444, NULL, tvutils_tvfe_debug_store);

static struct attribute *imgdev_attrs[] = {
	&dev_attr_imgbuf.attr,
	&dev_attr_tvdisp.attr,
	&dev_attr_tvcap.attr,
	&dev_attr_tvfe.attr,
	NULL
};

static const struct attribute_group imgdev_attr_group = {
	.attrs = imgdev_attrs
};

static const struct attribute_group *imgdev_attr_groups[] = {
	&imgdev_attr_group,
	NULL
};

static struct file_operations tvutils_ops = {
	.owner	 = THIS_MODULE,
	.open	 = tvutils_open,
	.release = tvutils_release,
	.unlocked_ioctl = tvutils_ioctl,
};

static u64 disp_dmamask = DMA_BIT_MASK(32);
struct miscdevice tvutils_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "tvutils",
	.fops  = &tvutils_ops,
	.groups = imgdev_attr_groups,
};

static struct device *get_imagedevice(void)
{
	tvutils_device.this_device->dma_mask = &disp_dmamask;
	return tvutils_device.this_device;
}

static void tvutils_ccmu_init(void)
{
	void __iomem *va = ioremap(CCMU_REG_BASE, CCMU_REG_LENGTH);
	if (IS_ERR_OR_NULL(va)) {
		pr_err("ioremap ccmu error: %pK\n", va);
		return;
	}

	// Init uart0/uart1/uart2 clk and reset
	writel(0x00050005, va + UART_BUS_GATING_RST_REG);

	iounmap(va);
}

static void tvutils_nsi_config(void)
{
#define NSI_BASE (0x02020000)
#define NSI_LEN  (0x00001000)

	void __iomem *va = ioremap(NSI_BASE, NSI_LEN);
	if (IS_ERR_OR_NULL(va)) {
		pr_err("ioremap nsi error: %pK\n", va);
		return;
	}
	writel(0x0000000a, va + 0x0814);
	iounmap(va);
}

static int tvutils_init(void)
{
	int result = 0;
	result = misc_register(&tvutils_device);
	if (result)
		pr_err(KERN_WARNING "Error %d adding tvutils", result);

	tvutils_ccmu_init();
	tvutils_nsi_config();
	return 0;
}

static void tvutils_exit(void)
{
	tvutils_clear();
	misc_deregister(&tvutils_device);
}

module_init(tvutils_init);
module_exit(tvutils_exit);

MODULE_AUTHOR("allwinnertech.com");
MODULE_LICENSE("Dual BSD/GPL");
