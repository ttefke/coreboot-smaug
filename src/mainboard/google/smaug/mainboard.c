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

#include <arch/io.h>
#include <arch/mmu.h>
#include <boardid.h>
#include <boot/coreboot_tables.h>
#include <device/device.h>
#include <device/i2c.h>
#include <elog.h>
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/clock.h>
#include <soc/funitcfg.h>
#include <soc/padconfig.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/nvidia/tegra/pingroup.h>
#include <soc/nvidia/tegra/dc.h>
#include <soc/display.h>
#include <soc/mtc.h>
#include <soc/pmc.h>
#include <soc/power.h>

#include <vboot_struct.h>
#include <vendorcode/google/chromeos/vboot_handoff.h>
#include <vendorcode/google/chromeos/vboot2/misc.h>
#include <delay.h>

#include "gpio.h"
#include "pmic.h"

static const struct pad_config evt_padcfgs[] = {
	PAD_CFG_GPIO_INPUT(GPIO_PZ2, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(SPI2_MOSI, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(GPIO_PE7, PINMUX_PULL_UP),
};

static const struct pad_config dvt_padcfgs[] = {
	PAD_CFG_GPIO_INPUT(GPIO_PZ2, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(SPI2_MOSI, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(GPIO_PE7, PINMUX_PULL_NONE),
};

static const struct pad_config additional_padcfgs[] = {

	PAD_CFG_UNUSED_WITH_RES(PEX_L0_RST_N, RES1),
	PAD_CFG_UNUSED_WITH_RES(PEX_L0_CLKREQ_N, RES1),
	PAD_CFG_UNUSED_WITH_RES(PEX_WAKE_N, RES1),
	PAD_CFG_UNUSED_WITH_RES(PEX_L1_RST_N, RES1),
	PAD_CFG_UNUSED_WITH_RES(PEX_L1_CLKREQ_N, RES1),
	PAD_CFG_UNUSED_WITH_RES(SATA_LED_ACTIVE, RES1),

	PAD_CFG_GPIO_INPUT(GPIO_PA6, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP |
			   PINMUX_GPIO_PA6_FUNC_RES1),

	/* DAP1_FS - already set in i2s1_pad. */
	/* DAP1_DIN - already set in i2s1_pad. */
	/* DAP1_DOUT - already set in i2s1_pad. */
	/* DAP1_SCLK - already set in i2s1_pad. */

	PAD_CFG_UNUSED_WITH_RES(SPI2_MISO, RES2),
	PAD_CFG_UNUSED_WITH_RES(SPI2_SCK, RES2),
	PAD_CFG_UNUSED_WITH_RES(SPI2_CS0, RES2),
	PAD_CFG_UNUSED_WITH_RES(SPI2_CS1, RES1),

	PAD_CFG_SFIO(SPI1_MOSI, PINMUX_PULL_NONE, SPI1),
	PAD_CFG_SFIO(SPI1_MISO, PINMUX_INPUT_ENABLE, SPI1),
	PAD_CFG_SFIO(SPI1_SCK, PINMUX_PULL_NONE, SPI1),
	PAD_CFG_SFIO(SPI1_CS0, PINMUX_PULL_NONE, SPI1),
	PAD_CFG_UNUSED_WITH_RES(SPI1_CS1, RES1),

	PAD_CFG_UNUSED_WITH_RES(SPI4_SCK, RES1),
	PAD_CFG_UNUSED_WITH_RES(SPI4_CS0, RES1),
	PAD_CFG_UNUSED_WITH_RES(SPI4_MOSI, RES1),
	PAD_CFG_UNUSED_WITH_RES(SPI4_MISO, RES1),

	PAD_CFG_SFIO(UART3_TX, PINMUX_PULL_NONE, UARTC),
	PAD_CFG_SFIO(UART3_RX, PINMUX_INPUT_ENABLE, UARTC),
	PAD_CFG_SFIO(UART3_RTS, PINMUX_PULL_NONE, UARTC),
	PAD_CFG_SFIO(UART3_CTS, PINMUX_INPUT_ENABLE, UARTC),

	PAD_CFG_GPIO_INPUT(DMIC1_CLK, PINMUX_PULL_NONE),
	/* DMIC1_DAT - already set in audio_codec_pads. */
	PAD_CFG_GPIO_INPUT(DMIC2_CLK, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(DMIC2_DAT, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(DMIC3_CLK, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED_WITH_RES(DMIC3_DAT, RES2),

	PAD_CFG_GPIO_INPUT(GPIO_PE6, PINMUX_PULL_UP),

	/* GEN3_I2C_SCL - already set in tpm_pads. */
	/* GEN3_I2C_SDA - already set in tpm_pads. */

	PAD_CFG_UNUSED_WITH_RES(UART2_TX, UARTB),
	PAD_CFG_UNUSED_WITH_RES(UART2_RX, UARTB),
	PAD_CFG_UNUSED_WITH_RES(UART2_RTS, RES2),
	PAD_CFG_GPIO_OUT0(UART2_CTS, PINMUX_PULL_NONE),

	PAD_CFG_GPIO_OUT0(WIFI_EN, PINMUX_PULL_UP),
	PAD_CFG_GPIO_OUT0(WIFI_RST, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(WIFI_WAKE_AP, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_OUT0(AP_WAKE_BT, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_OUT0(BT_RST, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(BT_WAKE_AP, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(GPIO_PH6, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED(AP_WAKE_NFC),
	PAD_CFG_UNUSED(NFC_EN),
	PAD_CFG_UNUSED(NFC_INT),
	PAD_CFG_GPIO_OUT0(GPS_EN, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED(GPS_RST),

	PAD_CFG_SFIO(UART4_TX, PINMUX_PULL_NONE, UARTD),
	PAD_CFG_SFIO(UART4_RX, PINMUX_INPUT_ENABLE | PINMUX_PULL_NONE, UARTD),
	PAD_CFG_SFIO(UART4_RTS, PINMUX_PULL_NONE, UARTD),
	PAD_CFG_SFIO(UART4_CTS, PINMUX_INPUT_ENABLE | PINMUX_PULL_NONE, UARTD),

	PAD_CFG_SFIO(GEN1_I2C_SCL, PINMUX_INPUT_ENABLE, I2C1),
	PAD_CFG_SFIO(GEN1_I2C_SDA, PINMUX_INPUT_ENABLE, I2C1),

	/* GEN2_I2C_SCL - Already set in ec_i2c_pads. */
	/* GEN2_I2C_SDA - Already set in ec_i2c_pads. */

	PAD_CFG_UNUSED_WITH_RES(DAP4_FS, RES1),
	PAD_CFG_UNUSED_WITH_RES(DAP4_DIN, RES1),
	PAD_CFG_UNUSED_WITH_RES(DAP4_DOUT, RES1),
	PAD_CFG_UNUSED_WITH_RES(DAP4_SCLK, RES1),

	/* GPIO_PK0 - Already set in padcfgs in bootblock. */
	/* GPIO_PK1 - Already set in padcfgs in bootblock. */
	PAD_CFG_GPIO_INPUT(GPIO_PK2, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_OUT0(GPIO_PK3, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PK4, RES1),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PK5, RES1),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PK6, RES1),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PK7, RES1),

	PAD_CFG_UNUSED_WITH_RES(GPIO_PL0, RES0),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PL1, RES1),

	PAD_CFG_UNUSED_WITH_RES(SDMMC1_CLK, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC1_CMD, RES2),
	/* SDMMC1_DAT0 - already set in padcfgs in romstage. */
	/* SDMMC1_DAT1 - already set in padcfgs in romstage. */
	PAD_CFG_UNUSED_WITH_RES(SDMMC1_DAT2, RES2),
	PAD_CFG_UNUSED_WITH_RES(SDMMC1_DAT3, RES2),

	PAD_CFG_UNUSED_WITH_RES(SDMMC3_CLK, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC3_CMD, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC3_DAT0, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC3_DAT1, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC3_DAT2, RES1),
	PAD_CFG_UNUSED_WITH_RES(SDMMC3_DAT3, RES1),

	PAD_CFG_SFIO(CAM1_MCLK, PINMUX_PULL_NONE, EXTPERIPH3),
	PAD_CFG_SFIO(CAM2_MCLK, PINMUX_PULL_NONE, EXTPERIPH3),
	/* CAM_I2C_SCL - already set in cam_i2c_pads. */
	/* CAM_I2C_SDA - already set in cam_i2c_pads. */
	PAD_CFG_UNUSED_WITH_RES(CAM_RST, RES1),
	PAD_CFG_GPIO_OUT0(CAM_AF_EN, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED_WITH_RES(CAM_FLASH_EN, RES2),
	PAD_CFG_GPIO_OUT0(CAM1_PWDN, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_OUT0(CAM2_PWDN, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED_WITH_RES(CAM1_STROBE, RES1),

	/* UART1_TX - Already set in uart_console_pads. */
	/* UART1_RX - Already set in uart_console_pads. */
	PAD_CFG_UNUSED_WITH_RES(UART1_RTS, RES1),
	PAD_CFG_UNUSED_WITH_RES(UART1_CTS, RES1),

	PAD_CFG_UNUSED_WITH_RES(LCD_BL_PWM, RES3),
	/* LCD_BL_EN - already set in lcd_gpio_padcfgs. */
	/* LCD_RST - already set in lcd_gpio_padcfgs. */
	/* LCD_GPIO1 - already set in lcd_gpio_padcfgs. */
	/* LCD_GPIO2 - already set in lcd_gpio_padcfgs. */
	PAD_CFG_UNUSED_WITH_RES(AP_READY, RES0),

	PAD_CFG_GPIO_OUT0(TOUCH_RST, PINMUX_PULL_NONE),
	PAD_CFG_SFIO(TOUCH_CLK, PINMUX_PULL_NONE, TOUCH),
	PAD_CFG_GPIO_INPUT(MODEM_WAKE_AP, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(TOUCH_INT, PINMUX_PULL_NONE),
	PAD_CFG_UNUSED_WITH_RES(MOTION_INT, RES0),
	PAD_CFG_UNUSED_WITH_RES(ALS_PROX_INT, RES0),

	PAD_CFG_GPIO_INPUT(TEMP_ALERT, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP),

	PAD_CFG_GPIO_INPUT(BUTTON_POWER_ON, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(BUTTON_VOL_UP, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(BUTTON_VOL_DOWN, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(BUTTON_SLIDE_SW, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(BUTTON_HOME, PINMUX_PULL_UP),

	PAD_CFG_SFIO(LCD_TE, PINMUX_INPUT_ENABLE, DISPLAYA),

	/* PWR_I2C_SCL - already set in pmic_pads. */
	/* PWR_I2C_SDA - already set in pmic_pads. */

	PAD_CFG_SFIO(CLK_32K_OUT, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SOC),

	PAD_CFG_GPIO_INPUT(GPIO_PZ0, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(GPIO_PZ1, PINMUX_PULL_UP),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PZ3, RES1),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PZ4, RES1),
	PAD_CFG_UNUSED_WITH_RES(GPIO_PZ5, RES1),

	PAD_CFG_SFIO(DAP2_FS, PINMUX_PULL_NONE, I2S2),
	PAD_CFG_SFIO(DAP2_DIN, PINMUX_INPUT_ENABLE, I2S2),
	PAD_CFG_SFIO(DAP2_DOUT, PINMUX_PULL_NONE, I2S2),
	PAD_CFG_SFIO(DAP2_SCLK, PINMUX_PULL_NONE, I2S2),

	/* AUD_MCLK - already set in audio_codec_pads. */
	PAD_CFG_UNUSED_WITH_RES(DVFS_PWM, RES0),
	PAD_CFG_UNUSED_WITH_RES(DVFS_CLK, RES0),
	/* GPIO_X1_AUD - already set in audio_codec_pads. */
	PAD_CFG_UNUSED_WITH_RES(GPIO_X3_AUD, RES0),

	PAD_CFG_UNUSED_WITH_RES(HDMI_CEC, RES1),
	/* HPD pins */
	PAD_CFG_SFIO(HDMI_INT_DP_HPD, PINMUX_PULL_NONE | PINMUX_INPUT_ENABLE, DP),
	PAD_CFG_SFIO(DP_HPD0, PINMUX_PULL_DOWN | PINMUX_TRISTATE, RES1),

	PAD_CFG_UNUSED_WITH_RES(SPDIF_OUT, RES1),
	PAD_CFG_UNUSED_WITH_RES(SPDIF_IN, RES1),
	PAD_CFG_UNUSED_WITH_RES(USB_VBUS_EN0, RES1),
	PAD_CFG_UNUSED_WITH_RES(USB_VBUS_EN1, RES1),

	PAD_CFG_GPIO_OUT0(GPIO_PCC7, PINMUX_PULL_DOWN | PINMUX_TRISTATE),

	/* QSPI_IO0 - Already set in spiflash_pads. */
	/* QSPI_IO1 - Already set in spiflash_pads. */
	/* QSPI_SCK - Already set in spiflash_pads. */
	/* QSPI_CS_N - Already set in spiflash_pads. */
	/* QSPI_IO2 - Already set in spiflash_pads. */
	/* QSPI_IO3 - Already set in spiflash_pads. */

	PAD_CFG_SFIO(CORE_PWR_REQ, PINMUX_PULL_NONE, PWRON),
	PAD_CFG_SFIO(CPU_PWR_REQ, PINMUX_PULL_NONE, CPU),
	PAD_CFG_SFIO(PWR_INT_N, PINMUX_PULL_UP | PINMUX_INPUT_ENABLE, PMI),
	PAD_CFG_SFIO(CLK_32K_IN, PINMUX_INPUT_ENABLE | PINMUX_PULL_NONE,
		     CLK_32K_IN),
	PAD_CFG_SFIO(JTAG_RTCK, PINMUX_PULL_UP, JTAG),
	PAD_CFG_UNUSED_WITH_RES(CLK_REQ, RES1),
	PAD_CFG_SFIO(SHUTDOWN, PINMUX_PULL_NONE, SHUTDOWN),
};

static const struct pad_config audio_codec_pads[] = {
	/* GPIO_X1_AUD(BB3) is CODEC_RST_L and DMIC1_DAT(E1) is AUDIO_ENABLE */
	PAD_CFG_GPIO_OUT1(GPIO_X1_AUD, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_OUT1(DMIC1_DAT, PINMUX_PULL_NONE),
};

static const struct pad_config i2s1_pad[] = {
	/* I2S1 */
	PAD_CFG_SFIO(DAP1_FS, PINMUX_PULL_NONE, I2S1),
	PAD_CFG_SFIO(DAP1_DIN, PINMUX_INPUT_ENABLE, I2S1),
	PAD_CFG_SFIO(DAP1_DOUT, PINMUX_PULL_NONE, I2S1),
	PAD_CFG_SFIO(DAP1_SCLK, PINMUX_PULL_NONE, I2S1),

	/* codec MCLK via AUD SFIO */
	PAD_CFG_SFIO(AUD_MCLK, PINMUX_PULL_NONE, AUD),
};

static const struct funit_cfg audio_funit[] = {
	/* We need 1.5MHz for I2S1. So we use CLK_M */
	FUNIT_CFG(I2S1, CLK_M, 1500, i2s1_pad, ARRAY_SIZE(i2s1_pad)),
};

static const struct funit_cfg funits[] = {
	FUNIT_CFG_USB(USBD),
	FUNIT_CFG(SDMMC4, PLLP, 48000, NULL, 0),
	/* I2C6 for audio, temp sensor, etc. Enable codec via GPIOs/muxes */
	FUNIT_CFG(I2C6, PLLP, 400, audio_codec_pads, ARRAY_SIZE(audio_codec_pads)),
};

/* thermal config */
static void setup_thermal(void)
{
	/*
	 * Set remote temperature THERM limit to 105degC.  This thermistor
	 * resides inside the SOC
	 */
	i2c_writeb(I2C6_BUS, 0x4c, 0x19, 105);
	/*
	 * Set local temperature THERM limit to 60degC.  This thermistor resides
	 * within the TMP451 and approximates the board temperature
	 */
	i2c_writeb(I2C6_BUS, 0x4c, 0x20, 60);
}

/* Audio init: clocks and enables/resets */
static void setup_audio(void)
{
	/* Audio codec (RT5677) uses 12MHz CLK1/EXTPERIPH1 */
	clock_configure_source(extperiph1, PLLP, 12000);

	/* Configure AUD_MCLK pad drive strength */
	write32(TEGRA_APB_MISC_GP_REGS(AUD_MCLK),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));

	/* Set up audio peripheral clocks/muxes */
	soc_configure_funits(audio_funit, ARRAY_SIZE(audio_funit));

	/* Enable CLK1_OUT */
	clock_external_output(1);

	/*
	 * As per NVIDIA hardware team, we need to take ALL audio devices
	 * connected to AHUB (AUDIO, APB2APE, I2S, SPDIF, etc.) out of reset
	 * and clock-enabled, otherwise reading AHUB devices (in our case,
	 * I2S/APBIF/AUDIO<XBAR>) will hang.
	 */
	soc_configure_ape();
	clock_enable_audio();
}

static const struct pad_config lcd_gpio_padcfgs[] = {
	/* LCD_EN */
	PAD_CFG_GPIO_OUT0(LCD_BL_EN, PINMUX_PULL_NONE),
	/* LCD_RST_L */
	PAD_CFG_GPIO_OUT0(LCD_RST, PINMUX_PULL_NONE),
	/* EN_VDD_LCD */
	PAD_CFG_GPIO_OUT0(LCD_GPIO2, PINMUX_PULL_NONE),
	/* EN_VDD18_LCD */
	PAD_CFG_GPIO_OUT0(LCD_GPIO1, PINMUX_PULL_NONE),
};

static void configure_display_clocks(void)
{
	u32 lclks = CLK_L_HOST1X | CLK_L_DISP1;	/* dc */
	u32 hclks = CLK_H_MIPI_CAL | CLK_H_DSI;	/* mipi phy, mipi-dsi a */
	u32 uclks = CLK_U_DSIB;			/* mipi-dsi b */
	u32 xclks = CLK_X_UART_FST_MIPI_CAL;	/* uart_fst_mipi_cal */

	clock_enable_clear_reset(lclks, hclks, uclks, 0, 0, xclks, 0);

	/* Give clocks time to stabilize. */
	udelay(IO_STABILIZATION_DELAY);

	/* CLK72MHZ_CLK_SRC */
	clock_configure_source(uart_fst_mipi_cal, PLLP_OUT3, 68000);
}

static int enable_lcd_vdd(void)
{
	/* Set 1.20V to power AVDD_DSI_CSI */
	/* LD0: 1.20v CNF1: 0x0d */
	pmic_write_reg_77620(I2CPWR_BUS, MAX77620_CNFG1_L0_REG, 0xd0, 1);

	/* Enable VDD_LCD */
	gpio_set(EN_VDD_LCD, 1);
	/* wait for 4ms */
	mdelay(4);

	/* Enable PP1800_LCDIO to panel */
	gpio_set(EN_VDD18_LCD, 1);
	/* wait for 2ms */
	mdelay(2);

	/* Set panel EN and RST signals */
	gpio_set(LCD_EN, 1);		/* enable */
	/* wait for min 10ms */
	mdelay(10);
	gpio_set(LCD_RST_L, 1);		/* clear reset */
	/* wait for min 3ms */
	mdelay(3);

	return 0;
}

static int configure_display_blocks(void)
{
	/* enable display related clocks */
	configure_display_clocks();

	/* configure panel gpio pads */
	soc_configure_pads(lcd_gpio_padcfgs, ARRAY_SIZE(lcd_gpio_padcfgs));

	/* set and enable panel related vdd */
	if (enable_lcd_vdd())
		return -1;

	return 0;
}

static void powergate_unused_partitions(void)
{
	static const uint32_t partitions[] = {
		POWER_PARTID_PCX,
		POWER_PARTID_SAX,
		POWER_PARTID_XUSBA,
		POWER_PARTID_XUSBB,
		POWER_PARTID_XUSBC,
		POWER_PARTID_NVDEC,
		POWER_PARTID_NVJPG,
		POWER_PARTID_DFD,
		POWER_PARTID_VE2,
	};

	int i;
	for (i = 0; i < ARRAY_SIZE(partitions); i++)
		power_gate_partition(partitions[i]);
}

static void set_touch_and_camera_pads(void)
{
	/* Configure CAMERA pad drive strength */
	write32(TEGRA_APB_MISC_GP_REGS(CAM1_MCLK),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	write32(TEGRA_APB_MISC_GP_REGS(CAM2_MCLK),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	write32(TEGRA_APB_MISC_GP_REGS(CAM_AF_EN),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	write32(TEGRA_APB_MISC_GP_REGS(CAM_FLASH_EN),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	write32(TEGRA_APB_MISC_GP_REGS(GPIO_PZ0),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	write32(TEGRA_APB_MISC_GP_REGS(GPIO_PZ1),
		(0x10 << PINGROUP_DRVUP_SHIFT | 0x10 << PINGROUP_DRVDN_SHIFT));
	/* Configure TOUCH pad drive strength */
	write32(TEGRA_APB_MISC_GP_REGS(TOUCH_CLK),
		(0x1F << PINGROUP_DRVUP_SHIFT | 0x1F << PINGROUP_DRVDN_SHIFT));
}

static void low_power_sdmmc_pads(void)
{
	u32 lclks = CLK_L_SDMMC1;
	u32 uclks = CLK_U_SDMMC3;

	/*
	 * SDMMC1/SDMMC3, although unused on Smaug, need some
	 * bits cleared to reduce leakage.
	 */
	clock_enable_clear_reset(lclks, 0, uclks, 0, 0, 0, 0);
	clrbits_le32((void *)TEGRA_SDMMC1_BASE + VENDOR_IO_TRIM_CNTRL, SEL_VREG);
	clrbits_le32((void *)TEGRA_SDMMC3_BASE + VENDOR_IO_TRIM_CNTRL, SEL_VREG);
	clrbits_le32((void *)TEGRA_SDMMC1_BASE + SDMEMCOMPPADCTRL, PAD_E_INPUT);
	clrbits_le32((void *)TEGRA_SDMMC3_BASE + SDMEMCOMPPADCTRL, PAD_E_INPUT);
	/* Re-read the last regs to ensure the write goes thru before disabling clocks */
	read32((void *)TEGRA_SDMMC1_BASE + SDMEMCOMPPADCTRL);
	read32((void *)TEGRA_SDMMC3_BASE + SDMEMCOMPPADCTRL);

	/*
	 * Disable SDMMC1/3 clocks, but do NOT place back in reset,
	 * or the bits will revert to POR settings.
	 */
	clock_disable(lclks, 0, uclks, 0, 0, 0, 0);
}

static void mainboard_init(device_t dev)
{
	soc_configure_pads(additional_padcfgs, ARRAY_SIZE(additional_padcfgs));

	if (board_id() <= BOARD_ID_EVT2)
		soc_configure_pads(evt_padcfgs, ARRAY_SIZE(evt_padcfgs));
	else
		soc_configure_pads(dvt_padcfgs, ARRAY_SIZE(dvt_padcfgs));

	soc_configure_funits(funits, ARRAY_SIZE(funits));
	set_touch_and_camera_pads();
	low_power_sdmmc_pads();

	/* I2C6 bus (audio, etc.) */
	soc_configure_i2c6pad();
	i2c_init(I2C6_BUS);
	setup_thermal();
	setup_audio();

	/* if panel needs to bringup */
	if (!vboot_skip_display_init())
		configure_display_blocks();

	powergate_unused_partitions();

	elog_init();
	elog_add_boot_reason();
}

void display_startup(device_t dev)
{
	dsi_display_startup(dev);
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name   = "smaug",
	.enable_dev = mainboard_enable,
};

void lb_board(struct lb_header *header)
{
	lb_table_add_serialno_from_vpd(header);
	soc_add_mtc(header);
}
