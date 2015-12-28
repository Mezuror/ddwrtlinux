/*
 * R-Car Gen3 processor support - PFC hardware block.
 *
 * Copyright (C) 2015  Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <linux/kernel.h>

#include "core.h"
#include "sh_pfc.h"

#define PORT_GP_3(bank, fn, sfx)					\
	PORT_GP_1(bank, 0,  fn, sfx), PORT_GP_1(bank, 1,  fn, sfx),	\
	PORT_GP_1(bank, 2,  fn, sfx), PORT_GP_1(bank, 3,  fn, sfx)

#define PORT_GP_14(bank, fn, sfx)					\
	PORT_GP_3(bank, fn, sfx),					\
	PORT_GP_1(bank, 4,  fn, sfx), PORT_GP_1(bank, 5,  fn, sfx),	\
	PORT_GP_1(bank, 6,  fn, sfx), PORT_GP_1(bank, 7,  fn, sfx),	\
	PORT_GP_1(bank, 8,  fn, sfx), PORT_GP_1(bank, 9,  fn, sfx),	\
	PORT_GP_1(bank, 10, fn, sfx), PORT_GP_1(bank, 11, fn, sfx),	\
	PORT_GP_1(bank, 12, fn, sfx), PORT_GP_1(bank, 13, fn, sfx),	\
	PORT_GP_1(bank, 14, fn, sfx)

#define PORT_GP_15(bank, fn, sfx)					\
	PORT_GP_14(bank, fn, sfx),   PORT_GP_1(bank, 15, fn, sfx)

#define PORT_GP_17(bank, fn, sfx)					\
	PORT_GP_15(bank, fn, sfx),					\
	PORT_GP_1(bank, 16, fn, sfx), PORT_GP_1(bank, 17, fn, sfx)

#define PORT_GP_25(bank, fn, sfx)					\
	PORT_GP_17(bank, fn, sfx),					\
	PORT_GP_1(bank, 18, fn, sfx), PORT_GP_1(bank, 19, fn, sfx),	\
	PORT_GP_1(bank, 20, fn, sfx), PORT_GP_1(bank, 21, fn, sfx),	\
	PORT_GP_1(bank, 22, fn, sfx), PORT_GP_1(bank, 23, fn, sfx),	\
	PORT_GP_1(bank, 24, fn, sfx), PORT_GP_1(bank, 25, fn, sfx)

#define PORT_GP_27(bank, fn, sfx)					\
	PORT_GP_25(bank, fn, sfx),					\
	PORT_GP_1(bank, 26, fn, sfx), PORT_GP_1(bank, 27, fn, sfx)

#define CPU_ALL_PORT(fn, sfx)						\
	PORT_GP_15(0, fn, sfx),						\
	PORT_GP_27(1, fn, sfx),						\
	PORT_GP_14(2, fn, sfx),						\
	PORT_GP_15(3, fn, sfx),						\
	PORT_GP_17(4, fn, sfx),						\
	PORT_GP_25(5, fn, sfx),						\
	PORT_GP_32(6, fn, sfx),						\
	PORT_GP_3(7, fn, sfx)
/*
 * F_() : just information
 * FM() : macro for FN_xxx / xxx_MARK
 */

/* GPSR0 */
#define GPSR0_15	F_(D15,			IP7_11_8)
#define GPSR0_14	F_(D14,			IP7_7_4)
#define GPSR0_13	F_(D13,			IP7_3_0)
#define GPSR0_12	F_(D12,			IP6_31_28)
#define GPSR0_11	F_(D11,			IP6_27_24)
#define GPSR0_10	F_(D10,			IP6_23_20)
#define GPSR0_9		F_(D9,			IP6_19_16)
#define GPSR0_8		F_(D8,			IP6_15_12)
#define GPSR0_7		F_(D7,			IP6_11_8)
#define GPSR0_6		F_(D6,			IP6_7_4)
#define GPSR0_5		F_(D5,			IP6_3_0)
#define GPSR0_4		F_(D4,			IP5_31_28)
#define GPSR0_3		F_(D3,			IP5_27_24)
#define GPSR0_2		F_(D2,			IP5_23_20)
#define GPSR0_1		F_(D1,			IP5_19_16)
#define GPSR0_0		F_(D0,			IP5_15_12)

/* GPSR1 */
#define GPSR1_27	F_(EX_WAIT0_A,		IP5_11_8)
#define GPSR1_26	F_(WE1_N,		IP5_7_4)
#define GPSR1_25	F_(WE0_N,		IP5_3_0)
#define GPSR1_24	F_(RD_WR_N,		IP4_31_28)
#define GPSR1_23	F_(RD_N,		IP4_27_24)
#define GPSR1_22	F_(BS_N,		IP4_23_20)
#define GPSR1_21	F_(CS1_N_A26,		IP4_19_16)
#define GPSR1_20	F_(CS0_N,		IP4_15_12)
#define GPSR1_19	F_(A19,			IP4_11_8)
#define GPSR1_18	F_(A18,			IP4_7_4)
#define GPSR1_17	F_(A17,			IP4_3_0)
#define GPSR1_16	F_(A16,			IP3_31_28)
#define GPSR1_15	F_(A15,			IP3_27_24)
#define GPSR1_14	F_(A14,			IP3_23_20)
#define GPSR1_13	F_(A13,			IP3_19_16)
#define GPSR1_12	F_(A12,			IP3_15_12)
#define GPSR1_11	F_(A11,			IP3_11_8)
#define GPSR1_10	F_(A10,			IP3_7_4)
#define GPSR1_9		F_(A9,			IP3_3_0)
#define GPSR1_8		F_(A8,			IP2_31_28)
#define GPSR1_7		F_(A7,			IP2_27_24)
#define GPSR1_6		F_(A6,			IP2_23_20)
#define GPSR1_5		F_(A5,			IP2_19_16)
#define GPSR1_4		F_(A4,			IP2_15_12)
#define GPSR1_3		F_(A3,			IP2_11_8)
#define GPSR1_2		F_(A2,			IP2_7_4)
#define GPSR1_1		F_(A1,			IP2_3_0)
#define GPSR1_0		F_(A0,			IP1_31_28)

/* GPSR2 */
#define GPSR2_14	F_(AVB_AVTP_CAPTURE_A,	IP0_23_20)
#define GPSR2_13	F_(AVB_AVTP_MATCH_A,	IP0_19_16)
#define GPSR2_12	F_(AVB_LINK,		IP0_15_12)
#define GPSR2_11	F_(AVB_PHY_INT,		IP0_11_8)
#define GPSR2_10	F_(AVB_MAGIC,		IP0_7_4)
#define GPSR2_9		F_(AVB_MDC,		IP0_3_0)
#define GPSR2_8		F_(PWM2_A,		IP1_27_24)
#define GPSR2_7		F_(PWM1_A,		IP1_23_20)
#define GPSR2_6		F_(PWM0,		IP1_19_16)
#define GPSR2_5		F_(IRQ5,		IP1_15_12)
#define GPSR2_4		F_(IRQ4,		IP1_11_8)
#define GPSR2_3		F_(IRQ3,		IP1_7_4)
#define GPSR2_2		F_(IRQ2,		IP1_3_0)
#define GPSR2_1		F_(IRQ1,		IP0_31_28)
#define GPSR2_0		F_(IRQ0,		IP0_27_24)

/* GPSR3 */
#define GPSR3_15	F_(SD1_WP,		IP10_23_20)
#define GPSR3_14	F_(SD1_CD,		IP10_19_16)
#define GPSR3_13	F_(SD0_WP,		IP10_15_12)
#define GPSR3_12	F_(SD0_CD,		IP10_11_8)
#define GPSR3_11	F_(SD1_DAT3,		IP8_31_28)
#define GPSR3_10	F_(SD1_DAT2,		IP8_27_24)
#define GPSR3_9		F_(SD1_DAT1,		IP8_23_20)
#define GPSR3_8		F_(SD1_DAT0,		IP8_19_16)
#define GPSR3_7		F_(SD1_CMD,		IP8_15_12)
#define GPSR3_6		F_(SD1_CLK,		IP8_11_8)
#define GPSR3_5		F_(SD0_DAT3,		IP8_7_4)
#define GPSR3_4		F_(SD0_DAT2,		IP8_3_0)
#define GPSR3_3		F_(SD0_DAT1,		IP7_31_28)
#define GPSR3_2		F_(SD0_DAT0,		IP7_27_24)
#define GPSR3_1		F_(SD0_CMD,		IP7_23_20)
#define GPSR3_0		F_(SD0_CLK,		IP7_19_16)

/* GPSR4 */
#define GPSR4_17	FM(SD3_DS)
#define GPSR4_16	F_(SD3_DAT7,		IP10_7_4)
#define GPSR4_15	F_(SD3_DAT6,		IP10_3_0)
#define GPSR4_14	F_(SD3_DAT5,		IP9_31_28)
#define GPSR4_13	F_(SD3_DAT4,		IP9_27_24)
#define GPSR4_12	FM(SD3_DAT3)
#define GPSR4_11	FM(SD3_DAT2)
#define GPSR4_10	FM(SD3_DAT1)
#define GPSR4_9		FM(SD3_DAT0)
#define GPSR4_8		FM(SD3_CMD)
#define GPSR4_7		FM(SD3_CLK)
#define GPSR4_6		F_(SD2_DS,		IP9_23_20)
#define GPSR4_5		F_(SD2_DAT3,		IP9_19_16)
#define GPSR4_4		F_(SD2_DAT2,		IP9_15_12)
#define GPSR4_3		F_(SD2_DAT1,		IP9_11_8)
#define GPSR4_2		F_(SD2_DAT0,		IP9_7_4)
#define GPSR4_1		FM(SD2_CMD)
#define GPSR4_0		F_(SD2_CLK,		IP9_3_0)

/* GPSR5 */
#define GPSR5_25	F_(MLB_DAT,		IP13_19_16)
#define GPSR5_24	F_(MLB_SIG,		IP13_15_12)
#define GPSR5_23	F_(MLB_CLK,		IP13_11_8)
#define GPSR5_22	FM(MSIOF0_RXD)
#define GPSR5_21	F_(MSIOF0_SS2,		IP13_7_4)
#define GPSR5_20	FM(MSIOF0_TXD)
#define GPSR5_19	F_(MSIOF0_SS1,		IP13_3_0)
#define GPSR5_18	F_(MSIOF0_SYNC,		IP12_31_28)
#define GPSR5_17	FM(MSIOF0_SCK)
#define GPSR5_16	F_(HRTS0_N,		IP12_27_24)
#define GPSR5_15	F_(HCTS0_N,		IP12_23_20)
#define GPSR5_14	F_(HTX0,		IP12_19_16)
#define GPSR5_13	F_(HRX0,		IP12_15_12)
#define GPSR5_12	F_(HSCK0,		IP12_11_8)
#define GPSR5_11	F_(RX2_A,		IP12_7_4)
#define GPSR5_10	F_(TX2_A,		IP12_3_0)
#define GPSR5_9		F_(SCK2,		IP11_31_28)
#define GPSR5_8		F_(RTS1_N_TANS,		IP11_27_24)
#define GPSR5_7		F_(CTS1_N,		IP11_23_20)
#define GPSR5_6		F_(TX1_A,		IP11_19_16)
#define GPSR5_5		F_(RX1_A,		IP11_15_12)
#define GPSR5_4		F_(RTS0_N_TANS,		IP11_11_8)
#define GPSR5_3		F_(CTS0_N,		IP11_7_4)
#define GPSR5_2		F_(TX0,			IP11_3_0)
#define GPSR5_1		F_(RX0,			IP10_31_28)
#define GPSR5_0		F_(SCK0,		IP10_27_24)

/* GPSR6 */
#define GPSR6_31	F_(USB31_OVC,		IP17_7_4)
#define GPSR6_30	F_(USB31_PWEN,		IP17_3_0)
#define GPSR6_29	F_(USB30_OVC,		IP16_31_28)
#define GPSR6_28	F_(USB30_PWEN,		IP16_27_24)
#define GPSR6_27	F_(USB1_OVC,		IP16_23_20)
#define GPSR6_26	F_(USB1_PWEN,		IP16_19_16)
#define GPSR6_25	F_(USB0_OVC,		IP16_15_12)
#define GPSR6_24	F_(USB0_PWEN,		IP16_11_8)
#define GPSR6_23	F_(AUDIO_CLKB_B,	IP16_7_4)
#define GPSR6_22	F_(AUDIO_CLKA_A,	IP16_3_0)
#define GPSR6_21	F_(SSI_SDATA9_A,	IP15_31_28)
#define GPSR6_20	F_(SSI_SDATA8,		IP15_27_24)
#define GPSR6_19	F_(SSI_SDATA7,		IP15_23_20)
#define GPSR6_18	F_(SSI_WS78,		IP15_19_16)
#define GPSR6_17	F_(SSI_SCK78,		IP15_15_12)
#define GPSR6_16	F_(SSI_SDATA6,		IP15_11_8)
#define GPSR6_15	F_(SSI_WS6,		IP15_7_4)
#define GPSR6_14	F_(SSI_SCK6,		IP15_3_0)
#define GPSR6_13	FM(SSI_SDATA5)
#define GPSR6_12	FM(SSI_WS5)
#define GPSR6_11	FM(SSI_SCK5)
#define GPSR6_10	F_(SSI_SDATA4,		IP14_31_28)
#define GPSR6_9		F_(SSI_WS4,		IP14_27_24)
#define GPSR6_8		F_(SSI_SCK4,		IP14_23_20)
#define GPSR6_7		F_(SSI_SDATA3,		IP14_19_16)
#define GPSR6_6		F_(SSI_WS34,		IP14_15_12)
#define GPSR6_5		F_(SSI_SCK34,		IP14_11_8)
#define GPSR6_4		F_(SSI_SDATA2_A,	IP14_7_4)
#define GPSR6_3		F_(SSI_SDATA1_A,	IP14_3_0)
#define GPSR6_2		F_(SSI_SDATA0,		IP13_31_28)
#define GPSR6_1		F_(SSI_WS0129,		IP13_27_24)
#define GPSR6_0		F_(SSI_SCK0129,		IP13_23_20)

/* GPSR7 */
#define GPSR7_3		FM(HDMI1_CEC)
#define GPSR7_2		FM(HDMI0_CEC)
#define GPSR7_1		FM(AVS2)
#define GPSR7_0		FM(AVS1)


