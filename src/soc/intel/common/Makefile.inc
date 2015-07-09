subdirs-$(CONFIG_COMMON_BOOTBLOCK) += bootblock

romstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
romstage-$(CONFIG_COMMON_RAM_INIT) += raminit.c
romstage-$(CONFIG_COMMON_RESET) += reset.c
romstage-$(CONFIG_COMMON_ROMSTAGE) += romstage.c
romstage-$(CONFIG_COMMON_STACK) += stack.c
romstage-$(CONFIG_PLATFORM_USES_FSP1_1) += util.c

ramstage-$(CONFIG_PLATFORM_USES_FSP1_1) += fsp_ramstage.c
ramstage-y += hda_verb.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += nvm.c
ramstage-$(CONFIG_COMMON_RESET) += reset.c
ramstage-$(CONFIG_COMMON_SPI_LOADING) += spi_loading.c
ramstage-$(CONFIG_PLATFORM_USES_FSP1_1) += util.c
ramstage-$(CONFIG_GOP_SUPPORT) += vbt.c
