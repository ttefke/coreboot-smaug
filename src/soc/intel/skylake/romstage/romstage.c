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

#include <stddef.h>
#include <stdint.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/cbfs.h>
#include <arch/stages.h>
#include <arch/early_variables.h>
#include <cbmem.h>
#include <chip.h>
#include <console/console.h>
#include <cpu/x86/mtrr.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <elog.h>
#include <ramstage_cache.h>
#include <reset.h>
#include <romstage_handoff.h>
#include <soc/pci_devs.h>
#include <soc/pei_wrapper.h>
#include <soc/pm.h>
#include <soc/serialio.h>
#include <soc/romstage.h>
#include <soc/spi.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

/* SOC initialization before the console is enabled */
void soc_pre_console_init(struct romstage_params *params)
{
	/* System Agent Early Initialization */
	systemagent_early_init();
}

/* SOC initialization before RAM is enabled */
void soc_pre_ram_init(struct romstage_params *params)
{
	/* Prepare to initialize memory */
	skylake_fill_pei_data(params->pei_data);
}

void soc_romstage_init(struct romstage_params *params)
{
	pch_early_init();
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
#if IS_ENABLED(CONFIG_RESET_ON_INVALID_RAMSTAGE_CACHE)
	/* Perform hard reset on invalid ramstage cache. */
	hard_reset();
#endif
}

#if IS_ENABLED(CONFIG_CHROMEOS)
int vboot_get_sw_write_protect(void)
{
	u8 status;

	/* Return unprotected status if status read fails. */
	return early_spi_read_wpsr(&status) ? 0 : !!(status & 0x80);
}
#endif

/* UPD parameters to be initialized before MemoryInit */
void soc_memory_init_params(UPD_DATA_REGION *upd_ptr)
{
	const struct device *dev;
	const struct soc_intel_skylake_config *config;
	int i;

	/* Set the parameters for MemoryInit */
	dev = dev_find_slot(0, PCI_DEVFN(PCH_DEV_SLOT_LPC, 0));
	config = dev->chip_info;

	upd_ptr->ProbelessTrace = config->probeless_trace;
	upd_ptr->EnableLan = config->enable_lan;
	upd_ptr->EnableSata = config->enable_sata;
	upd_ptr->SataMode = config->sata_mode;
	upd_ptr->SataSalpSupport = config->sata_salp_support;
	upd_ptr->SataPortsEnable[0] = config->sata_ports_enable;
	upd_ptr->SsicPortEnable = config->ssic_port_enable;
	upd_ptr->EnableTraceHub = config->enable_trace_hub;

	for (i = 0; i < PchSerialIoIndexMax; i++) {
		switch (i) {
		case	PchSerialIoIndexSpi0:
		case	PchSerialIoIndexSpi1:
		case	PchSerialIoIndexUart1:
		case	PchSerialIoIndexI2C2:
		case	PchSerialIoIndexI2C3:
			upd_ptr->SerialIoDevMode[i] = config->sio_disable;
			break;
		case	PchSerialIoIndexI2C0:
		case	PchSerialIoIndexI2C1:
		case	PchSerialIoIndexI2C4:
		case	PchSerialIoIndexI2C5:
		case	PchSerialIoIndexUart0:
		case	PchSerialIoIndexUart2:
			upd_ptr->SerialIoDevMode[i] = config->sio_pci_mode;
			break;
		}
	}
}

