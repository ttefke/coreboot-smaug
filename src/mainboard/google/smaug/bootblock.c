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

#include <2sha.h>
#include <2struct.h>
#include <arch/io.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <delay.h>
#include <device/i2c.h>
#include <fmap.h>
#include <gbb_header.h>
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/clock.h>
#include <soc/funitcfg.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/padconfig.h>
#include <soc/pmc.h>
#include <soc/power.h>
#include <soc/spi.h>
#include <string.h>

#include "pmic.h"

/********************** PMIC **********************************/
static const struct pad_config pmic_pads[] = {
	PAD_CFG_SFIO(PWR_I2C_SCL, PINMUX_INPUT_ENABLE, I2CPMU),
	PAD_CFG_SFIO(PWR_I2C_SDA, PINMUX_INPUT_ENABLE, I2CPMU),
};

/********************** SPI Flash *****************************/
static const struct pad_config spiflash_pads[] = {
	/* QSPI fLash: mosi, miso, clk, cs0, hold, wp  */
	PAD_CFG_SFIO(QSPI_IO0, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP |
		     PINMUX_DRIVE_2X, QSPI),
	PAD_CFG_SFIO(QSPI_IO1, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP |
		     PINMUX_DRIVE_2X, QSPI),
	PAD_CFG_SFIO(QSPI_SCK, PINMUX_INPUT_ENABLE | PINMUX_DRIVE_2X, QSPI),
	PAD_CFG_SFIO(QSPI_CS_N, PINMUX_INPUT_ENABLE | PINMUX_DRIVE_2X, QSPI),
	PAD_CFG_SFIO(QSPI_IO2, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP |
		     PINMUX_DRIVE_2X, QSPI),
	PAD_CFG_SFIO(QSPI_IO3, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP |
		     PINMUX_DRIVE_2X, QSPI),
};

/********************* TPM ************************************/
static const struct pad_config tpm_pads[] = {
	PAD_CFG_SFIO(GEN3_I2C_SCL, PINMUX_INPUT_ENABLE, I2C3),
	PAD_CFG_SFIO(GEN3_I2C_SDA, PINMUX_INPUT_ENABLE, I2C3),
};

/********************* EC *************************************/
static const struct pad_config ec_i2c_pads[] = {
	PAD_CFG_SFIO(GEN2_I2C_SCL, PINMUX_INPUT_ENABLE, I2C2),
	PAD_CFG_SFIO(GEN2_I2C_SDA, PINMUX_INPUT_ENABLE, I2C2),
};

/********************* Funits *********************************/
static const struct funit_cfg funits[] = {
	/* PMIC on I2C5 (PWR_I2C* pads) at 400kHz. */
	FUNIT_CFG(I2C5, PLLP, 400, pmic_pads, ARRAY_SIZE(pmic_pads)),
	/* SPI flash at 24MHz on QSPI controller. */
	FUNIT_CFG(QSPI, PLLP, 24000, spiflash_pads, ARRAY_SIZE(spiflash_pads)),
	/* TPM on I2C3  @ 400kHz */
	FUNIT_CFG(I2C3, PLLP, 400, tpm_pads, ARRAY_SIZE(tpm_pads)),
	/* EC on I2C2 - pulled to 3.3V @ 100kHz */
	FUNIT_CFG(I2C2, PLLP, 100, ec_i2c_pads, ARRAY_SIZE(ec_i2c_pads)),
};

/********************* UART ***********************************/
static const struct pad_config uart_console_pads[] = {
	/* UARTA: tx, rx, rts, cts */
	PAD_CFG_SFIO(UART1_TX, PINMUX_PULL_NONE, UARTA),
	PAD_CFG_SFIO(UART1_RX, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, UARTA),
	PAD_CFG_SFIO(UART1_RTS, PINMUX_PULL_UP, UARTA),
	PAD_CFG_SFIO(UART1_CTS, PINMUX_PULL_UP, UARTA),
};

/********************* CAM ************************************/
/* Avoid interference with TPM on GEN3_I2C */
static const struct pad_config cam_i2c_pads[] = {
	PAD_CFG_SFIO(CAM_I2C_SCL, PINMUX_PULL_NONE, I2CVI),
	PAD_CFG_SFIO(CAM_I2C_SDA, PINMUX_PULL_NONE, I2CVI),
};

