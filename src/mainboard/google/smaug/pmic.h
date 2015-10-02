/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
 * Copyright (c) 2015, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef __MAINBOARD_GOOGLE_FOSTER_PMIC_H__
#define __MAINBOARD_GOOGLE_FOSTER_PMIC_H__

#define MAX77620_CNFGGLBL2_REG		0x01
#define MAX77620_I2CTWD_SHIFT		6
#define MAX77620_I2CTWD_1_33_MS	(1 << MAX77620_I2CTWD_SHIFT)
#define MAX77620_I2CTWD_35_7_MS	(2 << MAX77620_I2CTWD_SHIFT)
#define MAX77620_I2CTWD_41_7_MS	(3 << MAX77620_I2CTWD_SHIFT)
#define MAX77620_I2CTWD_MASK		(3 << MAX77620_I2CTWD_SHIFT)

#define MAX77620_SD0_REG		0x16
#define MAX77620_SD1_REG		0x17
#define MAX77620_SD2_REG		0x18
#define MAX77620_SD3_REG		0x19
#define MAX77620_VDVSSD0_REG		0x1B
#define MAX77620_CNFG2SD_REG		0x22

#define MAX77620_CNFG1_L0_REG		0x23
#define MAX77620_CNFG2_L0_REG		0x24
#define MAX77620_CNFG1_L1_REG		0x25
#define MAX77620_CNFG2_L1_REG		0x26
#define MAX77620_CNFG1_L2_REG		0x27
#define MAX77620_CNFG2_L2_REG		0x28
#define MAX77620_CNFG1_L3_REG		0x29
#define MAX77620_CNFG2_L3_REG		0x2A
#define MAX77620_CNFG1_L4_REG		0x2B
#define MAX77620_CNFG2_L4_REG		0x2C
#define MAX77620_CNFG1_L5_REG		0x2D
#define MAX77620_CNFG2_L5_REG		0x2E
#define MAX77620_CNFG1_L6_REG		0x2F
#define MAX77620_CNFG2_L6_REG		0x30
#define MAX77620_CNFG1_L7_REG		0x31
#define MAX77620_CNFG2_L7_REG		0x32
#define MAX77620_CNFG1_L8_REG		0x33
#define MAX77620_CNFG2_L8_REG		0x34
#define MAX77620_CNFG3_LDO_REG		0x35

#define MAX77620_GPIO0_REG		0x36
#define MAX77620_GPIO1_REG		0x37
#define MAX77620_GPIO2_REG		0x38
#define MAX77620_GPIO3_REG		0x39
#define MAX77620_GPIO4_REG		0x3A
#define MAX77620_GPIO5_REG		0x3B
#define MAX77620_GPIO6_REG		0x3C
#define MAX77620_GPIO7_REG		0x3D
#define MAX77620_GPIO_PUE_GPIO		0x3E
#define MAX77620_GPIO_PDE_GPIO		0x3F

#define MAX77620_AME_GPIO		0x40
#define MAX77620_REG_ONOFF_CFG1		0x41
#define MAX77620_REG_ONOFF_CFG2		0x42
#define MAX77620_CNFGFPS0_REG		0x43
#define MAX77620_CNFGFPS1_REG		0x44
#define MAX77620_CNFGFPS2_REG		0x45

#define MAX77620_FPS_L0_REG		0x46
#define MAX77620_FPS_L1_REG		0x47
#define MAX77620_FPS_L2_REG		0x48
#define MAX77620_FPS_L3_REG		0x49
#define MAX77620_FPS_L4_REG		0x4A
#define MAX77620_FPS_L5_REG		0x4B
#define MAX77620_FPS_L6_REG		0x4C
#define MAX77620_FPS_L7_REG		0x4D
#define MAX77620_FPS_L8_REG		0x4E
#define MAX77620_FPS_SD0_REG		0x4F
#define MAX77620_FPS_SD1_REG		0x50
#define MAX77620_FPS_SD2_REG		0x51
#define MAX77620_FPS_SD3_REG		0x52
#define MAX77620_FPS_SD4_REG		0x53
#define MAX77620_FPS_GPIO1_REG		0x54
#define MAX77620_FPS_GPIO2_REG		0x55
#define MAX77620_FPS_GPIO3_REG		0x56
#define MAX77620_FPS_RSO_REG		0x57

#define MAX77620_TFPS_SHIFT		3
#define MAX77620_TFPS_MASK		(0x7 << MAX77620_TFPS_SHIFT)

#define MAX77620_CID0_REG		0x58
#define MAX77620_CID1_REG		0x59
#define MAX77620_CID2_REG		0x5A
#define MAX77620_CID3_REG		0x5B
#define MAX77620_CID4_REG		0x5C
#define MAX77620_CID5_REG		0x5D

#define MAX77621_VOUT_REG		0x00
#define MAX77621_VOUT_DVC_REG		0x01
#define MAX77621_CONTROL1_REG		0x02
#define MAX77621_CONTROL2_REG		0x03

void pmic_init(unsigned bus);
void pmic_write_reg_77620(unsigned bus, uint8_t reg, uint8_t val,
					int delay);

#endif /* __MAINBOARD_GOOGLE_FOSTER_PMIC_H__ */
