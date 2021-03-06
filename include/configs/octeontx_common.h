/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_COMMON_H__
#define __OCTEONTX_COMMON_H__

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
        func(MMC, mmc, 0) \
        func(MMC, mmc, 1) \
        func(USB, usb, 0) \
        func(SCSI, scsi, 0)

#include <config_distro_bootcmd.h>
#endif

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

/** Maximum size of image supported for bootm (and bootable FIT images) */
#define CONFIG_SYS_BOOTM_LEN		(256 << 20)

/** Memory base address */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_TEXT_BASE

/** Stack starting address */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xffff0)

/** Memory test starting address */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE

/** Memory test end address */
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0xf0000)

/** Heap size for U-Boot */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 64 * 1024 * 1024)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/* Allow environment variable to be overwritten */
#define CONFIG_ENV_OVERWRITE

/** Reduce hashes printed out */
#define CONFIG_TFTP_TSIZE

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* AHCI support Definitions */
#ifdef CONFIG_DM_SCSI
/** Maximum number of SATA devices per controller*/
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
/** Enable 48-bit SATA addressing */
# define CONFIG_LBA48
/** Enable 64-bit addressing */
# define CONFIG_SYS_64BIT_LBA
#endif

/***** SPI Defines *********/
#ifdef CONFIG_DM_SPI_FLASH
# define CONFIG_SF_DEFAULT_BUS	0
# define CONFIG_SF_DEFAULT_CS	0
#endif

/** Extra environment settings */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"loadaddr=040080000\0"	\
					"autoload=0\0" \
					"kernel_addr_r=0x02000000\0" \
					"ramdisk_addr_r=0x03000000\0" \
					"scriptaddr=0x04000000\0" \
					BOOTENV

/** Environment defines */
#define CONFIG_ENV_SIZE			0x8000
#define CONFIG_ENV_OFFSET		0xf00000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + 0x8000)
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/** Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MAXARGS		64	/** max command args */

#define CONFIG_SYS_MMC_MAX_BLK_COUNT	8191

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		env_get("prompt")

/** EMMC specific defines */
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_CMD_BKOPS_ENABLE

#if defined(CONFIG_NAND_OCTEONTX)
#define CONFIG_MTD_DEVICE
/*#define CONFIG_MTD_CONCAT */
#define CONFIG_SYS_MAX_NAND_DEVICE 8
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

#endif /* __OCTEONTX_COMMON_H__ */
