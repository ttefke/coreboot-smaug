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

#include <stddef.h>
#include <stdint.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/cbfs.h>
#include <arch/stages.h>
#include <arch/early_variables.h>
#include <console/console.h>
#include <cbmem.h>
#include <cpu/x86/mtrr.h>
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/ec_commands.h>
#include <elog.h>
#include <ramstage_cache.h>
#include <romstage_handoff.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <soc/me.h>
#include <soc/intel/common/mrc_cache.h>
#include <soc/pei_wrapper.h>
#include <soc/pm.h>
#include <soc/reset.h>
#include <soc/romstage.h>
#include <soc/spi.h>

/* SOC initialization before the console is enabled */
void soc_pre_console_init(struct romstage_params *params)
{
	/* System Agent Early Initialization */
	systemagent_early_init();

	/* PCH Early Initialization */
	pch_early_init();
}

/* SOC initialization before RAM is enabled */
void soc_pre_ram_init(struct romstage_params *params)
{
	/* Print ME state before MRC */
	intel_me_status();

	/* Save ME HSIO version */
	intel_me_hsio_version(&params->power_state->hsio_version,
			      &params->power_state->hsio_checksum);


	/* Prepare to initialize memory */
	broadwell_fill_pei_data(params->pei_data);
}

/* Used by MRC images to save DIMM information */
void mainboard_save_dimm_info(struct romstage_params *params)
{
	struct memory_info *mem_info;
	struct pei_data *pei_data;

	printk(BIOS_DEBUG, "create cbmem for dimm information\n");
	mem_info = cbmem_add(CBMEM_ID_MEMINFO, sizeof(struct memory_info));
	pei_data = params->pei_data;
	memcpy(mem_info, &pei_data->meminfo, sizeof(struct memory_info));
}

#if CONFIG_CHROMEOS
int vboot_get_sw_write_protect(void)
{
	u8 status;
	/* Return unprotected status if status read fails. */
	return (early_spi_read_wpsr(&status) ? 0 : !!(status & 0x80));
}
#endif
