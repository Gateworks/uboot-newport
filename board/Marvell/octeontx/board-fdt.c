// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <env.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/board.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long fdt_base_addr;

#ifdef CONFIG_NET_OCTEONTX
static int fdt_get_mdio_bus(const void *fdt, int phy_offset)
{
	int node, bus = -1;
	const u64 *reg;
	u64 addr;

	if (phy_offset < 0)
		return -1;
	/* obtain mdio node and get the reg prop */
	node = fdt_parent_offset(fdt, phy_offset);
	if (node < 0)
		return -1;

	reg = fdt_getprop(fdt, node, "reg", NULL);
	addr = fdt64_to_cpu(*reg);
	bus = (addr & (1 << 7)) ? 1 : 0;
	return bus;
}

static int fdt_get_phy_addr(const void *fdt, int phy_offset)
{
	const u32 *reg;
	int addr = -1;

	if (phy_offset < 0)
		return -1;
	reg = fdt_getprop(fdt, phy_offset, "reg", NULL);
	addr = fdt32_to_cpu(*reg);
	return addr;
}

void fdt_parse_phy_info(void)
{
	const void *fdt = gd->fdt_blob;
	int offset = 0, node, bgx_id = 0, lmacid = 0;
	const u32 *val;
	char bgxname[24];
	int len, rgx_id = 0, eth_id = 0;
	int phandle, phy_offset;
	int subnode, i;
	int bdknode;

	bdknode = fdt_path_offset(fdt, "/cavium,bdk");
	if (bdknode < 0) {
		printf("%s: bdk node is missing from device tree: %s\n",
		       __func__, fdt_strerror(bdknode));
	}

	offset = fdt_node_offset_by_compatible(fdt, -1, "pci-bridge");
	if (offset < 1)
		return;

	for (bgx_id = 0; bgx_id < MAX_BGX_PER_NODE; bgx_id++) {
		int phy_addr[LMAC_CNT] = {[0 ... LMAC_CNT - 1] = -1};
		bool autoneg_dis[LMAC_CNT] = {[0 ... LMAC_CNT - 1] = 0};
		int mdio_bus[LMAC_CNT] = {[0 ... LMAC_CNT - 1] = -1};
		bool lmac_reg[LMAC_CNT] = {[0 ... LMAC_CNT - 1] = 0};
		bool lmac_enable[LMAC_CNT] = {[0 ... LMAC_CNT - 1] = 0};

		snprintf(bgxname, sizeof(bgxname), "bgx%d", bgx_id);
		node = fdt_subnode_offset(fdt, offset, bgxname);
		if (node < 0) {
			/* check if it is rgx node */
			snprintf(bgxname, sizeof(bgxname), "rgx%d", rgx_id);
			node = fdt_subnode_offset(fdt, offset, bgxname);
			if (node < 0) {
				debug("bgx%d/rgx0 node not found\n", bgx_id);
				return;
			}
		}
		debug("bgx%d node found\n", bgx_id);

		/*
		 * loop through each of the bgx/rgx nodes
		 * to find PHY nodes
		 */
		fdt_for_each_subnode(subnode, fdt, node) {
			/* Check for reg property */
			val = fdt_getprop(fdt, subnode, "reg", &len);
			if (val) {
				debug("lmacid = %d\n", lmacid);
				lmac_reg[lmacid] = 1;
			}
			/* check for phy-handle property */
			val = fdt_getprop(fdt, subnode, "phy-handle", &len);
			if (val) {
				phandle = fdt32_to_cpu(*val);
				if (!phandle) {
					debug("phandle not valid %d\n", lmacid);
				} else {
					phy_offset = fdt_node_offset_by_phandle
							(fdt, phandle);
					phy_addr[lmacid] = fdt_get_phy_addr
							(fdt, phy_offset);
					mdio_bus[lmacid] = fdt_get_mdio_bus
							(fdt, phy_offset);
					}
				} else {
					debug("phy-handle prop not found %d\n",
					      lmacid);
				}
				/* check for autonegotiation property */
				val = fdt_getprop(fdt, subnode,
						  "cavium,disable-autonegotiation",
						  &len);
				if (val)
					autoneg_dis[lmacid] = 1;

				eth_id++;
				lmacid++;
			}

			for (i = 0; i < MAX_LMAC_PER_BGX; i++) {
				const char *str;

				snprintf(bgxname, sizeof(bgxname),
					 "BGX-ENABLE.N0.BGX%d.P%d", bgx_id, i);
				if (bdknode >= 0) {
					str = fdt_getprop(fdt, bdknode,
							  bgxname, &len);
					if (str)
						lmac_enable[i] =
							simple_strtol(str, NULL,
								      10);
				}
			}

			lmacid = 0;
			bgx_set_board_info(bgx_id, mdio_bus, phy_addr,
					   autoneg_dis, lmac_reg, lmac_enable);
		}
}
#endif

