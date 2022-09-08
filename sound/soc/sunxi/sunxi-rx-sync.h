/* sound\soc\sunxi\sunxi-rx-sync.h
 * (C) Copyright 2019-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef __SUNXI_RX_SYNC_H_
#define __SUNXI_RX_SYNC_H_

typedef enum {
	RX_SYNC_SYS_DOMAIN,
	RX_SYNC_DSP_DOMAIN,
	RX_SYNC_DOMAIN_CNT
} rx_sync_domain_t;

#ifdef CONFIG_SUNXI_RX_SYNC
int sunxi_rx_sync_probe(rx_sync_domain_t domain);
void sunxi_rx_sync_remove(rx_sync_domain_t domain);
void sunxi_rx_sync_startup(void *data, rx_sync_domain_t domain, int id,
		//void (*route_enable)(void *data, bool enable),
		void (*rx_enable)(void *data, bool enable));
void sunxi_rx_sync_shutdown(rx_sync_domain_t domain, int id);
void sunxi_rx_sync_control(rx_sync_domain_t domain, int id, bool enable);
#else
static inline int sunxi_rx_sync_probe(rx_sync_domain_t domain)
{
	return 0;
}
static inline void sunxi_rx_sync_remove(rx_sync_domain_t domain)
{
	return;
}
static inline void sunxi_rx_sync_startup(void *data, rx_sync_domain_t domain, int id,
		//void (*route_enable)(void *data, bool enable),
		void (*rx_enable)(void *data, bool enable))
{
	return;
}
static inline void sunxi_rx_sync_shutdown(rx_sync_domain_t domain, int id)
{
	return;
}
static inline void sunxi_rx_sync_control(rx_sync_domain_t domain, int id, bool enable)
{
	return;
}
#endif

#endif /* __SUNXI_RX_SYNC_H_ */