void bootblock_mainboard_early_init(void)
{
	soc_configure_pads(cam_i2c_pads, ARRAY_SIZE(cam_i2c_pads));
	soc_configure_pads(uart_console_pads, ARRAY_SIZE(uart_console_pads));
}

static void set_clock_sources(void)
{
	/* UARTA gets PLLP, deactivate CLK_UART_DIV_OVERRIDE */
	write32(CLK_RST_REG(clk_src_uarta), PLLP << CLK_SOURCE_SHIFT);
}

/********************* PADs ***********************************/
static const struct pad_config padcfgs[] = {
	/* Board build id bits 1:0 */
	PAD_CFG_GPIO_INPUT(GPIO_PK1, PINMUX_PULL_NONE),
	PAD_CFG_GPIO_INPUT(GPIO_PK0, PINMUX_PULL_NONE),
};

static const struct vb2_ryu_root_key_hash root_key_hash = {
	.magic = RYU_ROOT_KEY_HASH_MAGIC,
	.header_version_major = RYU_ROOT_KEY_HASH_VERSION_MAJOR,
	.header_version_minor = RYU_ROOT_KEY_HASH_VERSION_MINOR,
	.struct_size = EXPECTED_VB2_RYU_ROOT_KEY_HASH_SIZE,
	/*
	 * root_key_hash_digest field is filled in by an external entity with
	 * the hash of root key. It is used to confirm that the hash on the root
	 * key stored in GBB is the same as the hash stored in this field by the
	 * external signer entity.
	 */
	.root_key_hash_digest = {0},
};

static int valid_ryu_root_header(const struct vb2_ryu_root_key_hash *hash,
				 size_t size)
{
	if (memcmp(hash->magic, RYU_ROOT_KEY_HASH_MAGIC,
		   RYU_ROOT_KEY_HASH_MAGIC_SIZE))
		return 0;

	if (hash->struct_size > size)
		return 0;

	return 1;
}

static int verify_root_key(void)
{
	if (!valid_ryu_root_header(&root_key_hash, sizeof(root_key_hash)))
		return 0;

	uintptr_t gbb_addr;
	size_t gbb_size;

	gbb_size = find_fmap_entry("GBB", (void **)&gbb_addr);

	printk(BIOS_ERR, "gbb_addr: %lx\n", (unsigned long)gbb_addr);

	const GoogleBinaryBlockHeader *gbb_header;
	struct cbfs_media media;

	if (init_default_cbfs_media(&media)) {
		printk(BIOS_ERR, "Failed to init default cbfs media\n");
		return 0;
	}

	media.open(&media);
	gbb_header = media.map(&media, gbb_addr,
			       sizeof(GoogleBinaryBlockHeader));

	if (gbb_header == CBFS_MEDIA_INVALID_MAP_ADDRESS) {
		printk(BIOS_ERR, "Media map failed\n");
		media.close(&media);
		return 0;
	}

	if (memcmp(gbb_header->signature, "$GBB", GBB_SIGNATURE_SIZE)) {
		printk(BIOS_ERR, "GBB Signature mismatch\n");
		media.unmap(&media, gbb_header);
		media.close(&media);
		return 0;
	}

	uintptr_t rootkey_offset = gbb_header->rootkey_offset;
	size_t rootkey_size = gbb_header->rootkey_size;
	void *rootkey;

	media.unmap(&media, gbb_header);
	gbb_header = NULL;

	rootkey = media.map(&media, gbb_addr + rootkey_offset, rootkey_size);

	if (rootkey == CBFS_MEDIA_INVALID_MAP_ADDRESS) {
		printk(BIOS_ERR, "Media map failed\n");
		media.close(&media);
		return 0;
	}

	printk(BIOS_ERR, "Root key found! ");
	uint8_t digest[VB2_SHA256_DIGEST_SIZE] = {0};
	if (vb2_digest_buffer(rootkey, rootkey_size, VB2_HASH_SHA256, digest,
			      sizeof(digest))) {
		printk(BIOS_ERR, "Root hash calculation failed\n");
		media.unmap(&media, rootkey);
		media.close(&media);
		return 0;
	}

	media.unmap(&media, rootkey);
	media.close(&media);

	if (memcmp(root_key_hash.root_key_hash_digest, digest,
		   VB2_SHA256_DIGEST_SIZE)) {
		printk(BIOS_ERR, "HASH COMPARE FAILED!\n");
		return 0;
	}

	printk(BIOS_INFO, "HASH COMPARE SUCCESSFUL!\n");

	return 1;
}

