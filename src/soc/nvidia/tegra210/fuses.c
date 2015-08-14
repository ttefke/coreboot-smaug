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

#include <arch/io.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/fuses.h>

/*
 * Set visibility of fuse registers.
 *
 * param @val: 1 = Make fuse registers visible.
 *             0 = Make fuse registers invisible.
 */
static inline void fuse_set_reg_visibility(uint32_t val)
{
	uint32_t reg_val = read32(CLK_RST_REG(misc_clk_enb));
	reg_val &= ~(1ULL << FUSE_ALL_VISIBLE_SHIFT);
	reg_val |= ((val & 0x1) << FUSE_ALL_VISIBLE_SHIFT);
	write32(CLK_RST_REG(misc_clk_enb), reg_val);
}

/*
 * Get visibility of fuse registers.
 *
 * Returns 1 if fuse registers are visible, 0 otherwise.
 */
static inline uint32_t fuse_get_reg_visibility(void)
{
	uint32_t reg_val = read32(CLK_RST_REG(misc_clk_enb));
	return !!(reg_val >> FUSE_ALL_VISIBLE_SHIFT);
}

int fuse_get_uid(uint32_t *uid, size_t size)
{
	if (uid == NULL) {
		printk(BIOS_ERR, "ERROR: Invalid uid buffer address\n");
		return -1;
	}

	if (size != FUSE_UID_SIZE) {
		printk(BIOS_ERR, "ERROR: Oops size %zd, expected %ld\n", size,
		       (unsigned long)FUSE_UID_SIZE);
		return -1;
	}

	/*
	 * If fuse regs visibility is true, no need to change anything.
	 * Else, make fuse regs visible.
	 */
	uint32_t fuse_visible = fuse_get_reg_visibility();
	if (fuse_visible == 0)
		fuse_set_reg_visibility(1);

	uint32_t vendor = FUSE_REG_READ(OPT_VENDOR_CODE);
	uint32_t fab = FUSE_REG_READ(OPT_FAB_CODE);
	uint32_t lot0 = FUSE_REG_READ(OPT_LOT_CODE_0);
	uint32_t lot1 = FUSE_REG_READ(OPT_LOT_CODE_1);
	uint32_t wafer = FUSE_REG_READ(OPT_WAFER_ID);
	uint32_t x_coor = FUSE_REG_READ(OPT_X_COOR);
	uint32_t y_coor = FUSE_REG_READ(OPT_Y_COOR);
	uint32_t rsvd = FUSE_REG_READ(OPT_OPS_RSVD);

	uid[0] = ID(0, rsvd) | ID(0, y_coor) | ID(0, x_coor) | ID(0, wafer)
		| ID(0, lot1);
	uid[1] = ID(1, lot1) | ID(1, lot0);
	uid[2] = ID(2, lot0) | ID(2, fab);
	uid[3] = ID(3, vendor);

	/* Restore original fuse regs visibility. */
	if (fuse_visible == 0)
		fuse_set_reg_visibility(0);

	return 0;
}
