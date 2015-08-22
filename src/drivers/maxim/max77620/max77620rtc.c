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

#include <bcd.h>
#include <console/console.h>
#include <delay.h>
#include <device/i2c.h>
#include <rtc.h>
#include <stdint.h>

enum MAX77620_RTC_REG {
	MAX77620_RTCINT_REG		= 0x00,
	MAX77620_RTCINTM_REG		= 0x01,
	MAX77620_RTCCNTLM_REG		= 0x02,
	MAX77620_RTCCNTL_REG		= 0x03,
	MAX77620_RTCUPDATE0_REG	= 0x04,
	MAX77620_RTCUPDATE1_REG	= 0x05,
	MAX77620_RTCSMPL_REG		= 0x06,
	MAX77620_RTCSEC_REG		= 0x07,
	MAX77620_RTCMIN_REG		= 0x08,
	MAX77620_RTCHOUR_REG		= 0x09,
	MAX77620_RTCDOW_REG		= 0x0A,
	MAX77620_RTCMONTH_REG		= 0x0B,
	MAX77620_RTCYEAR_REG		= 0x0C,
	MAX77620_RTCDOM_REG		= 0x0D,
};

enum {
	RTCCNTL_24HR_MODE		= (1 << 1),
	RTCCNTL_12HR_MODE		= (0 << 1),
	RTCCNTL_BCD_MODE		= (1 << 0),
	RTCCNTL_BINARY_MODE		= (0 << 0),

	RTCUPDATE0_UDR			= (1 << 0),
	RTCUPDATE0_FCUR		= (1 << 1),
	RTCUPDATE0_FREEZE_SEC		= (1 << 2),
	RTCUPDATE0_RBUDR		= (1 << 4),

	RTCSEC_MASK			= 0x7F,
	RTCMIN_MASK			= 0x7F,
	RTCHOUR_MASK			= 0x3F,
	RTCDOM_MASK			= 0x3F,
	RTCMONTH_MASK			= 0x1F,
	RTCYEAR_MASK			= 0xFF,
};

static inline uint8_t max77620_read(enum MAX77620_RTC_REG reg)
{
	uint8_t val;
	i2c_readb(CONFIG_DRIVERS_MAXIM_MAX77620_RTC_BUS,
		  CONFIG_DRIVERS_MAXIM_MAX77620_RTC_ADDR, reg, &val);

	return val;
}

static inline void max77620_write(enum MAX77620_RTC_REG reg, uint8_t val)
{
	i2c_writeb(CONFIG_DRIVERS_MAXIM_MAX77620_RTC_BUS,
		   CONFIG_DRIVERS_MAXIM_MAX77620_RTC_ADDR, reg, val);
}

static void max77620_setbit(enum MAX77620_RTC_REG reg, uint8_t val,
				   uint8_t mask)
{
	uint8_t v = max77620_read(reg);

	v &= ~mask;
	v |= val;

	max77620_write(reg, v);
}

static void max77620_rtc_init(void)
{
	static int initialized = 0;

	if (initialized)
		return;

	/* Set 24hr mode, binary mode. */
	max77620_write(MAX77620_RTCCNTL_REG, RTCCNTL_24HR_MODE |
		       RTCCNTL_BINARY_MODE);

	/* Transfer RTCCNTL modification to RTC. */
	max77620_setbit(MAX77620_RTCUPDATE0_REG, RTCUPDATE0_UDR,
			RTCUPDATE0_UDR);

	/* Wait 16ms for write to complete. */
	mdelay(16);

	initialized = 1;
}

int rtc_set(const struct rtc_time *time)
{
	max77620_rtc_init();

	max77620_write(MAX77620_RTCSEC_REG, time->sec & RTCSEC_MASK);
	max77620_write(MAX77620_RTCMIN_REG, time->min & RTCMIN_MASK);
	max77620_write(MAX77620_RTCHOUR_REG, time->hour & RTCHOUR_MASK);
	max77620_write(MAX77620_RTCDOM_REG, time->mday & RTCDOM_MASK);
	max77620_write(MAX77620_RTCMONTH_REG, time->mon & RTCMONTH_MASK);
	max77620_write(MAX77620_RTCYEAR_REG, time->year & RTCYEAR_MASK);

	/* Transfer write buffers to counters. */
	max77620_setbit(MAX77620_RTCUPDATE0_REG, RTCUPDATE0_UDR,
			RTCUPDATE0_UDR);

	/* Wait 16ms for write to complete. */
	mdelay(16);

	return 0;
}


int rtc_get(struct rtc_time *time)
{
	max77620_rtc_init();

	/* Transfer timekeeper counters to read buffers. */
	max77620_setbit(MAX77620_RTCUPDATE0_REG, RTCUPDATE0_RBUDR,
			RTCUPDATE0_RBUDR);

	/* Wait 16ms for read to complete. */
	mdelay(16);

	time->sec = max77620_read(MAX77620_RTCSEC_REG) & RTCSEC_MASK;
	time->min = max77620_read(MAX77620_RTCMIN_REG) & RTCMIN_MASK;
	time->hour = max77620_read(MAX77620_RTCHOUR_REG) & RTCHOUR_MASK;
	time->mday = max77620_read(MAX77620_RTCDOM_REG) & RTCDOM_MASK;
	time->mon = max77620_read(MAX77620_RTCMONTH_REG) & RTCMONTH_MASK;
	time->year = max77620_read(MAX77620_RTCYEAR_REG) & RTCYEAR_MASK;

	return 0;
}