enum {
	SE_AES_SBK_KEY_SLOT = 14,
	SE_AES_SBK_KEY_SIZE_BYTES = 16,

	SE_CRYPTO_KEYTABLE_ADDR_0_WORD_NUM_SHIFT = 0,
	SE_CRYPTO_KEYTABLE_ADDR_0_WORD_NUM_MASK  = 0xF,

	SE_CRYPTO_KEYTABLE_ADDR_0_KEY_SLOT_NO_SHIFT = 4,
	SE_CRYPTO_KEYTABLE_ADDR_0_KEY_SLOT_NO_MASK  = 0xF,

	SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_KEYIV = 0,
	SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_SHIFT = 8,
	SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_MASK = 0x1,

	SE_CRYPTO_KEYTABLE_ADDR_0_OP_READ = 0,
	SE_CRYPTO_KEYTABLE_ADDR_0_OP_WRITE = 1,
	SE_CRYPTO_KEYTABLE_ADDR_0_OP_SHIFT = 9,
	SE_CRYPTO_KEYTABLE_ADDR_0_OP_MASK = 0x1,

	SE_APB_BASE = 0x70012000,
	SE_CRYPTO_KEYTABLE_ADDR_0_OFFSET = 0x31c,
	SE_CRYPTO_KEYTABLE_DATA_0_OFFSET = 0x320,
};

static void erase_key_slots(void)
{
	printk(BIOS_ERR, "Erasing key slots\n");

	int word_num;
	uint32_t val;
	for (word_num = 0;
	     word_num < (SE_AES_SBK_KEY_SIZE_BYTES / sizeof(uint32_t));
	     word_num++) {

		/* Word number */
		val = (word_num & SE_CRYPTO_KEYTABLE_ADDR_0_WORD_NUM_MASK) <<
			SE_CRYPTO_KEYTABLE_ADDR_0_WORD_NUM_SHIFT;

		/* Key slot number */
		val |= (SE_AES_SBK_KEY_SLOT &
			SE_CRYPTO_KEYTABLE_ADDR_0_KEY_SLOT_NO_MASK) <<
			SE_CRYPTO_KEYTABLE_ADDR_0_KEY_SLOT_NO_SHIFT;

		/* KeyIV Table Selection */
		val |= (SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_KEYIV &
			SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_MASK) <<
			SE_CRYPTO_KEYTABLE_ADDR_0_TABLE_SEL_SHIFT;

		/* Write Operation */
		val |= (SE_CRYPTO_KEYTABLE_ADDR_0_OP_WRITE &
			SE_CRYPTO_KEYTABLE_ADDR_0_OP_MASK) <<
			SE_CRYPTO_KEYTABLE_ADDR_0_OP_SHIFT;

		/* Program KEYTABLE_ADDR_0 */
		write32((void *)(uintptr_t)(SE_APB_BASE +
					    SE_CRYPTO_KEYTABLE_ADDR_0_OFFSET),
			val);

		/* Write data(0xffffffff) into data registers */
		write32((void *)(uintptr_t)(SE_APB_BASE +
					    SE_CRYPTO_KEYTABLE_DATA_0_OFFSET),
			0xffffffff);
	}
}

enum {
	MAX77620_RTC_ADDR = 0x68,
	MAX77620_RTC_RTCUPDATE0_REG = 0x04,
	UDR_BIT = 0x1,
	FCUR_BIT = 0x2,
	RBUDR_BIT = 0x10,
	MAX77620_RTC_RTCSECA2_REG = 0x15,
	WARM_BOOT_BIT = 0x1,
};

