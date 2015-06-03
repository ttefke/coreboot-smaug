/*
 * This file is part of the coreboot project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <console/console.h>
#include <console/vtxprintf.h>
#if CONFIG_CONSOLE_SERIAL8250 || CONFIG_CONSOLE_SERIAL8250MEM
#include <uart8250.h>
#endif
#if CONFIG_USBDEBUG
#include <usbdebug.h>
#endif
#if CONFIG_CONSOLE_NE2K
#include <console/ne2k.h>
#endif

void console_tx_byte(unsigned char byte)
{
	if (byte == '\n')
		console_tx_byte('\r');

#if CONFIG_CONSOLE_SERIAL8250MEM
#if CONFIG_DRIVERS_OXFORD_OXPCIE
	if (oxford_oxpcie_present) {
		uart8250_mem_tx_byte(
			CONFIG_OXFORD_OXPCIE_BASE_ADDRESS + 0x1000, byte);
	}
#else
	uart8250_mem_tx_byte(CONFIG_TTYS0_BASE, byte);
#endif
#endif
#if CONFIG_CONSOLE_SERIAL8250
	uart8250_tx_byte(CONFIG_TTYS0_BASE, byte);
#endif
#if CONFIG_USBDEBUG
	usbdebug_tx_byte(0, byte);
#endif
#if CONFIG_CONSOLE_NE2K
	ne2k_append_data(&byte, 1, CONFIG_CONSOLE_NE2K_IO_PORT);
#endif
#if CONFIG_CONSOLE_CBMEM
	cbmemc_tx_byte(byte);
#endif
}

void console_tx_flush(void)
{
#if CONFIG_CONSOLE_SERIAL8250MEM
#if CONFIG_DRIVERS_OXFORD_OXPCIE
	uart8250_mem_tx_flush(CONFIG_OXFORD_OXPCIE_BASE_ADDRESS + 0x1000);
#else
	uart8250_mem_tx_flush(CONFIG_TTYS0_BASE);
#endif
#endif
#if CONFIG_CONSOLE_SERIAL8250
	uart8250_tx_flush(CONFIG_TTYS0_BASE);
#endif
#if CONFIG_CONSOLE_NE2K
	ne2k_transmit(CONFIG_CONSOLE_NE2K_IO_PORT);
#endif
#if CONFIG_USBDEBUG
	usbdebug_tx_flush(0);
#endif
}
