ifeq ($(CONFIG_HAVE_MRC),y)
cpu_incs += $(src)/soc/intel/broadwell/romstage/cache_as_ram.inc
else
ifeq ($(CONFIG_PLATFORM_USES_FSP1_1),y)
cpu_incs += $(src)/drivers/intel/fsp/fsp_1_1.inc
endif
endif

romstage-y += cpu.c
romstage-y += pch.c
romstage-y += power_state.c
romstage-$(CONFIG_HAVE_MRC) += raminit.c
romstage-y += report_platform.c
romstage-y += romstage.c
romstage-y += smbus.c
romstage-y += spi.c
romstage-y += stack.c
romstage-y += systemagent.c
romstage-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart.c

