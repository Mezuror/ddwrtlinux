/*
 * imx6q pinctrl driver based on imx pinmux core
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Copyright (C) 2012 Linaro, Inc.
 *
 * Author: Dong Aisheng <dong.aisheng@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-imx.h"

enum imx6q_pads {
	MX6Q_PAD_RESERVE0 = 0,
	MX6Q_PAD_RESERVE1 = 1,
	MX6Q_PAD_RESERVE2 = 2,
	MX6Q_PAD_RESERVE3 = 3,
	MX6Q_PAD_RESERVE4 = 4,
	MX6Q_PAD_RESERVE5 = 5,
	MX6Q_PAD_RESERVE6 = 6,
	MX6Q_PAD_RESERVE7 = 7,
	MX6Q_PAD_RESERVE8 = 8,
	MX6Q_PAD_RESERVE9 = 9,
	MX6Q_PAD_RESERVE10 = 10,
	MX6Q_PAD_RESERVE11 = 11,
	MX6Q_PAD_RESERVE12 = 12,
	MX6Q_PAD_RESERVE13 = 13,
	MX6Q_PAD_RESERVE14 = 14,
	MX6Q_PAD_RESERVE15 = 15,
	MX6Q_PAD_RESERVE16 = 16,
	MX6Q_PAD_RESERVE17 = 17,
	MX6Q_PAD_RESERVE18 = 18,
	MX6Q_PAD_SD2_DAT1 = 19,
	MX6Q_PAD_SD2_DAT2 = 20,
	MX6Q_PAD_SD2_DAT0 = 21,
	MX6Q_PAD_RGMII_TXC = 22,
	MX6Q_PAD_RGMII_TD0 = 23,
	MX6Q_PAD_RGMII_TD1 = 24,
	MX6Q_PAD_RGMII_TD2 = 25,
	MX6Q_PAD_RGMII_TD3 = 26,
	MX6Q_PAD_RGMII_RX_CTL = 27,
	MX6Q_PAD_RGMII_RD0 = 28,
	MX6Q_PAD_RGMII_TX_CTL = 29,
	MX6Q_PAD_RGMII_RD1 = 30,
	MX6Q_PAD_RGMII_RD2 = 31,
	MX6Q_PAD_RGMII_RD3 = 32,
	MX6Q_PAD_RGMII_RXC = 33,
	MX6Q_PAD_EIM_A25 = 34,
	MX6Q_PAD_EIM_EB2 = 35,
	MX6Q_PAD_EIM_D16 = 36,
	MX6Q_PAD_EIM_D17 = 37,
	MX6Q_PAD_EIM_D18 = 38,
	MX6Q_PAD_EIM_D19 = 39,
	MX6Q_PAD_EIM_D20 = 40,
	MX6Q_PAD_EIM_D21 = 41,
	MX6Q_PAD_EIM_D22 = 42,
	MX6Q_PAD_EIM_D23 = 43,
	MX6Q_PAD_EIM_EB3 = 44,
	MX6Q_PAD_EIM_D24 = 45,
	MX6Q_PAD_EIM_D25 = 46,
	MX6Q_PAD_EIM_D26 = 47,
	MX6Q_PAD_EIM_D27 = 48,
	MX6Q_PAD_EIM_D28 = 49,
	MX6Q_PAD_EIM_D29 = 50,
	MX6Q_PAD_EIM_D30 = 51,
	MX6Q_PAD_EIM_D31 = 52,
	MX6Q_PAD_EIM_A24 = 53,
	MX6Q_PAD_EIM_A23 = 54,
	MX6Q_PAD_EIM_A22 = 55,
	MX6Q_PAD_EIM_A21 = 56,
	MX6Q_PAD_EIM_A20 = 57,
	MX6Q_PAD_EIM_A19 = 58,
	MX6Q_PAD_EIM_A18 = 59,
	MX6Q_PAD_EIM_A17 = 60,
	MX6Q_PAD_EIM_A16 = 61,
	MX6Q_PAD_EIM_CS0 = 62,
	MX6Q_PAD_EIM_CS1 = 63,
	MX6Q_PAD_EIM_OE = 64,
	MX6Q_PAD_EIM_RW = 65,
	MX6Q_PAD_EIM_LBA = 66,
	MX6Q_PAD_EIM_EB0 = 67,
	MX6Q_PAD_EIM_EB1 = 68,
	MX6Q_PAD_EIM_DA0 = 69,
	MX6Q_PAD_EIM_DA1 = 70,
	MX6Q_PAD_EIM_DA2 = 71,
	MX6Q_PAD_EIM_DA3 = 72,
	MX6Q_PAD_EIM_DA4 = 73,
	MX6Q_PAD_EIM_DA5 = 74,
	MX6Q_PAD_EIM_DA6 = 75,
	MX6Q_PAD_EIM_DA7 = 76,
	MX6Q_PAD_EIM_DA8 = 77,
	MX6Q_PAD_EIM_DA9 = 78,
	MX6Q_PAD_EIM_DA10 = 79,
	MX6Q_PAD_EIM_DA11 = 80,
	MX6Q_PAD_EIM_DA12 = 81,
	MX6Q_PAD_EIM_DA13 = 82,
	MX6Q_PAD_EIM_DA14 = 83,
	MX6Q_PAD_EIM_DA15 = 84,
	MX6Q_PAD_EIM_WAIT = 85,
	MX6Q_PAD_EIM_BCLK = 86,
	MX6Q_PAD_DI0_DISP_CLK = 87,
	MX6Q_PAD_DI0_PIN15 = 88,
	MX6Q_PAD_DI0_PIN2 = 89,
	MX6Q_PAD_DI0_PIN3 = 90,
	MX6Q_PAD_DI0_PIN4 = 91,
	MX6Q_PAD_DISP0_DAT0 = 92,
	MX6Q_PAD_DISP0_DAT1 = 93,
	MX6Q_PAD_DISP0_DAT2 = 94,
	MX6Q_PAD_DISP0_DAT3 = 95,
	MX6Q_PAD_DISP0_DAT4 = 96,
	MX6Q_PAD_DISP0_DAT5 = 97,
	MX6Q_PAD_DISP0_DAT6 = 98,
	MX6Q_PAD_DISP0_DAT7 = 99,
	MX6Q_PAD_DISP0_DAT8 = 100,
	MX6Q_PAD_DISP0_DAT9 = 101,
	MX6Q_PAD_DISP0_DAT10 = 102,
	MX6Q_PAD_DISP0_DAT11 = 103,
	MX6Q_PAD_DISP0_DAT12 = 104,
	MX6Q_PAD_DISP0_DAT13 = 105,
	MX6Q_PAD_DISP0_DAT14 = 106,
	MX6Q_PAD_DISP0_DAT15 = 107,
	MX6Q_PAD_DISP0_DAT16 = 108,
	MX6Q_PAD_DISP0_DAT17 = 109,
	MX6Q_PAD_DISP0_DAT18 = 110,
	MX6Q_PAD_DISP0_DAT19 = 111,
	MX6Q_PAD_DISP0_DAT20 = 112,
	MX6Q_PAD_DISP0_DAT21 = 113,
	MX6Q_PAD_DISP0_DAT22 = 114,
	MX6Q_PAD_DISP0_DAT23 = 115,
	MX6Q_PAD_ENET_MDIO = 116,
	MX6Q_PAD_ENET_REF_CLK = 117,
	MX6Q_PAD_ENET_RX_ER = 118,
	MX6Q_PAD_ENET_CRS_DV = 119,
	MX6Q_PAD_ENET_RXD1 = 120,
	MX6Q_PAD_ENET_RXD0 = 121,
	MX6Q_PAD_ENET_TX_EN = 122,
	MX6Q_PAD_ENET_TXD1 = 123,
	MX6Q_PAD_ENET_TXD0 = 124,
	MX6Q_PAD_ENET_MDC = 125,
	MX6Q_PAD_KEY_COL0 = 126,
	MX6Q_PAD_KEY_ROW0 = 127,
	MX6Q_PAD_KEY_COL1 = 128,
	MX6Q_PAD_KEY_ROW1 = 129,
	MX6Q_PAD_KEY_COL2 = 130,
	MX6Q_PAD_KEY_ROW2 = 131,
	MX6Q_PAD_KEY_COL3 = 132,
	MX6Q_PAD_KEY_ROW3 = 133,
	MX6Q_PAD_KEY_COL4 = 134,
	MX6Q_PAD_KEY_ROW4 = 135,
	MX6Q_PAD_GPIO_0 = 136,
	MX6Q_PAD_GPIO_1 = 137,
	MX6Q_PAD_GPIO_9 = 138,
	MX6Q_PAD_GPIO_3 = 139,
	MX6Q_PAD_GPIO_6 = 140,
	MX6Q_PAD_GPIO_2 = 141,
	MX6Q_PAD_GPIO_4 = 142,
	MX6Q_PAD_GPIO_5 = 143,
	MX6Q_PAD_GPIO_7 = 144,
	MX6Q_PAD_GPIO_8 = 145,
	MX6Q_PAD_GPIO_16 = 146,
	MX6Q_PAD_GPIO_17 = 147,
	MX6Q_PAD_GPIO_18 = 148,
	MX6Q_PAD_GPIO_19 = 149,
	MX6Q_PAD_CSI0_PIXCLK = 150,
	MX6Q_PAD_CSI0_MCLK = 151,
	MX6Q_PAD_CSI0_DATA_EN = 152,
	MX6Q_PAD_CSI0_VSYNC = 153,
	MX6Q_PAD_CSI0_DAT4 = 154,
	MX6Q_PAD_CSI0_DAT5 = 155,
	MX6Q_PAD_CSI0_DAT6 = 156,
	MX6Q_PAD_CSI0_DAT7 = 157,
	MX6Q_PAD_CSI0_DAT8 = 158,
	MX6Q_PAD_CSI0_DAT9 = 159,
	MX6Q_PAD_CSI0_DAT10 = 160,
	MX6Q_PAD_CSI0_DAT11 = 161,
	MX6Q_PAD_CSI0_DAT12 = 162,
	MX6Q_PAD_CSI0_DAT13 = 163,
	MX6Q_PAD_CSI0_DAT14 = 164,
	MX6Q_PAD_CSI0_DAT15 = 165,
	MX6Q_PAD_CSI0_DAT16 = 166,
	MX6Q_PAD_CSI0_DAT17 = 167,
	MX6Q_PAD_CSI0_DAT18 = 168,
	MX6Q_PAD_CSI0_DAT19 = 169,
	MX6Q_PAD_SD3_DAT7 = 170,
	MX6Q_PAD_SD3_DAT6 = 171,
	MX6Q_PAD_SD3_DAT5 = 172,
	MX6Q_PAD_SD3_DAT4 = 173,
	MX6Q_PAD_SD3_CMD = 174,
	MX6Q_PAD_SD3_CLK = 175,
	MX6Q_PAD_SD3_DAT0 = 176,
	MX6Q_PAD_SD3_DAT1 = 177,
	MX6Q_PAD_SD3_DAT2 = 178,
	MX6Q_PAD_SD3_DAT3 = 179,
	MX6Q_PAD_SD3_RST = 180,
	MX6Q_PAD_NANDF_CLE = 181,
	MX6Q_PAD_NANDF_ALE = 182,
	MX6Q_PAD_NANDF_WP_B = 183,
	MX6Q_PAD_NANDF_RB0 = 184,
	MX6Q_PAD_NANDF_CS0 = 185,
	MX6Q_PAD_NANDF_CS1 = 186,
	MX6Q_PAD_NANDF_CS2 = 187,
	MX6Q_PAD_NANDF_CS3 = 188,
	MX6Q_PAD_SD4_CMD = 189,
	MX6Q_PAD_SD4_CLK = 190,
	MX6Q_PAD_NANDF_D0 = 191,
	MX6Q_PAD_NANDF_D1 = 192,
	MX6Q_PAD_NANDF_D2 = 193,
	MX6Q_PAD_NANDF_D3 = 194,
	MX6Q_PAD_NANDF_D4 = 195,
	MX6Q_PAD_NANDF_D5 = 196,
	MX6Q_PAD_NANDF_D6 = 197,
	MX6Q_PAD_NANDF_D7 = 198,
	MX6Q_PAD_SD4_DAT0 = 199,
	MX6Q_PAD_SD4_DAT1 = 200,
	MX6Q_PAD_SD4_DAT2 = 201,
	MX6Q_PAD_SD4_DAT3 = 202,
	MX6Q_PAD_SD4_DAT4 = 203,
	MX6Q_PAD_SD4_DAT5 = 204,
	MX6Q_PAD_SD4_DAT6 = 205,
	MX6Q_PAD_SD4_DAT7 = 206,
	MX6Q_PAD_SD1_DAT1 = 207,
	MX6Q_PAD_SD1_DAT0 = 208,
	MX6Q_PAD_SD1_DAT3 = 209,
	MX6Q_PAD_SD1_CMD = 210,
	MX6Q_PAD_SD1_DAT2 = 211,
	MX6Q_PAD_SD1_CLK = 212,
	MX6Q_PAD_SD2_CLK = 213,
	MX6Q_PAD_SD2_CMD = 214,
	MX6Q_PAD_SD2_DAT3 = 215,
};

/* Pad names for the pinmux subsystem */
static const struct pinctrl_pin_desc imx6q_pinctrl_pads[] = {
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE0),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE1),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE2),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE3),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE4),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE5),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE6),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE7),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE8),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE9),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE10),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE11),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE12),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE13),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE14),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE15),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE16),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE17),
	IMX_PINCTRL_PIN(MX6Q_PAD_RESERVE18),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_DAT1),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_DAT2),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_DAT0),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TXC),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TD0),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TD1),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TD2),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TD3),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RX_CTL),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RD0),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_TX_CTL),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RD1),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RD2),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RD3),
	IMX_PINCTRL_PIN(MX6Q_PAD_RGMII_RXC),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A25),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_EB2),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D16),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D17),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D18),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D19),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D20),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D21),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D22),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D23),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_EB3),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D24),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D25),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D26),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D27),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D28),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D29),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D30),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_D31),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A24),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A23),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A22),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A21),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A20),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A19),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A18),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A17),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_A16),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_CS0),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_CS1),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_OE),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_RW),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_LBA),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_EB0),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_EB1),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA0),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA1),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA2),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA3),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA4),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA5),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA6),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA7),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA8),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA9),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA10),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA11),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA12),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA13),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA14),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_DA15),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_WAIT),
	IMX_PINCTRL_PIN(MX6Q_PAD_EIM_BCLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_DI0_DISP_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_DI0_PIN15),
	IMX_PINCTRL_PIN(MX6Q_PAD_DI0_PIN2),
	IMX_PINCTRL_PIN(MX6Q_PAD_DI0_PIN3),
	IMX_PINCTRL_PIN(MX6Q_PAD_DI0_PIN4),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT0),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT1),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT2),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT3),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT4),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT5),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT6),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT7),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT8),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT9),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT10),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT11),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT12),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT13),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT14),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT15),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT16),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT17),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT18),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT19),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT20),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT21),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT22),
	IMX_PINCTRL_PIN(MX6Q_PAD_DISP0_DAT23),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_MDIO),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_REF_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_RX_ER),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_CRS_DV),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_RXD1),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_RXD0),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_TX_EN),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_TXD1),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_TXD0),
	IMX_PINCTRL_PIN(MX6Q_PAD_ENET_MDC),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_COL0),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_ROW0),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_COL1),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_ROW1),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_COL2),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_ROW2),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_COL3),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_ROW3),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_COL4),
	IMX_PINCTRL_PIN(MX6Q_PAD_KEY_ROW4),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_0),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_1),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_9),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_3),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_6),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_2),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_4),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_5),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_7),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_8),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_16),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_17),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_18),
	IMX_PINCTRL_PIN(MX6Q_PAD_GPIO_19),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_PIXCLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_MCLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DATA_EN),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_VSYNC),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT4),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT5),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT6),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT7),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT8),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT9),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT10),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT11),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT12),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT13),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT14),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT15),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT16),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT17),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT18),
	IMX_PINCTRL_PIN(MX6Q_PAD_CSI0_DAT19),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT7),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT6),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT5),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT4),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_CMD),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT0),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT1),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT2),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_DAT3),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD3_RST),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_CLE),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_ALE),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_WP_B),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_RB0),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_CS0),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_CS1),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_CS2),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_CS3),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_CMD),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D0),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D1),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D2),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D3),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D4),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D5),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D6),
	IMX_PINCTRL_PIN(MX6Q_PAD_NANDF_D7),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT0),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT1),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT2),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT3),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT4),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT5),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT6),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD4_DAT7),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_DAT1),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_DAT0),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_DAT3),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_CMD),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_DAT2),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD1_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_CLK),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_CMD),
	IMX_PINCTRL_PIN(MX6Q_PAD_SD2_DAT3),
};

