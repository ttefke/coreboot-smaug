/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2015 Google Inc.
 * Copyright (C) 2015 Intel Corporation
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
 * Foundation, Inc.
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <soc/gpio.h>
#include <string.h>
#include <vendorcode/google/chromeos/chromeos.h>

#ifndef __PRE_RAM__
#include <boot/coreboot_tables.h>

#define GPIO_COUNT	6

int get_lid_switch(void)
{
	/* Default to force open */
	return 1;
}

static void fill_lb_gpio(struct lb_gpio *gpio, int num,
			 int polarity, const char *name, int force)
{
	memset(gpio, 0, sizeof(*gpio));
	gpio->port = num;
	gpio->polarity = polarity;

	/*TODO: Update right values when GPIO API is ready */
	if (force >= 0)
		gpio->value = force;
	else if (num >= 0)
		gpio->value = 0;
	strncpy((char *)gpio->name, name, GPIO_MAX_NAME_LENGTH);
}

void fill_lb_gpios(struct lb_gpios *gpios)
{
	struct lb_gpio *gpio;

	gpios->size = sizeof(*gpios) + (GPIO_COUNT * sizeof(struct lb_gpio));
	gpios->count = GPIO_COUNT;

	gpio = gpios->gpios;
	fill_lb_gpio(gpio++, -1, ACTIVE_HIGH, "recovery",
		     get_recovery_mode_switch());
	fill_lb_gpio(gpio++, -1, ACTIVE_HIGH, "developer",
		     get_developer_mode_switch());
	fill_lb_gpio(gpio++, -1, ACTIVE_HIGH, "lid",
		     get_lid_switch());
	fill_lb_gpio(gpio++, -1, ACTIVE_HIGH, "power", 0);
	fill_lb_gpio(gpio++, -1, ACTIVE_HIGH, "oprom", oprom_is_loaded);
}
#endif

/* The dev-switch is virtual */
int get_developer_mode_switch(void)
{
	return 0;
}

int get_recovery_mode_switch(void)
{
	return 0;
}

int get_write_protect_state(void)
{
	return 0;
}
