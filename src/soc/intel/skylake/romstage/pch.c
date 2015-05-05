/*
 * This file is part of the coreboot project.
 *
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

#include <arch/io.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <reg_script.h>
#include <soc/iomap.h>
#include <soc/lpc.h>
#include <soc/pch.h>
#include <soc/pcr.h>
#include <soc/pci_devs.h>
#include <soc/pm.h>
#include <soc/romstage.h>
#include <soc/smbus.h>
#include <soc/intel/skylake/chip.h>

/* Max PXRC registers in ITSS*/
#define MAX_PXRC_CONFIG		0x08

static const u8 pch_interrupt_routing[] = {
		11,	/* PARC: PIRQA -> IRQ11 */
		10,	/* PBRC: PIRQB -> IRQ10 */
		11,	/* PCRC: PIRQC -> IRQ11 */
		11,	/* PDRC: PIRQD -> IRQ11 */
		11,	/* PERC: PIRQE -> IRQ11 */
		11,	/* PFRC: PIRQF -> IRQ11 */
		11,	/* PGRC: PIRQG -> IRQ11 */
		11	/* PHRC: PIRQH -> IRQ11 */
};

static void pch_enable_lpc(void)
{
	/* Lookup device tree in romstage */
	const struct device *dev;
	const config_t *config;

	dev = dev_find_slot(0, PCI_DEVFN(PCH_DEV_SLOT_LPC, 0));
	if (!dev || !dev->chip_info)
		return;
	config = dev->chip_info;

	pci_write_config32(PCH_DEV_LPC, LPC_GEN1_DEC, config->gen1_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN2_DEC, config->gen2_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN3_DEC, config->gen3_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN4_DEC, config->gen4_dec);
}

static void pch_device_init(void)
{
	device_t dev;
	u32 reg32 = 0;
	u16 reg16 = 0;
	uint32_t tcobase;
	u16 tcocnt = 0;

	/* Enable ACPI in PMC Config*/
	dev = PCH_DEV_PMC;
	reg32 = pci_read_config32(dev, ACPI_CNTL);
	reg32 |= ACPI_EN;
	pci_write_config32(dev, ACPI_CNTL, reg32);

	/* TCO timer halt */
	dev = PCH_DEV_SMBUS;
	reg16 = pci_read_config16(dev, PCH_SMBUS_TCOBASE);
	tcobase = reg16 & PCH_SMBUS_TCOBASE_BAR;
	tcocnt = inw(tcobase + PCH_TCO1_CNT);
	tcocnt |= TCO_TMR_HLT;
	outw(tcocnt, tcobase + PCH_TCO1_CNT);

	/* Enable upper 128 bytes of CMOS */
	pcr_andthenor32(PID_RTC, R_PCH_PCR_RTC_CONF, (u32)~0,
			B_PCH_PCR_RTC_CONF_UCMOS_EN);
}

static void pch_interrupt_init(void)
{
	u8 index = 0;

	for (index = 0; index < MAX_PXRC_CONFIG; index++) {
		if (pch_interrupt_routing[index] < 16 &&
			pch_interrupt_routing[index] > 2 &&
			pch_interrupt_routing[index] != 8 &&
			pch_interrupt_routing[index] != 13) {
				pcr_write8(PID_ITSS,
					(R_PCH_PCR_ITSS_PIRQA_ROUT + index),
					pch_interrupt_routing[index]);
		}
	}
}

void pch_early_init(void)
{
	pch_device_init();

	pch_interrupt_init();

	pch_enable_lpc();

	enable_smbus();
}
