/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BROADWELL_ROMSTAGE_H_
#define _BROADWELL_ROMSTAGE_H_

#include <stdint.h>
#include <arch/cpu.h>
#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP1_1)
#include <fsp_util.h>
#endif	/* CONFIG_PLATFORM_USES_FSP1_1 */
#include <soc/pei_data.h>
#include <soc/pm.h>
#include <soc/intel/common/romstage.h>

struct chipset_power_state;
struct chipset_power_state *fill_power_state(void);

void systemagent_early_init(void);
void pch_early_init(void);
void intel_early_me_status(void);

void enable_smbus(void);
int smbus_read_byte(unsigned device, unsigned address);

int early_spi_read(u32 offset, u32 size, u8 *buffer);
int early_spi_read_wpsr(u8 *sr);
#endif
