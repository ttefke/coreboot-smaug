/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _SKYLAKE_PCH_H_
#define _SKYLAKE_PCH_H_

#include <device/device.h>

/* Skylake PCH (SunRisePoint LP) */
#define PCH_SPT_LP_SAMPLE		0x9d41
#define PCH_SPT_LP_U_BASE		0x9d43
#define PCH_SPT_LP_U_PREMIUM		0x9d48
#define PCH_SPT_LP_Y_PREMIUM		0x9d46

/* Power Management Control and Status */
#define PCH_DVID			0x0
#define PCH_MBASE			0x48
#define PCH_PCS				0x84
#define  PCH_PCS_PS_D3HOT		3
#define B_PCH_PMC_BAR0_MASK		0xFFFF0000
#define PCH_PWRM_PMSYNC_TPR_CONFIG	0xC4
#define PMSYNC_LOCK			(1 << 31)
/* SMBUS TCO Config */
#define PCH_SMBUS_TCOBASE		0x50
#define PCH_SMBUS_TCOBASE_BAR		0x0000FFE0
#define PCH_TCO1_CNT			0x08
#define PCH_TCO_LOCK			0x1000
#define PCH_TCO1_STS			0x04
#define PCH_TCO2_STS			0x06
#define PCH_TCO2_STS_SEC_TO		0x02
#define PCH_TCO2_STS_BOOT		0x04

u8 pch_revision(void);
u16 pch_type(void);
u32 pch_read_soft_strap(int id);
void pch_log_state(void);
#ifndef __PRE_RAM__
void pch_disable_devfn(device_t dev);
#endif

#endif /* _SKYLAKE_PCH_H_ */
