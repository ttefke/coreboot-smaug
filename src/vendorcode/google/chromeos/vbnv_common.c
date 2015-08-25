/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2015 Google Inc
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

#include "chromeos.h"
#include "vbnv_layout.h"

static uint8_t crc8(const uint8_t *data, int len)
{
	unsigned crc = 0;
	int i, j;

	for (j = len; j; j--, data++) {
		crc ^= (*data << 8);
		for (i = 8; i; i--) {
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}

	return (uint8_t) (crc >> 8);
}

void vbnv_update_recovery(uint8_t reason)
{
	uint8_t vbnv_copy[VBNV_BLOCK_SIZE];

	read_vbnv(vbnv_copy);

	vbnv_copy[RECOVERY_OFFSET] = reason;
	vbnv_copy[CRC_OFFSET] = crc8(vbnv_copy, CRC_OFFSET);

	save_vbnv(vbnv_copy);
}
