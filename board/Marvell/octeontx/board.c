// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;
extern unsigned long fdt_base_addr;

void octeontx_cleanup_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

int octeontx_board_has_pmp(void)
{
	return (otx_is_board("sff8104") || otx_is_board("nas8104"));
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
#ifdef CONFIG_NET_OCTEONTX
	fdt_parse_phy_info();
#endif
	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = smc_dram_size(0);
	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

	mem_map_fill();

	return 0;
}

#ifdef CONFIG_NET_OCTEONTX
void board_late_probe_devices(void)
{
	struct udevice *dev;
	int err, bgx_cnt, i;

	/* Probe MAC(BGX) and NIC PF devices before Network stack init */
	bgx_cnt = otx_is_soc(CN81XX) ? 2 : 4;
	for (i = 0; i < bgx_cnt; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM, 0xA026, i,
					 &dev);
		if (err)
			debug("%s BGX%d device not found\n", __func__, i);
	}
	if (otx_is_soc(CN81XX)) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM, 0xA054, 0,
					 &dev);
		if (err)
			debug("%s RGX device not found\n", __func__);
	}
	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM, 0xA01E, 0, &dev);
	if (err)
		debug("NIC PF device not found\n");
}
#endif

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char prompt[32];

	/*
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of network interfaces can change.
	 */
	octeontx_cleanup_ethaddr();

	snprintf(prompt, sizeof(prompt), "%s> ", fdt_get_board_model());
	env_set("prompt", prompt);
	env_set("board", fdt_get_board_model());
	env_set("serial#", fdt_get_board_serial());

	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

#ifdef CONFIG_NET_OCTEONTX
	board_late_probe_devices();
#endif

	return 0;
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int show_board_info(void)
{
	char *str = NULL;

	if (otx_is_soc(CN81XX))
		str = "CN81XX";
	if (otx_is_soc(CN83XX))
		str = "CN83XX";
	printf("OcteonTX %s ARM V8 Core\n", str);

	printf("Board: %s\n", fdt_get_board_model());
	printf("Serial:%s\n", fdt_get_board_serial());

	return 0;
}

