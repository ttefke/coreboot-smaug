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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <console/console.h>
#include <delay.h>
#include <soc/verstage.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

void __attribute__((weak)) verstage_mainboard_init(void)
{
	/* Default empty implementation. */
}

static void verstage(void)
{
	void *entry;

	timestamp_add_now(TS_START_VBOOT);
	verstage_mainboard_init();

	/*
	 * SLB9645 implies TPM requires 150 ms power-on tests.
	 * add +5 ms margin
	 */
	while (timestamp_get() < (uint64_t)155000)
		mdelay(1);

	entry = vboot2_verify_firmware();
	if (entry != (void *)-1)
		stage_exit(entry);
}

void soc_verstage_main(void)
{
	verstage();
	hlt();
}
