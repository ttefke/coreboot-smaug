ifeq ($(CONFIG_ARCH_ROMSTAGE_X86_32),y)

romstage-$(CONFIG_EARLY_CONSOLE) += romstage_console.c
romstage-y += cbfs_and_run.c
romstage-$(CONFIG_ARCH_RAMSTAGE_X86_32) += cpu_common.c
romstage-y += memset.c
romstage-y += memcpy.c
romstage-y += memmove.c
romstage-y += rom_media.c
romstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

endif

ifeq ($(CONFIG_ARCH_RAMSTAGE_X86_32),y)

ramstage-y += c_start.S
ramstage-y += cpu.c
ramstage-y += cpu_common.c
ramstage-y += pci_ops_conf1.c
ramstage-$(CONFIG_MMCONF_SUPPORT) += pci_ops_mmconf.c
ramstage-y += exception.c
ramstage-$(CONFIG_IOAPIC) += ioapic.c
ramstage-y += memset.c
ramstage-y += memcpy.c
ramstage-y += memmove.c
ramstage-y += ebda.c
ramstage-y += rom_media.c
ramstage-$(CONFIG_COOP_MULTITASKING) += thread.c
ramstage-$(CONFIG_COOP_MULTITASKING) += thread_switch.S
ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

smm-y += memset.c
smm-y += memcpy.c
smm-y += memmove.c
smm-y += rom_media.c

rmodules_x86_32-y += memset.c
rmodules_x86_32-y += memcpy.c
rmodules_x86_32-y += memmove.c

endif