/* IPSRx */		/* 0 */			/* 1 */		/* 2 */			/* 3 */				/* 4 */		/* 5 */		/* 6 */			/* 7 */		/* 8 */			/* 9 */		/* A */		/* B */		/* C - F */
#define IP0_3_0		FM(AVB_MDC)		F_(0, 0)	FM(MSIOF2_SS2_C)	F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_7_4		FM(AVB_MAGIC)		F_(0, 0)	FM(MSIOF2_SS1_C)	FM(SCK4_A)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_11_8	FM(AVB_PHY_INT)		F_(0, 0)	FM(MSIOF2_SYNC_C)	FM(RX4_A)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_15_12	FM(AVB_LINK)		F_(0, 0)	FM(MSIOF2_SCK_C)	FM(TX4_A)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_19_16	FM(AVB_AVTP_MATCH_A)	F_(0, 0)	FM(MSIOF2_RXD_C)	FM(CTS4_N_A)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_23_20	FM(AVB_AVTP_CAPTURE_A)	F_(0, 0)	FM(MSIOF2_TXD_C)	FM(RTS4_N_TANS_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_27_24	FM(IRQ0)		FM(QPOLB)	F_(0, 0)		FM(DU_CDE)			FM(VI4_DATA0_B)	FM(CAN0_TX_B)	FM(CANFD0_TX_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_31_28	FM(IRQ1)		FM(QPOLA)	F_(0, 0)		FM(DU_DISP)			FM(VI4_DATA1_B)	FM(CAN0_RX_B)	FM(CANFD0_RX_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_3_0		FM(IRQ2)		FM(QCPV_QDE)	F_(0, 0)		FM(DU_EXODDF_DU_ODDF_DISP_CDE)	FM(VI4_DATA2_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(PWM3_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_7_4		FM(IRQ3)		FM(QSTVB_QVE)	FM(A25)			FM(DU_DOTCLKOUT1)		FM(VI4_DATA3_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(PWM4_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_11_8	FM(IRQ4)		FM(QSTH_QHS)	FM(A24)			FM(DU_EXHSYNC_DU_HSYNC)		FM(VI4_DATA4_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(PWM5_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_15_12	FM(IRQ5)		FM(QSTB_QHE)	FM(A23)			FM(DU_EXVSYNC_DU_VSYNC)		FM(VI4_DATA5_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(PWM6_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_19_16	FM(PWM0)		FM(AVB_AVTP_PPS)FM(A22)			F_(0, 0)			FM(VI4_DATA6_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(IECLK_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_23_20	FM(PWM1_A)		F_(0, 0)	FM(A21)			FM(HRX3_D)			FM(VI4_DATA7_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(IERX_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_27_24	FM(PWM2_A)		F_(0, 0)	FM(A20)			FM(HTX3_D)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(IETX_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_31_28	FM(A0)			FM(LCDOUT16)	FM(MSIOF3_SYNC_B)	F_(0, 0)			FM(VI4_DATA8)	F_(0, 0)	FM(DU_DB0)		F_(0, 0)	F_(0, 0)		FM(PWM3_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_3_0		FM(A1)			FM(LCDOUT17)	FM(MSIOF3_TXD_B)	F_(0, 0)			FM(VI4_DATA9)	F_(0, 0)	FM(DU_DB1)		F_(0, 0)	F_(0, 0)		FM(PWM4_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_7_4		FM(A2)			FM(LCDOUT18)	FM(MSIOF3_SCK_B)	F_(0, 0)			FM(VI4_DATA10)	F_(0, 0)	FM(DU_DB2)		F_(0, 0)	F_(0, 0)		FM(PWM5_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_11_8	FM(A3)			FM(LCDOUT19)	FM(MSIOF3_RXD_B)	F_(0, 0)			FM(VI4_DATA11)	F_(0, 0)	FM(DU_DB3)		F_(0, 0)	F_(0, 0)		FM(PWM6_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */		/* 2 */			/* 3 */				/* 4 */		/* 5 */		/* 6 */			/* 7 */		/* 8 */			/* 9 */		/* A */		/* B */		/* C - F */
#define IP2_15_12	FM(A4)			FM(LCDOUT20)	FM(MSIOF3_SS1_B)	F_(0, 0)			FM(VI4_DATA12)	FM(VI5_DATA12)	FM(DU_DB4)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_19_16	FM(A5)			FM(LCDOUT21)	FM(MSIOF3_SS2_B)	FM(SCK4_B)			FM(VI4_DATA13)	FM(VI5_DATA13)	FM(DU_DB5)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_23_20	FM(A6)			FM(LCDOUT22)	FM(MSIOF2_SS1_A)	FM(RX4_B)			FM(VI4_DATA14)	FM(VI5_DATA14)	FM(DU_DB6)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_27_24	FM(A7)			FM(LCDOUT23)	FM(MSIOF2_SS2_A)	FM(TX4_B)			FM(VI4_DATA15)	FM(VI5_DATA15)	FM(DU_DB7)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_31_28	FM(A8)			FM(RX3_B)	FM(MSIOF2_SYNC_A)	FM(HRX4_B)			F_(0, 0)	F_(0, 0)	F_(0, 0)		FM(SDA6_A)	FM(AVB_AVTP_MATCH_B)	FM(PWM1_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_3_0		FM(A9)			F_(0, 0)	FM(MSIOF2_SCK_A)	FM(CTS4_N_B)			F_(0, 0)	FM(VI5_VSYNC_N)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_7_4		FM(A10)			F_(0, 0)	FM(MSIOF2_RXD_A)	FM(RTS4_N_TANS_B)		F_(0, 0)	FM(VI5_HSYNC_N)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_11_8	FM(A11)			FM(TX3_B)	FM(MSIOF2_TXD_A)	FM(HTX4_B)			FM(HSCK4)	FM(VI5_FIELD)	F_(0, 0)		FM(SCL6_A)	FM(AVB_AVTP_CAPTURE_B)	FM(PWM2_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_15_12	FM(A12)			FM(LCDOUT12)	FM(MSIOF3_SCK_C)	F_(0, 0)			FM(HRX4_A)	FM(VI5_DATA8)	FM(DU_DG4)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_19_16	FM(A13)			FM(LCDOUT13)	FM(MSIOF3_SYNC_C)	F_(0, 0)			FM(HTX4_A)	FM(VI5_DATA9)	FM(DU_DG5)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_23_20	FM(A14)			FM(LCDOUT14)	FM(MSIOF3_RXD_C)	F_(0, 0)			FM(HCTS4_N)	FM(VI5_DATA10)	FM(DU_DG6)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_27_24	FM(A15)			FM(LCDOUT15)	FM(MSIOF3_TXD_C)	F_(0, 0)			FM(HRTS4_N)	FM(VI5_DATA11)	FM(DU_DG7)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_31_28	FM(A16)			FM(LCDOUT8)	F_(0, 0)		F_(0, 0)			FM(VI4_FIELD)	F_(0, 0)	FM(DU_DG0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_3_0		FM(A17)			FM(LCDOUT9)	F_(0, 0)		F_(0, 0)			FM(VI4_VSYNC_N)	F_(0, 0)	FM(DU_DG1)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_7_4		FM(A18)			FM(LCDOUT10)	F_(0, 0)		F_(0, 0)			FM(VI4_HSYNC_N)	F_(0, 0)	FM(DU_DG2)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_11_8	FM(A19)			FM(LCDOUT11)	F_(0, 0)		F_(0, 0)			FM(VI4_CLKENB)	F_(0, 0)	FM(DU_DG3)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_15_12	FM(CS0_N)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(VI5_CLKENB)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_19_16	FM(CS1_N_A26)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(VI5_CLK)	F_(0, 0)		FM(EX_WAIT0_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_23_20	FM(BS_N)		FM(QSTVA_QVS)	FM(MSIOF3_SCK_D)	FM(SCK3)			FM(HSCK3)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(CAN1_TX)		FM(CANFD1_TX)	FM(IETX_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_27_24	FM(RD_N)		F_(0, 0)	FM(MSIOF3_SYNC_D)	FM(RX3_A)			FM(HRX3_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(CAN0_TX_A)		FM(CANFD0_TX_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_31_28	FM(RD_WR_N)		F_(0, 0)	FM(MSIOF3_RXD_D)	FM(TX3_A)			FM(HTX3_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(CAN0_RX_A)		FM(CANFD0_RX_A)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_3_0		FM(WE0_N)		F_(0, 0)	FM(MSIOF3_TXD_D)	FM(CTS3_N)			FM(HCTS3_N)	F_(0, 0)	F_(0, 0)		FM(SCL6_B)	FM(CAN_CLK)		F_(0, 0)	FM(IECLK_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_7_4		FM(WE1_N)		F_(0, 0)	FM(MSIOF3_SS1_D)	FM(RTS3_N_TANS)			FM(HRTS3_N)	F_(0, 0)	F_(0, 0)		FM(SDA6_B)	FM(CAN1_RX)		FM(CANFD1_RX)	FM(IERX_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_11_8	FM(EX_WAIT0_A)		FM(QCLK)	F_(0, 0)		F_(0, 0)			FM(VI4_CLK)	F_(0, 0)	FM(DU_DOTCLKOUT0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_15_12	FM(D0)			FM(MSIOF2_SS1_B)FM(MSIOF3_SCK_A)	F_(0, 0)			FM(VI4_DATA16)	FM(VI5_DATA0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_19_16	FM(D1)			FM(MSIOF2_SS2_B)FM(MSIOF3_SYNC_A)	F_(0, 0)			FM(VI4_DATA17)	FM(VI5_DATA1)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_23_20	FM(D2)			F_(0, 0)	FM(MSIOF3_RXD_A)	F_(0, 0)			FM(VI4_DATA18)	FM(VI5_DATA2)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_27_24	FM(D3)			F_(0, 0)	FM(MSIOF3_TXD_A)	F_(0, 0)			FM(VI4_DATA19)	FM(VI5_DATA3)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_31_28	FM(D4)			FM(MSIOF2_SCK_B)F_(0, 0)		F_(0, 0)			FM(VI4_DATA20)	FM(VI5_DATA4)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_3_0		FM(D5)			FM(MSIOF2_SYNC_B)F_(0, 0)		F_(0, 0)			FM(VI4_DATA21)	FM(VI5_DATA5)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_7_4		FM(D6)			FM(MSIOF2_RXD_B)F_(0, 0)		F_(0, 0)			FM(VI4_DATA22)	FM(VI5_DATA6)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_11_8	FM(D7)			FM(MSIOF2_TXD_B)F_(0, 0)		F_(0, 0)			FM(VI4_DATA23)	FM(VI5_DATA7)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_15_12	FM(D8)			FM(LCDOUT0)	FM(MSIOF2_SCK_D)	FM(SCK4_C)			FM(VI4_DATA0_A)	F_(0, 0)	FM(DU_DR0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_19_16	FM(D9)			FM(LCDOUT1)	FM(MSIOF2_SYNC_D)	F_(0, 0)			FM(VI4_DATA1_A)	F_(0, 0)	FM(DU_DR1)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_23_20	FM(D10)			FM(LCDOUT2)	FM(MSIOF2_RXD_D)	FM(HRX3_B)			FM(VI4_DATA2_A)	FM(CTS4_N_C)	FM(DU_DR2)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_27_24	FM(D11)			FM(LCDOUT3)	FM(MSIOF2_TXD_D)	FM(HTX3_B)			FM(VI4_DATA3_A)	FM(RTS4_N_TANS_C)FM(DU_DR3)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_31_28	FM(D12)			FM(LCDOUT4)	FM(MSIOF2_SS1_D)	FM(RX4_C)			FM(VI4_DATA4_A)	F_(0, 0)	FM(DU_DR4)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_3_0		FM(D13)			FM(LCDOUT5)	FM(MSIOF2_SS2_D)	FM(TX4_C)			FM(VI4_DATA5_A)	F_(0, 0)	FM(DU_DR5)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_7_4		FM(D14)			FM(LCDOUT6)	FM(MSIOF3_SS1_A)	FM(HRX3_C)			FM(VI4_DATA6_A)	F_(0, 0)	FM(DU_DR6)		FM(SCL6_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_11_8	FM(D15)			FM(LCDOUT7)	FM(MSIOF3_SS2_A)	FM(HTX3_C)			FM(VI4_DATA7_A)	F_(0, 0)	FM(DU_DR7)		FM(SDA6_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_15_12	FM(FSCLKST)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_19_16	FM(SD0_CLK)		F_(0, 0)	FM(MSIOF1_SCK_E)	F_(0, 0)			F_(0, 0)	F_(0, 0)	FM(STP_OPWM_0_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */		/* 2 */			/* 3 */				/* 4 */		/* 5 */		/* 6 */			/* 7 */		/* 8 */			/* 9 */		/* A */		/* B */		/* C - F */
#define IP7_23_20	FM(SD0_CMD)		F_(0, 0)	FM(MSIOF1_SYNC_E)	F_(0, 0)			F_(0, 0)	F_(0, 0)	FM(STP_IVCXO27_0_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_27_24	FM(SD0_DAT0)		F_(0, 0)	FM(MSIOF1_RXD_E)	F_(0, 0)			F_(0, 0)	FM(TS_SCK0_B)	FM(STP_ISCLK_0_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_31_28	FM(SD0_DAT1)		F_(0, 0)	FM(MSIOF1_TXD_E)	F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC0_B)FM(STP_ISSYNC_0_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_3_0		FM(SD0_DAT2)		F_(0, 0)	FM(MSIOF1_SS1_E)	F_(0, 0)			F_(0, 0)	FM(TS_SDAT0_B)	FM(STP_ISD_0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_7_4		FM(SD0_DAT3)		F_(0, 0)	FM(MSIOF1_SS2_E)	F_(0, 0)			F_(0, 0)	FM(TS_SDEN0_B)	FM(STP_ISEN_0_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_11_8	FM(SD1_CLK)		F_(0, 0)	FM(MSIOF1_SCK_G)	F_(0, 0)			F_(0, 0)	FM(SIM0_CLK_A)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_15_12	FM(SD1_CMD)		F_(0, 0)	FM(MSIOF1_SYNC_G)	F_(0, 0)			F_(0, 0)	FM(SIM0_D_A)	FM(STP_IVCXO27_1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_19_16	FM(SD1_DAT0)		FM(SD2_DAT4)	FM(MSIOF1_RXD_G)	F_(0, 0)			F_(0, 0)	FM(TS_SCK1_B)	FM(STP_ISCLK_1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_23_20	FM(SD1_DAT1)		FM(SD2_DAT5)	FM(MSIOF1_TXD_G)	F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC1_B)FM(STP_ISSYNC_1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_27_24	FM(SD1_DAT2)		FM(SD2_DAT6)	FM(MSIOF1_SS1_G)	F_(0, 0)			F_(0, 0)	FM(TS_SDAT1_B)	FM(STP_ISD_1_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_31_28	FM(SD1_DAT3)		FM(SD2_DAT7)	FM(MSIOF1_SS2_G)	F_(0, 0)			F_(0, 0)	FM(TS_SDEN1_B)	FM(STP_ISEN_1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_3_0		FM(SD2_CLK)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_7_4		FM(SD2_DAT0)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_11_8	FM(SD2_DAT1)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_15_12	FM(SD2_DAT2)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_19_16	FM(SD2_DAT3)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_23_20	FM(SD2_DS)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(SATA_DEVSLP_B)	F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_27_24	FM(SD3_DAT4)		FM(SD2_CD_A)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_31_28	FM(SD3_DAT5)		FM(SD2_WP_A)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_3_0	FM(SD3_DAT6)		FM(SD3_CD)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_7_4	FM(SD3_DAT7)		FM(SD3_WP)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_11_8	FM(SD0_CD)		F_(0, 0)	F_(0, 0)		F_(0, 0)			FM(SCL2_B)	FM(SIM0_RST_A)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_15_12	FM(SD0_WP)		F_(0, 0)	F_(0, 0)		F_(0, 0)			FM(SDA2_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_19_16	FM(SD1_CD)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(SIM0_CLK_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_23_20	FM(SD1_WP)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(SIM0_D_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_27_24	FM(SCK0)		FM(HSCK1_B)	FM(MSIOF1_SS2_B)	FM(AUDIO_CLKC_B)		FM(SDA2_A)	FM(SIM0_RST_B)	FM(STP_OPWM_0_C)	FM(RIF0_CLK_B)	F_(0, 0)		FM(ADICHS2)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_31_28	FM(RX0)			FM(HRX1_B)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(TS_SCK0_C)	FM(STP_ISCLK_0_C)	FM(RIF0_D0_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_3_0	FM(TX0)			FM(HTX1_B)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC0_C)FM(STP_ISSYNC_0_C)	FM(RIF0_D1_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_7_4	FM(CTS0_N)		FM(HCTS1_N_B)	FM(MSIOF1_SYNC_B)	F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC1_C)FM(STP_ISSYNC_1_C)	FM(RIF1_SYNC_B)	FM(AUDIO_CLKOUT_C)	FM(ADICS_SAMP)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_11_8	FM(RTS0_N_TANS)		FM(HRTS1_N_B)	FM(MSIOF1_SS1_B)	FM(AUDIO_CLKA_B)		FM(SCL2_A)	F_(0, 0)	FM(STP_IVCXO27_1_C)	FM(RIF0_SYNC_B)	F_(0, 0)		FM(ADICHS1)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_15_12	FM(RX1_A)		FM(HRX1_A)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(TS_SDAT0_C)	FM(STP_ISD_0_C)		FM(RIF1_CLK_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_19_16	FM(TX1_A)		FM(HTX1_A)	F_(0, 0)		F_(0, 0)			F_(0, 0)	FM(TS_SDEN0_C)	FM(STP_ISEN_0_C)	FM(RIF1_D0_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_23_20	FM(CTS1_N)		FM(HCTS1_N_A)	FM(MSIOF1_RXD_B)	F_(0, 0)			F_(0, 0)	FM(TS_SDEN1_C)	FM(STP_ISEN_1_C)	FM(RIF1_D0_B)	F_(0, 0)		FM(ADIDATA)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_27_24	FM(RTS1_N_TANS)		FM(HRTS1_N_A)	FM(MSIOF1_TXD_B)	F_(0, 0)			F_(0, 0)	FM(TS_SDAT1_C)	FM(STP_ISD_1_C)		FM(RIF1_D1_B)	F_(0, 0)		FM(ADICHS0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_31_28	FM(SCK2)		FM(SCIF_CLK_B)	FM(MSIOF1_SCK_B)	F_(0, 0)			F_(0, 0)	FM(TS_SCK1_C)	FM(STP_ISCLK_1_C)	FM(RIF1_CLK_B)	F_(0, 0)		FM(ADICLK)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_3_0	FM(TX2_A)		F_(0, 0)	F_(0, 0)		FM(SD2_CD_B)			FM(SCL1_A)	F_(0, 0)	FM(FMCLK_A)		FM(RIF1_D1_C)	F_(0, 0)		FM(FSO_CFE_0_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_7_4	FM(RX2_A)		F_(0, 0)	F_(0, 0)		FM(SD2_WP_B)			FM(SDA1_A)	F_(0, 0)	FM(FMIN_A)		FM(RIF1_SYNC_C)	F_(0, 0)		FM(FSO_CFE_1_B)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_11_8	FM(HSCK0)		F_(0, 0)	FM(MSIOF1_SCK_D)	FM(AUDIO_CLKB_A)		FM(SSI_SDATA1_B)FM(TS_SCK0_D)	FM(STP_ISCLK_0_D)	FM(RIF0_CLK_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_15_12	FM(HRX0)		F_(0, 0)	FM(MSIOF1_RXD_D)	F_(0, 0)			FM(SSI_SDATA2_B)FM(TS_SDEN0_D)	FM(STP_ISEN_0_D)	FM(RIF0_D0_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_19_16	FM(HTX0)		F_(0, 0)	FM(MSIOF1_TXD_D)	F_(0, 0)			FM(SSI_SDATA9_B)FM(TS_SDAT0_D)	FM(STP_ISD_0_D)		FM(RIF0_D1_C)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_23_20	FM(HCTS0_N)		FM(RX2_B)	FM(MSIOF1_SYNC_D)	F_(0, 0)			FM(SSI_SCK9_A)	FM(TS_SPSYNC0_D)FM(STP_ISSYNC_0_D)	FM(RIF0_SYNC_C)	FM(AUDIO_CLKOUT1_A)	F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_27_24	FM(HRTS0_N)		FM(TX2_B)	FM(MSIOF1_SS1_D)	F_(0, 0)			FM(SSI_WS9_A)	F_(0, 0)	FM(STP_IVCXO27_0_D)	FM(BPFCLK_A)	FM(AUDIO_CLKOUT2_A)	F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */		/* 2 */			/* 3 */				/* 4 */		/* 5 */		/* 6 */			/* 7 */		/* 8 */			/* 9 */		/* A */		/* B */		/* C - F */
#define IP12_31_28	FM(MSIOF0_SYNC)		F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(AUDIO_CLKOUT_A)	F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_3_0	FM(MSIOF0_SS1)		FM(RX5)		F_(0, 0)		FM(AUDIO_CLKA_C)		FM(SSI_SCK2_A)	F_(0, 0)	FM(STP_IVCXO27_0_C)	F_(0, 0)	FM(AUDIO_CLKOUT3_A)	F_(0, 0)	FM(TCLK1_B)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_7_4	FM(MSIOF0_SS2)		FM(TX5)		FM(MSIOF1_SS2_D)	FM(AUDIO_CLKC_A)		FM(SSI_WS2_A)	F_(0, 0)	FM(STP_OPWM_0_D)	F_(0, 0)	FM(AUDIO_CLKOUT_D)	F_(0, 0)	FM(SPEEDIN_B)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_11_8	FM(MLB_CLK)		F_(0, 0)	FM(MSIOF1_SCK_F)	F_(0, 0)			FM(SCL1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_15_12	FM(MLB_SIG)		FM(RX1_B)	FM(MSIOF1_SYNC_F)	F_(0, 0)			FM(SDA1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_19_16	FM(MLB_DAT)		FM(TX1_B)	FM(MSIOF1_RXD_F)	F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_23_20	FM(SSI_SCK0129)		F_(0, 0)	FM(MSIOF1_TXD_F)	F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_27_24	FM(SSI_WS0129)		F_(0, 0)	FM(MSIOF1_SS1_F)	F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_31_28	FM(SSI_SDATA0)		F_(0, 0)	FM(MSIOF1_SS2_F)	F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_3_0	FM(SSI_SDATA1_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_7_4	FM(SSI_SDATA2_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)			FM(SSI_SCK1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_11_8	FM(SSI_SCK34)		F_(0, 0)	FM(MSIOF1_SS1_A)	F_(0, 0)			F_(0, 0)	F_(0, 0)	FM(STP_OPWM_0_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_15_12	FM(SSI_WS34)		FM(HCTS2_N_A)	FM(MSIOF1_SS2_A)	F_(0, 0)			F_(0, 0)	F_(0, 0)	FM(STP_IVCXO27_0_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_19_16	FM(SSI_SDATA3)		FM(HRTS2_N_A)	FM(MSIOF1_TXD_A)	F_(0, 0)			F_(0, 0)	FM(TS_SCK0_A)	FM(STP_ISCLK_0_A)	FM(RIF0_D1_A)	FM(RIF2_D0_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_23_20	FM(SSI_SCK4)		FM(HRX2_A)	FM(MSIOF1_SCK_A)	F_(0, 0)			F_(0, 0)	FM(TS_SDAT0_A)	FM(STP_ISD_0_A)		FM(RIF0_CLK_A)	FM(RIF2_CLK_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_27_24	FM(SSI_WS4)		FM(HTX2_A)	FM(MSIOF1_SYNC_A)	F_(0, 0)			F_(0, 0)	FM(TS_SDEN0_A)	FM(STP_ISEN_0_A)	FM(RIF0_SYNC_A)	FM(RIF2_SYNC_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP14_31_28	FM(SSI_SDATA4)		FM(HSCK2_A)	FM(MSIOF1_RXD_A)	F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC0_A)FM(STP_ISSYNC_0_A)	FM(RIF0_D0_A)	FM(RIF2_D1_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_3_0	FM(SSI_SCK6)		FM(USB2_PWEN)	F_(0, 0)		FM(SIM0_RST_D)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_7_4	FM(SSI_WS6)		FM(USB2_OVC)	F_(0, 0)		FM(SIM0_D_D)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_11_8	FM(SSI_SDATA6)		F_(0, 0)	F_(0, 0)		FM(SIM0_CLK_D)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	FM(SATA_DEVSLP_A)	F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_15_12	FM(SSI_SCK78)		FM(HRX2_B)	FM(MSIOF1_SCK_C)	F_(0, 0)			F_(0, 0)	FM(TS_SCK1_A)	FM(STP_ISCLK_1_A)	FM(RIF1_CLK_A)	FM(RIF3_CLK_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_19_16	FM(SSI_WS78)		FM(HTX2_B)	FM(MSIOF1_SYNC_C)	F_(0, 0)			F_(0, 0)	FM(TS_SDAT1_A)	FM(STP_ISD_1_A)		FM(RIF1_SYNC_A)	FM(RIF3_SYNC_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_23_20	FM(SSI_SDATA7)		FM(HCTS2_N_B)	FM(MSIOF1_RXD_C)	F_(0, 0)			F_(0, 0)	FM(TS_SDEN1_A)	FM(STP_ISEN_1_A)	FM(RIF1_D0_A)	FM(RIF3_D0_A)		F_(0, 0)	FM(TCLK2_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_27_24	FM(SSI_SDATA8)		FM(HRTS2_N_B)	FM(MSIOF1_TXD_C)	F_(0, 0)			F_(0, 0)	FM(TS_SPSYNC1_A)FM(STP_ISSYNC_1_A)	FM(RIF1_D1_A)	FM(RIF3_D1_A)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP15_31_28	FM(SSI_SDATA9_A)	FM(HSCK2_B)	FM(MSIOF1_SS1_C)	FM(HSCK1_A)			FM(SSI_WS1_B)	FM(SCK1)	FM(STP_IVCXO27_1_A)	FM(SCK5)	F_(0, 0)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_3_0	FM(AUDIO_CLKA_A)	F_(0, 0)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0)	FM(CC5_OSCOUT)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_7_4	FM(AUDIO_CLKB_B)	FM(SCIF_CLK_A)	F_(0, 0)		F_(0, 0)			F_(0, 0)	F_(0, 0)	FM(STP_IVCXO27_1_D)	FM(REMOCON_A)	F_(0, 0)		F_(0, 0)	FM(TCLK1_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_11_8	FM(USB0_PWEN)		F_(0, 0)	F_(0, 0)		FM(SIM0_RST_C)			F_(0, 0)	FM(TS_SCK1_D)	FM(STP_ISCLK_1_D)	FM(BPFCLK_B)	FM(RIF3_CLK_B)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_15_12	FM(USB0_OVC)		F_(0, 0)	F_(0, 0)		FM(SIM0_D_C)			F_(0, 0)	FM(TS_SDAT1_D)	FM(STP_ISD_1_D)		F_(0, 0)	FM(RIF3_SYNC_B)		F_(0, 0)	F_(0, 0)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_19_16	FM(USB1_PWEN)		F_(0, 0)	F_(0, 0)		FM(SIM0_CLK_C)			FM(SSI_SCK1_A)	FM(TS_SCK0_E)	FM(STP_ISCLK_0_E)	FM(FMCLK_B)	FM(RIF2_CLK_B)		F_(0, 0)	FM(SPEEDIN_A)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_23_20	FM(USB1_OVC)		F_(0, 0)	FM(MSIOF1_SS2_C)	F_(0, 0)			FM(SSI_WS1_A)	FM(TS_SDAT0_E)	FM(STP_ISD_0_E)		FM(FMIN_B)	FM(RIF2_SYNC_B)		F_(0, 0)	FM(REMOCON_B)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_27_24	FM(USB30_PWEN)		F_(0, 0)	F_(0, 0)		FM(AUDIO_CLKOUT_B)		FM(SSI_SCK2_B)	FM(TS_SDEN1_D)	FM(STP_ISEN_1_D)	FM(STP_OPWM_0_E)FM(RIF3_D0_B)		F_(0, 0)	FM(TCLK2_B)	FM(TPU0TO0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP16_31_28	FM(USB30_OVC)		F_(0, 0)	F_(0, 0)		FM(AUDIO_CLKOUT1_B)		FM(SSI_WS2_B)	FM(TS_SPSYNC1_D)FM(STP_ISSYNC_1_D)	FM(STP_IVCXO27_0_E)FM(RIF3_D1_B)	F_(0, 0)	FM(FSO_TOE_B)	FM(TPU0TO1)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP17_3_0	FM(USB31_PWEN)		F_(0, 0)	F_(0, 0)		FM(AUDIO_CLKOUT2_B)		FM(SSI_SCK9_B)	FM(TS_SDEN0_E)	FM(STP_ISEN_0_E)	F_(0, 0)	FM(RIF2_D0_B)		F_(0, 0)	F_(0, 0)	FM(TPU0TO2)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP17_7_4	FM(USB31_OVC)		F_(0, 0)	F_(0, 0)		FM(AUDIO_CLKOUT3_B)		FM(SSI_WS9_B)	FM(TS_SPSYNC0_E)FM(STP_ISSYNC_0_E)	F_(0, 0)	FM(RIF2_D1_B)		F_(0, 0)	F_(0, 0)	FM(TPU0TO3)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

