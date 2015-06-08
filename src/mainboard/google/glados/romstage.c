/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2015 Google Inc.
 * Copyright (C) 2015 Intel Corporation
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

#include <cbfs.h>
#include <console/console.h>
#include <string.h>
#include <ec/google/chromeec/ec.h>
#include <soc/pei_data.h>
#include <soc/pei_wrapper.h>
#include <soc/romstage.h>
#include "spd/spd.h"

void mainboard_romstage_entry(struct romstage_params *params)
{
	/* Fill out PEI DATA */
	mainboard_fill_pei_data(params->pei_data);
	mainboard_fill_spd_data(params->pei_data);
	/* Initliaze memory */
	romstage_common(params);
}

void mainboard_memory_init_params(struct romstage_params *params,
				  UPD_DATA_REGION *upd_ptr)
{
	if (params->pei_data->spd_data[0][0][0] != 0) {
		upd_ptr->MemorySpdPtr00 =
				(UINT32)(params->pei_data->spd_data[0][0]);
		upd_ptr->MemorySpdPtr10 =
				(UINT32)(params->pei_data->spd_data[1][0]);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_0\n",
				upd_ptr->MemorySpdPtr00);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_1\n",
				upd_ptr->MemorySpdPtr01);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_0\n",
				upd_ptr->MemorySpdPtr10);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_1\n",
				upd_ptr->MemorySpdPtr11);
	}
	memcpy(upd_ptr->DqByteMapCh0, params->pei_data->dq_map[0],
			sizeof(params->pei_data->dq_map[0]));
	memcpy(upd_ptr->DqByteMapCh1, params->pei_data->dq_map[1],
			sizeof(params->pei_data->dq_map[1]));
	memcpy(upd_ptr->DqsMapCpu2DramCh0, params->pei_data->dqs_map[0],
			sizeof(params->pei_data->dqs_map[0]));
	memcpy(upd_ptr->DqsMapCpu2DramCh1, params->pei_data->dqs_map[1],
			sizeof(params->pei_data->dqs_map[1]));
	memcpy(upd_ptr->RcompResistor, params->pei_data->RcompResistorSkl,
			sizeof(params->pei_data->RcompResistorSkl));
	memcpy(upd_ptr->RcompTarget, params->pei_data->RcompTargetSkl,
			sizeof(params->pei_data->RcompTargetSkl));
	upd_ptr->MemorySpdDataLen = SPD_LEN;
	upd_ptr->DqPinsInterleaved = FALSE;
}
