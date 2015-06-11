/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <arch/exception.h>
#include <arch/stages.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/ccplex.h>
#include <soc/clock.h>
#include <soc/sdram.h>
#include <soc/sdram_configs.h>
#include <soc/romstage.h>
#include <soc/nvidia/tegra/apbmisc.h>
#include <timer.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

void __attribute__((weak)) romstage_mainboard_init(void)
{
	/* Default empty implementation. */
}

static void *load_ramstage(void)
{
	void *entry;
	struct stopwatch sw;

	stopwatch_init(&sw);

	timestamp_add_now(TS_START_COPYRAM);

#if IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)
	entry = vboot2_load_ramstage();
#else
	/*
	 * This platform does not need to cache a loaded ramstage nor do we
	 * go down this path on resume. Therefore, no romstage_handoff is
	 * required.
	 */
	entry = vboot_verify_firmware_get_entry(NULL);
#endif

	if (entry == NULL)
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/ramstage");

	timestamp_add_now(TS_END_COPYRAM);

	printk(BIOS_DEBUG, "Ramstage load time: %ld usecs.\n",
		stopwatch_duration_usecs(&sw));

	return entry;
}

void romstage(void)
{
	void *entry;

	timestamp_add_now(TS_START_ROMSTAGE);

	console_init();
	exception_init();

	printk(BIOS_INFO, "T210: romstage here\n");

	timestamp_add_now(TS_BEFORE_INITRAM);

#if CONFIG_BOOTROM_SDRAM_INIT
	printk(BIOS_INFO, "T210 romstage: SDRAM init done by BootROM, RAMCODE = %d\n",
		sdram_get_ram_code());
#else
	sdram_init(get_sdram_config());
	printk(BIOS_INFO, "T210 romstage: sdram_init done\n");
#endif

	timestamp_add_now(TS_AFTER_INITRAM);

	/* Setup carveout for GPU firmware first */
	gpu_region_init();

	/*
	 * Trust Zone needs to be initialized after the DRAM initialization
	 * because carveout registers are programmed during DRAM init.
	 * cbmem_initialize() is dependent on the Trust Zone region
	 * initalization because CBMEM lives right below the Trust Zone which
	 * needs to be properly identified.
	 */
	trustzone_region_init();

	/*
	 * When romstage is running it's always on the reboot path -- never a
	 * resume path where cbmem recovery is required. Therefore, always
	 * initialize the cbmem area to be empty.
	 */
	cbmem_initialize_empty();

	ccplex_cpu_prepare();
	printk(BIOS_INFO, "T210 romstage: cpu prepare done\n");

	romstage_mainboard_init();

	entry = load_ramstage();

	if (entry == NULL) {
		printk(BIOS_INFO, "T210 romstage: error loading ramstage\n");
		clock_halt_avp();
	}

	timestamp_add_now(TS_END_ROMSTAGE);

	ccplex_cpu_start(entry);

	clock_halt_avp();
}
