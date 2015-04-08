/*
 * This file is part of the coreboot project.
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
 */

/* ITSS */
/* Define the needed ITSS registers used by ASL on Interrupt */

Scope (\_SB)
{
	OperationRegion (ITSS, SystemMemory, 0xfdc43100, 0x8)
	Field (ITSS, ByteAcc, NoLock, Preserve)
	{
		PARC, 8,
		PBRC, 8,
		PCRC, 8,
		PDRC, 8,
		PERC, 8,
		PFRC, 8,
		PGRC, 8,
		PHRC, 8,
	}
}