void soc_display_memory_init_params(const UPD_DATA_REGION *original,
	UPD_DATA_REGION *upd_ptr)
{
	/* Display the parameters for MemoryInit */
	printk(BIOS_SPEW, "UPD values for MemoryInit:\n");
	soc_display_upd_value("TsegSize", 4, original->TsegSize,
		upd_ptr->TsegSize);
	soc_display_upd_value("IgdDvmt50PreAlloc", 1,
		original->IgdDvmt50PreAlloc, upd_ptr->IgdDvmt50PreAlloc);
	soc_display_upd_value("PrimaryDisplay", 1, original->PrimaryDisplay,
		upd_ptr->PrimaryDisplay);
	soc_display_upd_value("InternalGfx", 1, original->InternalGfx,
		upd_ptr->InternalGfx);
	soc_display_upd_value("MmioSize", 2, original->MmioSize,
		upd_ptr->MmioSize);
	soc_display_upd_value("ApertureSize", 1, original->ApertureSize,
		upd_ptr->ApertureSize);
	soc_display_upd_value("SkipExtGfxScan", 1, original->SkipExtGfxScan,
		upd_ptr->SkipExtGfxScan);
	soc_display_upd_value("ScanExtGfxForLegacyOpRom", 1,
		original->ScanExtGfxForLegacyOpRom,
		upd_ptr->ScanExtGfxForLegacyOpRom);
	soc_display_upd_value("ProbelessTrace", 1, original->ProbelessTrace,
		upd_ptr->ProbelessTrace);
	soc_display_upd_value("Device4Enable", 1, original->Device4Enable,
		upd_ptr->Device4Enable);
	soc_display_upd_value("PlatformMemorySize", 8,
		original->PlatformMemorySize, upd_ptr->PlatformMemorySize);
	soc_display_upd_value("MemorySpdPtr00", 4, original->MemorySpdPtr00,
		upd_ptr->MemorySpdPtr00);
	soc_display_upd_value("MemorySpdPtr01", 4, original->MemorySpdPtr01,
		upd_ptr->MemorySpdPtr01);
	soc_display_upd_value("MemorySpdPtr10", 4, original->MemorySpdPtr10,
		upd_ptr->MemorySpdPtr10);
	soc_display_upd_value("MemorySpdPtr11", 4, original->MemorySpdPtr11,
		upd_ptr->MemorySpdPtr11);
	soc_display_upd_value("MemorySpdDataLen", 2, original->MemorySpdDataLen,
		upd_ptr->MemorySpdDataLen);
	soc_display_upd_value("DqByteMapCh0[0]", 1, original->DqByteMapCh0[0],
		upd_ptr->DqByteMapCh0[0]);
	soc_display_upd_value("DqByteMapCh0[1]", 1, original->DqByteMapCh0[1],
		upd_ptr->DqByteMapCh0[1]);
	soc_display_upd_value("DqByteMapCh0[2]", 1, original->DqByteMapCh0[2],
		upd_ptr->DqByteMapCh0[2]);
	soc_display_upd_value("DqByteMapCh0[3]", 1, original->DqByteMapCh0[3],
		upd_ptr->DqByteMapCh0[3]);
	soc_display_upd_value("DqByteMapCh0[4]", 1, original->DqByteMapCh0[4],
		upd_ptr->DqByteMapCh0[4]);
	soc_display_upd_value("DqByteMapCh0[5]", 1, original->DqByteMapCh0[5],
		upd_ptr->DqByteMapCh0[5]);
	soc_display_upd_value("DqByteMapCh0[6]", 1, original->DqByteMapCh0[6],
		upd_ptr->DqByteMapCh0[6]);
	soc_display_upd_value("DqByteMapCh0[7]", 1, original->DqByteMapCh0[7],
		upd_ptr->DqByteMapCh0[7]);
	soc_display_upd_value("DqByteMapCh0[8]", 1, original->DqByteMapCh0[8],
		upd_ptr->DqByteMapCh0[8]);
	soc_display_upd_value("DqByteMapCh0[9]", 1, original->DqByteMapCh0[9],
		upd_ptr->DqByteMapCh0[9]);
	soc_display_upd_value("DqByteMapCh0[10]", 1, original->DqByteMapCh0[10],
		upd_ptr->DqByteMapCh0[10]);
	soc_display_upd_value("DqByteMapCh0[11]", 1, original->DqByteMapCh0[11],
		upd_ptr->DqByteMapCh0[11]);
	soc_display_upd_value("DqByteMapCh1[0]", 1, original->DqByteMapCh1[0],
		upd_ptr->DqByteMapCh1[0]);
	soc_display_upd_value("DqByteMapCh1[1]", 1, original->DqByteMapCh1[1],
		upd_ptr->DqByteMapCh1[1]);
	soc_display_upd_value("DqByteMapCh1[2]", 1, original->DqByteMapCh1[2],
		upd_ptr->DqByteMapCh1[2]);
	soc_display_upd_value("DqByteMapCh1[3]", 1, original->DqByteMapCh1[3],
		upd_ptr->DqByteMapCh1[3]);
	soc_display_upd_value("DqByteMapCh1[4]", 1, original->DqByteMapCh1[4],
		upd_ptr->DqByteMapCh1[4]);
	soc_display_upd_value("DqByteMapCh1[5]", 1, original->DqByteMapCh1[5],
		upd_ptr->DqByteMapCh1[5]);
	soc_display_upd_value("DqByteMapCh1[6]", 1, original->DqByteMapCh1[6],
		upd_ptr->DqByteMapCh1[6]);
	soc_display_upd_value("DqByteMapCh1[7]", 1, original->DqByteMapCh1[7],
		upd_ptr->DqByteMapCh1[7]);
	soc_display_upd_value("DqByteMapCh1[8]", 1, original->DqByteMapCh1[8],
		upd_ptr->DqByteMapCh1[8]);
	soc_display_upd_value("DqByteMapCh1[9]", 1, original->DqByteMapCh1[9],
		upd_ptr->DqByteMapCh1[9]);
	soc_display_upd_value("DqByteMapCh1[10]", 1, original->DqByteMapCh1[10],
		upd_ptr->DqByteMapCh1[10]);
	soc_display_upd_value("DqByteMapCh1[11]", 1, original->DqByteMapCh1[11],
		upd_ptr->DqByteMapCh1[11]);
	soc_display_upd_value("DqsMapCpu2DramCh0[0]", 1,
		original->DqsMapCpu2DramCh0[0],
		upd_ptr->DqsMapCpu2DramCh0[0]);
	soc_display_upd_value("DqsMapCpu2DramCh0[1]", 1,
		original->DqsMapCpu2DramCh0[1],
		upd_ptr->DqsMapCpu2DramCh0[1]);
	soc_display_upd_value("DqsMapCpu2DramCh0[2]", 1,
		original->DqsMapCpu2DramCh0[2],
		upd_ptr->DqsMapCpu2DramCh0[2]);
	soc_display_upd_value("DqsMapCpu2DramCh0[3]", 1,
		original->DqsMapCpu2DramCh0[3],
		upd_ptr->DqsMapCpu2DramCh0[3]);
	soc_display_upd_value("DqsMapCpu2DramCh0[4]", 1,
		original->DqsMapCpu2DramCh0[4],
		upd_ptr->DqsMapCpu2DramCh0[4]);
	soc_display_upd_value("DqsMapCpu2DramCh0[5]", 1,
		original->DqsMapCpu2DramCh0[5],
		upd_ptr->DqsMapCpu2DramCh0[5]);
	soc_display_upd_value("DqsMapCpu2DramCh0[6]", 1,
		original->DqsMapCpu2DramCh0[6],
		upd_ptr->DqsMapCpu2DramCh0[6]);
	soc_display_upd_value("DqsMapCpu2DramCh0[7]", 1,
		original->DqsMapCpu2DramCh0[7],
		upd_ptr->DqsMapCpu2DramCh0[7]);
	soc_display_upd_value("DqsMapCpu2DramCh1[0]", 1,
		original->DqsMapCpu2DramCh1[0],
		upd_ptr->DqsMapCpu2DramCh1[0]);
	soc_display_upd_value("DqsMapCpu2DramCh1[1]", 1,
		original->DqsMapCpu2DramCh1[1],
		upd_ptr->DqsMapCpu2DramCh1[1]);
	soc_display_upd_value("DqsMapCpu2DramCh1[2]", 1,
		original->DqsMapCpu2DramCh1[2],
		upd_ptr->DqsMapCpu2DramCh1[2]);
	soc_display_upd_value("DqsMapCpu2DramCh1[3]", 1,
		original->DqsMapCpu2DramCh1[3],
		upd_ptr->DqsMapCpu2DramCh1[3]);
	soc_display_upd_value("DqsMapCpu2DramCh1[4]", 1,
		original->DqsMapCpu2DramCh1[4],
		upd_ptr->DqsMapCpu2DramCh1[4]);
	soc_display_upd_value("DqsMapCpu2DramCh1[5]", 1,
		original->DqsMapCpu2DramCh1[5],
		upd_ptr->DqsMapCpu2DramCh1[5]);
	soc_display_upd_value("DqsMapCpu2DramCh1[6]", 1,
		original->DqsMapCpu2DramCh1[6],
		upd_ptr->DqsMapCpu2DramCh1[6]);
	soc_display_upd_value("DqsMapCpu2DramCh1[7]", 1,
		original->DqsMapCpu2DramCh1[7],
		upd_ptr->DqsMapCpu2DramCh1[7]);
	soc_display_upd_value("DqPinsInterleaved", 1,
		original->DqPinsInterleaved, upd_ptr->DqPinsInterleaved);
	soc_display_upd_value("RcompResistor[0]", 2, original->RcompResistor[0],
		upd_ptr->RcompResistor[0]);
	soc_display_upd_value("RcompResistor[1]", 2, original->RcompResistor[1],
		upd_ptr->RcompResistor[1]);
	soc_display_upd_value("RcompResistor[2]", 2, original->RcompResistor[2],
		upd_ptr->RcompResistor[2]);
	soc_display_upd_value("RcompTarget[0]", 1, original->RcompTarget[0],
		upd_ptr->RcompTarget[0]);
	soc_display_upd_value("RcompTarget[1]", 1, original->RcompTarget[1],
		upd_ptr->RcompTarget[1]);
	soc_display_upd_value("RcompTarget[2]", 1, original->RcompTarget[2],
		upd_ptr->RcompTarget[2]);
	soc_display_upd_value("RcompTarget[3]", 1, original->RcompTarget[3],
		upd_ptr->RcompTarget[3]);
	soc_display_upd_value("RcompTarget[4]", 1, original->RcompTarget[4],
		upd_ptr->RcompTarget[4]);
	soc_display_upd_value("CaVrefConfig", 1, original->CaVrefConfig,
		upd_ptr->CaVrefConfig);
	soc_display_upd_value("SmramMask", 1, original->SmramMask,
		upd_ptr->SmramMask);
	soc_display_upd_value("MrcFastBoot", 1, original->MrcFastBoot,
		upd_ptr->MrcFastBoot);
}
