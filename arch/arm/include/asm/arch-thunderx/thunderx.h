/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/
#ifndef __THUNDERX_H__
#define __THUNDERX_H__

#define CSR_PA(node, csr) ((csr) | ((uint64_t)(node) << 44))

//#define RST_BOOT 0x87E006001600ULL
#define CAVM_RST_SOFT_RST 0x87e006001680ll
#define CAVM_MIO_FUS_DAT2 0x87e003001410ll

bool alternate_pkg(void);

#define __cavm_csr_fatal(name, num_args, arg1, arg2, arg3, arg4) while(1)

/* Flag bits in top byte. The top byte of MIDR_EL1 is defined
   as ox43, the Cavium implementer code. In this number, bits
   7,5,4 are defiend as zero. We use these bits to signal
   that revision numbers should be ignored. It isn't ideal
   that these are in the middle of an already defined field,
   but this keeps the model numbers as 32 bits */
#define __OM_IGNORE_REVISION        0x80000000
#define __OM_IGNORE_MINOR_REVISION  0x20000000
#define __OM_IGNORE_MODEL           0x10000000

#define CAVIUM_CN88XX_PASS1_0   0x430f0a10
#define CAVIUM_CN88XX_PASS1_1   0x430f0a11
#define CAVIUM_CN88XX_PASS2_0   0x431f0a10
#define CAVIUM_CN88XX_PASS2_1   0x431f0a11
#define CAVIUM_CN88XX_PASS2_2   0x431f0a12
#define CAVIUM_CN88XX           (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN88XX_PASS1_X   (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)
#define CAVIUM_CN88XX_PASS2_X   (CAVIUM_CN88XX_PASS2_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN83XX_PASS1_0   0x430f0a30
#define CAVIUM_CN83XX           (CAVIUM_CN83XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN83XX_PASS1_X   (CAVIUM_CN83XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN81XX_PASS1_0   0x430f0a20
#define CAVIUM_CN81XX           (CAVIUM_CN81XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN81XX_PASS1_X   (CAVIUM_CN81XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN98XX_PASS1_0   0x430f0b10
#define CAVIUM_CN98XX           (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN98XX_PASS1_X   (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

/* These match entire families of chips */
#define CAVIUM_CN8XXX           (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_MODEL)
#define CAVIUM_CN9XXX           (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_MODEL)

static inline uint64_t cavium_get_model(void) __attribute__ ((pure, always_inline));
static inline uint64_t cavium_get_model(void)
{
#ifdef CAVM_BUILD_HOST
    extern uint32_t thunder_remote_get_model(void) __attribute__ ((pure));
    return thunder_remote_get_model();
#else
    uint64_t result;
    __asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (result));
    return result;
#endif
}

/**
 * Return non-zero if the chip matech the passed model.
 *
 * @param arg_model One of the CAVIUM_* constants for chip models and passes
 *
 * @return Non-zero if match
 */
static inline int CAVIUM_IS_MODEL(uint32_t arg_model) __attribute__ ((pure, always_inline));
static inline int CAVIUM_IS_MODEL(uint32_t arg_model)
{
    const uint32_t FAMILY = 0xff00;    /* Bits 15:8 */
    const uint32_t PARTNUM = 0xfff0;    /* Bits 15:4 */
    const uint32_t VARIANT = 0xf00000;  /* Bits 23:20 */
    const uint32_t REVISION = 0xf;      /* Bits 3:0 */

    uint32_t my_model = cavium_get_model();
    uint32_t mask;

    if (arg_model & __OM_IGNORE_MODEL)
        mask = FAMILY;
    else if (arg_model & __OM_IGNORE_REVISION)
        mask = PARTNUM;
    else if (arg_model & __OM_IGNORE_MINOR_REVISION)
        mask = PARTNUM | VARIANT;
    else
        mask = PARTNUM | VARIANT | REVISION;
    return ((arg_model & mask) == (my_model & mask));
}

/**
 * Register (RSL) xcv#_reset
 *
 * XCV Reset Registers
 * This register controls reset.
 */
