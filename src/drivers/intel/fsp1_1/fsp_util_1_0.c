/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
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

#include <console/console.h>
#include <cpu/x86/stack.h>
#include "fsp_util.h"
#include <string.h>

#ifdef __PRE_RAM__

/*
 * Call the FSP to do memory init. The FSP doesn't return to this function.
 * The FSP returns to the romstage_main_continue().
 */
void __attribute__ ((noreturn)) fsp_early_init(FSP_INFO_HEADER *fsp_ptr)
{
	FSP_FSP_INIT fsp_init_api;
	FSP_INIT_PARAMS fsp_init_params;
	FSP_INIT_RT_BUFFER fsp_rt_buffer;
#if IS_ENABLED(CONFIG_FSP_USES_UPD)
	UPD_DATA_REGION fsp_upd_data;
#endif

	memset((void *)&fsp_rt_buffer, 0, sizeof(FSP_INIT_RT_BUFFER));
	fsp_rt_buffer.Common.StackTop = (u32 *)ROMSTAGE_STACK;
	fsp_init_params.NvsBufferPtr = NULL;

#if IS_ENABLED(CONFIG_FSP_USES_UPD)
	fsp_rt_buffer.Common.UpdDataRgnPtr = &fsp_upd_data;
#endif
	fsp_init_params.RtBufferPtr = (FSP_INIT_RT_BUFFER *)&fsp_rt_buffer;
	fsp_init_params.ContinuationFunc =
		(CONTINUATION_PROC)chipset_fsp_return_point;
	fsp_init_api = (FSP_FSP_INIT)(fsp_ptr->ImageBase
		+ fsp_ptr->FspInitEntryOffset);

	/* Call the chipset code to fill in the chipset specific structures */
	chipset_fsp_early_init(&fsp_init_params, fsp_ptr);

	/* Call back to romstage for board specific changes */
	romstage_fsp_rt_buffer_callback(&fsp_rt_buffer);

	fsp_init_api(&fsp_init_params);

	/* Should never return. Control will continue from ContinuationFunc */
	die("Uh Oh! FspInit returned");
}
#endif	/* __PRE_RAM__ */
