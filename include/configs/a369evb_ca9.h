/*
 * (C) Copyright 2012 Faraday Technology
 * Bing-Yao Luo <bjluo@faraday-tech.com>
 *
 * Configuration settings for the Faraday A369 board.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/a369.h>

/*
 * mach-type definition
 */
#define MACH_TYPE_FARADAY	758
#define CONFIG_MACH_TYPE	MACH_TYPE_FARADAY

/*
 * CPU
 */
#define CONFIG_DISPLAY_CPUINFO
#undef CONFIG_SYS_DCACHE_OFF 	/* define this to disable dcache */

/* Uncache controller register base area */
#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_ARCH_EARLY_INIT_R
#endif

/*
 * Linux kernel tagged list
 * Make the "bootargs" environment variable is used by Linux kernel as
 * command-line tag.
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/*
 * CPU and Board Configuration Options
 */
#undef CONFIG_USE_IRQ		/* we don't need IRQ/FIQ stuff */


/*
 * Timer
 */
#define CONFIG_SYS_HZ		1000	/* timer ticks per second */
#define CONFIG_MAIN_CLK 	33000000


/*
 * Serial console configuration
 */

/* FTUART is a high speed NS 16C550A compatible UART */
#define CONFIG_BAUDRATE		38400
#define CONFIG_CONS_INDEX	1 /* Console Index port 1(COM1) */
#define CONFIG_SYS_NS16550_COM1	CONFIG_FTUART010_BASE
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK		18432000

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

/*
 * U-Boot general commands
 */
#define CONFIG_CMD_BDI		/* bdinfo */
#define CONFIG_CMD_BOOTD	/* bootd */
#define CONFIG_CMD_ECHO 	/* echo arguments */
#define CONFIG_CMD_IMI		/* iminfo */
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot    */
#define CONFIG_CMD_RUN		/* run command in env variable  */
#define CONFIG_CMD_CACHE	/* cache enable/disable command */
#define CONFIG_VERSION_VARIABLE

/*
 * Environment variables
 */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x07FC0000
#define CONFIG_ENV_OFFSET_REDUND	0x07FE0000
#define CONFIG_ENV_SIZE 		0x20000
#define CONFIG_CMD_SAVEENV

/* Init DDR on A369 board from zynq board(SoC) */
#undef CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.                                                                                          * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */
#define CONFIG_SYS_TEXT_BASE        0x00800000

#define PHYS_SDRAM_1		0x00000000
#define PHYS_SDRAM_1_SIZE	SZ_256M

#define CONFIG_NR_DRAM_BANKS	2

#if (CONFIG_NR_DRAM_BANKS == 2)
#define PHYS_SDRAM_2		0xA0000000
#define PHYS_SDRAM_2_SIZE	SZ_256M
#endif

#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE       PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_INIT_RAM_SIZE    SZ_4K
/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_SDRAM_BASE + \
				     CONFIG_SYS_INIT_RAM_SIZE - \
				     GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MEMTEST_START    (CONFIG_SYS_SDRAM_BASE + SZ_16M)
#define CONFIG_SYS_MEMTEST_END      (CONFIG_SYS_SDRAM_BASE + SZ_32M)

/*
 * Default memory address to load image from NAND, flash, SD card, ..., etc
 */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE  + 0x2000000)

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN       SZ_8M

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* Long help messages included, undef to save memory */
#define CONFIG_SYS_PROMPT	"A369_zynq # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)


/* max number of command args */
#define CONFIG_SYS_MAXARGS	16

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE            SZ_512K
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ        SZ_32K
#define CONFIG_STACKSIZE_FIQ        SZ_32K
#endif

/*
 * Network
 */
#define CONFIG_FTGMAC100
#define CONFIG_ETHADDR		00:84:14:72:61:69  /* used by common/env_common.c */
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.68.67
#define CONFIG_SERVERIP 	192.168.68.66
#define CONFIG_PHY_MAX_ADDR	32
#define CONFIG_NET_RETRY_COUNT	20
#define CONFIG_DRIVER_ETHER
#define CONFIG_CMD_PING

/*
 * Boot argument for uImage to pass into Linux kernel after start up
 */
//#define CONFIG_BOOTARGS 	"mem=512M console=ttyS0,38400 root=/dev/mmcblk0p5 rw rootfstype=ext4 rootwait"

/*                                                                                                                             * FAT (USB & MMC)                                                                                                             */
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT

/*
 * MMC(SD) card
 */
#define CONFIG_GENERIC_MMC
#define CONFIG_FTSDC010
#define CONFIG_CMD_MMC
#define CONFIG_MMC

/*
 * Boot from SD card Command with "uImage"
 */
//#define CONFIG_BOOTCOMMAND	"mmcinfo; fatload mmc 0 uImage; bootm"
//#define CONFIG_BOOTDELAY	3

/*
 * FLASH and environment organization
 */
/*
 * NOR Flash
 */
#undef CONFIG_CMD_IMLS
#define CONFIG_SYS_NO_FLASH

#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_BASE    	0x20000000
#define PHYS_FLASH_SIZE  		SZ_64M
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* max. sector number */
#define CFG_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
#define CONFIG_CMD_FLASH
#endif  /* !CONFIG_SYS_NO_FLASH */

/*
 * NAND Flash
 */
#define CONFIG_NAND_FTNANDC021
#define CONFIG_FTNANDC021_ACTIMING_1    0x02240264
#define CONFIG_FTNANDC021_ACTIMING_2    0x42054209
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#define CONFIG_SYS_NAND_BASE		CONFIG_FTNANDC021_BASE
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_FTNANDC021_BASE }
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND

#endif	/* __CONFIG_H */
