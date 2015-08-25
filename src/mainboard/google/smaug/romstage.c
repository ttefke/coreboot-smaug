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

#include <delay.h>
#include <soc/addressmap.h>
#include <device/i2c.h>
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/ec_commands.h>
#include <soc/clock.h>
#include <soc/funitcfg.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/padconfig.h>
#include <soc/romstage.h>
#include <vendorcode/google/chromeos/vboot2/misc.h>
#include <vendorcode/google/chromeos/chromeos.h>

#include <vb2_api.h>
#include <vboot_struct.h>

#include "gpio.h"
#include "pmic.h"
#include "chromeos.h"

static const struct pad_config padcfgs[] = {
	/* AP_SYS_RESET_L - active low*/
	PAD_CFG_GPIO_OUT1(SDMMC1_DAT0, PINMUX_PULL_UP),
	/* WP_L - active low */
	PAD_CFG_GPIO_INPUT(GPIO_PK2, PINMUX_PULL_NONE),
	/* BTN_AP_PWR_L - active low */
	PAD_CFG_GPIO_INPUT(BUTTON_POWER_ON, PINMUX_PULL_UP),
	/* BTN_AP_VOLD_L - active low */
	PAD_CFG_GPIO_INPUT(BUTTON_VOL_DOWN, PINMUX_PULL_UP),
	/* BTN_AP_VOLU_L - active low */
	PAD_CFG_GPIO_INPUT(SDMMC1_DAT1, PINMUX_PULL_UP),
};

void romstage_mainboard_init(void)
{
	soc_configure_pads(padcfgs, ARRAY_SIZE(padcfgs));

	struct vb2_working_data *wd = vboot_get_working_data();
	struct vb2_shared_data *vb2_sd = vboot_get_work_buffer(wd);

	/*
	 * If we are trying to enter recovery and
	 * EC is not in RO, then we need to save recovery reason in nvstorage.
	 */
	if (vb2_sd->recovery_reason &&
	    (google_chromeec_get_ec_image_type() != EC_IMAGE_RO)) {
		uint8_t recovery_reason =
			get_recovery_reason(vb2_sd->recovery_reason);

		vbnv_update_recovery(recovery_reason);
	}

	google_chromeec_early_init();
}

void mainboard_configure_pmc(void)
{
}

void mainboard_enable_vdd_cpu(void)
{
	/* VDD_CPU is already enabled in bootblock. */
}
