/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#ifndef __SOC_NVIDIA_TEGRA210_PAD_CFG_H
#define __SOC_NVIDIA_TEGRA210_PAD_CFG_H

#include <stdint.h>
#include <soc/pinmux.h>

struct pad_config {
	uint16_t pinmux_flags;	/* PU/PU, OD, INPUT, SFIO, etc */
	uint8_t gpio_index;		/* bank, port, index */
	uint16_t pinmux_index:9;
	uint16_t unused:1;
	uint16_t sfio:1;
	uint16_t gpio_out0:1;
	uint16_t gpio_out1:1;
	uint16_t pad_has_gpio:1;
	uint16_t por_pullup:1;
};

#define PAD_CFG_GPIO_INPUT(ball_, pinmux_flgs_)		\
	{						\
		.pinmux_flags = pinmux_flgs_ | PINMUX_INPUT_ENABLE,	\
		.gpio_index = PAD_TO_GPIO_##ball_,	\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.sfio = 0,				\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,	\
	}

#define PAD_CFG_GPIO_OUT0(ball_, pinmux_flgs_)		\
	{						\
		.pinmux_flags = pinmux_flgs_,		\
		.gpio_index = PAD_TO_GPIO_##ball_,	\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.sfio = 0,				\
		.gpio_out0 = 1,				\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,	\
	}

#define PAD_CFG_GPIO_OUT1(ball_, pinmux_flgs_)		\
	{						\
		.pinmux_flags = pinmux_flgs_,		\
		.gpio_index = PAD_TO_GPIO_##ball_,	\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.sfio = 0,				\
		.gpio_out1 = 1,				\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,	\
	}

#define PAD_CFG_SFIO(ball_, pinmux_flgs_, sfio_)	\
	{						\
		.pinmux_flags = pinmux_flgs_ |		\
				PINMUX_##ball_##_FUNC_##sfio_,	\
		.gpio_index = PAD_TO_GPIO_##ball_,	\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.sfio = 1,				\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,	\
	}

#define PAD_CFG_UNUSED(ball_)				\
	{						\
		.pinmux_flags = 0,			\
		.gpio_index = PAD_TO_GPIO_##ball_,	\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.unused = 1,				\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,	\
	}

#define PAD_CFG_UNUSED_WITH_RES(ball_, res_)			\
	{							\
		.pinmux_flags = PINMUX_##ball_##_FUNC_##res_,	\
		.gpio_index = PAD_TO_GPIO_##ball_,		\
		.pinmux_index = PINMUX_##ball_##_INDEX,	\
		.unused = 1,					\
		.pad_has_gpio = PAD_HAS_GPIO_##ball_,		\
	}

/* CFGPADCTRL regs for audio, camera and touch */
enum {
	AUD_MCLK = 0xF4,
	CAM1_MCLK = 0x118,
	CAM2_MCLK = 0x124,
	CAM_AF_EN = 0x12C,
	CAM_FLASH_EN = 0x130,
	GPIO_PZ0 = 0x1FC,
	GPIO_PZ1 = 0x200,
	TOUCH_CLK = 0x310
};

#define TEGRA_APB_MISC_GP_REGS(x)	((unsigned char *)TEGRA_APB_MISC_GP_BASE + (x))

/*
 * Configure the pads associated with entry according to the configuration.
 */
void soc_configure_pads(const struct pad_config * const entries, size_t num);
/* I2C6 requires special init as its pad lives int the SOR/DPAUX block */
void soc_configure_i2c6pad(void);
void soc_configure_host1x(void);
/* APE (Audio Processing Engine) requires special init */
void soc_configure_ape(void);

#endif /* __SOC_NVIDIA_TEGRA210_PAD_CFG_H */