union cavm_xcvx_reset
{
    uint64_t u;
    struct cavm_xcvx_reset_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t enable                : 1;  /**< [ 63: 63](R/W) Port enable. */
        uint64_t reserved_16_62        : 47;
        uint64_t clkrst                : 1;  /**< [ 15: 15](R/W) DLL CLK reset. [CLKRST] must be set if DLL bypass mode
                                                                 XCV_DLL_CTL[CLKRX_BYP,CLKTX_BYP] is used. */
        uint64_t reserved_12_14        : 3;
        uint64_t dllrst                : 1;  /**< [ 11: 11](R/W) DLL reset. */
        uint64_t reserved_8_10         : 3;
        uint64_t comp                  : 1;  /**< [  7:  7](R/W) Compensation enable. */
        uint64_t reserved_4_6          : 3;
        uint64_t tx_pkt_rst_n          : 1;  /**< [  3:  3](R/W) Packet reset for TX. */
        uint64_t tx_dat_rst_n          : 1;  /**< [  2:  2](R/W) Datapath reset for TX. */
        uint64_t rx_pkt_rst_n          : 1;  /**< [  1:  1](R/W) Packet reset for RX. */
        uint64_t rx_dat_rst_n          : 1;  /**< [  0:  0](R/W) Datapath reset for RX. */
#else /* Word 0 - Little Endian */
        uint64_t rx_dat_rst_n          : 1;  /**< [  0:  0](R/W) Datapath reset for RX. */
        uint64_t rx_pkt_rst_n          : 1;  /**< [  1:  1](R/W) Packet reset for RX. */
        uint64_t tx_dat_rst_n          : 1;  /**< [  2:  2](R/W) Datapath reset for TX. */
        uint64_t tx_pkt_rst_n          : 1;  /**< [  3:  3](R/W) Packet reset for TX. */
        uint64_t reserved_4_6          : 3;
        uint64_t comp                  : 1;  /**< [  7:  7](R/W) Compensation enable. */
        uint64_t reserved_8_10         : 3;
        uint64_t dllrst                : 1;  /**< [ 11: 11](R/W) DLL reset. */
        uint64_t reserved_12_14        : 3;
        uint64_t clkrst                : 1;  /**< [ 15: 15](R/W) DLL CLK reset. [CLKRST] must be set if DLL bypass mode
                                                                 XCV_DLL_CTL[CLKRX_BYP,CLKTX_BYP] is used. */
        uint64_t reserved_16_62        : 47;
        uint64_t enable                : 1;  /**< [ 63: 63](R/W) Port enable. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_reset_s cn; */
};
typedef union cavm_xcvx_reset cavm_xcvx_reset_t;

