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

#include <assert.h>
#include <cbfs_core.h>
#include <cbmem.h>
#include <console/console.h>
#include <soc/mtc.h>
#include <string.h>
#include <vendorcode/google/chromeos/vboot_handoff.h>

static size_t mtc_table_size;

#define MAX_MTC_TABLE_ENTRIES	20
#define MTC_TABLE_ENTRY_SIZE	4880
#define MTC_TABLE_MAX_SIZE	(MAX_MTC_TABLE_ENTRIES * MTC_TABLE_ENTRY_SIZE)

static struct firmware_component *vboot_find_comp(uint8_t index)
{
	struct firmware_component *component;

	struct vboot_handoff *handoff = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (!handoff)
		return NULL;

	assert(index < MAX_PARSED_FW_COMPONENTS);
	component = &handoff->components[index];

	return component;
}

int tegra210_run_mtc(void)
{
	struct cbfs_file file;
	size_t nread;

	void * const mtc = (void *)(uintptr_t)CONFIG_MTC_ADDRESS;
	struct cbfs_media *media = CBFS_DEFAULT_MEDIA;
	void *dvfs_table;
	size_t (*mtc_fw)(void **dvfs_table) = (void *)mtc;

	size_t mtc_size = 0;
	ssize_t mtc_offset = 0;
	const char *mtc_filename = CONFIG_CBFS_PREFIX"/tegra_mtc";

	if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)) {
		struct firmware_component *comp;
		comp = vboot_find_comp(CONFIG_VBOOT_TEGRA_MTC_INDEX);

		if (comp != NULL) {
			mtc_offset = comp->address;
			mtc_size = comp->size;
		}
	}

	if (mtc_size == 0) {
		mtc_offset = cbfs_locate_file(media, &file, mtc_filename);
		if (mtc_offset < 0) {
			printk(BIOS_ERR, "MTC file not found: %s\n",
			       mtc_filename);
			return -1;
		}
		mtc_size = file.len;
	}

	/* Read MTC file into predefined region. */
	nread = cbfs_read(media, mtc, mtc_offset, mtc_size);

	if (nread != mtc_size) {
		printk(BIOS_ERR, "MTC bytes read (%zu) != file length(%zu)!\n",
		       nread, mtc_size);
		return -1;
	}

	printk(BIOS_INFO, "MTC: %zu bytes loaded from offset %p @ %p\n", nread,
	       (void *)mtc_offset, mtc);

	mtc_table_size = (*mtc_fw)(&dvfs_table);

	if ((mtc_table_size == 0) || (mtc_table_size > MTC_TABLE_MAX_SIZE)) {
		printk(BIOS_ERR, "MTC Training table size is invalid.!\n");
		return -1;
	}

	printk(BIOS_INFO, "MTC: Done. Entries size 0x%zx located at %p\n",
	       mtc_table_size, dvfs_table);

	void *cbmem_tab = cbmem_add(CBMEM_ID_MTC, mtc_table_size);
	if (cbmem_tab == NULL) {
		printk(BIOS_ERR, "MTC table allocation in cbmem failed!\n");
		return -1;
	}

	memcpy(cbmem_tab, dvfs_table, mtc_table_size);
	printk(BIOS_INFO, "MTC: Copied 0x%zx bytes from %p to %p\n",
	       mtc_table_size, dvfs_table, cbmem_tab);

	return 0;
}

void soc_add_mtc(struct lb_header *header)
{
	struct lb_range *mtc;
	mtc = (struct lb_range *)lb_new_record(header);
	mtc->tag = LB_TAG_MTC;
	mtc->size = sizeof(*mtc);

	mtc->range_start = (uintptr_t)cbmem_find(CBMEM_ID_MTC);
	mtc->range_size = mtc_table_size;
}
