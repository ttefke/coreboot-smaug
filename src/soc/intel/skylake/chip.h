/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2008 coresystems GmbH
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

#include <stdint.h>
#include <soc/pci_devs.h>
#include <soc/serialio.h>

#ifndef _SOC_INTEL_SKYLAKE_CHIP_H_
#define _SOC_INTEL_SKYLAKE_CHIP_H_

struct soc_intel_skylake_config {
	/*
	 * Interrupt Routing configuration
	 * If bit7 is 1, the interrupt is disabled.
	 */
	uint8_t pirqa_routing;
	uint8_t pirqb_routing;
	uint8_t pirqc_routing;
	uint8_t pirqd_routing;
	uint8_t pirqe_routing;
	uint8_t pirqf_routing;
	uint8_t pirqg_routing;
	uint8_t pirqh_routing;

	/* GPE configuration */
	uint32_t gpe0_en_1;
	uint32_t gpe0_en_2;
	uint32_t gpe0_en_3;
	uint32_t gpe0_en_4;

	/* GPIO SMI configuration */
	uint32_t ec_smi_gpio;
	uint32_t alt_gp_smi_en;

	/* Generic IO decode ranges */
	uint32_t gen1_dec;
	uint32_t gen2_dec;
	uint32_t gen3_dec;
	uint32_t gen4_dec;

	/*
	 * SerialIO device mode selection:
	 *
	 * Device index:
	 * PchSerialIoIndexI2C0
	 * PchSerialIoIndexI2C1
	 * PchSerialIoIndexI2C2
	 * PchSerialIoIndexI2C3
	 * PchSerialIoIndexI2C4
	 * PchSerialIoIndexI2C5
	 * PchSerialIoIndexI2C6
	 * PchSerialIoIndexSpi0
	 * PchSerialIoIndexSpi1
	 * PchSerialIoIndexUart0
	 * PchSerialIoIndexUart1
	 * PchSerialIoIndexUart2
	 *
	 * Mode select:
	 * PchSerialIoDisabled
	 * PchSerialIoAcpi
	 * PchSerialIoPci
	 * PchSerialIoAcpiHidden
	 * PchSerialIoLegacyUart
	 */
	u8 sio_device_mode[PchSerialIoIndexMax];

	/* Enable linear PCIe Root Port function numbers starting at zero */
	uint8_t pcie_port_coalesce;

	/* Force root port ASPM configuration with port bitmap */
	uint8_t pcie_port_force_aspm;

	/* Enable ADSP power gating features */
	uint8_t adsp_d3_pg_enable;
	uint8_t adsp_sram_pg_enable;

	/*
	 * Clock Disable Map:
	 * [21:16] = CLKOUT_PCIE# 5-0
	 *    [24] = CLKOUT_ITPXDP
	 */
	uint32_t icc_clock_disable;

	/*
	 * Digital Port Hotplug Enable:
	 *  0x04 = Enabled, 2ms short pulse
	 *  0x05 = Enabled, 4.5ms short pulse
	 *  0x06 = Enabled, 6ms short pulse
	 *  0x07 = Enabled, 100ms short pulse
	 */
	u8 gpu_dp_b_hotplug;
	u8 gpu_dp_c_hotplug;
	u8 gpu_dp_d_hotplug;

	/* Memory related */
	u8 probeless_trace;

	/* Lan */
	u8 enable_lan;

	/* SATA related */
	u8 enable_sata;
	u8 sata_salp_support;
	u8 sata_mode;
	u8 sata_ports_enable;
	u8 ssic_port_enable;

	/* Audio related */
	u8 enable_azalia;
	u8 enable_trace_hub;
	u8 dsp_enable;
	u8 io_buffer_ownership;

	/* SMBUS */
	u8 smbus_enable;

	/* Camera */
	u8 cio2_enable;

	/* eMMC */
	u8 scs_emmc_enabled;
	u8 scs_emmchs400_enabled;
	u8 scs_sdcard_enabled;

	/* Integrated Sensor */
	u8 ish_enable;

	/* Panel power sequence timings */
	u8 gpu_panel_port_select;
	u8 gpu_panel_power_cycle_delay;
	u16 gpu_panel_power_up_delay;
	u16 gpu_panel_power_down_delay;
	u16 gpu_panel_power_backlight_on_delay;
	u16 gpu_panel_power_backlight_off_delay;

	/* Panel backlight settings */
	u32 gpu_cpu_backlight;
	u32 gpu_pch_backlight;

	/*
	 * Graphics CD Clock Frequency
	 * 0 = 337.5MHz
	 * 1 = 450MHz
	 * 2 = 540MHz
	 * 3 = 675MHz
	 */
	int cdclk;

	/* Enable S0iX support */
	int s0ix_enable;

	/* Deep SX enable */
	int deep_sx_enable_ac;
	int deep_sx_enable_dc;

	/* TCC activation offset */
	int tcc_offset;
};

typedef struct soc_intel_skylake_config config_t;

extern struct chip_operations soc_ops;

#endif