static inline uint64_t CAVM_XCVX_RESET(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_XCVX_RESET(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e0db000000ll + 0ll * ((a) & 0x0);
    __cavm_csr_fatal("XCVX_RESET", 1, a, 0, 0, 0);
   return 0;
}

/**
 * Register (RSL) xcv#_ctl
 *
 * XCV Control Register
 * This register contains the status control bits.
 */
union cavm_xcvx_ctl
{
    uint64_t u;
    struct cavm_xcvx_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t lpbk_ext              : 1;  /**< [  3:  3](R/W) Enable external loopback mode. External loopback loops the RX datapath to the TX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 1.
                                                                 * XCV_RESET[CLKRST]       = 1. */
        uint64_t lpbk_int              : 1;  /**< [  2:  2](R/W) Enable internal loopback mode. Internal loopback loops the TX datapath to the RX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 0.
                                                                 * XCV_RESET[CLKRST]       = 0. */
        uint64_t speed                 : 2;  /**< [  1:  0](R/W) XCV operational speed:
                                                                   0x0 = 10 Mbps.
                                                                   0x1 = 100 Mbps.
                                                                   0x2 = 1 Gbps.
                                                                   0x3 = Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t speed                 : 2;  /**< [  1:  0](R/W) XCV operational speed:
                                                                   0x0 = 10 Mbps.
                                                                   0x1 = 100 Mbps.
                                                                   0x2 = 1 Gbps.
                                                                   0x3 = Reserved. */
        uint64_t lpbk_int              : 1;  /**< [  2:  2](R/W) Enable internal loopback mode. Internal loopback loops the TX datapath to the RX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 0.
                                                                 * XCV_RESET[CLKRST]       = 0. */
        uint64_t lpbk_ext              : 1;  /**< [  3:  3](R/W) Enable external loopback mode. External loopback loops the RX datapath to the TX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 1.
                                                                 * XCV_RESET[CLKRST]       = 1. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_ctl_s cn; */
};
typedef union cavm_xcvx_ctl cavm_xcvx_ctl_t;

static inline uint64_t CAVM_XCVX_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_XCVX_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e0db000030ll + 0ll * ((a) & 0x0);
    __cavm_csr_fatal("XCVX_CTL", 1, a, 0, 0, 0);
   return 0;
}
/**
 * Register (RSL) xcv#_dll_ctl
 *
 * XCV DLL Controller Register
 * The RGMII timing specification requires that devices transmit clock and
 * data synchronously. The specification requires external sources (namely
 * the PC board trace routes) to introduce the appropriate 1.5 to 2.0 ns of
 * delay.
 *
 * To eliminate the need for the PC board delays, the RGMII interface has optional
 * on-board DLLs for both transmit and receive. For correct operation, at most one
 * of the transmitter, board, or receiver involved in an RGMII link should
 * introduce delay. By default/reset, the RGMII receivers delay the received clock,
 * and the RGMII transmitters do not delay the transmitted clock. Whether this
 * default works as-is with a given link partner depends on the behavior of the
 * link partner and the PC board.
 *
 * These are the possible modes of RGMII receive operation:
 *
 * * XCV()_DLL_CTL[CLKRX_BYP] = 0 (reset value) - The RGMII
 * receive interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote
 * transmitter nor the PC board delays the clock.
 *
 * * XCV()_DLL_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The
 * RGMII receive interface introduces no clock delay. This mode
 * is appropriate if either the remote transmitter or the PC board
 * delays the clock.
 *
 * These are the possible modes of RGMII transmit operation:
 *
 * * XCV()_DLL_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) -
 * The RGMII transmit interface introduces no clock
 * delay. This mode is appropriate is either the remote receiver
 * or the PC board delays the clock.
 *
 * * XCV()_DLL_CTL[CLKTX_BYP] = 0 - The RGMII transmit
 * interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote receiver
 * nor the PC board delays the clock.
 */
union cavm_xcvx_dll_ctl
{
    uint64_t u;
    struct cavm_xcvx_dll_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_32_63        : 32;
        uint64_t lock                  : 1;  /**< [ 31: 31](RO/H) DLL is locked. */
        uint64_t clk_set               : 7;  /**< [ 30: 24](RO/H) The clock delay as determined by the on-board hardware DLL. */
        uint64_t clkrx_byp             : 1;  /**< [ 23: 23](R/W) Bypass the RX clock delay setting.
                                                                 Skews RXC from RXD, RXCTL.
                                                                 By default, hardware internally shifts the RXC clock
                                                                 to sample RXD,RXCTL assuming clock and data and
                                                                 sourced synchronously from the link partner. */
        uint64_t clkrx_set             : 7;  /**< [ 22: 16](R/W) RX clock delay setting to use in bypass mode.
                                                                 Skews RXC from RXD. */
        uint64_t clktx_byp             : 1;  /**< [ 15: 15](R/W) Bypass the TX clock delay setting.
                                                                 Skews TXC from TXD, TXCTL.
                                                                 By default, clock and data and sourced
                                                                 synchronously. */
        uint64_t clktx_set             : 7;  /**< [ 14:  8](R/W) TX clock delay setting to use in bypass mode.
                                                                 Skews TXC from TXD. */
        uint64_t reserved_2_7          : 6;
        uint64_t refclk_sel            : 2;  /**< [  1:  0](R/W) Select the reference clock to use.  Normal RGMII specification requires a 125MHz
                                                                 oscillator.
                                                                 To reduce system cost, a 500MHz coprocessor clock can be divided down and remove the
                                                                 requirements for the external oscillator. Additionally, in some well defined systems, the
                                                                 link partner may be able to source the RXC. The RGMII would operate correctly in 1000Mbs
                                                                 mode only.
                                                                 0x0 = RGMII REFCLK.
                                                                 0x1 = RGMII RXC (1000Mbs only).
                                                                 0x2 = Divided coprocessor clock.
                                                                 0x3 = Reserved.

                                                                 Internal:
                                                                 Some programming magic could allow for 10/100 operation if
                                                                 critical, use encoding 0x1 but some programming restrictions would apply. */
#else /* Word 0 - Little Endian */
        uint64_t refclk_sel            : 2;  /**< [  1:  0](R/W) Select the reference clock to use.  Normal RGMII specification requires a 125MHz
                                                                 oscillator.
                                                                 To reduce system cost, a 500MHz coprocessor clock can be divided down and remove the
                                                                 requirements for the external oscillator. Additionally, in some well defined systems, the
                                                                 link partner may be able to source the RXC. The RGMII would operate correctly in 1000Mbs
                                                                 mode only.
                                                                 0x0 = RGMII REFCLK.
                                                                 0x1 = RGMII RXC (1000Mbs only).
                                                                 0x2 = Divided coprocessor clock.
                                                                 0x3 = Reserved.

                                                                 Internal:
                                                                 Some programming magic could allow for 10/100 operation if
                                                                 critical, use encoding 0x1 but some programming restrictions would apply. */
        uint64_t reserved_2_7          : 6;
        uint64_t clktx_set             : 7;  /**< [ 14:  8](R/W) TX clock delay setting to use in bypass mode.
                                                                 Skews TXC from TXD. */
        uint64_t clktx_byp             : 1;  /**< [ 15: 15](R/W) Bypass the TX clock delay setting.
                                                                 Skews TXC from TXD, TXCTL.
                                                                 By default, clock and data and sourced
                                                                 synchronously. */
        uint64_t clkrx_set             : 7;  /**< [ 22: 16](R/W) RX clock delay setting to use in bypass mode.
                                                                 Skews RXC from RXD. */
        uint64_t clkrx_byp             : 1;  /**< [ 23: 23](R/W) Bypass the RX clock delay setting.
                                                                 Skews RXC from RXD, RXCTL.
                                                                 By default, hardware internally shifts the RXC clock
                                                                 to sample RXD,RXCTL assuming clock and data and
                                                                 sourced synchronously from the link partner. */
        uint64_t clk_set               : 7;  /**< [ 30: 24](RO/H) The clock delay as determined by the on-board hardware DLL. */
        uint64_t lock                  : 1;  /**< [ 31: 31](RO/H) DLL is locked. */
        uint64_t reserved_32_63        : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_dll_ctl_s cn; */
};
typedef union cavm_xcvx_dll_ctl cavm_xcvx_dll_ctl_t;

