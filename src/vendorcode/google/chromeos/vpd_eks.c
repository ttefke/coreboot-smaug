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

#include <console/console.h>
#include <string.h>

#include <vendorcode/google/chromeos/cros_vpd.h>
#include <vendorcode/google/chromeos/vpd_eks.h>

static int decode_digit(const uint8_t src, uint8_t *dst)
{
	if (isxdigit(src)) {
		if (isdigit(src))
			*dst = src - '0';
		else
			*dst = tolower(src) - 'a' + 10;
	} else
		return -1;

	return 0;
}

int vpd_read_eks(uint8_t *buff, size_t buff_len)
{
	const char eks_key[] = "eks";
	const uint8_t *addr;
	int len;
	int i, j;
	uint8_t val_h, val_l;

	addr = cros_vpd_find(eks_key, &len);

	if ((addr == NULL) || (len < 0) || (len > (buff_len * 2))) {
		printk(BIOS_ERR, "ERROR: EKS read failure\n");
		return -1;
	}

	for (i = 0, j = 0; i < len; i += 2, j++) {

		if (decode_digit(addr[i], &val_h) ||
		    decode_digit(addr[i+1], &val_l)) {
			printk(BIOS_ERR, "ERROR: Digit decode error\n");
			return -1;
		}
		buff[j] = (val_h << 4) | val_l;
	}

	return j;
}
