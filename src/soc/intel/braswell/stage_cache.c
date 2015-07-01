/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2015 Intel Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <cbmem.h>
#include <ramstage_cache.h>
#include <soc/intel/common/memmap.h>
#include <soc/smm.h>

/*
 * SMM Memory Map:
 *
 * +--------------------------+ smm_region_size() ----.
 * |     FSP Cache            | CONFIG_FSP_CACHE_SIZE |
 * +--------------------------+                       |
 * |     SMM Ramstage Cache   |                       + CONFIG_SMM_RESERVED_SIZE
 * +--------------------------+  ---------------------'
 * |     SMM Code             |
 * +--------------------------+ smm_base
 *
 */

struct ramstage_cache *ramstage_cache_location(long *size)
{
	char *smm_base;
	size_t smm_size;
	const long cache_size = CONFIG_SMM_RESERVED_SIZE;

	/* Ramstage cache lives in TSEG region. */
	smm_region((void **)&smm_base, &smm_size);
	*size = cache_size - CONFIG_FSP_CACHE_SIZE;
	return (void *)(&smm_base[smm_size - cache_size]);
}
