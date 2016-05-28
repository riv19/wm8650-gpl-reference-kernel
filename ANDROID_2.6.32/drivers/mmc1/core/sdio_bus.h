/*
 *  linux/drivers/mmc/core/sdio_bus.h
 *
 *  Copyright 2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */
#ifndef _MMC_CORE_SDIO_BUS_H
#define _MMC_CORE_SDIO_BUS_H

struct sdio_func *sdio1_alloc_func(struct mmc_card *card);
int sdio1_add_func(struct sdio_func *func);
void sdio1_remove_func(struct sdio_func *func);

int sdio1_register_bus(void);
void sdio1_unregister_bus(void);

#endif