const char *fdt_get_board_model(void)
{
	const void *fdt = gd->fdt_blob;
	int ret, node, len = 16;
	const char *str = NULL;

	if (!fdt) {
		printf("ERROR: %s: no valid device tree found\n", __func__);
		return NULL;
	}

	ret = fdt_check_header(fdt);
	if (ret < 0) {
		printf("fdt: %s\n", fdt_strerror(ret));
		return NULL;
	}

	node = fdt_path_offset(fdt, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return NULL;
	}

	str = fdt_getprop(fdt, node, "BOARD-MODEL", &len);
	if (!str)
		printf("Error: cannot retrieve board model from fdt\n");
	return str;
}

const char *fdt_get_board_serial(void)
{
	const void *fdt = gd->fdt_blob;
	int ret, node, len = 16;
	const char *str = NULL;

	if (!fdt) {
		printf("ERROR: %s: no valid device tree found\n", __func__);
		return NULL;
	}

	ret = fdt_check_header(fdt);
	if (ret < 0) {
		printf("fdt: %s\n", fdt_strerror(ret));
		return NULL;
	}

	node = fdt_path_offset(fdt, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return NULL;
	}

	str = fdt_getprop(fdt, node, "BOARD-SERIAL", &len);
	if (!str)
		printf("Error: cannot retrieve board serial from fdt\n");
	return str;
}

void fdt_board_get_ethaddr(int bgx, int lmac, unsigned char *eth)
{
	const void *fdt = gd->fdt_blob;
	const char *mac = NULL;
	int offset = 0, node, len;
	int subnode, i = 0;
	char bgxname[24];

	offset = fdt_node_offset_by_compatible(fdt, -1, "pci-bridge");
	if (offset < 0) {
		printf("%s couldn't find mrml bridge node in fdt\n",
		       __func__);
		return;
	}
	if (bgx == 2 && otx_is_soc(CN81XX)) {
		snprintf(bgxname, sizeof(bgxname), "rgx%d", 0);
		lmac = 0;
	} else {
		snprintf(bgxname, sizeof(bgxname), "bgx%d", bgx);
	}

	node = fdt_subnode_offset(fdt, offset, bgxname);

	fdt_for_each_subnode(subnode, fdt, node) {
		if (i++ != lmac)
			continue;
		/* check for local-mac-address */
		mac = fdt_getprop(fdt, subnode, "local-mac-address", &len);
		if (mac) {
			debug("%s mac %pM\n", __func__, mac);
			memcpy(eth, mac, ARP_HLEN);
		} else {
			memset(eth, 0, ARP_HLEN);
		}
		debug("%s eth %pM\n", __func__, eth);
		return;
	}
}

