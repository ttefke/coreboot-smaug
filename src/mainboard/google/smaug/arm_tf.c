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

#include <arch/cache.h>
#include <arch/lib_helpers.h>
#include <arm_tf.h>
#include <arm_tf_temp.h>
#include <cbfs.h>
#include <console/console.h>
#include <fmap.h>
#include <gbb_header.h>
#include <soc/addressmap.h>
#include <soc/fuses.h>
#include <stdlib.h>
#include <string.h>
#include <symbols.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <vendorcode/google/chromeos/vpd_eks.h>

void *bl32_get_load_addr(size_t bl32_len)
{
	uintptr_t tz_base_mib;
	size_t tz_size_mib;

	carveout_range(CARVEOUT_TZ, &tz_base_mib, &tz_size_mib);

	uintptr_t bl32_load_addr;
	bl32_load_addr = (tz_base_mib + CONFIG_BL31_SIZE_MB) * MiB;

	if ((bl32_load_addr + bl32_len) > ((tz_base_mib + tz_size_mib) * MiB)) {
		printk(BIOS_ERR, "ERROR: Not enough space to hold BL32.\n");
		return NULL;
	}

	return (void *)bl32_load_addr;
}

static inline uint32_t get_uart_id(void)
{
	if (CONFIG_CONSOLE_SERIAL_TEGRA210_UARTA)
		return 1;
	if (CONFIG_CONSOLE_SERIAL_TEGRA210_UARTB)
		return 2;
	if (CONFIG_CONSOLE_SERIAL_TEGRA210_UARTC)
		return 3;
	if (CONFIG_CONSOLE_SERIAL_TEGRA210_UARTD)
		return 4;
	if (CONFIG_CONSOLE_SERIAL_TEGRA210_UARTE)
		return 5;

	return 0;
}

#define MAX_KEYMASTER_KEY_SIZE	4096
#define MAX_EKS_SIZE		2048

typedef struct plat_bl32_atf_params {
	/* Boot args version - set to 1. */
	uint32_t version;
	/*
	 * UART_ID:
	 * None = 0, UARTA = 1, UARTB = 2, UARTC = 3, UARTD = 4, UARTE = 5
	 */
	uint32_t uart_id;
	uint32_t chip_uid[FUSE_UID_ELEMENT_NUM];
	/* Primary DRAM is DRAM memory below 32-bit address space. */
	uint64_t primary_dram_base_mib;
	uint64_t primary_dram_size_mib;
	/* Extended DRAM is DRAM memory above 32-bit address space. */
	uint64_t extended_dram_base_mib;
	uint64_t extended_dram_size_mib;
	/* TSEC carveout base address in MiB. */
	uint64_t tsec_carveout_base_mib;
	uint64_t tsec_carveout_size_mib;
	/* DTB - unused. */
	uint64_t dtb_load_addr;
	/* Keymaster params */
	/* Set to 1 if device is unlocked. */
	uint8_t is_unlocked;
	uint32_t keymaster_key_size;
	uint8_t keymaster_key[MAX_KEYMASTER_KEY_SIZE];
	/* EKS params */
	uint32_t encrypted_key_size;
	uint8_t encrypted_keys[MAX_EKS_SIZE];
} plat_bl32_atf_params_t;

static plat_bl32_atf_params_t boot_params;

static size_t read_root_key(void *key, size_t max_size)
{
	uintptr_t gbb_addr;
	size_t gbb_size;

	gbb_size = find_fmap_entry("GBB", (void **)&gbb_addr);

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

	media.unmap(&media, gbb_header);
	gbb_header = NULL;

	size_t ret = media.read(&media, boot_params.keymaster_key,
				gbb_addr + rootkey_offset, rootkey_size);

	media.close(&media);

	return ret;
}

/*
 * Arguments to be passed to TLK(bl32):
 * arg0 = TZDRAM size available to TLK
 * arg1 = NS entry point (unused)
 * arg2 = boot args
 * arg3 = magic value(0xCAFEBABE)
 */
#define BL32_TLK_MAGIC_VALUE	0xCAFEBABE
static void bl32_fill_args(aapcs64_params_t *args)
{
	uintptr_t base_mib;
	size_t size_mib;
	uint64_t start, end;
	int ret;

	carveout_range(CARVEOUT_TZ, &base_mib, &size_mib);
	args->arg0 = (base_mib + size_mib) * MiB -
		(uintptr_t)bl32_get_load_addr(0);

	args->arg1 = 0;

	args->arg2 = (uintptr_t)&boot_params;
	memset(&boot_params, 0, sizeof(boot_params));
	boot_params.version = 1;
	boot_params.uart_id = get_uart_id();

	fuse_get_uid(boot_params.chip_uid, sizeof(boot_params.chip_uid));

	memory_in_range_below_4gb(&start, &end);
	boot_params.primary_dram_base_mib = start;
	boot_params.primary_dram_size_mib = end - start;

	memory_in_range_above_4gb(&start, &end);
	boot_params.extended_dram_base_mib = start;
	boot_params.extended_dram_size_mib = end - start;

	carveout_range(CARVEOUT_TSEC, &base_mib, &size_mib);
	boot_params.tsec_carveout_base_mib = base_mib;
	boot_params.tsec_carveout_size_mib = size_mib;

	boot_params.is_unlocked = developer_mode_enabled();

	boot_params.keymaster_key_size =
		read_root_key(boot_params.keymaster_key,
			      MAX_KEYMASTER_KEY_SIZE);

	ret = vpd_read_eks(boot_params.encrypted_keys, MAX_EKS_SIZE);
	if (ret < 0) {
		printk(BIOS_ERR, "WARNING: No EKS\n");
		boot_params.encrypted_key_size = 0;
	} else
		boot_params.encrypted_key_size = ret;

	dcache_clean_by_mva(&boot_params, sizeof(boot_params));

	args->arg3 = BL32_TLK_MAGIC_VALUE;
}

void custom_prepare_bl32(entry_point_info_t *bl32_ep_info)
{
	/* TLK can only be loaded in 32-bit mode. */
	bl32_ep_info->spsr |= SPSR_ERET_32;

	bl32_fill_args(&bl32_ep_info->args);
}
