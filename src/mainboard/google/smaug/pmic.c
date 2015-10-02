/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
 * Copyright (c) 2013-2015, NVIDIA CORPORATION.  All rights reserved.
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

#include <boardid.h>
#include <console/console.h>
#include <delay.h>
#include <device/i2c.h>
#include <stdint.h>
#include <stdlib.h>

#include "pmic.h"
#include "reset.h"

enum {
	MAX77620_I2C_ADDR = 0x3c,
	MAX77621_CPU_I2C_ADDR = 0x1B,
	MAX77621_GPU_I2C_ADDR = 0x1C,
};

struct max77620_init_reg {
	u8 reg;
	u8 val;
	u8 delay;
};

static struct max77620_init_reg init_list[] = {
	/* TODO */
};

static void pmic_write_reg(unsigned bus, uint8_t chip, uint8_t reg, uint8_t val,
			   int delay)
{
	if (i2c_writeb(bus, chip, reg, val)) {
		printk(BIOS_ERR, "%s: reg = 0x%02X, value = 0x%02X failed!\n",
			__func__, reg, val);
		/* Reset the SoC on any PMIC write error */
		cpu_reset();
	} else {
		if (delay)
			udelay(500);
	}
}

static int pmic_read_reg(unsigned bus, uint8_t chip, uint8_t reg, uint8_t *data)
{
	if (i2c_readb(bus, chip, reg, data)) {
		printk(BIOS_ERR, "%s: reg = 0x%02X read failed\n", __func__,
		       reg);
		return -1;
	}

	return 0;
}

static int pmic_read_reg_77620(unsigned bus, uint8_t reg, uint8_t *data)
{
	return pmic_read_reg(bus, MAX77620_I2C_ADDR, reg, data);
}

void pmic_write_reg_77620(unsigned bus, uint8_t reg, uint8_t val,
					int delay)
{
	pmic_write_reg(bus, MAX77620_I2C_ADDR, reg, val, delay);
}

static inline void pmic_write_reg_77621(unsigned bus, uint8_t reg, uint8_t val,
					int delay)
{
	pmic_write_reg(bus, MAX77621_CPU_I2C_ADDR, reg, val, delay);
}

static void pmic_slam_defaults(unsigned bus)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(init_list); i++) {
		struct max77620_init_reg *reg = &init_list[i];
		pmic_write_reg_77620(bus, reg->reg, reg->val, reg->delay);
	}
}

void pmic_init(unsigned bus)
{
	/* Restore PMIC POR defaults, in case kernel changed 'em */
	pmic_slam_defaults(bus);

	/* MAX77620: Set I2C watchdog timer period to 35.7 ms */
	uint8_t data;
	if (pmic_read_reg_77620(bus, MAX77620_CNFGGLBL2_REG, &data) != 0)
		printk(BIOS_ERR, "PMIC Error: Cannot set PMIC I2CTWD.\n");
	else {
		data &= ~MAX77620_I2CTWD_MASK;
		data |= MAX77620_I2CTWD_35_7_MS;
		pmic_write_reg_77620(bus, MAX77620_CNFGGLBL2_REG, data, 1);
	}

	/* MAX77620: Set SD0 to 1.0V - VDD_CORE */
	pmic_write_reg_77620(bus, MAX77620_SD0_REG, 0x20, 1);
	pmic_write_reg_77620(bus, MAX77620_VDVSSD0_REG, 0x20, 1);

	/* MAX77620: GPIO 0,1,2,5,6,7 = GPIO, 3,4 = alt mode */
	pmic_write_reg_77620(bus, MAX77620_AME_GPIO, 0x18, 1);

	/* MAX77620: Disable SD1 Remote Sense, Set SD1 for LPDDR4 to 1.125V */
	pmic_write_reg_77620(bus, MAX77620_CNFG2SD_REG, 0x04, 1);
	pmic_write_reg_77620(bus, MAX77620_SD1_REG, 0x2a, 1);

	/*
	 * MAX77620: Set LDO2 output to 1.8V. LDO2 is used as always-on
	 * reference for the droop alert circuit. Match this setting with what
	 * the kernel expects.
	 */
	pmic_write_reg_77620(bus, MAX77620_CNFG1_L2_REG, 0x14, 1);

	/*
	 * MAX77620: Set TFPS0 and TFPS1 to 0x7 i.e. 5.120 ms. Required to
	 * ensure that 1.8V rails decay prior to RTC turn off in power down
	 * sequence.
	 */
	if (pmic_read_reg_77620(bus, MAX77620_CNFGFPS0_REG, &data) != 0)
		printk(BIOS_ERR, "PMIC Error: Cannot set PMIC TFPS0.\n");
	else {
		data &= ~MAX77620_TFPS_MASK;
		data |= (0x7 << MAX77620_TFPS_SHIFT);
		pmic_write_reg_77620(bus, MAX77620_CNFGFPS0_REG, data, 1);
	}

	if (pmic_read_reg_77620(bus, MAX77620_CNFGFPS1_REG, &data) != 0)
		printk(BIOS_ERR, "PMIC Error: Cannot set PMIC TFPS1.\n");
	else {
		data &= ~MAX77620_TFPS_MASK;
		data |= (0x7 << MAX77620_TFPS_SHIFT);
		pmic_write_reg_77620(bus, MAX77620_CNFGFPS1_REG, data, 1);
	}

	/* MAX77620: Move RTC to slot 7 */
	pmic_write_reg_77620(bus, MAX77620_FPS_L4_REG, 0x0F, 1);

	/* MAX77620: Mov SOC to slot 7 */
	pmic_write_reg_77620(bus, MAX77620_FPS_SD0_REG, 0x4F, 1);

	/* MAX77620: Move DRAM-1.1V to slot 1 */
	pmic_write_reg_77620(bus, MAX77620_FPS_SD1_REG, 0x29, 1);

	/* MAX77620: Move 1.8V to slot 3 */
	pmic_write_reg_77620(bus, MAX77620_FPS_SD3_REG, 0x1B, 1);

	/* MAX77620: Move 3.3V to slot 2 */
	pmic_write_reg_77620(bus, MAX77620_FPS_GPIO3_REG, 0x22, 1);

	/* MAX77621: Set VOUT_REG to 1.0V - CPU VREG */
	pmic_write_reg_77621(bus, MAX77621_VOUT_REG, 0xBF, 1);

	/* MAX77621: Set VOUT_DVC_REG to 1.0V - CPU VREG DVC */
	pmic_write_reg_77621(bus, MAX77621_VOUT_DVC_REG, 0xBF, 1);

	/* MAX77621: Set CONTROL1 to 0x38 */
	pmic_write_reg_77621(bus, MAX77621_CONTROL1_REG, 0x38, 1);

	/* MAX77621: Set CONTROL2 to 0xD2 */
	pmic_write_reg_77621(bus, MAX77621_CONTROL2_REG, 0xD2, 1);

	/* MAX77620: Setup/Enable GPIO5 - EN_VDD_CPU */
	pmic_write_reg_77620(bus, MAX77620_GPIO5_REG, 0x09, 1);

	/* Required delay of 2msec */
	udelay(2000);

	printk(BIOS_DEBUG, "PMIC init done\n");
}
