ramstage-y += printk.c
ramstage-y += console.c
ramstage-y += vtxprintf.c
ramstage-y += vsprintf.c
ramstage-y += post.c
ramstage-y += die.c

smm-y += printk.c
smm-y += vtxprintf.c
smm-$(CONFIG_SMM_TSEG) += die.c

verstage-$(CONFIG_EARLY_CONSOLE) += vtxprintf.c early_console.c
verstage-y += console.c
verstage-y += die.c

romstage-$(CONFIG_EARLY_CONSOLE) += vtxprintf.c early_console.c
romstage-y += console.c
romstage-y += post.c
romstage-y += die.c

bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += vtxprintf.c early_console.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += console.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += die.c

ramstage-$(CONFIG_CONSOLE_SERIAL8250) += uart8250_console.c
ramstage-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart8250mem_console.c
ramstage-$(CONFIG_CONSOLE_SERIAL8250MEM_32) += uart8250mem_console.c
ramstage-$(CONFIG_USBDEBUG) += usbdebug_console.c
ramstage-$(CONFIG_CONSOLE_NE2K) += ne2k_console.c
ramstage-$(CONFIG_CONSOLE_CBMEM) += cbmem_console.c
ramstage-$(CONFIG_CONSOLE_SERIAL_IPQ806X) += uart_wrapper.c
ramstage-$(CONFIG_CONSOLE_SERIAL_BG4CD) += uart_wrapper.c

secmon-y += vtxprintf.c
secmon-y += console.c
secmon-y += die.c
secmon-y += printk.c