static struct imx_pinctrl_soc_info imx6q_pinctrl_info = {
	.pins = imx6q_pinctrl_pads,
	.npins = ARRAY_SIZE(imx6q_pinctrl_pads),
};

static const struct of_device_id imx6q_pinctrl_of_match[] = {
	{ .compatible = "fsl,imx6q-iomuxc", },
	{ /* sentinel */ }
};

static int imx6q_pinctrl_probe(struct platform_device *pdev)
{
	return imx_pinctrl_probe(pdev, &imx6q_pinctrl_info);
}

static struct platform_driver imx6q_pinctrl_driver = {
	.driver = {
		.name = "imx6q-pinctrl",
		.of_match_table = imx6q_pinctrl_of_match,
	},
	.probe = imx6q_pinctrl_probe,
	.remove = imx_pinctrl_remove,
};

static int __init imx6q_pinctrl_init(void)
{
	return platform_driver_register(&imx6q_pinctrl_driver);
}
arch_initcall(imx6q_pinctrl_init);

static void __exit imx6q_pinctrl_exit(void)
{
	platform_driver_unregister(&imx6q_pinctrl_driver);
}
module_exit(imx6q_pinctrl_exit);
MODULE_AUTHOR("Dong Aisheng <dong.aisheng@linaro.org>");
MODULE_DESCRIPTION("Freescale IMX6Q pinctrl driver");
MODULE_LICENSE("GPL v2");
