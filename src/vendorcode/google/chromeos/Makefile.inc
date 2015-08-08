##
## This file is part of the coreboot project.
##
## Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
##

bootblock-y += chromeos.c
verstage-y += chromeos.c
romstage-y += chromeos.c
ramstage-y += chromeos.c

bootblock-$(CONFIG_CHROMEOS_VBNV_CMOS) += vbnv_cmos.c
verstage-$(CONFIG_CHROMEOS_VBNV_CMOS) += vbnv_cmos.c
romstage-$(CONFIG_CHROMEOS_VBNV_CMOS) += vbnv_cmos.c
ramstage-$(CONFIG_CHROMEOS_VBNV_CMOS) += vbnv_cmos.c
bootblock-$(CONFIG_CHROMEOS_VBNV_EC) += vbnv_ec.c
verstage-$(CONFIG_CHROMEOS_VBNV_EC) += vbnv_ec.c
romstage-$(CONFIG_CHROMEOS_VBNV_EC) += vbnv_ec.c
ramstage-$(CONFIG_CHROMEOS_VBNV_EC) += vbnv_ec.c
bootblock-$(CONFIG_CHROMEOS_VBNV_FLASH) += vbnv_flash.c
verstage-$(CONFIG_CHROMEOS_VBNV_FLASH) += vbnv_flash.c
romstage-$(CONFIG_CHROMEOS_VBNV_FLASH) += vbnv_flash.c
ramstage-$(CONFIG_CHROMEOS_VBNV_FLASH) += vbnv_flash.c

romstage-$(CONFIG_ARCH_ROMSTAGE_X86_32) += vboot.c
ramstage-$(CONFIG_ELOG) += elog.c
ramstage-$(CONFIG_HAVE_ACPI_TABLES) += gnvs.c
ramstage-$(CONFIG_CHROMEOS_RAMOOPS) += ramoops.c
ramstage-y += vpd_decode.c
ramstage-y += cros_vpd.c
ramstage-y += vpd_mac.c
ramstage-y += vpd_serialno.c
ramstage-y += vpd_calibration.c
ramstage-y += vpd_eks.c
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/chromeos.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/chromeos.c
romstage-srcs += src/mainboard/$(MAINBOARDDIR)/chromeos.c
endif
ifeq ($(CONFIG_ARCH_X86)$(CONFIG_ARCH_MIPS),)
bootblock-y += watchdog.c
ramstage-y += watchdog.c
endif

ifeq ($(MOCK_TPM),1)
CFLAGS_common += -DMOCK_TPM=1
else
CFLAGS_common += -DMOCK_TPM=0
endif

VB_SOURCE := 3rdparty/vboot
subdirs-$(CONFIG_VBOOT_VERIFY_FIRMWARE) += vboot1
subdirs-$(CONFIG_VBOOT2_VERIFY_FIRMWARE) += vboot2
INCLUDES += -I$(VB_SOURCE)/firmware/include

VB_GRAPHICS := 3rdparty/depthcharge/src/vboot/graphics

define add_cbfs_file
cbfs-files-y += $(1)
$(1)-file := $$(VB_GRAPHICS)/$(1)
$(1)-type := 0x50
endef

CBFS_FILES  = $(sort $(wildcard $(VB_GRAPHICS)/*.bmp))
CBFS_FILES += $(sort $(wildcard $(VB_GRAPHICS)/*/*.bmp))
$(foreach f,$(CBFS_FILES),$(eval $(call add_cbfs_file,$(subst $(VB_GRAPHICS)/,,$(f)))))
