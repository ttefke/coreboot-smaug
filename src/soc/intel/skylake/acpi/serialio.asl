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


/* Serial IO Device BAR0 and BAR1 is 4KB */
#define SIO_BAR_LEN 0x1000

/* Put SerialIO device in D0 state */
/* Arg0 - BAR1 of device */
/* Arg1 - Set if device is in ACPI mode */

Method (LPD0, 2, Serialized)
{
	/* PCI mode devices will be handled by OS PCI bus driver */
	If (LEqual (Arg1, 0)) {
		Return
	}

	OperationRegion (SPRT, SystemMemory, Add (Arg0, 0x84), 4)
	Field (SPRT, DWordAcc, NoLock, Preserve)
	{
		SPCS, 32
	}

	And (SPCS, 0xFFFFFFFC, SPCS)
	/* Read back after writing */
	Store (SPCS, Local0) 
}

/* Put SerialIO device in D3 state */
/* Arg0 - BAR1 of device */
/* Arg1 - Set if device is in ACPI mode */

Method (LPD3, 2, Serialized)
{
	/* PCI mode devices will be handled by OS PCI bus driver */
	If (LEqual (Arg1, 0)) {
		Return
	}
	OperationRegion (SPRT, SystemMemory, Add (Arg0, 0x84), 4)
	Field (SPRT, DWordAcc, NoLock, Preserve)
	{
		SPCS, 32
	}
	Or (SPCS, 0x3, SPCS)
	Store (SPCS, Local0) // Read back after writing
}

/* Serial IO Resource Consumption for BAR1 */
Device (SIOR)
{
	Name (_HID, EISAID("PNP0C02"))
	Name (_UID, 5)
	Method(ADDB,3,Serialized) {
	Name (BUFF, ResourceTemplate()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x1000, BUF)
	})
	CreateDWordField(BUFF,BUF._BAS,ADDR)
	CreateDWordField(BUFF,BUF._LEN,LENG)
	Store(ResourceTemplate(){}, Local0)
	//Return (RBUF)
	}
}

Device (I2C0)
{
	/* Serial IO I2C0 Controller */
	Name (_HID,"INT3442")
	Name (_UID, 1)
	Name (_ADR, 0x00150000)
	Name (SSCN, Package () { 432, 507, 30 })
	Name (FMCN, Package () { 72, 160, 30 })

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {10}
	})

	/* DMA channels are only used if Serial IO DMA controller is enabled */
	Name (DBUF, ResourceTemplate ()
	{
		FixedDMA (0x18, 4, Width32Bit, DMA1) // Tx
		FixedDMA (0x19, 5, Width32Bit, DMA2) // Rx
	})
	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S0B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S0B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}
		Return (RBUF)
	}
	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}
}

Device (I2C1)
{
	/* Serial IO I2C1 Controller */
	Name (_HID,"INT3443")
	Name (_UID, 1)
	Name (_ADR, 0x00150001)
	Name (SSCN, Package () { 528, 640, 30 })
	Name (FMCN, Package () { 128, 160, 30 })
	Name (FPCN, Package () { 48, 64, 30})

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {11}
	})

	/* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
	Name (DBUF, ResourceTemplate ()
	{
		FixedDMA (0x1A, 6, Width32Bit, DMA1) // Tx
		FixedDMA (0x1B, 7, Width32Bit, DMA2) // Rx
	})
	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S1B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S1B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}
		Return (RBUF)
	}
	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}
}


Device (I2C2)
{
        /* Serial IO I2C1 Controller */
	Name (_HID,"INT3444")
	Name (_UID, 1)
	Name (_ADR, 0x00150002)
	Name (SSCN, Package () { 432, 507, 30 })
	Name (FMCN, Package () { 72, 160, 30 })

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {12}
	})

	/* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
	Name (DBUF, ResourceTemplate ()
	{
		FixedDMA (0x1A, 6, Width32Bit, DMA1)
		FixedDMA (0x1B, 7, Width32Bit, DMA2)
	})

	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S2B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S2B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

		/* Check if Serial IO DMA Controller is enabled */
		Return (RBUF)
	}

	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}
}

Device (I2C3)
{
	/* Serial IO I2C3 Controller */
	Name (_HID,"INT3445")
	Name (_UID, 1)
	Name (_ADR, 0x00150003)
	Name (SSCN, Package () { 432, 507, 30 })
	Name (FMCN, Package () { 72, 160, 30 })

        /* BAR0 is assigned during PCI enumeration and saved into NVS */
        Name (RBUF, ResourceTemplate ()
        {
                Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
                Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {13}
        })

        /* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
        Name (DBUF, ResourceTemplate ()
        {
                FixedDMA (0x1A, 6, Width32Bit, DMA1)
                FixedDMA (0x1B, 7, Width32Bit, DMA2)
        })

        Method (_CRS, 0, NotSerialized)
        {
                /* Update BAR0 address and length if set in NVS */
                If (LNotEqual (\S3B0, Zero)) {
                        CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
                        CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
                        Store (\S3B0, B0AD)
                        Store (SIO_BAR_LEN, B0LN)
                }

                /* Check if Serial IO DMA Controller is enabled */
                Return (RBUF)
        }
        Method (_STA, 0, NotSerialized)
        {
                Return (0xF)
        }
}

