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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#include <arch/cache.h>
#include <arch/lib_helpers.h>
#include <arch/transition.h>
#include <arm_tf.h>
#include <assert.h>
#include <cbfs.h>
#include <cbmem.h>
#include <vendorcode/google/chromeos/vboot_handoff.h>

/*
 * TODO: Many of these structures are currently unused. Better not fill them out
 * to make future changes fail fast, rather than try to come up with content
 * that might turn out to not make sense. Implement later as required.
 *
static image_info_t bl31_image_info;
static image_info_t bl32_image_info;
static image_info_t bl33_image_info;
*/
static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;
static bl31_params_t bl31_params;

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

/* TODO: Replace with glorious new CBFSv1 solution when it's available. */
static void *vboot_get_bl3x_entry(uint8_t index)
{
	void *entry_addr;
	struct cbfs_media media;
	struct firmware_component *component;

	component = vboot_find_comp(index);

	/* components[] is zeroed out before filling, so size == 0 -> missing */
	if ((component == NULL) || (component->size == 0))
		return NULL;

	init_default_cbfs_media(&media);
	entry_addr = cbfs_load_stage_by_offset(&media, component->address);
	if (entry_addr == CBFS_LOAD_ERROR)
		return NULL;

	printk(BIOS_INFO, "Loaded %u bytes verified BLx from %#.8x to EP %p\n",
		component->size, component->address, entry_addr);
	return entry_addr;
}

void __attribute__((weak)) *soc_get_bl31_plat_params(bl31_params_t *params)
{
	/* Default weak implementation. */
	return NULL;
}

void __attribute__((weak)) custom_prepare_bl32(entry_point_info_t *ep_info)
{
	/* Default weak implementation. */
	return;
}

static entry_point_info_t *prepare_bl32(void)
{
	const char *bl32_filename = CONFIG_CBFS_PREFIX"/secure_os";
	void *pc = NULL;

	if (IS_ENABLED(CONFIG_ARM64_SECURE_OS_LOAD_RAW)) {
		ssize_t bl32_offset = 0;
		size_t bl32_size = 0;
		size_t nread;

		if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)) {
			struct firmware_component *comp;

			comp = vboot_find_comp(CONFIG_VBOOT_SECURE_OS_INDEX);
			if (comp != NULL) {
				bl32_offset = comp->address;
				bl32_size = comp->size;
			}
		}

		if (bl32_size == 0) {
			struct cbfs_file file;

			printk(BIOS_INFO, "Looking for %s\n", bl32_filename);

			bl32_offset = cbfs_locate_file(CBFS_DEFAULT_MEDIA,
						       &file, bl32_filename);

			if (bl32_offset < 0)
				die("ERROR: Cannot locate BL32");

			bl32_size = file.len;
		}

		pc = bl32_get_load_addr(bl32_size);
		if (!pc)
			die("ERROR: Invalid BL32 address");

		nread = cbfs_read(CBFS_DEFAULT_MEDIA, pc, bl32_offset,
				  bl32_size);

		if (nread != bl32_size)
			die("ERROR: BL32 load failed");

		printk(BIOS_INFO,
		       "Loaded %zu bytes verified BL32 from %p to EP %p\n",
		       bl32_size, (void *)bl32_offset, pc);

	} else {

		if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE))
			pc = vboot_get_bl3x_entry(CONFIG_VBOOT_SECURE_OS_INDEX);

		if (!pc) {
			pc = cbfs_load_stage(CBFS_DEFAULT_MEDIA, bl32_filename);
			if (pc == CBFS_LOAD_ERROR)
				die("ERROR: Secure OS not found in CBFS");
		}
	}

	SET_PARAM_HEAD(&bl32_ep_info, PARAM_EP, VERSION_1, PARAM_EP_SECURE);
	bl32_ep_info.pc = (uintptr_t)pc;
	bl32_ep_info.spsr = SPSR_EXCEPTION_MASK | get_eret_el(EL1, SPSR_USE_L);

	custom_prepare_bl32(&bl32_ep_info);

	dcache_clean_by_mva(&bl32_ep_info, sizeof(bl32_ep_info));

	return &bl32_ep_info;
}

void arm_tf_run_bl31(u64 payload_entry, u64 payload_arg0, u64 payload_spsr)
{
	const char *bl31_filename = CONFIG_CBFS_PREFIX"/bl31";
	void (*bl31_entry)(bl31_params_t *params, void *plat_params) = NULL;

	if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE))
		bl31_entry = vboot_get_bl3x_entry(CONFIG_VBOOT_BL31_INDEX);

	if (!bl31_entry) {
		bl31_entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, bl31_filename);
		if (bl31_entry == CBFS_LOAD_ERROR)
			die("BL31 not found in CBFS");
	}

	SET_PARAM_HEAD(&bl31_params, PARAM_BL31, VERSION_1, 0);

	if (IS_ENABLED(CONFIG_ARM64_USE_SECURE_OS))
		bl31_params.bl32_ep_info = prepare_bl32();

	bl31_params.bl33_ep_info = &bl33_ep_info;
	SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_1, PARAM_EP_NON_SECURE);
	bl33_ep_info.pc = payload_entry;
	bl33_ep_info.spsr = payload_spsr;
	bl33_ep_info.args.arg0 = payload_arg0;

	/* May update bl31_params if necessary. Must flush all added structs. */
	void *bl31_plat_params = soc_get_bl31_plat_params(&bl31_params);

	dcache_clean_by_mva(&bl31_params, sizeof(bl31_params));
	dcache_clean_by_mva(&bl33_ep_info, sizeof(bl33_ep_info));
	dcache_mmu_disable();
	bl31_entry(&bl31_params, bl31_plat_params);
	die("BL31 returned!");
}
