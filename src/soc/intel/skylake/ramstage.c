/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Intel Corporation.
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

#include <bootstate.h>
#include <console/console.h>
#include <fsp_util.h>
#include <soc/ramstage.h>
#include <soc/intel/common/ramstage.h>

void skylake_init_pre_device(void *chip_info)
{
	/* Perform silicon specific init. */
	intel_silicon_init();
}

static void issue_ready_to_boot_event(void *unused)
{
	/*
	 * Notify FSP for EnumInitPhaseReadyToBoot.
	 */
	printk(BIOS_DEBUG, "fsp_notify(EnumInitPhaseReadyToBoot)\n");
	fsp_notify(EnumInitPhaseReadyToBoot);
}

BOOT_STATE_INIT_ENTRIES(finalize_bscb) = {
	BOOT_STATE_INIT_ENTRY(BS_PAYLOAD_LOAD, BS_ON_EXIT,
		issue_ready_to_boot_event, NULL)
};