Device (I2C4)
{
	/* Serial IO I2C4 Controller */
	Name (_HID,"INT3446")
	Name (_UID, 1)
	Name (_ADR, 0x00190002)
	Name (SSCN, Package () { 432, 507, 30 })
	Name (FMCN, Package () { 72, 160, 30 })

        /* BAR0 is assigned during PCI enumeration and saved into NVS */
        Name (RBUF, ResourceTemplate ()
        {
                Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
                Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {22}
        })

        /* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
        Name (DBUF, ResourceTemplate ()
        {
                FixedDMA (0x1A, 6, Width32Bit, DMA1)
                FixedDMA (0x1B, 7, Width32Bit, DMA2)
        })
        Method (_CRS, 0, NotSerialized)
        {
                /* Update BAR0 address and length if set in NVS*/
		 If (LNotEqual (\S4B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S4B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

                /* Check if Serial IO DMA Controller is enabled */
                Return (RBUF)
        }
        Method (_STA, 0, NotSerialized)
        {
                Return (0xF)
        }
}

Device (I2C5)
{
	/* Serial IO I2C1 Controller */
	Name (_HID,"INT3447")
	Name (_UID, 1)
	Name (_ADR, 0x00190002)
	Name (SSCN, Package () { 432, 507, 30 })
	Name (FMCN, Package () { 72, 160, 30 })

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
	Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {21}
	})

        /* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
        Name (DBUF, ResourceTemplate ()
        {
                FixedDMA (0x1A, 6, Width32Bit, DMA1)
                FixedDMA (0x1B, 7, Width32Bit, DMA2)
        })

        Method (_CRS, 0, NotSerialized)
        {
                /* Update BAR0 address and length if set in NVS */
                If (LNotEqual (\S5B0, Zero)) {
                        CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
                        CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
                        Store (\S5B0, B0AD)
                        Store (SIO_BAR_LEN, B0LN)
                }

                /* Check if Serial IO DMA Controller is enabled */
                Return (RBUF)
        }
        Method (_STA, 0, NotSerialized)
        {
                Return (0xF)
        }
}

Device (SPI0)
{
	/* Serial IO PI0 Controller */
	Name (_HID,"INT3440")
	Name (_UID, 1)
	Name (_ADR, 0x001E0002)

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {7}
	})

	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S6B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S6B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

		Return (RBUF)
	}

	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}
}

Device (SPI1)
{
	/* Serial IO SPI1 Controller */
	Name (_HID,"INT3441")
	Name (_UID, 1)
	Name (_ADR, 0x001E0003)

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {7}
	})

	/* DMA channels are only used if Serial IO DMA controller is enabled */
	Name (DBUF, ResourceTemplate ()
	{
		FixedDMA (0x10, 0, Width32Bit, DMA1) /* Tx */
		FixedDMA (0x11, 1, Width32Bit, DMA2) /* Rx */
	})
	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S7B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S7B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

		Return (RBUF)
	}
	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}
}

Device (UAR0)
{
	/* Serial IO UART0 Controller */
	Name (_HID,"INT3448")
	Name (_UID, 1)
	Name (_ADR, 0x001E0000)

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {13}
	})

	/* DMA channels are only used if
	 * Serial IO DMA controller is enabled
	 */
	Name (DBUF, ResourceTemplate ()
	{
		FixedDMA (0x16, 2, Width32Bit, DMA1) 
		FixedDMA (0x17, 3, Width32Bit, DMA2) 
	})

	Method (_CRS, 0, NotSerialized)
	{
		/* Update BAR0 address and length if set in NVS */
		If (LNotEqual (\S8B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S8B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

		Return (RBUF)
	}

	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}

}

Device (UAR1)
{
	/* Serial IO UART1 Controller */
	Name (_HID,"INT3449")
	Name (_UID, 1)
	Name (_ADR, 0x001E0001)

	/* BAR0 is assigned during PCI enumeration and saved into NVS */
	Name (RBUF, ResourceTemplate ()
	{
		Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
		Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {13}
	})

	Method (_CRS, 0, NotSerialized)
	{
		// Update BAR0 address and length if set in NVS
		If (LNotEqual (\S9B0, Zero)) {
			CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
			CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
			Store (\S9B0, B0AD)
			Store (SIO_BAR_LEN, B0LN)
		}

		Return (RBUF)
	}

	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}

}

Device (UAR2)
{
        /* Serial IO UART1 Controller */
        Name (_HID,"INT344A")
        Name (_UID, 1)
        Name (_ADR, 0x00190000)

 	/* BAR0 is assigned during PCI enumeration and saved into NVS */
        Name (RBUF, ResourceTemplate ()
        {
                Memory32Fixed (ReadWrite, 0x00000000, 0x00000000, BAR0)
                Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {13}
        })
        Method (_CRS, 0, NotSerialized)
        {
                /* Update BAR0 address and length if set in NVS */
                If (LNotEqual (\SAB0, Zero)) {
                        CreateDwordField (^RBUF, ^BAR0._BAS, B0AD)
                        CreateDwordField (^RBUF, ^BAR0._LEN, B0LN)
                        Store (\SAB0, B0AD)
                        Store (SIO_BAR_LEN, B0LN)
                }

                Return (RBUF)
        }
        Method (_STA, 0, NotSerialized)
        {
                Return (0xF)
        }
}


Device (PEMC)
{
	Name (_ADR, 0x001E0004)
	Device (CARD)
	{
		Name (_ADR, 0x00000008)
		Method (_RMV, 0x0, NotSerialized)
		{
			Return (0)
		} 
	}
}

/* SD controller */
Device (PSDC)
{
	Name (_ADR, 0x001E0006)
	Device (CARD)
	{
		Name (_ADR, 0x00000008)
		Method (_RMV, 0x0, NotSerialized)
		{
			Return (1)
		}
	}
}

