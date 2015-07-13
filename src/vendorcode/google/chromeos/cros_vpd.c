/*
 * Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <console/console.h>

#include <cbfs.h>
#include <cbmem.h>
#include <fmap.h>
#include <stdlib.h>
#include <string.h>

#include "cros_vpd.h"
#include "lib_vpd.h"
#include "vpd_tables.h"

/* Currently we only support Google VPD 2.0, which has a fixed offset. */
enum {
	GOOGLE_VPD_2_0_OFFSET = 0x600,
	CROSVPD_CBMEM_MAGIC = 0x43524f53,
	CROSVPD_CBMEM_VERSION = 0x0001,
};

struct vpd_gets_arg {
	const uint8_t *key;
	const uint8_t *value;
	int32_t key_len, value_len;
	int matched;
};

struct vpd_cbmem {
	uint32_t magic;
	uint32_t version;
	uint32_t ro_size;
	uint32_t rw_size;
	uint8_t blob[0];
	/* The blob contains both RO and RW data. It starts with RO (0 ..
	 * ro_size) and then RW (ro_size .. ro_size+rw_size).
	 */
};

/* Finds the the VPD blob from CBFS media. */
static int cros_vpd_find_blob(struct cbfs_media *media,
			      const struct fmap_area *area,
			      size_t *base,
			      size_t *size)
{
	struct google_vpd_info info;
	size_t sz_info = sizeof(info);

	if (!area || area->size <= GOOGLE_VPD_2_0_OFFSET + sz_info)
		return -1;

	/* In Google VPD 2.0, the VPD blob lives in VPD area starting from
	 * GOOGLE_VPD_2_0_OFFSET without precise size information. Some newer
	 * systems may have an optinal google_vpd_info that indicates the real
	 * size occupied by VPD blob. To support both, we compute base and size
	 * in original way, and then reduce if google_vpd_info can be found.
	 */
	*base = area->offset + GOOGLE_VPD_2_0_OFFSET;
	*size = area->size - GOOGLE_VPD_2_0_OFFSET;

	if (media->read(media, &info, *base, sz_info) == sz_info &&
	    memcmp(info.header.magic, VPD_INFO_MAGIC,
		   sizeof(info.header.magic)) == 0 &&
	    *size >= info.size + sz_info) {

		*base += sz_info;
		*size = info.size;
	}
	return 0;
}

/* Loads VPD blob from CBFS media and save into CBMEM. */
static void cbmem_add_cros_vpd(void)
{
	const struct fmap_area *ro_area, *rw_area;
	size_t ro_base = 0, rw_base = 0;
	size_t ro_size = 0, rw_size = 0;
	struct cbfs_media media;
	struct vpd_cbmem *vpd;
	const struct fmap *fmap_base = fmap_find();

	ro_area = find_fmap_area(fmap_base, "RO_VPD");
	rw_area = find_fmap_area(fmap_base, "RW_VPD");

	init_default_cbfs_media(&media);
	media.open(&media);

	cros_vpd_find_blob(&media, ro_area, &ro_base, &ro_size);
	cros_vpd_find_blob(&media, rw_area, &rw_base, &rw_size);

	vpd = cbmem_add(CBMEM_ID_VPD, sizeof(*vpd) + ro_size + rw_size);
	if (vpd) {
		vpd->magic = CROSVPD_CBMEM_MAGIC;
		vpd->version = CROSVPD_CBMEM_VERSION;
		if (ro_size && ro_size != media.read(&media, vpd->blob, ro_base,
						     ro_size)) {
			ro_size = 0;
			printk(BIOS_ERR, "%s: Failed to read RO (%#zx+%#zx).\n",
			       __func__, ro_base, ro_size);
		}
		if (rw_size && rw_size != media.read(&media,
						     vpd->blob + ro_size,
						     rw_base, rw_size)) {
			rw_size = 0;
			printk(BIOS_ERR, "%s: Failed to read RW (%#zx+%#zx).\n",
			       __func__, rw_base, rw_size);
		}
		vpd->ro_size = ro_size;
		vpd->rw_size = rw_size;
		printk(BIOS_INFO, "%s: RO (%#zx+%#zx), RW (%#zx+%#zx).\n",
		       __func__, ro_base, ro_size, rw_base, rw_size);

	} else {
		printk(BIOS_ERR, "%s: Failed to allocate CBMEM (%zu+%zu).\n",
		       __func__, ro_size, rw_size);
	}
	media.close(&media);
}

static int vpd_gets_callback(const uint8_t *key, int32_t key_len,
			       const uint8_t *value, int32_t value_len,
			       void *arg)
{
	struct vpd_gets_arg *result = (struct vpd_gets_arg *)arg;
	if (key_len != result->key_len ||
	    memcmp(key, result->key, key_len) != 0)
		/* Returns VPD_OK to continue parsing. */
		return VPD_OK;

	result->matched = 1;
	result->value = value;
	result->value_len = value_len;
	/* Returns VPD_FAIL to stop parsing. */
	return VPD_FAIL;
}

const void *cros_vpd_find(const char *key, int *size)
{
	struct vpd_gets_arg arg = {0};
	int consumed = 0;
	const struct vpd_cbmem *vpd;

	vpd = cbmem_find(CBMEM_ID_VPD);
	if (!vpd || !vpd->ro_size)
		return NULL;

	arg.key = (const uint8_t *)key;
	arg.key_len = strlen(key);

	/* This API currently only supports reading RO VPD. */
	while (VPD_OK == decodeVpdString(vpd->ro_size, vpd->blob, &consumed,
					 vpd_gets_callback, &arg)) {
		/* Iterate until found or no more entries. */
	}

	if (!arg.matched)
		return NULL;

	*size = arg.value_len;
	return arg.value;
}

char *cros_vpd_gets(const char *key, char *buffer, int size)
{
	const void *string_address;
	int string_size;

	string_address = cros_vpd_find(key, &string_size);

	if (!string_address)
		return NULL;

	if (size > (string_size + 1)) {
		memcpy(buffer, string_address, string_size);
		buffer[string_size] = '\0';
	} else {
		memcpy(buffer, string_address, size - 1);
		buffer[size - 1] = '\0';
	}
	return buffer;
}

RAMSTAGE_CBMEM_INIT_HOOK(cbmem_add_cros_vpd)
