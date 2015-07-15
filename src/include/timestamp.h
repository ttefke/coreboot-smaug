/*
 * This file is part of the coreboot project.
 *
 * Copyright 2011 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <stdint.h>

struct timestamp_entry {
	uint32_t	entry_id;
	uint64_t	entry_stamp;
} __attribute__((packed));

struct timestamp_table {
	uint64_t	base_time;
	uint32_t	max_entries;
	uint32_t	num_entries;
	struct timestamp_entry entries[0]; /* Variable number of entries */
} __attribute__((packed));

enum timestamp_id {
	TS_START_ROMSTAGE = 1,
	TS_BEFORE_INITRAM = 2,
	TS_AFTER_INITRAM = 3,
	TS_END_ROMSTAGE = 4,
	TS_START_VBOOT = 5,
	TS_END_VBOOT = 6,
	TS_START_COPYRAM = 8,
	TS_END_COPYRAM = 9,
	TS_START_RAMSTAGE = 10,
	TS_START_BOOTBLOCK = 11,
	TS_END_BOOTBLOCK = 12,
	TS_START_COPYROM = 13,
	TS_END_COPYROM = 14,
	TS_START_ULZMA = 15,
	TS_END_ULZMA = 16,
	TS_DEVICE_ENUMERATE = 30,
	TS_FSP_BEFORE_ENUMERATE = 31,
	TS_FSP_AFTER_ENUMERATE = 32,
	TS_DEVICE_CONFIGURE = 40,
	TS_DEVICE_ENABLE = 50,
	TS_DEVICE_INITIALIZE = 60,
	TS_DEVICE_DONE = 70,
	TS_CBMEM_POST = 75,
	TS_WRITE_TABLES = 80,
	TS_FSP_BEFORE_FINALIZE = 81,
	TS_FSP_AFTER_FINALIZE = 82,
	TS_LOAD_PAYLOAD = 90,
	TS_ACPI_WAKE_JUMP = 98,
	TS_SELFBOOT_JUMP = 99,

	/* 500+ reserved for vendorcode extensions (500-600: google/chromeos) */
	TS_START_COPYVER = 501,
	TS_END_COPYVER = 502,
	TS_START_TPMINIT = 503,
	TS_END_TPMINIT = 504,
	TS_START_VERIFY_SLOT = 505,
	TS_END_VERIFY_SLOT = 506,
	TS_START_HASH_BODY = 507,
	TS_DONE_LOADING = 508,
	TS_DONE_HASHING = 509,
	TS_END_HASH_BODY = 510,
	TS_FSP_MEMORY_INIT_START = 511,
	TS_FSP_MEMORY_INIT_END = 512,
	TS_FSP_TEMP_RAM_EXIT_START = 513,
	TS_FSP_TEMP_RAM_EXIT_END = 514,
	TS_FSP_FIND_START = 515,
	TS_FSP_FIND_END = 516,
	TS_FSP_SILICON_INIT_START = 517,
	TS_FSP_SILICON_INIT_END = 518,
	TS_FSP_START_RELOCATION = 519,
	TS_FSP_RW_FOUND = 520,
	TS_FSP_COPIED_FROM_FLASH_TO_MEMORY = 521,
	TS_FSP_FV_RELOCATED = 522,
	TS_FSP_RELOCATED_REMAINING_ITEMS = 523,
	TS_FSP_UPDATED_FIH_POINTER = 524,
	TS_FSP_COPIED_FROM_MEMORY_TO_CACHE = 525,
	TS_FSP_COPIED_FROM_CACHE_TO_MEMORY = 526,
	TS_FSP_GET_ORIGINAL_UPD_DATA = 527,
	TS_FSP_UPD_SOC_UPDATE = 528,
	TS_FSP_LOAD_VBT = 529,
	TS_FSP_UPD_MAINBOARD_UPDATE = 530,

	/* 1000+ reserved for payloads (1000-1200: ChromeOS depthcharge) */
};

#if CONFIG_COLLECT_TIMESTAMPS
/*
 * Order of usage of timestamp library is:
 * Call timestamp_early_init / timestamp_init to set base time before any
 * timestamp_add function is called. timestamp_early_init also ensures that the
 * cache is valid in _timestamp region.
 * After this, timestamp_add / timestamp_add_now can be used to record
 * timestamps. Sync will be automatically taken care of by cbmem_initialize
 */
/*
 * Initialize the cache to a valid state and set the base time.
 * This function is used before DRAM is setup so that the timestamp cache can
 * be initialized in _timestamp region.
 * Both, timestamp_init and timestamp_early_init reset the cbmem state to
 * timestamp table reset required. Thus, whenever a timestamp_add or
 * timestamp_sync is done to add new entries into the cbmem timestamp table, it
 * first resets the table to 0 entries.
 */
void timestamp_early_init(uint64_t base);
/* Initialize the base time for timestamps and mark cache as valid */
void timestamp_init(uint64_t base);
/*
 * Add a new timestamp. Depending on cbmem is available or not, this timestamp
 * will be stored to cbmem / timestamp cache.
 */
void timestamp_add(enum timestamp_id id, uint64_t ts_time);
/* Calls timestamp_add with current timestamp. */
void timestamp_add_now(enum timestamp_id id);
/* Implemented by the architecture code */
uint64_t timestamp_get(void);
#else
static inline void timestamp_early_init(uint64_t base) { }
static inline void timestamp_init(uint64_t base) { }
static inline void timestamp_add(enum timestamp_id id, uint64_t ts_time) { }
static inline void timestamp_add_now(enum timestamp_id id) { }
static inline void timestamp_sync(void) { }
static inline uint64_t timestamp_get(void) { return 0; }
#endif

#endif