/*
 * Handling of bootreason:
 *
 * PMC_SCRATCH202 register bits 2:0 have been assigned to store boot reason and
 * communicate between OS and bootloader. However, this register is not
 * guaranteed to have any sane value in case of cold boot. Thus, we need a
 * different way to distinguish cold vs warm boot.
 *
 * Thus, in case of warm-boot, OS sets bit0 in PMIC RTCSECA2 REG. Bootloader is
 * expected to check the value of this bit to identify if current boot is cold
 * or warm boot. If it is cold boot, we need to clear value PMC_SCRATCH202
 * register to reason=reboot. However, if it is a warm boot, PMC_SCRATCH202
 * would contain valid value and thus, we just need to clear bit 0 of RTCSECA2
 * reg.
 *
 * Since the OS uses BOOTROM I2C commands to configure the PMIC bit, we need to
 * ensure that write buffers are flushed properly, before trying to update the
 * read buffers.
 *
 */
static void validate_boot_reason(void)
{
	uint8_t val;

	/*
	 * Flush the write buffers.  This *should* be done by now, but it
	 * doesn't hurt to make sure.
	 */
	val = UDR_BIT | FCUR_BIT;
	if (i2c_writeb(I2CPWR_BUS, MAX77620_RTC_ADDR,
		       MAX77620_RTC_RTCUPDATE0_REG, val)) {
		printk(BIOS_ERR, "ERROR: Could not write RTCUPDATE0\n");
		pmc_set_bootreason(PMC_BOOTREASON_REBOOT);
		return;
	}

	mdelay(16);

	/* Update the read buffers. */
	val = RBUDR_BIT | FCUR_BIT;
	if (i2c_writeb(I2CPWR_BUS, MAX77620_RTC_ADDR,
		       MAX77620_RTC_RTCUPDATE0_REG, val)) {
		printk(BIOS_ERR, "ERROR: Could not write RTCUPDATE0\n");
		pmc_set_bootreason(PMC_BOOTREASON_REBOOT);
		return;
	}

	mdelay(16);

	if (i2c_readb(I2CPWR_BUS, MAX77620_RTC_ADDR, MAX77620_RTC_RTCSECA2_REG,
		      &val)) {
		printk(BIOS_ERR, "ERROR: Could not read warm-boot flag\n");
		pmc_set_bootreason(PMC_BOOTREASON_REBOOT);
		return;
	}

	/* This is a cold boot, so set bootreason to reboot. */
	if ((val & WARM_BOOT_BIT) == 0) {
		printk(BIOS_ERR, "Cold boot: Setting bootreason to reboot\n");
		pmc_set_bootreason(PMC_BOOTREASON_REBOOT);
		return;
	}

	/* This is warm boot, do not touch boot reason in scratch202. */
	val &= ~WARM_BOOT_BIT;
	if (i2c_writeb(I2CPWR_BUS, MAX77620_RTC_ADDR, MAX77620_RTC_RTCSECA2_REG,
		       val)) {
		printk(BIOS_ERR, "ERROR: Could not clear warm-boot bit\n");
		return;
	}

	val = FCUR_BIT | UDR_BIT;
	if (i2c_writeb(I2CPWR_BUS, MAX77620_RTC_ADDR,
		       MAX77620_RTC_RTCUPDATE0_REG, val))
		printk(BIOS_ERR, "ERROR: Could not write RTCUPDATE0\n");

	printk(BIOS_ERR, "Warm boot: validated boot reason\n");
}

void bootblock_mainboard_init(void)
{
	set_clock_sources();

	/* Set up the pads required to load romstage. */
	soc_configure_pads(padcfgs, ARRAY_SIZE(padcfgs));
	soc_configure_funits(funits, ARRAY_SIZE(funits));

	/* PMIC */
	i2c_init(I2CPWR_BUS);
	pmic_init(I2CPWR_BUS);

	/* TPM */
	i2c_init(I2C3_BUS);

	/* EC */
	i2c_init(I2C2_BUS);

	/*
	 * Set power detect override for GPIO, audio & sdmmc3 rails.
	 * GPIO rail override is required to put it into 1.8V mode.
	 */
	pmc_override_pwr_det(PMC_GPIO_RAIL_AO_MASK | PMC_AUDIO_RAIL_AO_MASK |
			     PMC_SDMMC3_RAIL_AO_MASK, PMC_GPIO_RAIL_AO_DISABLE |
			     PMC_AUDIO_RAIL_AO_DISABLE |
			     PMC_SDMMC3_RAIL_AO_DISABLE);

	/* Validate boot reason */
	validate_boot_reason();

	if (!verify_root_key())
		erase_key_slots();
}