static inline uint64_t CAVM_XCVX_DLL_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_XCVX_DLL_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e0db000010ll + 0ll * ((a) & 0x0);
    __cavm_csr_fatal("XCVX_DLL_CTL", 1, a, 0, 0, 0);
   return 0;
    return 0;
}


/**
 * Register (RSL) xcv#_batch_crd_ret
 *
 * XCV Batch Credit Return Register
 */
union cavm_xcvx_batch_crd_ret
{
    uint64_t u;
    struct cavm_xcvx_batch_crd_ret_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t crd_ret               : 1;  /**< [  0:  0](WO) In case of the reset event, when this register is written XCV sends out the
                                                                 initial credit batch to BGX. Initial credit value of 16. The write will only
                                                                 take effect when XCV_RESET[ENABLE] is set. */
#else /* Word 0 - Little Endian */
        uint64_t crd_ret               : 1;  /**< [  0:  0](WO) In case of the reset event, when this register is written XCV sends out the
                                                                 initial credit batch to BGX. Initial credit value of 16. The write will only
                                                                 take effect when XCV_RESET[ENABLE] is set. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_batch_crd_ret_s cn; */
};
typedef union cavm_xcvx_batch_crd_ret cavm_xcvx_batch_crd_ret_t;

static inline uint64_t CAVM_XCVX_BATCH_CRD_RET(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_XCVX_BATCH_CRD_RET(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e0db000100ll + 0ll * ((a) & 0x0);
    __cavm_csr_fatal("XCVX_BATCH_CRD_RET", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_XCVX_BATCH_CRD_RET(a) cavm_xcvx_batch_crd_ret_t
#define bustype_CAVM_XCVX_BATCH_CRD_RET(a) CSR_TYPE_RSL
#define basename_CAVM_XCVX_BATCH_CRD_RET(a) "XCVX_BATCH_CRD_RET"
#define device_bar_CAVM_XCVX_BATCH_CRD_RET(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_XCVX_BATCH_CRD_RET(a) (a)
#define arguments_CAVM_XCVX_BATCH_CRD_RET(a) (a),-1,-1,-1

#endif