#define PINMUX_GPSR	\
\
												GPSR6_31 \
												GPSR6_30 \
												GPSR6_29 \
												GPSR6_28 \
		GPSR1_27									GPSR6_27 \
		GPSR1_26									GPSR6_26 \
		GPSR1_25							GPSR5_25	GPSR6_25 \
		GPSR1_24							GPSR5_24	GPSR6_24 \
		GPSR1_23							GPSR5_23	GPSR6_23 \
		GPSR1_22							GPSR5_22	GPSR6_22 \
		GPSR1_21							GPSR5_21	GPSR6_21 \
		GPSR1_20							GPSR5_20	GPSR6_20 \
		GPSR1_19							GPSR5_19	GPSR6_19 \
		GPSR1_18							GPSR5_18	GPSR6_18 \
		GPSR1_17					GPSR4_17	GPSR5_17	GPSR6_17 \
		GPSR1_16					GPSR4_16	GPSR5_16	GPSR6_16 \
GPSR0_15	GPSR1_15			GPSR3_15	GPSR4_15	GPSR5_15	GPSR6_15 \
GPSR0_14	GPSR1_14	GPSR2_14	GPSR3_14	GPSR4_14	GPSR5_14	GPSR6_14 \
GPSR0_13	GPSR1_13	GPSR2_13	GPSR3_13	GPSR4_13	GPSR5_13	GPSR6_13 \
GPSR0_12	GPSR1_12	GPSR2_12	GPSR3_12	GPSR4_12	GPSR5_12	GPSR6_12 \
GPSR0_11	GPSR1_11	GPSR2_11	GPSR3_11	GPSR4_11	GPSR5_11	GPSR6_11 \
GPSR0_10	GPSR1_10	GPSR2_10	GPSR3_10	GPSR4_10	GPSR5_10	GPSR6_10 \
GPSR0_9		GPSR1_9		GPSR2_9		GPSR3_9		GPSR4_9		GPSR5_9		GPSR6_9 \
GPSR0_8		GPSR1_8		GPSR2_8		GPSR3_8		GPSR4_8		GPSR5_8		GPSR6_8 \
GPSR0_7		GPSR1_7		GPSR2_7		GPSR3_7		GPSR4_7		GPSR5_7		GPSR6_7 \
GPSR0_6		GPSR1_6		GPSR2_6		GPSR3_6		GPSR4_6		GPSR5_6		GPSR6_6 \
GPSR0_5		GPSR1_5		GPSR2_5		GPSR3_5		GPSR4_5		GPSR5_5		GPSR6_5 \
GPSR0_4		GPSR1_4		GPSR2_4		GPSR3_4		GPSR4_4		GPSR5_4		GPSR6_4 \
GPSR0_3		GPSR1_3		GPSR2_3		GPSR3_3		GPSR4_3		GPSR5_3		GPSR6_3		GPSR7_3 \
GPSR0_2		GPSR1_2		GPSR2_2		GPSR3_2		GPSR4_2		GPSR5_2		GPSR6_2		GPSR7_2 \
GPSR0_1		GPSR1_1		GPSR2_1		GPSR3_1		GPSR4_1		GPSR5_1		GPSR6_1		GPSR7_1 \
GPSR0_0		GPSR1_0		GPSR2_0		GPSR3_0		GPSR4_0		GPSR5_0		GPSR6_0		GPSR7_0

#define PINMUX_IPSR				\
\
FM(IP0_3_0)	IP0_3_0		FM(IP1_3_0)	IP1_3_0		FM(IP2_3_0)	IP2_3_0		FM(IP3_3_0)	IP3_3_0 \
FM(IP0_7_4)	IP0_7_4		FM(IP1_7_4)	IP1_7_4		FM(IP2_7_4)	IP2_7_4		FM(IP3_7_4)	IP3_7_4 \
FM(IP0_11_8)	IP0_11_8	FM(IP1_11_8)	IP1_11_8	FM(IP2_11_8)	IP2_11_8	FM(IP3_11_8)	IP3_11_8 \
FM(IP0_15_12)	IP0_15_12	FM(IP1_15_12)	IP1_15_12	FM(IP2_15_12)	IP2_15_12	FM(IP3_15_12)	IP3_15_12 \
FM(IP0_19_16)	IP0_19_16	FM(IP1_19_16)	IP1_19_16	FM(IP2_19_16)	IP2_19_16	FM(IP3_19_16)	IP3_19_16 \
FM(IP0_23_20)	IP0_23_20	FM(IP1_23_20)	IP1_23_20	FM(IP2_23_20)	IP2_23_20	FM(IP3_23_20)	IP3_23_20 \
FM(IP0_27_24)	IP0_27_24	FM(IP1_27_24)	IP1_27_24	FM(IP2_27_24)	IP2_27_24	FM(IP3_27_24)	IP3_27_24 \
FM(IP0_31_28)	IP0_31_28	FM(IP1_31_28)	IP1_31_28	FM(IP2_31_28)	IP2_31_28	FM(IP3_31_28)	IP3_31_28 \
\
FM(IP4_3_0)	IP4_3_0		FM(IP5_3_0)	IP5_3_0		FM(IP6_3_0)	IP6_3_0		FM(IP7_3_0)	IP7_3_0 \
FM(IP4_7_4)	IP4_7_4		FM(IP5_7_4)	IP5_7_4		FM(IP6_7_4)	IP6_7_4		FM(IP7_7_4)	IP7_7_4 \
FM(IP4_11_8)	IP4_11_8	FM(IP5_11_8)	IP5_11_8	FM(IP6_11_8)	IP6_11_8	FM(IP7_11_8)	IP7_11_8 \
FM(IP4_15_12)	IP4_15_12	FM(IP5_15_12)	IP5_15_12	FM(IP6_15_12)	IP6_15_12	FM(IP7_15_12)	IP7_15_12 \
FM(IP4_19_16)	IP4_19_16	FM(IP5_19_16)	IP5_19_16	FM(IP6_19_16)	IP6_19_16	FM(IP7_19_16)	IP7_19_16 \
FM(IP4_23_20)	IP4_23_20	FM(IP5_23_20)	IP5_23_20	FM(IP6_23_20)	IP6_23_20	FM(IP7_23_20)	IP7_23_20 \
FM(IP4_27_24)	IP4_27_24	FM(IP5_27_24)	IP5_27_24	FM(IP6_27_24)	IP6_27_24	FM(IP7_27_24)	IP7_27_24 \
FM(IP4_31_28)	IP4_31_28	FM(IP5_31_28)	IP5_31_28	FM(IP6_31_28)	IP6_31_28	FM(IP7_31_28)	IP7_31_28 \
\
FM(IP8_3_0)	IP8_3_0		FM(IP9_3_0)	IP9_3_0		FM(IP10_3_0)	IP10_3_0	FM(IP11_3_0)	IP11_3_0 \
FM(IP8_7_4)	IP8_7_4		FM(IP9_7_4)	IP9_7_4		FM(IP10_7_4)	IP10_7_4	FM(IP11_7_4)	IP11_7_4 \
FM(IP8_11_8)	IP8_11_8	FM(IP9_11_8)	IP9_11_8	FM(IP10_11_8)	IP10_11_8	FM(IP11_11_8)	IP11_11_8 \
FM(IP8_15_12)	IP8_15_12	FM(IP9_15_12)	IP9_15_12	FM(IP10_15_12)	IP10_15_12	FM(IP11_15_12)	IP11_15_12 \
FM(IP8_19_16)	IP8_19_16	FM(IP9_19_16)	IP9_19_16	FM(IP10_19_16)	IP10_19_16	FM(IP11_19_16)	IP11_19_16 \
FM(IP8_23_20)	IP8_23_20	FM(IP9_23_20)	IP9_23_20	FM(IP10_23_20)	IP10_23_20	FM(IP11_23_20)	IP11_23_20 \
FM(IP8_27_24)	IP8_27_24	FM(IP9_27_24)	IP9_27_24	FM(IP10_27_24)	IP10_27_24	FM(IP11_27_24)	IP11_27_24 \
FM(IP8_31_28)	IP8_31_28	FM(IP9_31_28)	IP9_31_28	FM(IP10_31_28)	IP10_31_28	FM(IP11_31_28)	IP11_31_28 \
\
FM(IP12_3_0)	IP12_3_0	FM(IP13_3_0)	IP13_3_0	FM(IP14_3_0)	IP14_3_0	FM(IP15_3_0)	IP15_3_0 \
FM(IP12_7_4)	IP12_7_4	FM(IP13_7_4)	IP13_7_4	FM(IP14_7_4)	IP14_7_4	FM(IP15_7_4)	IP15_7_4 \
FM(IP12_11_8)	IP12_11_8	FM(IP13_11_8)	IP13_11_8	FM(IP14_11_8)	IP14_11_8	FM(IP15_11_8)	IP15_11_8 \
FM(IP12_15_12)	IP12_15_12	FM(IP13_15_12)	IP13_15_12	FM(IP14_15_12)	IP14_15_12	FM(IP15_15_12)	IP15_15_12 \
FM(IP12_19_16)	IP12_19_16	FM(IP13_19_16)	IP13_19_16	FM(IP14_19_16)	IP14_19_16	FM(IP15_19_16)	IP15_19_16 \
FM(IP12_23_20)	IP12_23_20	FM(IP13_23_20)	IP13_23_20	FM(IP14_23_20)	IP14_23_20	FM(IP15_23_20)	IP15_23_20 \
FM(IP12_27_24)	IP12_27_24	FM(IP13_27_24)	IP13_27_24	FM(IP14_27_24)	IP14_27_24	FM(IP15_27_24)	IP15_27_24 \
FM(IP12_31_28)	IP12_31_28	FM(IP13_31_28)	IP13_31_28	FM(IP14_31_28)	IP14_31_28	FM(IP15_31_28)	IP15_31_28 \
\
FM(IP16_3_0)	IP16_3_0	FM(IP17_3_0)	IP17_3_0 \
FM(IP16_7_4)	IP16_7_4	FM(IP17_7_4)	IP17_7_4 \
FM(IP16_11_8)	IP16_11_8 \
FM(IP16_15_12)	IP16_15_12 \
FM(IP16_19_16)	IP16_19_16 \
FM(IP16_23_20)	IP16_23_20 \
FM(IP16_27_24)	IP16_27_24 \
FM(IP16_31_28)	IP16_31_28

/* MOD_SEL0 */			/* 0 */			/* 1 */			/* 2 */			/* 3 */			/* 4 */			/* 5 */			/* 6 */			/* 7 */
#define MOD_SEL0_30_29		FM(SEL_MSIOF3_0)	FM(SEL_MSIOF3_1)	FM(SEL_MSIOF3_2)	FM(SEL_MSIOF3_3)
#define MOD_SEL0_28_27		FM(SEL_MSIOF2_0)	FM(SEL_MSIOF2_1)	FM(SEL_MSIOF2_2)	FM(SEL_MSIOF2_3)
#define MOD_SEL0_26_25_24	FM(SEL_MSIOF1_0)	FM(SEL_MSIOF1_1)	FM(SEL_MSIOF1_2)	FM(SEL_MSIOF1_3)	FM(SEL_MSIOF1_4)	FM(SEL_MSIOF1_5)	FM(SEL_MSIOF1_6)	F_(0, 0)
#define MOD_SEL0_23		FM(SEL_LBSC_0)		FM(SEL_LBSC_1)
#define MOD_SEL0_22		FM(SEL_IEBUS_0)		FM(SEL_IEBUS_1)
#define MOD_SEL0_21_20		FM(SEL_I2C6_0)		FM(SEL_I2C6_1)		FM(SEL_I2C6_2)		F_(0, 0)
#define MOD_SEL0_19		FM(SEL_I2C2_0)		FM(SEL_I2C2_1)
#define MOD_SEL0_18		FM(SEL_I2C1_0)		FM(SEL_I2C1_1)
#define MOD_SEL0_17		FM(SEL_HSCIF4_0)	FM(SEL_HSCIF4_1)
#define MOD_SEL0_16_15		FM(SEL_HSCIF3_0)	FM(SEL_HSCIF3_1)	FM(SEL_HSCIF3_2)	FM(SEL_HSCIF3_3)
#define MOD_SEL0_14		FM(SEL_HSCIF2_0)	FM(SEL_HSCIF2_1)
#define MOD_SEL0_13		FM(SEL_HSCIF1_0)	FM(SEL_HSCIF1_1)
#define MOD_SEL0_12		FM(SEL_FSO_0)		FM(SEL_FSO_1)
#define MOD_SEL0_11		FM(SEL_FM_0)		FM(SEL_FM_1)
#define MOD_SEL0_10		FM(SEL_ETHERAVB_0)	FM(SEL_ETHERAVB_1)
#define MOD_SEL0_9		FM(SEL_DRIF3_0)		FM(SEL_DRIF3_1)
#define MOD_SEL0_8		FM(SEL_DRIF2_0)		FM(SEL_DRIF2_1)
#define MOD_SEL0_7_6		FM(SEL_DRIF1_0)		FM(SEL_DRIF1_1)		FM(SEL_DRIF1_2)		F_(0, 0)
#define MOD_SEL0_5_4		FM(SEL_DRIF0_0)		FM(SEL_DRIF0_1)		FM(SEL_DRIF0_2)		F_(0, 0)
#define MOD_SEL0_3		FM(SEL_CANFD0_0)	FM(SEL_CANFD0_1)
#define MOD_SEL0_2_1		FM(SEL_ADG_0)		FM(SEL_ADG_1)		FM(SEL_ADG_2)		FM(SEL_ADG_3)

/* MOD_SEL1 */			/* 0 */			/* 1 */			/* 2 */			/* 3 */			/* 4 */			/* 5 */			/* 6 */			/* 7 */
#define MOD_SEL1_31_30		FM(SEL_TSIF1_0)		FM(SEL_TSIF1_1)		FM(SEL_TSIF1_2)		FM(SEL_TSIF1_3)
#define MOD_SEL1_29_28_27	FM(SEL_TSIF0_0)		FM(SEL_TSIF0_1)		FM(SEL_TSIF0_2)		FM(SEL_TSIF0_3)		FM(SEL_TSIF0_4)		F_(0, 0)		F_(0, 0)		F_(0, 0)
#define MOD_SEL1_26		FM(SEL_TIMER_TMU_0)	FM(SEL_TIMER_TMU_1)
#define MOD_SEL1_25_24		FM(SEL_SSP1_1_0)	FM(SEL_SSP1_1_1)	FM(SEL_SSP1_1_2)	FM(SEL_SSP1_1_3)
#define MOD_SEL1_23_22_21	FM(SEL_SSP1_0_0)	FM(SEL_SSP1_0_1)	FM(SEL_SSP1_0_2)	FM(SEL_SSP1_0_3)	FM(SEL_SSP1_0_4)	F_(0, 0)		F_(0, 0)		F_(0, 0)
#define MOD_SEL1_20		FM(SEL_SSI_0)		FM(SEL_SSI_1)
#define MOD_SEL1_19		FM(SEL_SPEED_PULSE_0)	FM(SEL_SPEED_PULSE_1)
#define MOD_SEL1_18_17		FM(SEL_SIMCARD_0)	FM(SEL_SIMCARD_1)	FM(SEL_SIMCARD_2)	FM(SEL_SIMCARD_3)
#define MOD_SEL1_16		FM(SEL_SDHI2_0)		FM(SEL_SDHI2_1)
#define MOD_SEL1_15_14		FM(SEL_SCIF4_0)		FM(SEL_SCIF4_1)		FM(SEL_SCIF4_2)		F_(0, 0)
#define MOD_SEL1_13		FM(SEL_SCIF3_0)		FM(SEL_SCIF3_1)
#define MOD_SEL1_12		FM(SEL_SCIF2_0)		FM(SEL_SCIF2_1)
#define MOD_SEL1_11		FM(SEL_SCIF1_0)		FM(SEL_SCIF1_1)
#define MOD_SEL1_10		FM(SEL_SCIF_0)		FM(SEL_SCIF_1)
#define MOD_SEL1_9		FM(SEL_REMOCON_0)	FM(SEL_REMOCON_1)
#define MOD_SEL1_6		FM(SEL_RCAN0_0)		FM(SEL_RCAN0_1)
#define MOD_SEL1_5		FM(SEL_PWM6_0)		FM(SEL_PWM6_1)
#define MOD_SEL1_4		FM(SEL_PWM5_0)		FM(SEL_PWM5_1)
#define MOD_SEL1_3		FM(SEL_PWM4_0)		FM(SEL_PWM4_1)
#define MOD_SEL1_2		FM(SEL_PWM3_0)		FM(SEL_PWM3_1)
#define MOD_SEL1_1		FM(SEL_PWM2_0)		FM(SEL_PWM2_1)
#define MOD_SEL1_0		FM(SEL_PWM1_0)		FM(SEL_PWM1_1)

/* MOD_SEL2 */			/* 0 */			/* 1 */			/* 2 */			/* 3 */
#define MOD_SEL2_31		FM(I2C_SEL_5_0)		FM(I2C_SEL_5_1)
#define MOD_SEL2_30		FM(I2C_SEL_3_0)		FM(I2C_SEL_3_1)
#define MOD_SEL2_29		FM(I2C_SEL_0_0)		FM(I2C_SEL_0_1)
#define MOD_SEL2_2_1		FM(SEL_VSP_0)		FM(SEL_VSP_1)		FM(SEL_VSP_2)		FM(SEL_VSP_3)
#define MOD_SEL2_0		FM(SEL_VIN4_0)		FM(SEL_VIN4_1)

#define PINMUX_MOD_SELS\
\
			MOD_SEL1_31_30		MOD_SEL2_31 \
MOD_SEL0_30_29					MOD_SEL2_30 \
			MOD_SEL1_29_28_27	MOD_SEL2_29 \
MOD_SEL0_28_27 \
\
MOD_SEL0_26_25_24	MOD_SEL1_26 \
			MOD_SEL1_25_24 \
\
MOD_SEL0_23		MOD_SEL1_23_22_21 \
MOD_SEL0_22 \
MOD_SEL0_21_20 \
			MOD_SEL1_20 \
MOD_SEL0_19		MOD_SEL1_19 \
MOD_SEL0_18		MOD_SEL1_18_17 \
MOD_SEL0_17 \
MOD_SEL0_16_15		MOD_SEL1_16 \
			MOD_SEL1_15_14 \
MOD_SEL0_14 \
MOD_SEL0_13		MOD_SEL1_13 \
MOD_SEL0_12		MOD_SEL1_12 \
MOD_SEL0_11		MOD_SEL1_11 \
MOD_SEL0_10		MOD_SEL1_10 \
MOD_SEL0_9		MOD_SEL1_9 \
MOD_SEL0_8 \
MOD_SEL0_7_6 \
			MOD_SEL1_6 \
MOD_SEL0_5_4		MOD_SEL1_5 \
			MOD_SEL1_4 \
MOD_SEL0_3		MOD_SEL1_3 \
MOD_SEL0_2_1		MOD_SEL1_2		MOD_SEL2_2_1 \
			MOD_SEL1_1 \
			MOD_SEL1_0		MOD_SEL2_0


enum {
	PINMUX_RESERVED = 0,

	PINMUX_DATA_BEGIN,
	GP_ALL(DATA),
	PINMUX_DATA_END,

#define F_(x, y)
#define FM(x)	FN_##x,
	PINMUX_FUNCTION_BEGIN,
	GP_ALL(FN),
	PINMUX_GPSR
	PINMUX_IPSR
	PINMUX_MOD_SELS
	PINMUX_FUNCTION_END,
#undef F_
#undef FM

#define F_(x, y)
#define FM(x)	x##_MARK,
	PINMUX_MARK_BEGIN,
	PINMUX_GPSR
	PINMUX_IPSR
	PINMUX_MOD_SELS
	PINMUX_MARK_END,
#undef F_
#undef FM
};

