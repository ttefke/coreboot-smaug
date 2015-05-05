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
#include <bootstate.h>
#include <console/console.h>
#include <console/post_codes.h>
#include <cpu/x86/smm.h>
#include <reg_script.h>
#include <spi-generic.h>
#include <stdlib.h>
#include <soc/pci_devs.h>
#include <soc/pch.h>
#include <soc/pcr.h>
#include <soc/pmc.h>
#include <soc/spi.h>
#include <soc/systemagent.h>
#include <device/pci.h>

static void pch_finalize_script(void)
{
	device_t dev;
	uint32_t reg32, hsfs, tcobase;
	void *spibar = get_spi_bar();
	u8 reg8;
	u16 reg16;
	u16 tcocnt = 0;
	uint32_t pmcbase;
	u32 pmsyncreg = 0;

	/* Set SPI opcode menu */
	write16(spibar + SPIBAR_PREOP, SPI_OPPREFIX);
	write16(spibar + SPIBAR_OPTYPE, SPI_OPTYPE);
	write32(spibar + SPIBAR_OPMENU_LOWER, SPI_OPMENU_LOWER);
	write32(spibar + SPIBAR_OPMENU_UPPER, SPI_OPMENU_UPPER);
	/* Lock SPIBAR */
	hsfs = read32(spibar + SPIBAR_HSFS);
	hsfs |= SPIBAR_HSFS_FLOCKDN;
	write32(spibar + SPIBAR_HSFS, hsfs);

	/*TCO Lock down*/
	dev = PCH_DEV_SMBUS;
	reg16 = pci_read_config16(dev, PCH_SMBUS_TCOBASE);
	tcobase = reg16 & PCH_SMBUS_TCOBASE_BAR;
	tcocnt = inw(tcobase + PCH_TCO1_CNT);
	tcocnt |= PCH_TCO_LOCK;
	outw(tcocnt, tcobase + PCH_TCO1_CNT);

	/*Global SMI Lock*/
	/*PMC Controller Device 0x1F, Func 02*/
	dev = PCH_DEV_PMC;
	reg8 = pci_read_config8(dev, GEN_PMCON_A_1);
	reg8 |= SMI_LOCK;
	pci_write_config8(dev, GEN_PMCON_A_1, reg8);

	/*GEN_PMCON Lock*/
	reg8 = pci_read_config8(dev, GEN_PMCON_LOCK_B_2);
	reg8 |= (SLP_STR_POL_LOCK | ACPI_BASE_LOCK);
	pci_write_config8(dev, GEN_PMCON_LOCK_B_2, reg8);

	/* PMSYNC */
	dev = PCH_DEV_PMC;
	reg32 = pci_read_config32(dev, PCH_MBASE);
	pmcbase = reg32 & B_PCH_PMC_BAR0_MASK;
	pmsyncreg = read32(&pmcbase + PCH_PWRM_PMSYNC_TPR_CONFIG);
	pmsyncreg |= PMSYNC_LOCK;
	write32(&pmcbase + PCH_PWRM_PMSYNC_TPR_CONFIG, pmsyncreg);
}

static void skylake_finalize(void *unused)
{
	printk(BIOS_DEBUG, "Finalizing chipset.\n");
	pch_finalize_script();

	/* Indicate finalize step with post code */
	post_code(POST_OS_BOOT);
}

BOOT_STATE_INIT_ENTRIES(finalize) = {
	BOOT_STATE_INIT_ENTRY(BS_OS_RESUME, BS_ON_ENTRY,
			      skylake_finalize, NULL),
	BOOT_STATE_INIT_ENTRY(BS_PAYLOAD_LOAD, BS_ON_EXIT,
			      skylake_finalize, NULL),
};
