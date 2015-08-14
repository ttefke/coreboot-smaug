/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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

#ifndef __SOC_NVIDIA_TEGRA210_INCLUDE_SOC_FUSES_H__
#define __SOC_NVIDIA_TEGRA210_INCLUDE_SOC_FUSES_H__

#include <stdint.h>
#include <stddef.h>

#define FUSE_ALL_VISIBLE_SHIFT		(28)

#define FUSE_REG(reg, off, mask)	\
	reg##_OFFSET = off,		\
	reg##_MASK = mask

enum {
	FUSE_REG(OPT_VENDOR_CODE, 0x200, 0xF),
	FUSE_REG(OPT_FAB_CODE, 0x204, 0x3F),
	FUSE_REG(OPT_LOT_CODE_0, 0x208, 0xFFFFFFFF),
	FUSE_REG(OPT_LOT_CODE_1, 0x20C, 0xFFFFFFF),
	FUSE_REG(OPT_WAFER_ID, 0x210, 0x3F),
	FUSE_REG(OPT_X_COOR, 0x214, 0x1FF),
	FUSE_REG(OPT_Y_COOR, 0x218, 0x1FF),
	FUSE_REG(OPT_OPS_RSVD, 0x220, 0x3F),
};

#define FUSE_REG_READ(reg)	\
	(read32((void *)(TEGRA_FUSE_BASE + (reg##_OFFSET))) & (reg##_MASK))

#define FUSE_UID_ELEMENT_SIZE	sizeof(uint32_t)
#define FUSE_UID_ELEMENT_NUM	4
#define FUSE_UID_SIZE		(FUSE_UID_ELEMENT_NUM * FUSE_UID_ELEMENT_SIZE)

/*
 *  ID0:
 *
 *   31   30 29        24 23                15 14                6 5        0
 *  +------------------------------------------------------------------------+
 *  |       |            |                    |                   |          |
 *  | LOT1  |   WAFER    |    X_COORDINATE    |  Y_COORDINATE     | OPS_RSVD |
 *  | (1:0) |            |                    |                   |          |
 *  +------------------------------------------------------------------------+
 *
 * ID1:
 *
 *   31          26  25                                                     0
 *  +------------------------------------------------------------------------+
 *  |              |                                                         |
 *  |  LOT0 (5:0)  |                  LOT1 (27:2)                            |
 *  |              |                                                         |
 *  +------------------------------------------------------------------------+
 *
 * ID2:
 *
 *   31           26  25                                                    0
 *  +------------------------------------------------------------------------+
 *  |              |                                                         |
 *  |     FAB      |                  LOT0 (31:6)                            |
 *  |              |                                                         |
 *  +------------------------------------------------------------------------+
 *
 *
 * ID3:
 *
 *   31                                                            5  4     0
 *  +------------------------------------------------------------------------+
 *  |                                                               |        |
 *  |                    UNUSED (all 0s)                            | VENDOR |
 *  |                                                               |        |
 *  +------------------------------------------------------------------------+
 *
 */

#define BIT_MASK(bits)		((1ULL << (bits + 1)) - 1)

#define ID_ELEM(id, var, var_shift, range_shift, range_bits)		\
	ID##id##_##var##_VAR_SHIFT = var_shift,			\
	ID##id##_##var##_RANGE_SHIFT = range_shift,			\
	ID##id##_##var##_RANGE_MASK = BIT_MASK(range_bits)

enum {
	ID_ELEM(0, rsvd, 0, 0, 6),
	ID_ELEM(0, y_coor, 0, 6, 9),
	ID_ELEM(0, x_coor, 0, 15, 9),
	ID_ELEM(0, wafer, 0, 24, 6),
	ID_ELEM(0, lot1, 0, 30, 2),

	ID_ELEM(1, lot1, 2, 0, 26),
	ID_ELEM(1, lot0, 0, 26, 6),

	ID_ELEM(2, lot0, 6, 0, 26),
	ID_ELEM(2, fab, 0, 26, 6),

	ID_ELEM(3, vendor, 0, 0, 4),
};

#define ID(i, var)	\
	(((var >> ID##i##_##var##_VAR_SHIFT) & (ID##i##_##var##_RANGE_MASK)) \
	 << (ID##i##_##var##_RANGE_SHIFT))

/*
 * Get UID from fuses.
 *
 * Returns 0 if success and uid contains the uid read and processed from fuses.
 * Returns -1 on error.
 */
int fuse_get_uid(uint32_t *uid, size_t size);

#endif /* __SOC_NVIDIA_TEGRA210_INCLUDE_SOC_FUSES_H__ */