static const u16 pinmux_data[] = {
	PINMUX_DATA_GP_ALL(),

	/* IPSR0 */
	PINMUX_IPSR_DATA(IP0_3_0,	AVB_MDC),
	PINMUX_IPSR_MSEL(IP0_3_0,	MSIOF2_SS2_C,		SEL_MSIOF2_2),

	PINMUX_IPSR_DATA(IP0_7_4,	AVB_MAGIC),
	PINMUX_IPSR_MSEL(IP0_7_4,	MSIOF2_SS1_C,		SEL_MSIOF2_2),
	PINMUX_IPSR_MSEL(IP0_7_4,	SCK4_A,			SEL_SCIF4_0),

	PINMUX_IPSR_DATA(IP0_11_8,	AVB_PHY_INT),
	PINMUX_IPSR_MSEL(IP0_11_8,	MSIOF2_SYNC_C,		SEL_MSIOF2_2),
	PINMUX_IPSR_MSEL(IP0_11_8,	RX4_A,			SEL_SCIF4_0),

	PINMUX_IPSR_DATA(IP0_15_12,	AVB_LINK),
	PINMUX_IPSR_MSEL(IP0_15_12,	MSIOF2_SCK_C,		SEL_MSIOF2_2),
	PINMUX_IPSR_MSEL(IP0_15_12,	TX4_A,			SEL_SCIF4_0),

	PINMUX_IPSR_MSEL(IP0_19_16,	AVB_AVTP_MATCH_A,	SEL_ETHERAVB_0),
	PINMUX_IPSR_MSEL(IP0_19_16,	MSIOF2_RXD_C,		SEL_MSIOF2_2),
	PINMUX_IPSR_MSEL(IP0_19_16,	CTS4_N_A,		SEL_SCIF4_0),

	PINMUX_IPSR_MSEL(IP0_23_20,	AVB_AVTP_CAPTURE_A,	SEL_ETHERAVB_0),
	PINMUX_IPSR_MSEL(IP0_23_20,	MSIOF2_TXD_C,		SEL_MSIOF2_2),
	PINMUX_IPSR_MSEL(IP0_23_20,	RTS4_N_TANS_A,		SEL_SCIF4_0),

	PINMUX_IPSR_DATA(IP0_27_24,	IRQ0),
	PINMUX_IPSR_DATA(IP0_27_24,	QPOLB),
	PINMUX_IPSR_DATA(IP0_27_24,	DU_CDE),
	PINMUX_IPSR_MSEL(IP0_27_24,	VI4_DATA0_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP0_27_24,	CAN0_TX_B,		SEL_RCAN0_1),
	PINMUX_IPSR_MSEL(IP0_27_24,	CANFD0_TX_B,		SEL_CANFD0_1),

	PINMUX_IPSR_DATA(IP0_31_28,	IRQ1),
	PINMUX_IPSR_DATA(IP0_31_28,	QPOLA),
	PINMUX_IPSR_DATA(IP0_31_28,	DU_DISP),
	PINMUX_IPSR_MSEL(IP0_31_28,	VI4_DATA1_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP0_31_28,	CAN0_RX_B,		SEL_RCAN0_1),
	PINMUX_IPSR_MSEL(IP0_31_28,	CANFD0_RX_B,		SEL_CANFD0_1),

	/* IPSR1 */
	PINMUX_IPSR_DATA(IP1_3_0,	IRQ2),
	PINMUX_IPSR_DATA(IP1_3_0,	QCPV_QDE),
	PINMUX_IPSR_DATA(IP1_3_0,	DU_EXODDF_DU_ODDF_DISP_CDE),
	PINMUX_IPSR_MSEL(IP1_3_0,	VI4_DATA2_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_3_0,	PWM3_B,			SEL_PWM3_1),

	PINMUX_IPSR_DATA(IP1_7_4,	IRQ3),
	PINMUX_IPSR_DATA(IP1_7_4,	QSTVB_QVE),
	PINMUX_IPSR_DATA(IP1_7_4,	A25),
	PINMUX_IPSR_DATA(IP1_7_4,	DU_DOTCLKOUT1),
	PINMUX_IPSR_MSEL(IP1_7_4,	VI4_DATA3_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_7_4,	PWM4_B,			SEL_PWM4_1),

	PINMUX_IPSR_DATA(IP1_11_8,	IRQ4),
	PINMUX_IPSR_DATA(IP1_11_8,	QSTH_QHS),
	PINMUX_IPSR_DATA(IP1_11_8,	A24),
	PINMUX_IPSR_DATA(IP1_11_8,	DU_EXHSYNC_DU_HSYNC),
	PINMUX_IPSR_MSEL(IP1_11_8,	VI4_DATA4_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_11_8,	PWM5_B,			SEL_PWM5_1),

	PINMUX_IPSR_DATA(IP1_15_12,	IRQ5),
	PINMUX_IPSR_DATA(IP1_15_12,	QSTB_QHE),
	PINMUX_IPSR_DATA(IP1_15_12,	A23),
	PINMUX_IPSR_DATA(IP1_15_12,	DU_EXVSYNC_DU_VSYNC),
	PINMUX_IPSR_MSEL(IP1_15_12,	VI4_DATA5_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_15_12,	PWM6_B,			SEL_PWM6_1),

	PINMUX_IPSR_DATA(IP1_19_16,	PWM0),
	PINMUX_IPSR_DATA(IP1_19_16,	AVB_AVTP_PPS),
	PINMUX_IPSR_DATA(IP1_19_16,	A22),
	PINMUX_IPSR_MSEL(IP1_19_16,	VI4_DATA6_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_19_16,	IECLK_B,		SEL_IEBUS_1),

	PINMUX_IPSR_MSEL(IP1_23_20,	PWM1_A,			SEL_PWM1_0),
	PINMUX_IPSR_DATA(IP1_23_20,	A21),
	PINMUX_IPSR_MSEL(IP1_23_20,	HRX3_D,			SEL_HSCIF3_3),
	PINMUX_IPSR_MSEL(IP1_23_20,	VI4_DATA7_B,		SEL_VIN4_1),
	PINMUX_IPSR_MSEL(IP1_23_20,	IERX_B,			SEL_IEBUS_1),

	PINMUX_IPSR_MSEL(IP1_27_24,	PWM2_A,			SEL_PWM2_0),
	PINMUX_IPSR_DATA(IP1_27_24,	A20),
	PINMUX_IPSR_MSEL(IP1_27_24,	HTX3_D,			SEL_HSCIF3_3),
	PINMUX_IPSR_MSEL(IP1_27_24,	IETX_B,			SEL_IEBUS_1),

	PINMUX_IPSR_DATA(IP1_31_28,	A0),
	PINMUX_IPSR_DATA(IP1_31_28,	LCDOUT16),
	PINMUX_IPSR_MSEL(IP1_31_28,	MSIOF3_SYNC_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_DATA(IP1_31_28,	VI4_DATA8),
	PINMUX_IPSR_DATA(IP1_31_28,	DU_DB0),
	PINMUX_IPSR_MSEL(IP1_31_28,	PWM3_A,			SEL_PWM3_0),

	/* IPSR2 */
	PINMUX_IPSR_DATA(IP2_3_0,	A1),
	PINMUX_IPSR_DATA(IP2_3_0,	LCDOUT17),
	PINMUX_IPSR_MSEL(IP2_3_0,	MSIOF3_TXD_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_DATA(IP2_3_0,	VI4_DATA9),
	PINMUX_IPSR_DATA(IP2_3_0,	DU_DB1),
	PINMUX_IPSR_MSEL(IP2_3_0,	PWM4_A,			SEL_PWM4_0),

	PINMUX_IPSR_DATA(IP2_7_4,	A2),
	PINMUX_IPSR_DATA(IP2_7_4,	LCDOUT18),
	PINMUX_IPSR_MSEL(IP2_7_4,	MSIOF3_SCK_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_DATA(IP2_7_4,	VI4_DATA10),
	PINMUX_IPSR_DATA(IP2_7_4,	DU_DB2),
	PINMUX_IPSR_MSEL(IP2_7_4,	PWM5_A,			SEL_PWM5_0),

	PINMUX_IPSR_DATA(IP2_11_8,	A3),
	PINMUX_IPSR_DATA(IP2_11_8,	LCDOUT19),
	PINMUX_IPSR_MSEL(IP2_11_8,	MSIOF3_RXD_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_DATA(IP2_11_8,	VI4_DATA11),
	PINMUX_IPSR_DATA(IP2_11_8,	DU_DB3),
	PINMUX_IPSR_MSEL(IP2_11_8,	PWM6_A,			SEL_PWM6_0),

	PINMUX_IPSR_DATA(IP2_15_12,	A4),
	PINMUX_IPSR_DATA(IP2_15_12,	LCDOUT20),
	PINMUX_IPSR_MSEL(IP2_15_12,	MSIOF3_SS1_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_DATA(IP2_15_12,	VI4_DATA12),
	PINMUX_IPSR_DATA(IP2_15_12,	VI5_DATA12),
	PINMUX_IPSR_DATA(IP2_15_12,	DU_DB4),

	PINMUX_IPSR_DATA(IP2_19_16,	A5),
	PINMUX_IPSR_DATA(IP2_19_16,	LCDOUT21),
	PINMUX_IPSR_MSEL(IP2_19_16,	MSIOF3_SS2_B,		SEL_MSIOF3_1),
	PINMUX_IPSR_MSEL(IP2_19_16,	SCK4_B,			SEL_SCIF4_1),
	PINMUX_IPSR_DATA(IP2_19_16,	VI4_DATA13),
	PINMUX_IPSR_DATA(IP2_19_16,	VI5_DATA13),
	PINMUX_IPSR_DATA(IP2_19_16,	DU_DB5),

	PINMUX_IPSR_DATA(IP2_23_20,	A6),
	PINMUX_IPSR_DATA(IP2_23_20,	LCDOUT22),
	PINMUX_IPSR_MSEL(IP2_23_20,	MSIOF2_SS1_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP2_23_20,	RX4_B,			SEL_SCIF4_1),
	PINMUX_IPSR_DATA(IP2_23_20,	VI4_DATA14),
	PINMUX_IPSR_DATA(IP2_23_20,	VI5_DATA14),
	PINMUX_IPSR_DATA(IP2_23_20,	DU_DB6),

	PINMUX_IPSR_DATA(IP2_27_24,	A7),
	PINMUX_IPSR_DATA(IP2_27_24,	LCDOUT23),
	PINMUX_IPSR_MSEL(IP2_27_24,	MSIOF2_SS2_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP2_27_24,	TX4_B,			SEL_SCIF4_1),
	PINMUX_IPSR_DATA(IP2_27_24,	VI4_DATA15),
	PINMUX_IPSR_DATA(IP2_27_24,	VI5_DATA15),
	PINMUX_IPSR_DATA(IP2_27_24,	DU_DB7),

	PINMUX_IPSR_DATA(IP2_31_28,	A8),
	PINMUX_IPSR_MSEL(IP2_31_28,	RX3_B,			SEL_SCIF3_1),
	PINMUX_IPSR_MSEL(IP2_31_28,	MSIOF2_SYNC_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP2_31_28,	HRX4_B,			SEL_HSCIF4_1),
	PINMUX_IPSR_MSEL(IP2_31_28,	SDA6_A,			SEL_I2C6_0),
	PINMUX_IPSR_MSEL(IP2_31_28,	AVB_AVTP_MATCH_B,	SEL_ETHERAVB_1),
	PINMUX_IPSR_MSEL(IP2_31_28,	PWM1_B,			SEL_PWM1_1),

	/* IPSR3 */
	PINMUX_IPSR_DATA(IP3_3_0,	A9),
	PINMUX_IPSR_MSEL(IP3_3_0,	MSIOF2_SCK_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP3_3_0,	CTS4_N_B,		SEL_SCIF4_1),
	PINMUX_IPSR_DATA(IP3_3_0,	VI5_VSYNC_N),

	PINMUX_IPSR_DATA(IP3_7_4,	A10),
	PINMUX_IPSR_MSEL(IP3_7_4,	MSIOF2_RXD_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP3_7_4,	RTS4_N_TANS_B,		SEL_SCIF4_1),
	PINMUX_IPSR_DATA(IP3_7_4,	VI5_HSYNC_N),

	PINMUX_IPSR_DATA(IP3_11_8,	A11),
	PINMUX_IPSR_MSEL(IP3_11_8,	TX3_B,			SEL_SCIF3_1),
	PINMUX_IPSR_MSEL(IP3_11_8,	MSIOF2_TXD_A,		SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP3_11_8,	HTX4_B,			SEL_HSCIF4_1),
	PINMUX_IPSR_DATA(IP3_11_8,	HSCK4),
	PINMUX_IPSR_DATA(IP3_11_8,	VI5_FIELD),
	PINMUX_IPSR_MSEL(IP3_11_8,	SCL6_A,			SEL_I2C6_0),
	PINMUX_IPSR_MSEL(IP3_11_8,	AVB_AVTP_CAPTURE_B,	SEL_ETHERAVB_1),
	PINMUX_IPSR_MSEL(IP3_11_8,	PWM2_B,			SEL_PWM2_1),

	PINMUX_IPSR_DATA(IP3_15_12,	A12),
	PINMUX_IPSR_DATA(IP3_15_12,	LCDOUT12),
	PINMUX_IPSR_MSEL(IP3_15_12,	MSIOF3_SCK_C,		SEL_MSIOF3_2),
	PINMUX_IPSR_MSEL(IP3_15_12,	HRX4_A,			SEL_HSCIF4_0),
	PINMUX_IPSR_DATA(IP3_15_12,	VI5_DATA8),
	PINMUX_IPSR_DATA(IP3_15_12,	DU_DG4),

	PINMUX_IPSR_DATA(IP3_19_16,	A13),
	PINMUX_IPSR_DATA(IP3_19_16,	LCDOUT13),
	PINMUX_IPSR_MSEL(IP3_19_16,	MSIOF3_SYNC_C,		SEL_MSIOF3_2),
	PINMUX_IPSR_MSEL(IP3_19_16,	HTX4_A,			SEL_HSCIF4_0),
	PINMUX_IPSR_DATA(IP3_19_16,	VI5_DATA9),
	PINMUX_IPSR_DATA(IP3_19_16,	DU_DG5),

	PINMUX_IPSR_DATA(IP3_23_20,	A14),
	PINMUX_IPSR_DATA(IP3_23_20,	LCDOUT14),
	PINMUX_IPSR_MSEL(IP3_23_20,	MSIOF3_RXD_C,		SEL_MSIOF3_2),
	PINMUX_IPSR_DATA(IP3_23_20,	HCTS4_N),
	PINMUX_IPSR_DATA(IP3_23_20,	VI5_DATA10),
	PINMUX_IPSR_DATA(IP3_23_20,	DU_DG6),

	PINMUX_IPSR_DATA(IP3_27_24,	A15),
	PINMUX_IPSR_DATA(IP3_27_24,	LCDOUT15),
	PINMUX_IPSR_MSEL(IP3_27_24,	MSIOF3_TXD_C,		SEL_MSIOF3_2),
	PINMUX_IPSR_DATA(IP3_27_24,	HRTS4_N),
	PINMUX_IPSR_DATA(IP3_27_24,	VI5_DATA11),
	PINMUX_IPSR_DATA(IP3_27_24,	DU_DG7),

	PINMUX_IPSR_DATA(IP3_31_28,	A16),
	PINMUX_IPSR_DATA(IP3_31_28,	LCDOUT8),
	PINMUX_IPSR_DATA(IP3_31_28,	VI4_FIELD),
	PINMUX_IPSR_DATA(IP3_31_28,	DU_DG0),

	/* IPSR4 */
	PINMUX_IPSR_DATA(IP4_3_0,	A17),
	PINMUX_IPSR_DATA(IP4_3_0,	LCDOUT9),
	PINMUX_IPSR_DATA(IP4_3_0,	VI4_VSYNC_N),
	PINMUX_IPSR_DATA(IP4_3_0,	DU_DG1),

	PINMUX_IPSR_DATA(IP4_7_4,	A18),
	PINMUX_IPSR_DATA(IP4_7_4,	LCDOUT10),
	PINMUX_IPSR_DATA(IP4_7_4,	VI4_HSYNC_N),
	PINMUX_IPSR_DATA(IP4_7_4,	DU_DG2),

	PINMUX_IPSR_DATA(IP4_11_8,	A19),
	PINMUX_IPSR_DATA(IP4_11_8,	LCDOUT11),
	PINMUX_IPSR_DATA(IP4_11_8,	VI4_CLKENB),
	PINMUX_IPSR_DATA(IP4_11_8,	DU_DG3),

	PINMUX_IPSR_DATA(IP4_15_12,	CS0_N),
	PINMUX_IPSR_DATA(IP4_15_12,	VI5_CLKENB),

	PINMUX_IPSR_DATA(IP4_19_16,	CS1_N_A26),
	PINMUX_IPSR_DATA(IP4_19_16,	VI5_CLK),
	PINMUX_IPSR_MSEL(IP4_19_16,	EX_WAIT0_B,		SEL_LBSC_1),

	PINMUX_IPSR_DATA(IP4_23_20,	BS_N),
	PINMUX_IPSR_DATA(IP4_23_20,	QSTVA_QVS),
	PINMUX_IPSR_MSEL(IP4_23_20,	MSIOF3_SCK_D,		SEL_MSIOF3_3),
	PINMUX_IPSR_DATA(IP4_23_20,	SCK3),
	PINMUX_IPSR_DATA(IP4_23_20,	HSCK3),
	PINMUX_IPSR_DATA(IP4_23_20,	CAN1_TX),
	PINMUX_IPSR_DATA(IP4_23_20,	CANFD1_TX),
	PINMUX_IPSR_MSEL(IP4_23_20,	IETX_A,			SEL_IEBUS_0),

	PINMUX_IPSR_DATA(IP4_27_24,	RD_N),
	PINMUX_IPSR_MSEL(IP4_27_24,	MSIOF3_SYNC_D,		SEL_MSIOF3_3),
	PINMUX_IPSR_MSEL(IP4_27_24,	RX3_A,			SEL_SCIF3_0),
	PINMUX_IPSR_MSEL(IP4_27_24,	HRX3_A,			SEL_HSCIF3_0),
	PINMUX_IPSR_MSEL(IP4_27_24,	CAN0_TX_A,		SEL_RCAN0_0),
	PINMUX_IPSR_MSEL(IP4_27_24,	CANFD0_TX_A,		SEL_CANFD0_0),

	PINMUX_IPSR_DATA(IP4_31_28,	RD_WR_N),
	PINMUX_IPSR_MSEL(IP4_31_28,	MSIOF3_RXD_D,		SEL_MSIOF3_3),
	PINMUX_IPSR_MSEL(IP4_31_28,	TX3_A,			SEL_SCIF3_0),
	PINMUX_IPSR_MSEL(IP4_31_28,	HTX3_A,			SEL_HSCIF3_0),
	PINMUX_IPSR_MSEL(IP4_31_28,	CAN0_RX_A,		SEL_RCAN0_0),
	PINMUX_IPSR_MSEL(IP4_31_28,	CANFD0_RX_A,		SEL_CANFD0_0),

	/* IPSR5 */
	PINMUX_IPSR_DATA(IP5_3_0,	WE0_N),
	PINMUX_IPSR_MSEL(IP5_3_0,	MSIOF3_TXD_D,		SEL_MSIOF3_3),
	PINMUX_IPSR_DATA(IP5_3_0,	CTS3_N),
	PINMUX_IPSR_DATA(IP5_3_0,	HCTS3_N),
	PINMUX_IPSR_MSEL(IP5_3_0,	SCL6_B,			SEL_I2C6_1),
	PINMUX_IPSR_DATA(IP5_3_0,	CAN_CLK),
	PINMUX_IPSR_MSEL(IP5_3_0,	IECLK_A,		SEL_IEBUS_0),

	PINMUX_IPSR_DATA(IP5_7_4,	WE1_N),
	PINMUX_IPSR_MSEL(IP5_7_4,	MSIOF3_SS1_D,		SEL_MSIOF3_3),
	PINMUX_IPSR_DATA(IP5_7_4,	RTS3_N_TANS),
	PINMUX_IPSR_DATA(IP5_7_4,	HRTS3_N),
	PINMUX_IPSR_MSEL(IP5_7_4,	SDA6_B,			SEL_I2C6_1),
	PINMUX_IPSR_DATA(IP5_7_4,	CAN1_RX),
	PINMUX_IPSR_DATA(IP5_7_4,	CANFD1_RX),
	PINMUX_IPSR_MSEL(IP5_7_4,	IERX_A,			SEL_IEBUS_0),

	PINMUX_IPSR_MSEL(IP5_11_8,	EX_WAIT0_A,		SEL_LBSC_0),
	PINMUX_IPSR_DATA(IP5_11_8,	QCLK),
	PINMUX_IPSR_DATA(IP5_11_8,	VI4_CLK),
	PINMUX_IPSR_DATA(IP5_11_8,	DU_DOTCLKOUT0),

	PINMUX_IPSR_DATA(IP5_15_12,	D0),
	PINMUX_IPSR_MSEL(IP5_15_12,	MSIOF2_SS1_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_MSEL(IP5_15_12,	MSIOF3_SCK_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_DATA(IP5_15_12,	VI4_DATA16),
	PINMUX_IPSR_DATA(IP5_15_12,	VI5_DATA0),

	PINMUX_IPSR_DATA(IP5_19_16,	D1),
	PINMUX_IPSR_MSEL(IP5_19_16,	MSIOF2_SS2_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_MSEL(IP5_19_16,	MSIOF3_SYNC_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_DATA(IP5_19_16,	VI4_DATA17),
	PINMUX_IPSR_DATA(IP5_19_16,	VI5_DATA1),

	PINMUX_IPSR_DATA(IP5_23_20,	D2),
	PINMUX_IPSR_MSEL(IP5_23_20,	MSIOF3_RXD_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_DATA(IP5_23_20,	VI4_DATA18),
	PINMUX_IPSR_DATA(IP5_23_20,	VI5_DATA2),

	PINMUX_IPSR_DATA(IP5_27_24,	D3),
	PINMUX_IPSR_MSEL(IP5_27_24,	MSIOF3_TXD_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_DATA(IP5_27_24,	VI4_DATA19),
	PINMUX_IPSR_DATA(IP5_27_24,	VI5_DATA3),

	PINMUX_IPSR_DATA(IP5_31_28,	D4),
	PINMUX_IPSR_MSEL(IP5_31_28,	MSIOF2_SCK_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_DATA(IP5_31_28,	VI4_DATA20),
	PINMUX_IPSR_DATA(IP5_31_28,	VI5_DATA4),

	/* IPSR6 */
	PINMUX_IPSR_DATA(IP6_3_0,	D5),
	PINMUX_IPSR_MSEL(IP6_3_0,	MSIOF2_SYNC_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_DATA(IP6_3_0,	VI4_DATA21),
	PINMUX_IPSR_DATA(IP6_3_0,	VI5_DATA5),

	PINMUX_IPSR_DATA(IP6_7_4,	D6),
	PINMUX_IPSR_MSEL(IP6_7_4,	MSIOF2_RXD_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_DATA(IP6_7_4,	VI4_DATA22),
	PINMUX_IPSR_DATA(IP6_7_4,	VI5_DATA6),

	PINMUX_IPSR_DATA(IP6_11_8,	D7),
	PINMUX_IPSR_MSEL(IP6_11_8,	MSIOF2_TXD_B,		SEL_MSIOF2_1),
	PINMUX_IPSR_DATA(IP6_11_8,	VI4_DATA23),
	PINMUX_IPSR_DATA(IP6_11_8,	VI5_DATA7),

	PINMUX_IPSR_DATA(IP6_15_12,	D8),
	PINMUX_IPSR_DATA(IP6_15_12,	LCDOUT0),
	PINMUX_IPSR_MSEL(IP6_15_12,	MSIOF2_SCK_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP6_15_12,	SCK4_C,			SEL_SCIF4_2),
	PINMUX_IPSR_MSEL(IP6_15_12,	VI4_DATA0_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP6_15_12,	DU_DR0),

	PINMUX_IPSR_DATA(IP6_19_16,	D9),
	PINMUX_IPSR_DATA(IP6_19_16,	LCDOUT1),
	PINMUX_IPSR_MSEL(IP6_19_16,	MSIOF2_SYNC_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP6_19_16,	VI4_DATA1_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP6_19_16,	DU_DR1),

	PINMUX_IPSR_DATA(IP6_23_20,	D10),
	PINMUX_IPSR_DATA(IP6_23_20,	LCDOUT2),
	PINMUX_IPSR_MSEL(IP6_23_20,	MSIOF2_RXD_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP6_23_20,	HRX3_B,			SEL_HSCIF3_1),
	PINMUX_IPSR_MSEL(IP6_23_20,	VI4_DATA2_A,		SEL_VIN4_0),
	PINMUX_IPSR_MSEL(IP6_23_20,	CTS4_N_C,		SEL_SCIF4_2),
	PINMUX_IPSR_DATA(IP6_23_20,	DU_DR2),

	PINMUX_IPSR_DATA(IP6_27_24,	D11),
	PINMUX_IPSR_DATA(IP6_27_24,	LCDOUT3),
	PINMUX_IPSR_MSEL(IP6_27_24,	MSIOF2_TXD_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP6_27_24,	HTX3_B,			SEL_HSCIF3_1),
	PINMUX_IPSR_MSEL(IP6_27_24,	VI4_DATA3_A,		SEL_VIN4_0),
	PINMUX_IPSR_MSEL(IP6_27_24,	RTS4_N_TANS_C,		SEL_SCIF4_2),
	PINMUX_IPSR_DATA(IP6_27_24,	DU_DR3),

	PINMUX_IPSR_DATA(IP6_31_28,	D12),
	PINMUX_IPSR_DATA(IP6_31_28,	LCDOUT4),
	PINMUX_IPSR_MSEL(IP6_31_28,	MSIOF2_SS1_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP6_31_28,	RX4_C,			SEL_SCIF4_2),
	PINMUX_IPSR_MSEL(IP6_31_28,	VI4_DATA4_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP6_31_28,	DU_DR4),

	/* IPSR7 */
	PINMUX_IPSR_DATA(IP7_3_0,	D13),
	PINMUX_IPSR_DATA(IP7_3_0,	LCDOUT5),
	PINMUX_IPSR_MSEL(IP7_3_0,	MSIOF2_SS2_D,		SEL_MSIOF2_3),
	PINMUX_IPSR_MSEL(IP7_3_0,	TX4_C,			SEL_SCIF4_2),
	PINMUX_IPSR_MSEL(IP7_3_0,	VI4_DATA5_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP7_3_0,	DU_DR5),

	PINMUX_IPSR_DATA(IP7_7_4,	D14),
	PINMUX_IPSR_DATA(IP7_7_4,	LCDOUT6),
	PINMUX_IPSR_MSEL(IP7_7_4,	MSIOF3_SS1_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_MSEL(IP7_7_4,	HRX3_C,			SEL_HSCIF3_2),
	PINMUX_IPSR_MSEL(IP7_7_4,	VI4_DATA6_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP7_7_4,	DU_DR6),
	PINMUX_IPSR_MSEL(IP7_7_4,	SCL6_C,			SEL_I2C6_2),

	PINMUX_IPSR_DATA(IP7_11_8,	D15),
	PINMUX_IPSR_DATA(IP7_11_8,	LCDOUT7),
	PINMUX_IPSR_MSEL(IP7_11_8,	MSIOF3_SS2_A,		SEL_MSIOF3_0),
	PINMUX_IPSR_MSEL(IP7_11_8,	HTX3_C,			SEL_HSCIF3_2),
	PINMUX_IPSR_MSEL(IP7_11_8,	VI4_DATA7_A,		SEL_VIN4_0),
	PINMUX_IPSR_DATA(IP7_11_8,	DU_DR7),
	PINMUX_IPSR_MSEL(IP7_11_8,	SDA6_C,			SEL_I2C6_2),

	PINMUX_IPSR_DATA(IP7_15_12,	FSCLKST),

	PINMUX_IPSR_DATA(IP7_19_16,	SD0_CLK),
	PINMUX_IPSR_MSEL(IP7_19_16,	MSIOF1_SCK_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP7_19_16,	STP_OPWM_0_B,		SEL_SSP1_0_1),

	PINMUX_IPSR_DATA(IP7_23_20,	SD0_CMD),
	PINMUX_IPSR_MSEL(IP7_23_20,	MSIOF1_SYNC_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP7_23_20,	STP_IVCXO27_0_B,	SEL_SSP1_0_1),

	PINMUX_IPSR_DATA(IP7_27_24,	SD0_DAT0),
	PINMUX_IPSR_MSEL(IP7_27_24,	MSIOF1_RXD_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP7_27_24,	TS_SCK0_B,		SEL_TSIF0_1),
	PINMUX_IPSR_MSEL(IP7_27_24,	STP_ISCLK_0_B,		SEL_SSP1_0_1),

	PINMUX_IPSR_DATA(IP7_31_28,	SD0_DAT1),
	PINMUX_IPSR_MSEL(IP7_31_28,	MSIOF1_TXD_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP7_31_28,	TS_SPSYNC0_B,		SEL_TSIF0_1),
	PINMUX_IPSR_MSEL(IP7_31_28,	STP_ISSYNC_0_B,		SEL_SSP1_0_1),

	/* IPSR8 */
	PINMUX_IPSR_DATA(IP8_3_0,	SD0_DAT2),
	PINMUX_IPSR_MSEL(IP8_3_0,	MSIOF1_SS1_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP8_3_0,	TS_SDAT0_B,		SEL_TSIF0_1),
	PINMUX_IPSR_MSEL(IP8_3_0,	STP_ISD_0_B,		SEL_SSP1_0_1),

	PINMUX_IPSR_DATA(IP8_7_4,	SD0_DAT3),
	PINMUX_IPSR_MSEL(IP8_7_4,	MSIOF1_SS2_E,		SEL_MSIOF1_4),
	PINMUX_IPSR_MSEL(IP8_7_4,	TS_SDEN0_B,		SEL_TSIF0_1),
	PINMUX_IPSR_MSEL(IP8_7_4,	STP_ISEN_0_B,		SEL_SSP1_0_1),

	PINMUX_IPSR_DATA(IP8_11_8,	SD1_CLK),
	PINMUX_IPSR_MSEL(IP8_11_8,	MSIOF1_SCK_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_11_8,	SIM0_CLK_A,		SEL_SIMCARD_0),

	PINMUX_IPSR_DATA(IP8_15_12,	SD1_CMD),
	PINMUX_IPSR_MSEL(IP8_15_12,	MSIOF1_SYNC_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_15_12,	SIM0_D_A,		SEL_SIMCARD_0),
	PINMUX_IPSR_MSEL(IP8_15_12,	STP_IVCXO27_1_B,	SEL_SSP1_1_1),

	PINMUX_IPSR_DATA(IP8_19_16,	SD1_DAT0),
	PINMUX_IPSR_DATA(IP8_19_16,	SD2_DAT4),
	PINMUX_IPSR_MSEL(IP8_19_16,	MSIOF1_RXD_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_19_16,	TS_SCK1_B,		SEL_TSIF1_1),
	PINMUX_IPSR_MSEL(IP8_19_16,	STP_ISCLK_1_B,		SEL_SSP1_1_1),

	PINMUX_IPSR_DATA(IP8_23_20,	SD1_DAT1),
	PINMUX_IPSR_DATA(IP8_23_20,	SD2_DAT5),
	PINMUX_IPSR_MSEL(IP8_23_20,	MSIOF1_TXD_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_23_20,	TS_SPSYNC1_B,		SEL_TSIF1_1),
	PINMUX_IPSR_MSEL(IP8_23_20,	STP_ISSYNC_1_B,		SEL_SSP1_1_1),

	PINMUX_IPSR_DATA(IP8_27_24,	SD1_DAT2),
	PINMUX_IPSR_DATA(IP8_27_24,	SD2_DAT6),
	PINMUX_IPSR_MSEL(IP8_27_24,	MSIOF1_SS1_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_27_24,	TS_SDAT1_B,		SEL_TSIF1_1),
	PINMUX_IPSR_MSEL(IP8_27_24,	STP_ISD_1_B,		SEL_SSP1_1_1),

	PINMUX_IPSR_DATA(IP8_31_28,	SD1_DAT3),
	PINMUX_IPSR_DATA(IP8_31_28,	SD2_DAT7),
	PINMUX_IPSR_MSEL(IP8_31_28,	MSIOF1_SS2_G,		SEL_MSIOF1_6),
	PINMUX_IPSR_MSEL(IP8_31_28,	TS_SDEN1_B,		SEL_TSIF1_1),
	PINMUX_IPSR_MSEL(IP8_31_28,	STP_ISEN_1_B,		SEL_SSP1_1_1),

	/* IPSR9 */
	PINMUX_IPSR_DATA(IP9_3_0,	SD2_CLK),

	PINMUX_IPSR_DATA(IP9_7_4,	SD2_DAT0),

	PINMUX_IPSR_DATA(IP9_11_8,	SD2_DAT1),

	PINMUX_IPSR_DATA(IP9_15_12,	SD2_DAT2),

	PINMUX_IPSR_DATA(IP9_19_16,	SD2_DAT3),

	PINMUX_IPSR_DATA(IP9_23_20,	SD2_DS),
	PINMUX_IPSR_MSEL(IP9_23_20,	SATA_DEVSLP_B,		SEL_SCIF_1),

	PINMUX_IPSR_DATA(IP9_27_24,	SD3_DAT4),
	PINMUX_IPSR_MSEL(IP9_27_24,	SD2_CD_A,		SEL_SDHI2_0),

	PINMUX_IPSR_DATA(IP9_31_28,	SD3_DAT5),
	PINMUX_IPSR_MSEL(IP9_31_28,	SD2_WP_A,		SEL_SDHI2_0),

	/* IPSR10 */
	PINMUX_IPSR_DATA(IP10_3_0,	SD3_DAT6),
	PINMUX_IPSR_DATA(IP10_3_0,	SD3_CD),

	PINMUX_IPSR_DATA(IP10_7_4,	SD3_DAT7),
	PINMUX_IPSR_DATA(IP10_7_4,	SD3_WP),

	PINMUX_IPSR_DATA(IP10_11_8,	SD0_CD),
	PINMUX_IPSR_MSEL(IP10_11_8,	SCL2_B,			SEL_I2C2_1),
	PINMUX_IPSR_MSEL(IP10_11_8,	SIM0_RST_A,		SEL_SIMCARD_0),

	PINMUX_IPSR_DATA(IP10_15_12,	SD0_WP),
	PINMUX_IPSR_MSEL(IP10_15_12,	SDA2_B,			SEL_I2C2_1),

	PINMUX_IPSR_DATA(IP10_19_16,	SD1_CD),
	PINMUX_IPSR_MSEL(IP10_19_16,	SIM0_CLK_B,		SEL_SIMCARD_1),

	PINMUX_IPSR_DATA(IP10_23_20,	SD1_WP),
	PINMUX_IPSR_MSEL(IP10_23_20,	SIM0_D_B,		SEL_SIMCARD_1),

	PINMUX_IPSR_DATA(IP10_27_24,	SCK0),
	PINMUX_IPSR_MSEL(IP10_27_24,	HSCK1_B,		SEL_HSCIF1_1),
	PINMUX_IPSR_MSEL(IP10_27_24,	MSIOF1_SS2_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP10_27_24,	AUDIO_CLKC_B,		SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP10_27_24,	SDA2_A,			SEL_I2C2_0),
	PINMUX_IPSR_MSEL(IP10_27_24,	SIM0_RST_B,		SEL_SIMCARD_1),
	PINMUX_IPSR_MSEL(IP10_27_24,	STP_OPWM_0_C,		SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP10_27_24,	RIF0_CLK_B,		SEL_DRIF0_1),
	PINMUX_IPSR_DATA(IP10_27_24,	ADICHS2),

	PINMUX_IPSR_DATA(IP10_31_28,	RX0),
	PINMUX_IPSR_MSEL(IP10_31_28,	HRX1_B,			SEL_HSCIF1_1),
	PINMUX_IPSR_MSEL(IP10_31_28,	TS_SCK0_C,		SEL_TSIF0_2),
	PINMUX_IPSR_MSEL(IP10_31_28,	STP_ISCLK_0_C,		SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP10_31_28,	RIF0_D0_B,		SEL_DRIF0_1),

	/* IPSR11 */
	PINMUX_IPSR_DATA(IP11_3_0,	TX0),
	PINMUX_IPSR_MSEL(IP11_3_0,	HTX1_B,			SEL_HSCIF1_1),
	PINMUX_IPSR_MSEL(IP11_3_0,	TS_SPSYNC0_C,		SEL_TSIF0_2),
	PINMUX_IPSR_MSEL(IP11_3_0,	STP_ISSYNC_0_C,		SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP11_3_0,	RIF0_D1_B,		SEL_DRIF0_1),

	PINMUX_IPSR_DATA(IP11_7_4,	CTS0_N),
	PINMUX_IPSR_MSEL(IP11_7_4,	HCTS1_N_B,		SEL_HSCIF1_1),
	PINMUX_IPSR_MSEL(IP11_7_4,	MSIOF1_SYNC_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP11_7_4,	TS_SPSYNC1_C,		SEL_TSIF1_2),
	PINMUX_IPSR_MSEL(IP11_7_4,	STP_ISSYNC_1_C,		SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP11_7_4,	RIF1_SYNC_B,		SEL_DRIF1_1),
	PINMUX_IPSR_MSEL(IP11_7_4,	AUDIO_CLKOUT_C,		SEL_ADG_2),
	PINMUX_IPSR_DATA(IP11_7_4,	ADICS_SAMP),

	PINMUX_IPSR_DATA(IP11_11_8,	RTS0_N_TANS),
	PINMUX_IPSR_MSEL(IP11_11_8,	HRTS1_N_B,		SEL_HSCIF1_1),
	PINMUX_IPSR_MSEL(IP11_11_8,	MSIOF1_SS1_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP11_11_8,	AUDIO_CLKA_B,		SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP11_11_8,	SCL2_A,			SEL_I2C2_0),
	PINMUX_IPSR_MSEL(IP11_11_8,	STP_IVCXO27_1_C,	SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP11_11_8,	RIF0_SYNC_B,		SEL_DRIF0_1),
	PINMUX_IPSR_DATA(IP11_11_8,	ADICHS1),

	PINMUX_IPSR_MSEL(IP11_15_12,	RX1_A,			SEL_SCIF1_0),
	PINMUX_IPSR_MSEL(IP11_15_12,	HRX1_A,			SEL_HSCIF1_0),
	PINMUX_IPSR_MSEL(IP11_15_12,	TS_SDAT0_C,		SEL_TSIF0_2),
	PINMUX_IPSR_MSEL(IP11_15_12,	STP_ISD_0_C,		SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP11_15_12,	RIF1_CLK_C,		SEL_DRIF1_2),

	PINMUX_IPSR_MSEL(IP11_19_16,	TX1_A,			SEL_SCIF1_0),
	PINMUX_IPSR_MSEL(IP11_19_16,	HTX1_A,			SEL_HSCIF1_0),
	PINMUX_IPSR_MSEL(IP11_19_16,	TS_SDEN0_C,		SEL_TSIF0_2),
	PINMUX_IPSR_MSEL(IP11_19_16,	STP_ISEN_0_C,		SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP11_19_16,	RIF1_D0_C,		SEL_DRIF1_2),

	PINMUX_IPSR_DATA(IP11_23_20,	CTS1_N),
	PINMUX_IPSR_MSEL(IP11_23_20,	HCTS1_N_A,		SEL_HSCIF1_0),
	PINMUX_IPSR_MSEL(IP11_23_20,	MSIOF1_RXD_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP11_23_20,	TS_SDEN1_C,		SEL_TSIF1_2),
	PINMUX_IPSR_MSEL(IP11_23_20,	STP_ISEN_1_C,		SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP11_23_20,	RIF1_D0_B,		SEL_DRIF1_1),
	PINMUX_IPSR_DATA(IP11_23_20,	ADIDATA),

	PINMUX_IPSR_DATA(IP11_27_24,	RTS1_N_TANS),
	PINMUX_IPSR_MSEL(IP11_27_24,	HRTS1_N_A,		SEL_HSCIF1_0),
	PINMUX_IPSR_MSEL(IP11_27_24,	MSIOF1_TXD_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP11_27_24,	TS_SDAT1_C,		SEL_TSIF1_2),
	PINMUX_IPSR_MSEL(IP11_27_24,	STP_ISD_1_C,		SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP11_27_24,	RIF1_D1_B,		SEL_DRIF1_1),
	PINMUX_IPSR_DATA(IP11_27_24,	ADICHS0),

	PINMUX_IPSR_DATA(IP11_31_28,	SCK2),
	PINMUX_IPSR_MSEL(IP11_31_28,	SCIF_CLK_B,		SEL_SCIF1_1),
	PINMUX_IPSR_MSEL(IP11_31_28,	MSIOF1_SCK_B,		SEL_MSIOF1_1),
	PINMUX_IPSR_MSEL(IP11_31_28,	TS_SCK1_C,		SEL_TSIF1_2),
	PINMUX_IPSR_MSEL(IP11_31_28,	STP_ISCLK_1_C,		SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP11_31_28,	RIF1_CLK_B,		SEL_DRIF1_1),
	PINMUX_IPSR_DATA(IP11_31_28,	ADICLK),

	/* IPSR12 */
	PINMUX_IPSR_MSEL(IP12_3_0,	TX2_A,			SEL_SCIF2_0),
	PINMUX_IPSR_MSEL(IP12_3_0,	SD2_CD_B,		SEL_SDHI2_1),
	PINMUX_IPSR_MSEL(IP12_3_0,	SCL1_A,			SEL_I2C1_0),
	PINMUX_IPSR_MSEL(IP12_3_0,	FMCLK_A,		SEL_FM_0),
	PINMUX_IPSR_MSEL(IP12_3_0,	RIF1_D1_C,		SEL_DRIF1_2),
	PINMUX_IPSR_MSEL(IP12_3_0,	FSO_CFE_0_B,		SEL_FSO_1),

	PINMUX_IPSR_MSEL(IP12_7_4,	RX2_A,			SEL_SCIF2_0),
	PINMUX_IPSR_MSEL(IP12_7_4,	SD2_WP_B,		SEL_SDHI2_1),
	PINMUX_IPSR_MSEL(IP12_7_4,	SDA1_A,			SEL_I2C1_0),
	PINMUX_IPSR_MSEL(IP12_7_4,	FMIN_A,			SEL_FM_0),
	PINMUX_IPSR_MSEL(IP12_7_4,	RIF1_SYNC_C,		SEL_DRIF1_2),
	PINMUX_IPSR_MSEL(IP12_7_4,	FSO_CFE_1_B,		SEL_FSO_1),

	PINMUX_IPSR_DATA(IP12_11_8,	HSCK0),
	PINMUX_IPSR_MSEL(IP12_11_8,	MSIOF1_SCK_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP12_11_8,	AUDIO_CLKB_A,		SEL_ADG_0),
	PINMUX_IPSR_MSEL(IP12_11_8,	SSI_SDATA1_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP12_11_8,	TS_SCK0_D,		SEL_TSIF0_3),
	PINMUX_IPSR_MSEL(IP12_11_8,	STP_ISCLK_0_D,		SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP12_11_8,	RIF0_CLK_C,		SEL_DRIF0_2),

	PINMUX_IPSR_DATA(IP12_15_12,	HRX0),
	PINMUX_IPSR_MSEL(IP12_15_12,	MSIOF1_RXD_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP12_15_12,	SSI_SDATA2_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP12_15_12,	TS_SDEN0_D,		SEL_TSIF0_3),
	PINMUX_IPSR_MSEL(IP12_15_12,	STP_ISEN_0_D,		SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP12_15_12,	RIF0_D0_C,		SEL_DRIF0_2),

	PINMUX_IPSR_DATA(IP12_19_16,	HTX0),
	PINMUX_IPSR_MSEL(IP12_19_16,	MSIOF1_TXD_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP12_19_16,	SSI_SDATA9_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP12_19_16,	TS_SDAT0_D,		SEL_TSIF0_3),
	PINMUX_IPSR_MSEL(IP12_19_16,	STP_ISD_0_D,		SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP12_19_16,	RIF0_D1_C,		SEL_DRIF0_2),

	PINMUX_IPSR_DATA(IP12_23_20,	HCTS0_N),
	PINMUX_IPSR_MSEL(IP12_23_20,	RX2_B,			SEL_SCIF2_1),
	PINMUX_IPSR_MSEL(IP12_23_20,	MSIOF1_SYNC_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP12_23_20,	SSI_SCK9_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP12_23_20,	TS_SPSYNC0_D,		SEL_TSIF0_3),
	PINMUX_IPSR_MSEL(IP12_23_20,	STP_ISSYNC_0_D,		SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP12_23_20,	RIF0_SYNC_C,		SEL_DRIF0_2),
	PINMUX_IPSR_MSEL(IP12_23_20,	AUDIO_CLKOUT1_A,	SEL_ADG_0),

	PINMUX_IPSR_DATA(IP12_27_24,	HRTS0_N),
	PINMUX_IPSR_MSEL(IP12_27_24,	TX2_B,			SEL_SCIF2_1),
	PINMUX_IPSR_MSEL(IP12_27_24,	MSIOF1_SS1_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP12_27_24,	SSI_WS9_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP12_27_24,	STP_IVCXO27_0_D,	SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP12_27_24,	BPFCLK_A,		SEL_FM_0),
	PINMUX_IPSR_MSEL(IP12_27_24,	AUDIO_CLKOUT2_A,	SEL_ADG_0),

	PINMUX_IPSR_DATA(IP12_31_28,	MSIOF0_SYNC),
	PINMUX_IPSR_MSEL(IP12_31_28,	AUDIO_CLKOUT_A,		SEL_ADG_0),

	/* IPSR13 */
	PINMUX_IPSR_DATA(IP13_3_0,	MSIOF0_SS1),
	PINMUX_IPSR_DATA(IP13_3_0,	RX5),
	PINMUX_IPSR_MSEL(IP13_3_0,	AUDIO_CLKA_C,		SEL_ADG_2),
	PINMUX_IPSR_MSEL(IP13_3_0,	SSI_SCK2_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP13_3_0,	STP_IVCXO27_0_C,	SEL_SSP1_0_2),
	PINMUX_IPSR_MSEL(IP13_3_0,	AUDIO_CLKOUT3_A,	SEL_ADG_0),
	PINMUX_IPSR_MSEL(IP13_3_0,	TCLK1_B,		SEL_TIMER_TMU_1),

	PINMUX_IPSR_DATA(IP13_7_4,	MSIOF0_SS2),
	PINMUX_IPSR_DATA(IP13_7_4,	TX5),
	PINMUX_IPSR_MSEL(IP13_7_4,	MSIOF1_SS2_D,		SEL_MSIOF1_3),
	PINMUX_IPSR_MSEL(IP13_7_4,	AUDIO_CLKC_A,		SEL_ADG_0),
	PINMUX_IPSR_MSEL(IP13_7_4,	SSI_WS2_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP13_7_4,	STP_OPWM_0_D,		SEL_SSP1_0_3),
	PINMUX_IPSR_MSEL(IP13_7_4,	AUDIO_CLKOUT_D,		SEL_ADG_3),
	PINMUX_IPSR_MSEL(IP13_7_4,	SPEEDIN_B,		SEL_SPEED_PULSE_1),

	PINMUX_IPSR_DATA(IP13_11_8,	MLB_CLK),
	PINMUX_IPSR_MSEL(IP13_11_8,	MSIOF1_SCK_F,		SEL_MSIOF1_5),
	PINMUX_IPSR_MSEL(IP13_11_8,	SCL1_B,			SEL_I2C1_1),

	PINMUX_IPSR_DATA(IP13_15_12,	MLB_SIG),
	PINMUX_IPSR_MSEL(IP13_15_12,	RX1_B,			SEL_SCIF1_1),
	PINMUX_IPSR_MSEL(IP13_15_12,	MSIOF1_SYNC_F,		SEL_MSIOF1_5),
	PINMUX_IPSR_MSEL(IP13_15_12,	SDA1_B,			SEL_I2C1_1),

	PINMUX_IPSR_DATA(IP13_19_16,	MLB_DAT),
	PINMUX_IPSR_MSEL(IP13_19_16,	TX1_B,			SEL_SCIF1_1),
	PINMUX_IPSR_MSEL(IP13_19_16,	MSIOF1_RXD_F,		SEL_MSIOF1_5),

	PINMUX_IPSR_DATA(IP13_23_20,	SSI_SCK0129),
	PINMUX_IPSR_MSEL(IP13_23_20,	MSIOF1_TXD_F,		SEL_MSIOF1_5),

	PINMUX_IPSR_DATA(IP13_27_24,	SSI_WS0129),
	PINMUX_IPSR_MSEL(IP13_27_24,	MSIOF1_SS1_F,		SEL_MSIOF1_5),

	PINMUX_IPSR_DATA(IP13_31_28,	SSI_SDATA0),
	PINMUX_IPSR_MSEL(IP13_31_28,	MSIOF1_SS2_F,		SEL_MSIOF1_5),

	/* IPSR14 */
	PINMUX_IPSR_MSEL(IP14_3_0,	SSI_SDATA1_A,		SEL_SSI_0),

	PINMUX_IPSR_MSEL(IP14_7_4,	SSI_SDATA2_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP14_7_4,	SSI_SCK1_B,		SEL_SSI_1),

	PINMUX_IPSR_DATA(IP14_11_8,	SSI_SCK34),
	PINMUX_IPSR_MSEL(IP14_11_8,	MSIOF1_SS1_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_11_8,	STP_OPWM_0_A,		SEL_SSP1_0_0),

	PINMUX_IPSR_DATA(IP14_15_12,	SSI_WS34),
	PINMUX_IPSR_MSEL(IP14_15_12,	HCTS2_N_A,		SEL_HSCIF2_0),
	PINMUX_IPSR_MSEL(IP14_15_12,	MSIOF1_SS2_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_15_12,	STP_IVCXO27_0_A,	SEL_SSP1_0_0),

	PINMUX_IPSR_DATA(IP14_19_16,	SSI_SDATA3),
	PINMUX_IPSR_MSEL(IP14_19_16,	HRTS2_N_A,		SEL_HSCIF2_0),
	PINMUX_IPSR_MSEL(IP14_19_16,	MSIOF1_TXD_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_19_16,	TS_SCK0_A,		SEL_TSIF0_0),
	PINMUX_IPSR_MSEL(IP14_19_16,	STP_ISCLK_0_A,		SEL_SSP1_0_0),
	PINMUX_IPSR_MSEL(IP14_19_16,	RIF0_D1_A,		SEL_DRIF0_0),
	PINMUX_IPSR_MSEL(IP14_19_16,	RIF2_D0_A,		SEL_DRIF2_0),

	PINMUX_IPSR_DATA(IP14_23_20,	SSI_SCK4),
	PINMUX_IPSR_MSEL(IP14_23_20,	HRX2_A,			SEL_HSCIF2_0),
	PINMUX_IPSR_MSEL(IP14_23_20,	MSIOF1_SCK_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_23_20,	TS_SDAT0_A,		SEL_TSIF0_0),
	PINMUX_IPSR_MSEL(IP14_23_20,	STP_ISD_0_A,		SEL_SSP1_0_0),
	PINMUX_IPSR_MSEL(IP14_23_20,	RIF0_CLK_A,		SEL_DRIF0_0),
	PINMUX_IPSR_MSEL(IP14_23_20,	RIF2_CLK_A,		SEL_DRIF2_0),

	PINMUX_IPSR_DATA(IP14_27_24,	SSI_WS4),
	PINMUX_IPSR_MSEL(IP14_27_24,	HTX2_A,			SEL_HSCIF2_0),
	PINMUX_IPSR_MSEL(IP14_27_24,	MSIOF1_SYNC_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_27_24,	TS_SDEN0_A,		SEL_TSIF0_0),
	PINMUX_IPSR_MSEL(IP14_27_24,	STP_ISEN_0_A,		SEL_SSP1_0_0),
	PINMUX_IPSR_MSEL(IP14_27_24,	RIF0_SYNC_A,		SEL_DRIF0_0),
	PINMUX_IPSR_MSEL(IP14_27_24,	RIF2_SYNC_A,		SEL_DRIF2_0),

	PINMUX_IPSR_DATA(IP14_31_28,	SSI_SDATA4),
	PINMUX_IPSR_MSEL(IP14_31_28,	HSCK2_A,		SEL_HSCIF2_0),
	PINMUX_IPSR_MSEL(IP14_31_28,	MSIOF1_RXD_A,		SEL_MSIOF1_0),
	PINMUX_IPSR_MSEL(IP14_31_28,	TS_SPSYNC0_A,		SEL_TSIF0_0),
	PINMUX_IPSR_MSEL(IP14_31_28,	STP_ISSYNC_0_A,		SEL_SSP1_0_0),
	PINMUX_IPSR_MSEL(IP14_31_28,	RIF0_D0_A,		SEL_DRIF0_0),
	PINMUX_IPSR_MSEL(IP14_31_28,	RIF2_D1_A,		SEL_DRIF2_0),

	/* IPSR15 */
	PINMUX_IPSR_DATA(IP15_3_0,	SSI_SCK6),
	PINMUX_IPSR_DATA(IP15_3_0,	USB2_PWEN),
	PINMUX_IPSR_MSEL(IP15_3_0,	SIM0_RST_D,		SEL_SIMCARD_3),

	PINMUX_IPSR_DATA(IP15_7_4,	SSI_WS6),
	PINMUX_IPSR_DATA(IP15_7_4,	USB2_OVC),
	PINMUX_IPSR_MSEL(IP15_7_4,	SIM0_D_D,		SEL_SIMCARD_3),

	PINMUX_IPSR_DATA(IP15_11_8,	SSI_SDATA6),
	PINMUX_IPSR_MSEL(IP15_11_8,	SIM0_CLK_D,		SEL_SIMCARD_3),
	PINMUX_IPSR_MSEL(IP15_11_8,	SATA_DEVSLP_A,		SEL_SCIF_0),

	PINMUX_IPSR_DATA(IP15_15_12,	SSI_SCK78),
	PINMUX_IPSR_MSEL(IP15_15_12,	HRX2_B,			SEL_HSCIF2_1),
	PINMUX_IPSR_MSEL(IP15_15_12,	MSIOF1_SCK_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP15_15_12,	TS_SCK1_A,		SEL_TSIF1_0),
	PINMUX_IPSR_MSEL(IP15_15_12,	STP_ISCLK_1_A,		SEL_SSP1_1_0),
	PINMUX_IPSR_MSEL(IP15_15_12,	RIF1_CLK_A,		SEL_DRIF1_0),
	PINMUX_IPSR_MSEL(IP15_15_12,	RIF3_CLK_A,		SEL_DRIF3_0),

	PINMUX_IPSR_DATA(IP15_19_16,	SSI_WS78),
	PINMUX_IPSR_MSEL(IP15_19_16,	HTX2_B,			SEL_HSCIF2_1),
	PINMUX_IPSR_MSEL(IP15_19_16,	MSIOF1_SYNC_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP15_19_16,	TS_SDAT1_A,		SEL_TSIF1_0),
	PINMUX_IPSR_MSEL(IP15_19_16,	STP_ISD_1_A,		SEL_SSP1_1_0),
	PINMUX_IPSR_MSEL(IP15_19_16,	RIF1_SYNC_A,		SEL_DRIF1_0),
	PINMUX_IPSR_MSEL(IP15_19_16,	RIF3_SYNC_A,		SEL_DRIF3_0),

	PINMUX_IPSR_DATA(IP15_23_20,	SSI_SDATA7),
	PINMUX_IPSR_MSEL(IP15_23_20,	HCTS2_N_B,		SEL_HSCIF2_1),
	PINMUX_IPSR_MSEL(IP15_23_20,	MSIOF1_RXD_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP15_23_20,	TS_SDEN1_A,		SEL_TSIF1_0),
	PINMUX_IPSR_MSEL(IP15_23_20,	STP_ISEN_1_A,		SEL_SSP1_1_0),
	PINMUX_IPSR_MSEL(IP15_23_20,	RIF1_D0_A,		SEL_DRIF1_0),
	PINMUX_IPSR_MSEL(IP15_23_20,	RIF3_D0_A,		SEL_DRIF3_0),
	PINMUX_IPSR_MSEL(IP15_23_20,	TCLK2_A,		SEL_TIMER_TMU_0),

	PINMUX_IPSR_DATA(IP15_27_24,	SSI_SDATA8),
	PINMUX_IPSR_MSEL(IP15_27_24,	HRTS2_N_B,		SEL_HSCIF2_1),
	PINMUX_IPSR_MSEL(IP15_27_24,	MSIOF1_TXD_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP15_27_24,	TS_SPSYNC1_A,		SEL_TSIF1_0),
	PINMUX_IPSR_MSEL(IP15_27_24,	STP_ISSYNC_1_A,		SEL_SSP1_1_0),
	PINMUX_IPSR_MSEL(IP15_27_24,	RIF1_D1_A,		SEL_DRIF1_0),
	PINMUX_IPSR_MSEL(IP15_27_24,	RIF3_D1_A,		SEL_DRIF3_0),

	PINMUX_IPSR_MSEL(IP15_31_28,	SSI_SDATA9_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP15_31_28,	HSCK2_B,		SEL_HSCIF2_1),
	PINMUX_IPSR_MSEL(IP15_31_28,	MSIOF1_SS1_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP15_31_28,	HSCK1_A,		SEL_HSCIF1_0),
	PINMUX_IPSR_MSEL(IP15_31_28,	SSI_WS1_B,		SEL_SSI_1),
	PINMUX_IPSR_DATA(IP15_31_28,	SCK1),
	PINMUX_IPSR_MSEL(IP15_31_28,	STP_IVCXO27_1_A,	SEL_SSP1_1_0),
	PINMUX_IPSR_DATA(IP15_31_28,	SCK5),

	/* IPSR16 */
	PINMUX_IPSR_MSEL(IP16_3_0,	AUDIO_CLKA_A,		SEL_ADG_0),
	PINMUX_IPSR_DATA(IP16_3_0,	CC5_OSCOUT),

	PINMUX_IPSR_MSEL(IP16_7_4,	AUDIO_CLKB_B,		SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP16_7_4,	SCIF_CLK_A,		SEL_SCIF1_0),
	PINMUX_IPSR_MSEL(IP16_7_4,	STP_IVCXO27_1_D,	SEL_SSP1_1_3),
	PINMUX_IPSR_MSEL(IP16_7_4,	REMOCON_A,		SEL_REMOCON_0),
	PINMUX_IPSR_MSEL(IP16_7_4,	TCLK1_A,		SEL_TIMER_TMU_0),

	PINMUX_IPSR_DATA(IP16_11_8,	USB0_PWEN),
	PINMUX_IPSR_MSEL(IP16_11_8,	SIM0_RST_C,		SEL_SIMCARD_2),
	PINMUX_IPSR_MSEL(IP16_11_8,	TS_SCK1_D,		SEL_TSIF1_3),
	PINMUX_IPSR_MSEL(IP16_11_8,	STP_ISCLK_1_D,		SEL_SSP1_1_3),
	PINMUX_IPSR_MSEL(IP16_11_8,	BPFCLK_B,		SEL_FM_1),
	PINMUX_IPSR_MSEL(IP16_11_8,	RIF3_CLK_B,		SEL_DRIF3_1),

	PINMUX_IPSR_DATA(IP16_15_12,	USB0_OVC),
	PINMUX_IPSR_MSEL(IP16_11_8,	SIM0_D_C,		SEL_SIMCARD_2),
	PINMUX_IPSR_MSEL(IP16_11_8,	TS_SDAT1_D,		SEL_TSIF1_3),
	PINMUX_IPSR_MSEL(IP16_11_8,	STP_ISD_1_D,		SEL_SSP1_1_3),
	PINMUX_IPSR_MSEL(IP16_11_8,	RIF3_SYNC_B,		SEL_DRIF3_1),

	PINMUX_IPSR_DATA(IP16_19_16,	USB1_PWEN),
	PINMUX_IPSR_MSEL(IP16_19_16,	SIM0_CLK_C,		SEL_SIMCARD_2),
	PINMUX_IPSR_MSEL(IP16_19_16,	SSI_SCK1_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP16_19_16,	TS_SCK0_E,		SEL_TSIF0_4),
	PINMUX_IPSR_MSEL(IP16_19_16,	STP_ISCLK_0_E,		SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP16_19_16,	FMCLK_B,		SEL_FM_1),
	PINMUX_IPSR_MSEL(IP16_19_16,	RIF2_CLK_B,		SEL_DRIF2_1),
	PINMUX_IPSR_MSEL(IP16_19_16,	SPEEDIN_A,		SEL_SPEED_PULSE_0),

	PINMUX_IPSR_DATA(IP16_23_20,	USB1_OVC),
	PINMUX_IPSR_MSEL(IP16_23_20,	MSIOF1_SS2_C,		SEL_MSIOF1_2),
	PINMUX_IPSR_MSEL(IP16_23_20,	SSI_WS1_A,		SEL_SSI_0),
	PINMUX_IPSR_MSEL(IP16_23_20,	TS_SDAT0_E,		SEL_TSIF0_4),
	PINMUX_IPSR_MSEL(IP16_23_20,	STP_ISD_0_E,		SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP16_23_20,	FMIN_B,			SEL_FM_1),
	PINMUX_IPSR_MSEL(IP16_23_20,	RIF2_SYNC_B,		SEL_DRIF2_1),
	PINMUX_IPSR_MSEL(IP16_23_20,	REMOCON_B,		SEL_REMOCON_1),

	PINMUX_IPSR_DATA(IP16_27_24,	USB30_PWEN),
	PINMUX_IPSR_MSEL(IP16_27_24,	AUDIO_CLKOUT_B,		SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP16_27_24,	SSI_SCK2_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP16_27_24,	TS_SDEN1_D,		SEL_TSIF1_3),
	PINMUX_IPSR_MSEL(IP16_27_24,	STP_ISEN_1_D,		SEL_SSP1_1_2),
	PINMUX_IPSR_MSEL(IP16_27_24,	STP_OPWM_0_E,		SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP16_27_24,	RIF3_D0_B,		SEL_DRIF3_1),
	PINMUX_IPSR_MSEL(IP16_27_24,	TCLK2_B,		SEL_TIMER_TMU_1),
	PINMUX_IPSR_DATA(IP16_27_24,	TPU0TO0),

	PINMUX_IPSR_DATA(IP16_31_28,	USB30_OVC),
	PINMUX_IPSR_MSEL(IP16_31_28,	AUDIO_CLKOUT1_B,	SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP16_31_28,	SSI_WS2_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP16_31_28,	TS_SPSYNC1_D,		SEL_TSIF1_3),
	PINMUX_IPSR_MSEL(IP16_31_28,	STP_ISSYNC_1_D,		SEL_SSP1_1_3),
	PINMUX_IPSR_MSEL(IP16_31_28,	STP_IVCXO27_0_E,	SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP16_31_28,	RIF3_D1_B,		SEL_DRIF3_1),
	PINMUX_IPSR_MSEL(IP16_31_28,	FSO_TOE_B,		SEL_FSO_1),
	PINMUX_IPSR_DATA(IP16_31_28,	TPU0TO1),

	/* IPSR17 */
	PINMUX_IPSR_DATA(IP17_3_0,	USB31_PWEN),
	PINMUX_IPSR_MSEL(IP17_3_0,	AUDIO_CLKOUT2_B,	SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP17_3_0,	SSI_SCK9_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP17_3_0,	TS_SDEN0_E,		SEL_TSIF0_4),
	PINMUX_IPSR_MSEL(IP17_3_0,	STP_ISEN_0_E,		SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP17_3_0,	RIF2_D0_B,		SEL_DRIF2_1),
	PINMUX_IPSR_DATA(IP17_3_0,	TPU0TO2),

	PINMUX_IPSR_DATA(IP17_7_4,	USB31_OVC),
	PINMUX_IPSR_MSEL(IP17_7_4,	AUDIO_CLKOUT3_B,	SEL_ADG_1),
	PINMUX_IPSR_MSEL(IP17_7_4,	SSI_WS9_B,		SEL_SSI_1),
	PINMUX_IPSR_MSEL(IP17_7_4,	TS_SPSYNC0_E,		SEL_TSIF0_4),
	PINMUX_IPSR_MSEL(IP17_7_4,	STP_ISSYNC_0_E,		SEL_SSP1_0_4),
	PINMUX_IPSR_MSEL(IP17_7_4,	RIF2_D1_B,		SEL_DRIF2_1),
	PINMUX_IPSR_DATA(IP17_7_4,	TPU0TO3),

	/* I2C */
	PINMUX_IPSR_NOGP(0,		I2C_SEL_0_1),
	PINMUX_IPSR_NOGP(0,		I2C_SEL_3_1),
	PINMUX_IPSR_NOGP(0,		I2C_SEL_5_1),
};

static const struct sh_pfc_pin pinmux_pins[] = {
	PINMUX_GPIO_GP_ALL(),
};

/* - AUDIO CLOCK ------------------------------------------------------------ */
static const unsigned int audio_clk_a_a_pins[] = {
	/* CLK A */
	RCAR_GP_PIN(6, 22),
};
static const unsigned int audio_clk_a_a_mux[] = {
	AUDIO_CLKA_A_MARK,
};
static const unsigned int audio_clk_a_b_pins[] = {
	/* CLK A */
	RCAR_GP_PIN(5, 4),
};
static const unsigned int audio_clk_a_b_mux[] = {
	AUDIO_CLKA_B_MARK,
};
static const unsigned int audio_clk_a_c_pins[] = {
	/* CLK A */
	RCAR_GP_PIN(5, 19),
};
static const unsigned int audio_clk_a_c_mux[] = {
	AUDIO_CLKA_C_MARK,
};
static const unsigned int audio_clk_b_a_pins[] = {
	/* CLK B */
	RCAR_GP_PIN(5, 12),
};
static const unsigned int audio_clk_b_a_mux[] = {
	AUDIO_CLKB_A_MARK,
};
static const unsigned int audio_clk_b_b_pins[] = {
	/* CLK B */
	RCAR_GP_PIN(6, 23),
};
static const unsigned int audio_clk_b_b_mux[] = {
	AUDIO_CLKB_B_MARK,
};
static const unsigned int audio_clk_c_a_pins[] = {
	/* CLK C */
	RCAR_GP_PIN(5, 21),
};
static const unsigned int audio_clk_c_a_mux[] = {
	AUDIO_CLKC_A_MARK,
};
static const unsigned int audio_clk_c_b_pins[] = {
	/* CLK C */
	RCAR_GP_PIN(5, 0),
};
static const unsigned int audio_clk_c_b_mux[] = {
	AUDIO_CLKC_B_MARK,
};
static const unsigned int audio_clkout_a_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(5, 18),
};
static const unsigned int audio_clkout_a_mux[] = {
	AUDIO_CLKOUT_A_MARK,
};
static const unsigned int audio_clkout_b_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(6, 28),
};
static const unsigned int audio_clkout_b_mux[] = {
	AUDIO_CLKOUT_B_MARK,
};
static const unsigned int audio_clkout_c_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(5, 3),
};
static const unsigned int audio_clkout_c_mux[] = {
	AUDIO_CLKOUT_C_MARK,
};
static const unsigned int audio_clkout_d_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(5, 21),
};
static const unsigned int audio_clkout_d_mux[] = {
	AUDIO_CLKOUT_D_MARK,
};
static const unsigned int audio_clkout1_a_pins[] = {
	/* CLKOUT1 */
	RCAR_GP_PIN(5, 15),
};
static const unsigned int audio_clkout1_a_mux[] = {
	AUDIO_CLKOUT1_A_MARK,
};
static const unsigned int audio_clkout1_b_pins[] = {
	/* CLKOUT1 */
	RCAR_GP_PIN(6, 29),
};
static const unsigned int audio_clkout1_b_mux[] = {
	AUDIO_CLKOUT1_B_MARK,
};
static const unsigned int audio_clkout2_a_pins[] = {
	/* CLKOUT2 */
	RCAR_GP_PIN(5, 16),
};
static const unsigned int audio_clkout2_a_mux[] = {
	AUDIO_CLKOUT2_A_MARK,
};
static const unsigned int audio_clkout2_b_pins[] = {
	/* CLKOUT2 */
	RCAR_GP_PIN(6, 30),
};
static const unsigned int audio_clkout2_b_mux[] = {
	AUDIO_CLKOUT2_B_MARK,
};

static const unsigned int audio_clkout3_a_pins[] = {
	/* CLKOUT3 */
	RCAR_GP_PIN(5, 19),
};
static const unsigned int audio_clkout3_a_mux[] = {
	AUDIO_CLKOUT3_A_MARK,
};
static const unsigned int audio_clkout3_b_pins[] = {
	/* CLKOUT3 */
	RCAR_GP_PIN(6, 31),
};
static const unsigned int audio_clkout3_b_mux[] = {
	AUDIO_CLKOUT3_B_MARK,
};

/* - EtherAVB --------------------------------------------------------------- */
static const unsigned int avb_link_pins[] = {
	/* AVB_LINK */
	RCAR_GP_PIN(2, 12),
};
static const unsigned int avb_link_mux[] = {
	AVB_LINK_MARK,
};
static const unsigned int avb_magic_pins[] = {
	/* AVB_MAGIC_ */
	RCAR_GP_PIN(2, 10),
};
static const unsigned int avb_magic_mux[] = {
	AVB_MAGIC_MARK,
};
static const unsigned int avb_phy_int_pins[] = {
	/* AVB_PHY_INT */
	RCAR_GP_PIN(2, 11),
};
static const unsigned int avb_phy_int_mux[] = {
	AVB_PHY_INT_MARK,
};
static const unsigned int avb_mdc_pins[] = {
	/* AVB_MDC */
	RCAR_GP_PIN(2, 9),
};
static const unsigned int avb_mdc_mux[] = {
	AVB_MDC_MARK,
};
static const unsigned int avb_avtp_pps_pins[] = {
	/* AVB_AVTP_PPS */
	RCAR_GP_PIN(2, 6),
};
static const unsigned int avb_avtp_pps_mux[] = {
	AVB_AVTP_PPS_MARK,
};
static const unsigned int avb_avtp_match_a_pins[] = {
	/* AVB_AVTP_MATCH_A */
	RCAR_GP_PIN(2, 13),
};
static const unsigned int avb_avtp_match_a_mux[] = {
	AVB_AVTP_MATCH_A_MARK,
};
static const unsigned int avb_avtp_capture_a_pins[] = {
	/* AVB_AVTP_CAPTURE_A */
	RCAR_GP_PIN(2, 14),
};
static const unsigned int avb_avtp_capture_a_mux[] = {
	AVB_AVTP_CAPTURE_A_MARK,
};
static const unsigned int avb_avtp_match_b_pins[] = {
	/*  AVB_AVTP_MATCH_B */
	RCAR_GP_PIN(1, 8),
};
static const unsigned int avb_avtp_match_b_mux[] = {
	AVB_AVTP_MATCH_B_MARK,
};
static const unsigned int avb_avtp_capture_b_pins[] = {
	/* AVB_AVTP_CAPTURE_B */
	RCAR_GP_PIN(1, 11),
};
static const unsigned int avb_avtp_capture_b_mux[] = {
	AVB_AVTP_CAPTURE_B_MARK,
};

/* - I2C -------------------------------------------------------------------- */
static const unsigned int i2c1_a_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(5, 11), RCAR_GP_PIN(5, 10),
};
static const unsigned int i2c1_a_mux[] = {
	SDA1_A_MARK, SCL1_A_MARK,
};
static const unsigned int i2c1_b_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(5, 24), RCAR_GP_PIN(5, 23),
};
static const unsigned int i2c1_b_mux[] = {
	SDA1_B_MARK, SCL1_B_MARK,
};
static const unsigned int i2c2_a_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(5, 0), RCAR_GP_PIN(5, 4),
};
static const unsigned int i2c2_a_mux[] = {
	SDA2_A_MARK, SCL2_A_MARK,
};
static const unsigned int i2c2_b_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(3, 13), RCAR_GP_PIN(3, 12),
};
static const unsigned int i2c2_b_mux[] = {
	SDA2_B_MARK, SCL2_B_MARK,
};
static const unsigned int i2c6_a_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(1, 8), RCAR_GP_PIN(1, 11),
};
static const unsigned int i2c6_a_mux[] = {
	SDA6_A_MARK, SCL6_A_MARK,
};
static const unsigned int i2c6_b_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(1, 26), RCAR_GP_PIN(1, 25),
};
static const unsigned int i2c6_b_mux[] = {
	SDA6_B_MARK, SCL6_B_MARK,
};
static const unsigned int i2c6_c_pins[] = {
	/* SDA, SCL */
	RCAR_GP_PIN(0, 15), RCAR_GP_PIN(0, 14),
};
static const unsigned int i2c6_c_mux[] = {
	SDA6_C_MARK, SCL6_C_MARK,
};

/* - SCIF0 ------------------------------------------------------------------ */
static const unsigned int scif0_data_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 1), RCAR_GP_PIN(5, 2),
};
static const unsigned int scif0_data_mux[] = {
	RX0_MARK, TX0_MARK,
};
static const unsigned int scif0_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(5, 0),
};
static const unsigned int scif0_clk_mux[] = {
	SCK0_MARK,
};
static const unsigned int scif0_ctrl_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(5, 4), RCAR_GP_PIN(5, 3),
};
static const unsigned int scif0_ctrl_mux[] = {
	RTS0_N_TANS_MARK, CTS0_N_MARK,
};
/* - SCIF1 ------------------------------------------------------------------ */
static const unsigned int scif1_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 5), RCAR_GP_PIN(5, 6),
};
static const unsigned int scif1_data_a_mux[] = {
	RX1_A_MARK, TX1_A_MARK,
};
static const unsigned int scif1_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(6, 21),
};
static const unsigned int scif1_clk_mux[] = {
	SCK1_MARK,
};
static const unsigned int scif1_ctrl_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(5, 8), RCAR_GP_PIN(5, 7),
};
static const unsigned int scif1_ctrl_mux[] = {
	RTS1_N_TANS_MARK, CTS1_N_MARK,
};

