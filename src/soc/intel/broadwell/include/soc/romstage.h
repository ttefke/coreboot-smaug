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
#include <soc/pei_data.h>
#include <soc/pm.h>

struct romstage_params {
	unsigned long bist;
	struct chipset_power_state *power_state;
	struct pei_data *pei_data;
	void *chipset_context;
};

void mainboard_check_ec_image(struct romstage_params *params);
void mainboard_pre_console_init(struct romstage_params *params);
void mainboard_romstage_entry(struct romstage_params *params);
void mainboard_save_dimm_info(struct romstage_params *params);
void raminit(struct romstage_params *params);
void report_memory_config(void);
void report_platform_info(void);
asmlinkage void romstage_after_car(void *chipset_context);
void romstage_common(struct romstage_params *params);
asmlinkage void *romstage_main(unsigned int bist, uint32_t tsc_lo,
			       uint32_t tsc_high, void *chipset_context);
void *setup_stack_and_mtrrs(void);
void set_max_freq(void);
void soc_after_ram_init(struct romstage_params *params);
void soc_after_temp_ram_exit(void);
void soc_pre_console_init(struct romstage_params *params);
void soc_pre_ram_init(struct romstage_params *params);
void soc_romstage_init(struct romstage_params *params);

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