int arch_fixup_memory_node(void *blob)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	int offset, val, i;
	int ret = 0;

	ret = fdt_check_header(blob);
	if (ret < 0) {
		printf("ERROR: %s\n", fdt_strerror(ret));
		return ret;
	}

	/* delete cavium,bdk node if it exists */
	offset = fdt_path_offset(blob, "/cavium,bdk");
	if (offset >= 0) {
		ret = fdt_del_node(blob, offset);
		if (ret < 0) {
			printf("WARNING : could not remove bdk node\n");
			return ret;
		}
		debug("%s deleted bdk node\n", __func__);
	} else {
		const char *mmc[] = {
			"/soc@0/pci@848000000000/mrml-bridge0@1,0/mmc@1,4/mmc-slot@0",
			"/soc@0/pci@848000000000/mrml-bridge0@1,0/mmc@1,4/mmc-slot@1"
		};
		const char *pem[] = {
			"/soc@0/pci@87e0c0000000",
			"/soc@0/pci@87e0c1000000",
			"/soc@0/pci@87e0c2000000",
		};

		/* disable PEMs */
		printf("updating PCI\n");
		for (i = 0; i < ARRAY_SIZE(pem); i++) {
			offset = fdt_path_offset(gd->fdt_blob, pem[i]);
			if (offset < 0) {
				printf("disabling PEM%d\n", i);
				offset = fdt_path_offset(blob, pem[i]);
				if (offset)
					fdt_del_node(blob, offset);
			}
		}

		/* update MMC slots */
		printf("updating MMC\n");
		for (i = 0; i < ARRAY_SIZE(mmc); i++) {
			offset = fdt_path_offset(gd->fdt_blob, mmc[i]);
			if (offset >= 0) {
				val = fdt_getprop_u32_default(gd->fdt_blob, mmc[i], "bus-width", 0);
				offset = fdt_path_offset(blob, mmc[i]);
				if (offset < 0)
					continue;
				switch (val) {
				case 4: /* microSD */
					printf("configuring mmc-slot%d for microSD\n", i);
					fdt_setprop_u32(blob, offset, "bus-width", 4);
					fdt_setprop(blob, offset, "cap-sd-highspeed", NULL, 0);
					fdt_setprop(blob, offset, "no-mmc", NULL, 0);
					break;
				case 8: /* eMMC */
					printf("configuring mmc-slot%d for eMMC\n", i);
					fdt_setprop_u32(blob, offset, "bus-width", 8);
					fdt_setprop(blob, offset, "mmc-ddr-3_3v", NULL, 0);
					fdt_setprop(blob, offset, "cap-mmc-highspeed", NULL, 0);
					fdt_setprop(blob, offset, "no-sd", NULL, 0);
					break;
				}
			} else {
				printf("disabling mmc-slot%d\n", i);
				offset = fdt_path_offset(blob, mmc[i]);
				if (offset)
					fdt_del_node(blob, offset);
			}
		}

		/* update refclkuaa from U-Boot dt */
		val = fdt_getprop_u32_default(gd->fdt_blob, "/soc@0/refclkuaa", "clock-frequency", 0);
		offset = fdt_path_offset(blob, "/soc@0/refclkuaa");
		if (offset >= 0 && val) {
			printf("updating refclkuaa frequency:%d\n", val);
			fdt_setprop_inplace_u32(blob, offset, "clock-frequency", val);
		}

		/* update sclk from U-Boot dt */
		val = fdt_getprop_u32_default(gd->fdt_blob, "/soc@0/sclk", "clock-frequency", 0);
		offset = fdt_path_offset(blob, "/soc@0/sclk");
		if (offset >= 0 && val) {
			printf("updating sclk frequency:%d\n", val);
			fdt_setprop_inplace_u32(blob, offset, "clock-frequency", val);
		}

		/* add board and system-serial prop */
		offset = fdt_path_offset(blob, "/");
		if (offset >= 0) {
			fdt_setprop_string(blob, 0, "system-serial", fdt_get_board_serial());
			fdt_setprop_string(blob, 0, "board", fdt_get_board_model());
		}
	}

	return 0;
}

/**
 * Return the FDT base address that was passed by ATF
 *
 * @return	FDT base address received from ATF in x1 register
 */
void *board_fdt_blob_setup(void)
{
	return (void *)fdt_base_addr;
}