static const unsigned int scif1_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 24), RCAR_GP_PIN(5, 25),
};
static const unsigned int scif1_data_b_mux[] = {
	RX1_B_MARK, TX1_B_MARK,
};
/* - SCIF2 ------------------------------------------------------------------ */
static const unsigned int scif2_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 11), RCAR_GP_PIN(5, 10),
};
static const unsigned int scif2_data_a_mux[] = {
	RX2_A_MARK, TX2_A_MARK,
};
static const unsigned int scif2_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(5, 9),
};
static const unsigned int scif2_clk_mux[] = {
	SCK2_MARK,
};
static const unsigned int scif2_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 15), RCAR_GP_PIN(5, 16),
};
static const unsigned int scif2_data_b_mux[] = {
	RX2_B_MARK, TX2_B_MARK,
};
/* - SCIF3 ------------------------------------------------------------------ */
static const unsigned int scif3_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 23), RCAR_GP_PIN(1, 24),
};
static const unsigned int scif3_data_a_mux[] = {
	RX3_A_MARK, TX3_A_MARK,
};
static const unsigned int scif3_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 22),
};
static const unsigned int scif3_clk_mux[] = {
	SCK3_MARK,
};
static const unsigned int scif3_ctrl_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(1, 26), RCAR_GP_PIN(1, 25),
};
static const unsigned int scif3_ctrl_mux[] = {
	RTS3_N_TANS_MARK, CTS3_N_MARK,
};
static const unsigned int scif3_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 8), RCAR_GP_PIN(1, 11),
};
static const unsigned int scif3_data_b_mux[] = {
	RX3_B_MARK, TX3_B_MARK,
};
/* - SCIF4 ------------------------------------------------------------------ */
static const unsigned int scif4_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(2, 11), RCAR_GP_PIN(2, 12),
};
static const unsigned int scif4_data_a_mux[] = {
	RX4_A_MARK, TX4_A_MARK,
};
static const unsigned int scif4_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(2, 10),
};
static const unsigned int scif4_clk_a_mux[] = {
	SCK4_A_MARK,
};
static const unsigned int scif4_ctrl_a_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(2, 14), RCAR_GP_PIN(2, 13),
};
static const unsigned int scif4_ctrl_a_mux[] = {
	RTS4_N_TANS_A_MARK, CTS4_N_A_MARK,
};
static const unsigned int scif4_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 6), RCAR_GP_PIN(1, 7),
};
static const unsigned int scif4_data_b_mux[] = {
	RX4_B_MARK, TX4_B_MARK,
};
static const unsigned int scif4_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 5),
};
static const unsigned int scif4_clk_b_mux[] = {
	SCK4_B_MARK,
};
static const unsigned int scif4_ctrl_b_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(1, 10), RCAR_GP_PIN(1, 9),
};
static const unsigned int scif4_ctrl_b_mux[] = {
	RTS4_N_TANS_B_MARK, CTS4_N_B_MARK,
};
static const unsigned int scif4_data_c_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(0, 12), RCAR_GP_PIN(0, 13),
};
static const unsigned int scif4_data_c_mux[] = {
	RX4_C_MARK, TX4_C_MARK,
};
static const unsigned int scif4_clk_c_pins[] = {
	/* SCK */
	RCAR_GP_PIN(0, 8),
};
static const unsigned int scif4_clk_c_mux[] = {
	SCK4_C_MARK,
};
static const unsigned int scif4_ctrl_c_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(0, 11), RCAR_GP_PIN(0, 10),
};
static const unsigned int scif4_ctrl_c_mux[] = {
	RTS4_N_TANS_C_MARK, CTS4_N_C_MARK,
};
/* - SCIF5 ------------------------------------------------------------------ */
static const unsigned int scif5_data_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(5, 19), RCAR_GP_PIN(5, 21),
};
static const unsigned int scif5_data_mux[] = {
	RX5_MARK, TX5_MARK,
};
static const unsigned int scif5_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(6, 21),
};
static const unsigned int scif5_clk_mux[] = {
	SCK5_MARK,
};

