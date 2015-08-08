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

#ifndef __VENDORCODE_GOOGLE_CHROMEOS_VPD_EKS_H__
#define __VENDORCODE_GOOGLE_CHROMEOS_VPD_EKS_H__

/*
 * Read EKS from VPD and store it in proper format in buff. Ensures that EKS
 * size is not greater than buff_len. Otherwise returns error.
 *
 * buff = buffer in which EKS is stored after decoding.
 * buff_len = max length of buffer
 *
 * Returns -1 on error, length of EKS on success.
 */
int vpd_read_eks(uint8_t *buff, size_t buff_len);

#endif
