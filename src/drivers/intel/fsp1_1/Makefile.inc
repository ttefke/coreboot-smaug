#
# This file is part of the coreboot project.
#
# Copyright (C) 2014 Sage Electronic Engineering, LLC.
# Copyright (C) 2015 Intel Corp.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#

romstage-$(CONFIG_ENABLE_MRC_CACHE) += fastboot_cache.c
romstage-$(CONFIG_GOP_SUPPORT) += fsp_gop.c
romstage-y += fsp_hob.c
romstage-y += fsp_util.c
romstage-$(CONFIG_USING_FSP_1_0) += fsp_util_1_0.c

ramstage-$(CONFIG_ENABLE_MRC_CACHE) += fastboot_cache.c
ramstage-$(CONFIG_GOP_SUPPORT) += fsp_gop.c
ramstage-y += fsp_hob.c
ramstage-y += fsp_relocate.c
ramstage-y += fsp_util.c

CPPFLAGS_common += -Isrc/drivers/intel/fsp1_1

ifeq ($(CONFIG_USE_GENERIC_FSP_CAR_INC),y)
cpu_incs += $(src)/drivers/intel/fsp1_1/cache_as_ram.inc
endif


# Add the GOP Video BIOS Table to the cbfs image
cbfs-files-$(CONFIG_GOP_SUPPORT) += vbt.bin
vbt.bin-file := $(call strip_quotes,$(CONFIG_VBT_FILE))
vbt.bin-type := optionrom


# Add the FSP binary to the cbfs image
ifeq ($(CONFIG_HAVE_FSP_BIN),y)
cbfs-files-y += fsp.bin
fsp.bin-file := $(call strip_quotes,$(CONFIG_FSP_FILE))
fsp.bin-position := $(CONFIG_FSP_LOC)
fsp.bin-type := 0xab
endif


# Create and add the MRC cache to the cbfs image
ifeq ($(CONFIG_ENABLE_MRC_CACHE_FILE),y)
$(obj)/mrc.cache:
	dd if=/dev/zero count=1 \
	bs=$(shell printf "%d" $(CONFIG_MRC_CACHE_SIZE) ) | \
	tr '\000' '\377' > $@

cbfs-files-y += mrc.cache
mrc.cache-file := $(call strip_quotes,$(CONFIG_MRC_CACHE_FILE))
mrc.cache-position := $(CONFIG_MRC_CACHE_LOC)
mrc.cache-type := 0xac
endif