/* - SSI -------------------------------------------------------------------- */
static const unsigned int ssi0_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 2),
};
static const unsigned int ssi0_data_mux[] = {
	SSI_SDATA0_MARK,
};
static const unsigned int ssi01239_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 0), RCAR_GP_PIN(6, 1),
};
static const unsigned int ssi01239_ctrl_mux[] = {
	SSI_SCK0129_MARK, SSI_WS0129_MARK,
};
static const unsigned int ssi1_data_a_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 3),
};
static const unsigned int ssi1_data_a_mux[] = {
	SSI_SDATA1_A_MARK,
};
static const unsigned int ssi1_data_b_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(5, 12),
};
static const unsigned int ssi1_data_b_mux[] = {
	SSI_SDATA1_B_MARK,
};
static const unsigned int ssi1_ctrl_a_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 26), RCAR_GP_PIN(6, 27),
};
static const unsigned int ssi1_ctrl_a_mux[] = {
	SSI_SCK1_A_MARK, SSI_WS1_A_MARK,
};
static const unsigned int ssi1_ctrl_b_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 4), RCAR_GP_PIN(6, 21),
};
static const unsigned int ssi1_ctrl_b_mux[] = {
	SSI_SCK1_B_MARK, SSI_WS1_B_MARK,
};
static const unsigned int ssi2_data_a_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 4),
};
static const unsigned int ssi2_data_a_mux[] = {
	SSI_SDATA2_A_MARK,
};
static const unsigned int ssi2_data_b_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(5, 13),
};
static const unsigned int ssi2_data_b_mux[] = {
	SSI_SDATA2_B_MARK,
};
static const unsigned int ssi2_ctrl_a_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(5, 19), RCAR_GP_PIN(5, 21),
};
static const unsigned int ssi2_ctrl_a_mux[] = {
	SSI_SCK2_A_MARK, SSI_WS2_A_MARK,
};
static const unsigned int ssi2_ctrl_b_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 28), RCAR_GP_PIN(6, 29),
};
static const unsigned int ssi2_ctrl_b_mux[] = {
	SSI_SCK2_B_MARK, SSI_WS2_B_MARK,
};
static const unsigned int ssi3_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 7),
};
static const unsigned int ssi3_data_mux[] = {
	SSI_SDATA3_MARK,
};
static const unsigned int ssi34_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 5), RCAR_GP_PIN(6, 6),
};
static const unsigned int ssi34_ctrl_mux[] = {
	SSI_SCK34_MARK, SSI_WS34_MARK,
};
static const unsigned int ssi4_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 10),
};
static const unsigned int ssi4_data_mux[] = {
	SSI_SDATA4_MARK,
};
static const unsigned int ssi4_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 8), RCAR_GP_PIN(6, 9),
};
static const unsigned int ssi4_ctrl_mux[] = {
	SSI_SCK4_MARK, SSI_WS4_MARK,
};
static const unsigned int ssi5_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 13),
};
static const unsigned int ssi5_data_mux[] = {
	SSI_SDATA5_MARK,
};
static const unsigned int ssi5_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 11), RCAR_GP_PIN(6, 12),
};
static const unsigned int ssi5_ctrl_mux[] = {
	SSI_SCK5_MARK, SSI_WS5_MARK,
};
static const unsigned int ssi6_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 16),
};
static const unsigned int ssi6_data_mux[] = {
	SSI_SDATA6_MARK,
};
static const unsigned int ssi6_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 14), RCAR_GP_PIN(6, 15),
};
static const unsigned int ssi6_ctrl_mux[] = {
	SSI_SCK6_MARK, SSI_WS6_MARK,
};
static const unsigned int ssi7_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 19),
};
static const unsigned int ssi7_data_mux[] = {
	SSI_SDATA7_MARK,
};
static const unsigned int ssi78_ctrl_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 17), RCAR_GP_PIN(6, 18),
};
static const unsigned int ssi78_ctrl_mux[] = {
	SSI_SCK78_MARK, SSI_WS78_MARK,
};
static const unsigned int ssi8_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 20),
};
static const unsigned int ssi8_data_mux[] = {
	SSI_SDATA8_MARK,
};
static const unsigned int ssi9_data_a_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(6, 21),
};
static const unsigned int ssi9_data_a_mux[] = {
	SSI_SDATA9_A_MARK,
};
static const unsigned int ssi9_data_b_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(5, 14),
};
static const unsigned int ssi9_data_b_mux[] = {
	SSI_SDATA9_B_MARK,
};
static const unsigned int ssi9_ctrl_a_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(5, 15), RCAR_GP_PIN(5, 16),
};
static const unsigned int ssi9_ctrl_a_mux[] = {
	SSI_SCK9_A_MARK, SSI_WS9_A_MARK,
};
static const unsigned int ssi9_ctrl_b_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(6, 30), RCAR_GP_PIN(6, 31),
};
static const unsigned int ssi9_ctrl_b_mux[] = {
	SSI_SCK9_B_MARK, SSI_WS9_B_MARK,
};

