/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Intel Corporation.
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

/* REAKTEK Audio Jack Interrupt */
#define GPIO_SKL_LP_GPP_E22	0x02040016

/*
 * LPC Trusted Platform Module
 */
Scope (\_SB.PCI0.LPCB)
{
	#include <drivers/pc80/tpm/acpi/tpm.asl>
}

/*
 * WLAN connected to Root Port 3, becomes Root Port 1 after coalesce
 */
Scope (\_SB.PCI0.RP01)
{
	Device (WLAN)
	{
		Name (_ADR, 0x00000000)

		/* GPIO10 is PCH_WLAN_WAKE_L */
		Name (GPIO, 10)

		Name (_PRW, Package() { GPIO, 3 })

		Method (_DSW, 3, NotSerialized)
		{
			If (LEqual (Arg0, 1)) {
				// Enable GPIO as wake source
				\_SB.PCI0.LPCB.GPIO.GWAK (^GPIO)
			}
		}
	}
}

Scope (\_SB.PCI0.I2C0)
{
	//-----------------------------------
	//  HD Audio I2S Codec device
	//  Realtek ALC286S       (I2SC = 2)
	//-----------------------------------
	Device (HDAC)
	{
		Name (_HID, "INT343A")
		Name (_CID, "INT343A")
		Name (_DDN, "Intel(R) Smart Sound Technology Audio Codec")
		Name (_UID, 1)

		Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
		{
			/* update Audio codec slave address in runtime */
			Name (RBUF, ResourceTemplate ()
			{
				I2cSerialBus (0x1C, ControllerInitiated, 400000,
					AddressingMode7Bit, "\\_SB.PCI0.I2C0",
					0x00, ResourceConsumer, ,)
			})

			/* update interrupt number in runtime */
			Name (SBFI, ResourceTemplate ()
			{
				Interrupt (ResourceConsumer, Level, ActiveLow,
						ExclusiveAndWake, ,, _Y22)
				{
					0x00000000,
				}
			})

			/* _INT: Interrupts */
			CreateDWordField (SBFI, _Y22._INT, INT2)

			Store (INUM (GPIO_SKL_LP_GPP_E22), INT2)

			Return (ConcatenateResTemplate (RBUF, SBFI))
		}

		Method (_STA, 0, NotSerialized)
		{
			Return (0xF)	/* I2S Codec Enabled */
		}
	}
}