static const struct sh_pfc_pin_group pinmux_groups[] = {
	SH_PFC_PIN_GROUP(audio_clk_a_a),
	SH_PFC_PIN_GROUP(audio_clk_a_b),
	SH_PFC_PIN_GROUP(audio_clk_a_c),
	SH_PFC_PIN_GROUP(audio_clk_b_a),
	SH_PFC_PIN_GROUP(audio_clk_b_b),
	SH_PFC_PIN_GROUP(audio_clk_c_a),
	SH_PFC_PIN_GROUP(audio_clk_c_b),
	SH_PFC_PIN_GROUP(audio_clkout_a),
	SH_PFC_PIN_GROUP(audio_clkout_b),
	SH_PFC_PIN_GROUP(audio_clkout_c),
	SH_PFC_PIN_GROUP(audio_clkout_d),
	SH_PFC_PIN_GROUP(audio_clkout1_a),
	SH_PFC_PIN_GROUP(audio_clkout1_b),
	SH_PFC_PIN_GROUP(audio_clkout2_a),
	SH_PFC_PIN_GROUP(audio_clkout2_b),
	SH_PFC_PIN_GROUP(audio_clkout3_a),
	SH_PFC_PIN_GROUP(audio_clkout3_b),
	SH_PFC_PIN_GROUP(avb_link),
	SH_PFC_PIN_GROUP(avb_magic),
	SH_PFC_PIN_GROUP(avb_phy_int),
	SH_PFC_PIN_GROUP(avb_mdc),
	SH_PFC_PIN_GROUP(avb_avtp_pps),
	SH_PFC_PIN_GROUP(avb_avtp_match_a),
	SH_PFC_PIN_GROUP(avb_avtp_capture_a),
	SH_PFC_PIN_GROUP(avb_avtp_match_b),
	SH_PFC_PIN_GROUP(avb_avtp_capture_b),
	SH_PFC_PIN_GROUP(i2c1_a),
	SH_PFC_PIN_GROUP(i2c1_b),
	SH_PFC_PIN_GROUP(i2c2_a),
	SH_PFC_PIN_GROUP(i2c2_b),
	SH_PFC_PIN_GROUP(i2c6_a),
	SH_PFC_PIN_GROUP(i2c6_b),
	SH_PFC_PIN_GROUP(i2c6_c),
	SH_PFC_PIN_GROUP(scif0_data),
	SH_PFC_PIN_GROUP(scif0_clk),
	SH_PFC_PIN_GROUP(scif0_ctrl),
	SH_PFC_PIN_GROUP(scif1_data_a),
	SH_PFC_PIN_GROUP(scif1_clk),
	SH_PFC_PIN_GROUP(scif1_ctrl),
	SH_PFC_PIN_GROUP(scif1_data_b),
	SH_PFC_PIN_GROUP(scif2_data_a),
	SH_PFC_PIN_GROUP(scif2_clk),
	SH_PFC_PIN_GROUP(scif2_data_b),
	SH_PFC_PIN_GROUP(scif3_data_a),
	SH_PFC_PIN_GROUP(scif3_clk),
	SH_PFC_PIN_GROUP(scif3_ctrl),
	SH_PFC_PIN_GROUP(scif3_data_b),
	SH_PFC_PIN_GROUP(scif4_data_a),
	SH_PFC_PIN_GROUP(scif4_clk_a),
	SH_PFC_PIN_GROUP(scif4_ctrl_a),
	SH_PFC_PIN_GROUP(scif4_data_b),
	SH_PFC_PIN_GROUP(scif4_clk_b),
	SH_PFC_PIN_GROUP(scif4_ctrl_b),
	SH_PFC_PIN_GROUP(scif4_data_c),
	SH_PFC_PIN_GROUP(scif4_clk_c),
	SH_PFC_PIN_GROUP(scif4_ctrl_c),
	SH_PFC_PIN_GROUP(scif5_data),
	SH_PFC_PIN_GROUP(scif5_clk),
	SH_PFC_PIN_GROUP(ssi0_data),
	SH_PFC_PIN_GROUP(ssi01239_ctrl),
	SH_PFC_PIN_GROUP(ssi1_data_a),
	SH_PFC_PIN_GROUP(ssi1_data_b),
	SH_PFC_PIN_GROUP(ssi1_ctrl_a),
	SH_PFC_PIN_GROUP(ssi1_ctrl_b),
	SH_PFC_PIN_GROUP(ssi2_data_a),
	SH_PFC_PIN_GROUP(ssi2_data_b),
	SH_PFC_PIN_GROUP(ssi2_ctrl_a),
	SH_PFC_PIN_GROUP(ssi2_ctrl_b),
	SH_PFC_PIN_GROUP(ssi3_data),
	SH_PFC_PIN_GROUP(ssi34_ctrl),
	SH_PFC_PIN_GROUP(ssi4_data),
	SH_PFC_PIN_GROUP(ssi4_ctrl),
	SH_PFC_PIN_GROUP(ssi5_data),
	SH_PFC_PIN_GROUP(ssi5_ctrl),
	SH_PFC_PIN_GROUP(ssi6_data),
	SH_PFC_PIN_GROUP(ssi6_ctrl),
	SH_PFC_PIN_GROUP(ssi7_data),
	SH_PFC_PIN_GROUP(ssi78_ctrl),
	SH_PFC_PIN_GROUP(ssi8_data),
	SH_PFC_PIN_GROUP(ssi9_data_a),
	SH_PFC_PIN_GROUP(ssi9_data_b),
	SH_PFC_PIN_GROUP(ssi9_ctrl_a),
	SH_PFC_PIN_GROUP(ssi9_ctrl_b),
};

static const char * const audio_clk_groups[] = {
	"audio_clk_a_a",
	"audio_clk_a_b",
	"audio_clk_a_c",
	"audio_clk_b_a",
	"audio_clk_b_b",
	"audio_clk_c_a",
	"audio_clk_c_b",
	"audio_clkout_a",
	"audio_clkout_b",
	"audio_clkout_c",
	"audio_clkout_d",
	"audio_clkout1_a",
	"audio_clkout1_b",
	"audio_clkout2_a",
	"audio_clkout2_b",
	"audio_clkout3_a",
	"audio_clkout3_b",
};

static const char * const avb_groups[] = {
	"avb_link",
	"avb_magic",
	"avb_phy_int",
	"avb_mdc",
	"avb_avtp_pps",
	"avb_avtp_match_a",
	"avb_avtp_capture_a",
	"avb_avtp_match_b",
	"avb_avtp_capture_b",
};

static const char * const i2c1_groups[] = {
	"i2c1_a",
	"i2c1_b",
};

static const char * const i2c2_groups[] = {
	"i2c2_a",
	"i2c2_b",
};

static const char * const i2c6_groups[] = {
	"i2c6_a",
	"i2c6_b",
	"i2c6_c",
};

static const char * const scif0_groups[] = {
	"scif0_data",
	"scif0_clk",
	"scif0_ctrl",
};

static const char * const scif1_groups[] = {
	"scif1_data_a",
	"scif1_clk",
	"scif1_ctrl",
	"scif1_data_b",
};

static const char * const scif2_groups[] = {
	"scif2_data_a",
	"scif2_clk",
	"scif2_data_b",
};

static const char * const scif3_groups[] = {
	"scif3_data_a",
	"scif3_clk",
	"scif3_ctrl",
	"scif3_data_b",
};

static const char * const scif4_groups[] = {
	"scif4_data_a",
	"scif4_clk_a",
	"scif4_ctrl_a",
	"scif4_data_b",
	"scif4_clk_b",
	"scif4_ctrl_b",
	"scif4_data_c",
	"scif4_clk_c",
	"scif4_ctrl_c",
};

static const char * const scif5_groups[] = {
	"scif5_data",
	"scif5_clk",
};

static const char * const ssi_groups[] = {
	"ssi0_data",
	"ssi01239_ctrl",
	"ssi1_data_a",
	"ssi1_data_b",
	"ssi1_ctrl_a",
	"ssi1_ctrl_b",
	"ssi2_data_a",
	"ssi2_data_b",
	"ssi2_ctrl_a",
	"ssi2_ctrl_b",
	"ssi3_data",
	"ssi34_ctrl",
	"ssi4_data",
	"ssi4_ctrl",
	"ssi5_data",
	"ssi5_ctrl",
	"ssi6_data",
	"ssi6_ctrl",
	"ssi7_data",
	"ssi78_ctrl",
	"ssi8_data",
	"ssi9_data_a",
	"ssi9_data_b",
	"ssi9_ctrl_a",
	"ssi9_ctrl_b",
};

static const struct sh_pfc_function pinmux_functions[] = {
	SH_PFC_FUNCTION(audio_clk),
	SH_PFC_FUNCTION(avb),
	SH_PFC_FUNCTION(i2c1),
	SH_PFC_FUNCTION(i2c2),
	SH_PFC_FUNCTION(i2c6),
	SH_PFC_FUNCTION(scif0),
	SH_PFC_FUNCTION(scif1),
	SH_PFC_FUNCTION(scif2),
	SH_PFC_FUNCTION(scif3),
	SH_PFC_FUNCTION(scif4),
	SH_PFC_FUNCTION(scif5),
	SH_PFC_FUNCTION(ssi),
};

static const struct pinmux_cfg_reg pinmux_config_regs[] = {
#define F_(x, y)	FN_##y
#define FM(x)		FN_##x
	{ PINMUX_CFG_REG("GPSR0", 0xe6060100, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_0_15_FN,	GPSR0_15,
		GP_0_14_FN,	GPSR0_14,
		GP_0_13_FN,	GPSR0_13,
		GP_0_12_FN,	GPSR0_12,
		GP_0_11_FN,	GPSR0_11,
		GP_0_10_FN,	GPSR0_10,
		GP_0_9_FN,	GPSR0_9,
		GP_0_8_FN,	GPSR0_8,
		GP_0_7_FN,	GPSR0_7,
		GP_0_6_FN,	GPSR0_6,
		GP_0_5_FN,	GPSR0_5,
		GP_0_4_FN,	GPSR0_4,
		GP_0_3_FN,	GPSR0_3,
		GP_0_2_FN,	GPSR0_2,
		GP_0_1_FN,	GPSR0_1,
		GP_0_0_FN,	GPSR0_0, }
	},
	{ PINMUX_CFG_REG("GPSR1", 0xe6060104, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_1_27_FN,	GPSR1_27,
		GP_1_26_FN,	GPSR1_26,
		GP_1_25_FN,	GPSR1_25,
		GP_1_24_FN,	GPSR1_24,
		GP_1_23_FN,	GPSR1_23,
		GP_1_22_FN,	GPSR1_22,
		GP_1_21_FN,	GPSR1_21,
		GP_1_20_FN,	GPSR1_20,
		GP_1_19_FN,	GPSR1_19,
		GP_1_18_FN,	GPSR1_18,
		GP_1_17_FN,	GPSR1_17,
		GP_1_16_FN,	GPSR1_16,
		GP_1_15_FN,	GPSR1_15,
		GP_1_14_FN,	GPSR1_14,
		GP_1_13_FN,	GPSR1_13,
		GP_1_12_FN,	GPSR1_12,
		GP_1_11_FN,	GPSR1_11,
		GP_1_10_FN,	GPSR1_10,
		GP_1_9_FN,	GPSR1_9,
		GP_1_8_FN,	GPSR1_8,
		GP_1_7_FN,	GPSR1_7,
		GP_1_6_FN,	GPSR1_6,
		GP_1_5_FN,	GPSR1_5,
		GP_1_4_FN,	GPSR1_4,
		GP_1_3_FN,	GPSR1_3,
		GP_1_2_FN,	GPSR1_2,
		GP_1_1_FN,	GPSR1_1,
		GP_1_0_FN,	GPSR1_0, }
	},
	{ PINMUX_CFG_REG("GPSR2", 0xe6060108, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_2_14_FN,	GPSR2_14,
		GP_2_13_FN,	GPSR2_13,
		GP_2_12_FN,	GPSR2_12,
		GP_2_11_FN,	GPSR2_11,
		GP_2_10_FN,	GPSR2_10,
		GP_2_9_FN,	GPSR2_9,
		GP_2_8_FN,	GPSR2_8,
		GP_2_7_FN,	GPSR2_7,
		GP_2_6_FN,	GPSR2_6,
		GP_2_5_FN,	GPSR2_5,
		GP_2_4_FN,	GPSR2_4,
		GP_2_3_FN,	GPSR2_3,
		GP_2_2_FN,	GPSR2_2,
		GP_2_1_FN,	GPSR2_1,
		GP_2_0_FN,	GPSR2_0, }
	},
	{ PINMUX_CFG_REG("GPSR3", 0xe606010c, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_3_15_FN,	GPSR3_15,
		GP_3_14_FN,	GPSR3_14,
		GP_3_13_FN,	GPSR3_13,
		GP_3_12_FN,	GPSR3_12,
		GP_3_11_FN,	GPSR3_11,
		GP_3_10_FN,	GPSR3_10,
		GP_3_9_FN,	GPSR3_9,
		GP_3_8_FN,	GPSR3_8,
		GP_3_7_FN,	GPSR3_7,
		GP_3_6_FN,	GPSR3_6,
		GP_3_5_FN,	GPSR3_5,
		GP_3_4_FN,	GPSR3_4,
		GP_3_3_FN,	GPSR3_3,
		GP_3_2_FN,	GPSR3_2,
		GP_3_1_FN,	GPSR3_1,
		GP_3_0_FN,	GPSR3_0, }
	},
	{ PINMUX_CFG_REG("GPSR4", 0xe6060110, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_4_17_FN,	GPSR4_17,
		GP_4_16_FN,	GPSR4_16,
		GP_4_15_FN,	GPSR4_15,
		GP_4_14_FN,	GPSR4_14,
		GP_4_13_FN,	GPSR4_13,
		GP_4_12_FN,	GPSR4_12,
		GP_4_11_FN,	GPSR4_11,
		GP_4_10_FN,	GPSR4_10,
		GP_4_9_FN,	GPSR4_9,
		GP_4_8_FN,	GPSR4_8,
		GP_4_7_FN,	GPSR4_7,
		GP_4_6_FN,	GPSR4_6,
		GP_4_5_FN,	GPSR4_5,
		GP_4_4_FN,	GPSR4_4,
		GP_4_3_FN,	GPSR4_3,
		GP_4_2_FN,	GPSR4_2,
		GP_4_1_FN,	GPSR4_1,
		GP_4_0_FN,	GPSR4_0, }
	},
	{ PINMUX_CFG_REG("GPSR5", 0xe6060114, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_5_25_FN,	GPSR5_25,
		GP_5_24_FN,	GPSR5_24,
		GP_5_23_FN,	GPSR5_23,
		GP_5_22_FN,	GPSR5_22,
		GP_5_21_FN,	GPSR5_21,
		GP_5_20_FN,	GPSR5_20,
		GP_5_19_FN,	GPSR5_19,
		GP_5_18_FN,	GPSR5_18,
		GP_5_17_FN,	GPSR5_17,
		GP_5_16_FN,	GPSR5_16,
		GP_5_15_FN,	GPSR5_15,
		GP_5_14_FN,	GPSR5_14,
		GP_5_13_FN,	GPSR5_13,
		GP_5_12_FN,	GPSR5_12,
		GP_5_11_FN,	GPSR5_11,
		GP_5_10_FN,	GPSR5_10,
		GP_5_9_FN,	GPSR5_9,
		GP_5_8_FN,	GPSR5_8,
		GP_5_7_FN,	GPSR5_7,
		GP_5_6_FN,	GPSR5_6,
		GP_5_5_FN,	GPSR5_5,
		GP_5_4_FN,	GPSR5_4,
		GP_5_3_FN,	GPSR5_3,
		GP_5_2_FN,	GPSR5_2,
		GP_5_1_FN,	GPSR5_1,
		GP_5_0_FN,	GPSR5_0, }
	},
	{ PINMUX_CFG_REG("GPSR6", 0xe6060118, 32, 1) {
		GP_6_31_FN,	GPSR6_31,
		GP_6_30_FN,	GPSR6_30,
		GP_6_29_FN,	GPSR6_29,
		GP_6_28_FN,	GPSR6_28,
		GP_6_27_FN,	GPSR6_27,
		GP_6_26_FN,	GPSR6_26,
		GP_6_25_FN,	GPSR6_25,
		GP_6_24_FN,	GPSR6_24,
		GP_6_23_FN,	GPSR6_23,
		GP_6_22_FN,	GPSR6_22,
		GP_6_21_FN,	GPSR6_21,
		GP_6_20_FN,	GPSR6_20,
		GP_6_19_FN,	GPSR6_19,
		GP_6_18_FN,	GPSR6_18,
		GP_6_17_FN,	GPSR6_17,
		GP_6_16_FN,	GPSR6_16,
		GP_6_15_FN,	GPSR6_15,
		GP_6_14_FN,	GPSR6_14,
		GP_6_13_FN,	GPSR6_13,
		GP_6_12_FN,	GPSR6_12,
		GP_6_11_FN,	GPSR6_11,
		GP_6_10_FN,	GPSR6_10,
		GP_6_9_FN,	GPSR6_9,
		GP_6_8_FN,	GPSR6_8,
		GP_6_7_FN,	GPSR6_7,
		GP_6_6_FN,	GPSR6_6,
		GP_6_5_FN,	GPSR6_5,
		GP_6_4_FN,	GPSR6_4,
		GP_6_3_FN,	GPSR6_3,
		GP_6_2_FN,	GPSR6_2,
		GP_6_1_FN,	GPSR6_1,
		GP_6_0_FN,	GPSR6_0, }
	},
	{ PINMUX_CFG_REG("GPSR7", 0xe606011c, 32, 1) {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		GP_7_3_FN, GPSR7_3,
		GP_7_2_FN, GPSR7_2,
		GP_7_1_FN, GPSR7_1,
		GP_7_0_FN, GPSR7_0, }
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG("IPSR0", 0xe6060200, 32, 4) {
		IP0_31_28
		IP0_27_24
		IP0_23_20
		IP0_19_16
		IP0_15_12
		IP0_11_8
		IP0_7_4
		IP0_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR1", 0xe6060204, 32, 4) {
		IP1_31_28
		IP1_27_24
		IP1_23_20
		IP1_19_16
		IP1_15_12
		IP1_11_8
		IP1_7_4
		IP1_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR2", 0xe6060208, 32, 4) {
		IP2_31_28
		IP2_27_24
		IP2_23_20
		IP2_19_16
		IP2_15_12
		IP2_11_8
		IP2_7_4
		IP2_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR3", 0xe606020c, 32, 4) {
		IP3_31_28
		IP3_27_24
		IP3_23_20
		IP3_19_16
		IP3_15_12
		IP3_11_8
		IP3_7_4
		IP3_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR4", 0xe6060210, 32, 4) {
		IP4_31_28
		IP4_27_24
		IP4_23_20
		IP4_19_16
		IP4_15_12
		IP4_11_8
		IP4_7_4
		IP4_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR5", 0xe6060214, 32, 4) {
		IP5_31_28
		IP5_27_24
		IP5_23_20
		IP5_19_16
		IP5_15_12
		IP5_11_8
		IP5_7_4
		IP5_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR6", 0xe6060218, 32, 4) {
		IP6_31_28
		IP6_27_24
		IP6_23_20
		IP6_19_16
		IP6_15_12
		IP6_11_8
		IP6_7_4
		IP6_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR7", 0xe606021c, 32, 4) {
		IP7_31_28
		IP7_27_24
		IP7_23_20
		IP7_19_16
		IP7_15_12
		IP7_11_8
		IP7_7_4
		IP7_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR8", 0xe6060220, 32, 4) {
		IP8_31_28
		IP8_27_24
		IP8_23_20
		IP8_19_16
		IP8_15_12
		IP8_11_8
		IP8_7_4
		IP8_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR9", 0xe6060224, 32, 4) {
		IP9_31_28
		IP9_27_24
		IP9_23_20
		IP9_19_16
		IP9_15_12
		IP9_11_8
		IP9_7_4
		IP9_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR10", 0xe6060228, 32, 4) {
		IP10_31_28
		IP10_27_24
		IP10_23_20
		IP10_19_16
		IP10_15_12
		IP10_11_8
		IP10_7_4
		IP10_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR11", 0xe606022c, 32, 4) {
		IP11_31_28
		IP11_27_24
		IP11_23_20
		IP11_19_16
		IP11_15_12
		IP11_11_8
		IP11_7_4
		IP11_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR12", 0xe6060230, 32, 4) {
		IP12_31_28
		IP12_27_24
		IP12_23_20
		IP12_19_16
		IP12_15_12
		IP12_11_8
		IP12_7_4
		IP12_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR13", 0xe6060234, 32, 4) {
		IP13_31_28
		IP13_27_24
		IP13_23_20
		IP13_19_16
		IP13_15_12
		IP13_11_8
		IP13_7_4
		IP13_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR14", 0xe6060238, 32, 4) {
		IP14_31_28
		IP14_27_24
		IP14_23_20
		IP14_19_16
		IP14_15_12
		IP14_11_8
		IP14_7_4
		IP14_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR15", 0xe606023c, 32, 4) {
		IP15_31_28
		IP15_27_24
		IP15_23_20
		IP15_19_16
		IP15_15_12
		IP15_11_8
		IP15_7_4
		IP15_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR16", 0xe6060240, 32, 4) {
		IP16_31_28
		IP16_27_24
		IP16_23_20
		IP16_19_16
		IP16_15_12
		IP16_11_8
		IP16_7_4
		IP16_3_0 }
	},
	{ PINMUX_CFG_REG("IPSR17", 0xe6060244, 32, 4) {
		/* IP17_31_28 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* IP17_27_24 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* IP17_23_20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* IP17_19_16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* IP17_15_12 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* IP17_11_8  */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		IP17_7_4
		IP17_3_0 }
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG_VAR("MOD_SEL0", 0xe6060500, 32,
			     1, 2, 2, 3, 1, 1, 2, 1, 1, 1,
			     2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2, 1) {
		0, 0, /* RESERVED 31 */
		MOD_SEL0_30_29
		MOD_SEL0_28_27
		MOD_SEL0_26_25_24
		MOD_SEL0_23
		MOD_SEL0_22
		MOD_SEL0_21_20
		MOD_SEL0_19
		MOD_SEL0_18
		MOD_SEL0_17
		MOD_SEL0_16_15
		MOD_SEL0_14
		MOD_SEL0_13
		MOD_SEL0_12
		MOD_SEL0_11
		MOD_SEL0_10
		MOD_SEL0_9
		MOD_SEL0_8
		MOD_SEL0_7_6
		MOD_SEL0_5_4
		MOD_SEL0_3
		MOD_SEL0_2_1
		0, 0, /* RESERVED 0 */ }
	},
	{ PINMUX_CFG_REG_VAR("MOD_SEL1", 0xe6060504, 32,
			     2, 3, 1, 2, 3, 1, 1, 2, 1,
			     2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1) {
		MOD_SEL1_31_30
		MOD_SEL1_29_28_27
		MOD_SEL1_26
		MOD_SEL1_25_24
		MOD_SEL1_23_22_21
		MOD_SEL1_20
		MOD_SEL1_19
		MOD_SEL1_18_17
		MOD_SEL1_16
		MOD_SEL1_15_14
		MOD_SEL1_13
		MOD_SEL1_12
		MOD_SEL1_11
		MOD_SEL1_10
		MOD_SEL1_9
		0, 0, 0, 0, /* RESERVED 8, 7 */
		MOD_SEL1_6
		MOD_SEL1_5
		MOD_SEL1_4
		MOD_SEL1_3
		MOD_SEL1_2
		MOD_SEL1_1
		MOD_SEL1_0 }
	},
	{ PINMUX_CFG_REG_VAR("MOD_SEL2", 0xe6060508, 32,
			     1, 1, 1, 1, 4, 4, 4,
			     4, 4, 4, 1, 2, 1) {
		MOD_SEL2_31
		MOD_SEL2_30
		MOD_SEL2_29
		/* RESERVED 28 */
		0, 0,
		/* RESERVED 27, 26, 25, 24 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 23, 22, 21, 20 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 19, 18, 17, 16 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 15, 14, 13, 12 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 11, 10, 9, 8 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 7, 6, 5, 4 */
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		/* RESERVED 3 */
		0, 0,
		MOD_SEL2_2_1
		MOD_SEL2_0 }
	},
	{ },
};

const struct sh_pfc_soc_info r8a7795_pinmux_info = {
	.name = "r8a77950_pfc",
	.unlock_reg = 0xe6060000, /* PMMR */

	.function = { PINMUX_FUNCTION_BEGIN, PINMUX_FUNCTION_END },

	.pins = pinmux_pins,
	.nr_pins = ARRAY_SIZE(pinmux_pins),
	.groups = pinmux_groups,
	.nr_groups = ARRAY_SIZE(pinmux_groups),
	.functions = pinmux_functions,
	.nr_functions = ARRAY_SIZE(pinmux_functions),

	.cfg_regs = pinmux_config_regs,

	.pinmux_data = pinmux_data,
	.pinmux_data_size = ARRAY_SIZE(pinmux_data),
};
