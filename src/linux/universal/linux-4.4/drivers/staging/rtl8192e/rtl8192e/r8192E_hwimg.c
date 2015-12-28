/******************************************************************************
 * Copyright(c) 2008 - 2010 Realtek Corporation. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
******************************************************************************/
/*Created on  2008/11/18,  3: 7*/

#include "r8192E_hwimg.h"

u32 Rtl8192PciEPHY_REGArray[PHY_REGArrayLengthPciE] = {0x0,};

u32 Rtl8192PciEPHY_REG_1T2RArray[PHY_REG_1T2RArrayLengthPciE] = {
	0x800, 0x00000000,
	0x804, 0x00000001,
	0x808, 0x0000fc00,
	0x80c, 0x0000001c,
	0x810, 0x801010aa,
	0x814, 0x008514d0,
	0x818, 0x00000040,
	0x81c, 0x00000000,
	0x820, 0x00000004,
	0x824, 0x00690000,
	0x828, 0x00000004,
	0x82c, 0x00e90000,
	0x830, 0x00000004,
	0x834, 0x00690000,
	0x838, 0x00000004,
	0x83c, 0x00e90000,
	0x840, 0x00000000,
	0x844, 0x00000000,
	0x848, 0x00000000,
	0x84c, 0x00000000,
	0x850, 0x00000000,
	0x854, 0x00000000,
	0x858, 0x65a965a9,
	0x85c, 0x65a965a9,
	0x860, 0x001f0010,
	0x864, 0x007f0010,
	0x868, 0x001f0010,
	0x86c, 0x007f0010,
	0x870, 0x0f100f70,
	0x874, 0x0f100f70,
	0x878, 0x00000000,
	0x87c, 0x00000000,
	0x880, 0x6870e36c,
	0x884, 0xe3573600,
	0x888, 0x4260c340,
	0x88c, 0x0000ff00,
	0x890, 0x00000000,
	0x894, 0xfffffffe,
	0x898, 0x4c42382f,
	0x89c, 0x00656056,
	0x8b0, 0x00000000,
	0x8e0, 0x00000000,
	0x8e4, 0x00000000,
	0x900, 0x00000000,
	0x904, 0x00000023,
	0x908, 0x00000000,
	0x90c, 0x31121311,
	0xa00, 0x00d0c7d8,
	0xa04, 0x811f0008,
	0xa08, 0x80cd8300,
	0xa0c, 0x2e62740f,
	0xa10, 0x95009b78,
	0xa14, 0x11145008,
	0xa18, 0x00881117,
	0xa1c, 0x89140fa0,
	0xa20, 0x1a1b0000,
	0xa24, 0x090e1317,
	0xa28, 0x00000204,
	0xa2c, 0x00000000,
	0xc00, 0x00000040,
	0xc04, 0x00005433,
	0xc08, 0x000000e4,
	0xc0c, 0x6c6c6c6c,
	0xc10, 0x08800000,
	0xc14, 0x40000100,
	0xc18, 0x08000000,
	0xc1c, 0x40000100,
	0xc20, 0x08000000,
	0xc24, 0x40000100,
	0xc28, 0x08000000,
	0xc2c, 0x40000100,
	0xc30, 0x6de9ac44,
	0xc34, 0x465c52cd,
	0xc38, 0x497f5994,
	0xc3c, 0x0a969764,
	0xc40, 0x1f7c403f,
	0xc44, 0x000100b7,
	0xc48, 0xec020000,
	0xc4c, 0x00000300,
	0xc50, 0x69543420,
	0xc54, 0x433c0094,
	0xc58, 0x69543420,
	0xc5c, 0x433c0094,
	0xc60, 0x69543420,
	0xc64, 0x433c0094,
	0xc68, 0x69543420,
	0xc6c, 0x433c0094,
	0xc70, 0x2c7f000d,
	0xc74, 0x0186175b,
	0xc78, 0x0000001f,
	0xc7c, 0x00b91612,
	0xc80, 0x40000100,
	0xc84, 0x20000000,
	0xc88, 0x40000100,
	0xc8c, 0x20200000,
	0xc90, 0x40000100,
	0xc94, 0x00000000,
	0xc98, 0x40000100,
	0xc9c, 0x00000000,
	0xca0, 0x00492492,
	0xca4, 0x00000000,
	0xca8, 0x00000000,
	0xcac, 0x00000000,
	0xcb0, 0x00000000,
	0xcb4, 0x00000000,
	0xcb8, 0x00000000,
	0xcbc, 0x00492492,
	0xcc0, 0x00000000,
	0xcc4, 0x00000000,
	0xcc8, 0x00000000,
	0xccc, 0x00000000,
	0xcd0, 0x00000000,
	0xcd4, 0x00000000,
	0xcd8, 0x64b22427,
	0xcdc, 0x00766932,
	0xce0, 0x00222222,
	0xd00, 0x00000750,
	0xd04, 0x00000403,
	0xd08, 0x0000907f,
	0xd0c, 0x00000001,
	0xd10, 0xa0633333,
	0xd14, 0x33333c63,
	0xd18, 0x6a8f5b6b,
	0xd1c, 0x00000000,
	0xd20, 0x00000000,
	0xd24, 0x00000000,
	0xd28, 0x00000000,
	0xd2c, 0xcc979975,
	0xd30, 0x00000000,
	0xd34, 0x00000000,
	0xd38, 0x00000000,
	0xd3c, 0x00027293,
	0xd40, 0x00000000,
	0xd44, 0x00000000,
	0xd48, 0x00000000,
	0xd4c, 0x00000000,
	0xd50, 0x6437140a,
	0xd54, 0x024dbd02,
	0xd58, 0x00000000,
	0xd5c, 0x04032064,
	0xe00, 0x161a1a1a,
	0xe04, 0x12121416,
	0xe08, 0x00001800,
	0xe0c, 0x00000000,
	0xe10, 0x161a1a1a,
	0xe14, 0x12121416,
	0xe18, 0x161a1a1a,
	0xe1c, 0x12121416,
};

u32 Rtl8192PciERadioA_Array[RadioA_ArrayLengthPciE] = {
	0x019, 0x00000003,
	0x000, 0x000000bf,
	0x001, 0x00000ee0,
	0x002, 0x0000004c,
	0x003, 0x000007f1,
	0x004, 0x00000975,
	0x005, 0x00000c58,
	0x006, 0x00000ae6,
	0x007, 0x000000ca,
	0x008, 0x00000e1c,
	0x009, 0x000007f0,
	0x00a, 0x000009d0,
	0x00b, 0x000001ba,
	0x00c, 0x00000240,
	0x00e, 0x00000020,
	0x00f, 0x00000990,
	0x012, 0x00000806,
	0x014, 0x000005ab,
	0x015, 0x00000f80,
	0x016, 0x00000020,
	0x017, 0x00000597,
	0x018, 0x0000050a,
	0x01a, 0x00000f80,
	0x01b, 0x00000f5e,
	0x01c, 0x00000008,
	0x01d, 0x00000607,
	0x01e, 0x000006cc,
	0x01f, 0x00000000,
	0x020, 0x000001a5,
	0x01f, 0x00000001,
	0x020, 0x00000165,
	0x01f, 0x00000002,
	0x020, 0x000000c6,
	0x01f, 0x00000003,
	0x020, 0x00000086,
	0x01f, 0x00000004,
	0x020, 0x00000046,
	0x01f, 0x00000005,
	0x020, 0x000001e6,
	0x01f, 0x00000006,
	0x020, 0x000001a6,
	0x01f, 0x00000007,
	0x020, 0x00000166,
	0x01f, 0x00000008,
	0x020, 0x000000c7,
	0x01f, 0x00000009,
	0x020, 0x00000087,
	0x01f, 0x0000000a,
	0x020, 0x000000f7,
	0x01f, 0x0000000b,
	0x020, 0x000000d7,
	0x01f, 0x0000000c,
	0x020, 0x000000b7,
	0x01f, 0x0000000d,
	0x020, 0x00000097,
	0x01f, 0x0000000e,
	0x020, 0x00000077,
	0x01f, 0x0000000f,
	0x020, 0x00000057,
	0x01f, 0x00000010,
	0x020, 0x00000037,
	0x01f, 0x00000011,
	0x020, 0x000000fb,
	0x01f, 0x00000012,
	0x020, 0x000000db,
	0x01f, 0x00000013,
	0x020, 0x000000bb,
	0x01f, 0x00000014,
	0x020, 0x000000ff,
	0x01f, 0x00000015,
	0x020, 0x000000e3,
	0x01f, 0x00000016,
	0x020, 0x000000c3,
	0x01f, 0x00000017,
	0x020, 0x000000a3,
	0x01f, 0x00000018,
	0x020, 0x00000083,
	0x01f, 0x00000019,
	0x020, 0x00000063,
	0x01f, 0x0000001a,
	0x020, 0x00000043,
	0x01f, 0x0000001b,
	0x020, 0x00000023,
	0x01f, 0x0000001c,
	0x020, 0x00000003,
	0x01f, 0x0000001d,
	0x020, 0x000001e3,
	0x01f, 0x0000001e,
	0x020, 0x000001c3,
	0x01f, 0x0000001f,
	0x020, 0x000001a3,
	0x01f, 0x00000020,
	0x020, 0x00000183,
	0x01f, 0x00000021,
	0x020, 0x00000163,
	0x01f, 0x00000022,
	0x020, 0x00000143,
	0x01f, 0x00000023,
	0x020, 0x00000123,
	0x01f, 0x00000024,
	0x020, 0x00000103,
	0x023, 0x00000203,
	0x024, 0x00000100,
	0x00b, 0x000001ba,
	0x02c, 0x000003d7,
	0x02d, 0x00000ff0,
	0x000, 0x00000037,
	0x004, 0x00000160,
	0x007, 0x00000080,
	0x002, 0x0000088d,
	0x0fe, 0x00000000,
	0x0fe, 0x00000000,
	0x016, 0x00000200,
	0x016, 0x00000380,
	0x016, 0x00000020,
	0x016, 0x000001a0,
	0x000, 0x000000bf,
	0x00d, 0x0000001f,
	0x00d, 0x00000c9f,
	0x002, 0x0000004d,
	0x000, 0x00000cbf,
	0x004, 0x00000975,
	0x007, 0x00000700,
};

u32 Rtl8192PciERadioB_Array[RadioB_ArrayLengthPciE] = {
	0x019, 0x00000003,
	0x000, 0x000000bf,
	0x001, 0x000006e0,
	0x002, 0x0000004c,
	0x003, 0x000007f1,
	0x004, 0x00000975,
	0x005, 0x00000c58,
	0x006, 0x00000ae6,
	0x007, 0x000000ca,
	0x008, 0x00000e1c,
	0x000, 0x000000b7,
	0x00a, 0x00000850,
	0x000, 0x000000bf,
	0x00b, 0x000001ba,
	0x00c, 0x00000240,
	0x00e, 0x00000020,
	0x015, 0x00000f80,
	0x016, 0x00000020,
	0x017, 0x00000597,
	0x018, 0x0000050a,
	0x01a, 0x00000e00,
	0x01b, 0x00000f5e,
	0x01d, 0x00000607,
	0x01e, 0x000006cc,
	0x00b, 0x000001ba,
	0x023, 0x00000203,
	0x024, 0x00000100,
	0x000, 0x00000037,
	0x004, 0x00000160,
	0x016, 0x00000200,
	0x016, 0x00000380,
	0x016, 0x00000020,
	0x016, 0x000001a0,
	0x00d, 0x00000ccc,
	0x000, 0x000000bf,
	0x002, 0x0000004d,
	0x000, 0x00000cbf,
	0x004, 0x00000975,
	0x007, 0x00000700,
};

u32 Rtl8192PciERadioC_Array[RadioC_ArrayLengthPciE] = {
	0x0,  };

u32 Rtl8192PciERadioD_Array[RadioD_ArrayLengthPciE] = {
	0x0, };

u32 Rtl8192PciEMACPHY_Array[] = {
	0x03c, 0xffff0000, 0x00000f0f,
	0x340, 0xffffffff, 0x161a1a1a,
	0x344, 0xffffffff, 0x12121416,
	0x348, 0x0000ffff, 0x00001818,
	0x12c, 0xffffffff, 0x04000802,
	0x318, 0x00000fff, 0x00000100,
};

u32 Rtl8192PciEMACPHY_Array_PG[] = {
	0x03c, 0xffff0000, 0x00000f0f,
	0xe00, 0xffffffff, 0x06090909,
	0xe04, 0xffffffff, 0x00030306,
	0xe08, 0x0000ff00, 0x00000000,
	0xe10, 0xffffffff, 0x0a0c0d0f,
	0xe14, 0xffffffff, 0x06070809,
	0xe18, 0xffffffff, 0x0a0c0d0f,
	0xe1c, 0xffffffff, 0x06070809,
	0x12c, 0xffffffff, 0x04000802,
	0x318, 0x00000fff, 0x00000800,
};

u32 Rtl8192PciEAGCTAB_Array[AGCTAB_ArrayLengthPciE] = {
	0xc78, 0x7d000001,
	0xc78, 0x7d010001,
	0xc78, 0x7d020001,
	0xc78, 0x7d030001,
	0xc78, 0x7d040001,
	0xc78, 0x7d050001,
	0xc78, 0x7c060001,
	0xc78, 0x7b070001,
	0xc78, 0x7a080001,
	0xc78, 0x79090001,
	0xc78, 0x780a0001,
	0xc78, 0x770b0001,
	0xc78, 0x760c0001,
	0xc78, 0x750d0001,
	0xc78, 0x740e0001,
	0xc78, 0x730f0001,
	0xc78, 0x72100001,
	0xc78, 0x71110001,
	0xc78, 0x70120001,
	0xc78, 0x6f130001,
	0xc78, 0x6e140001,
	0xc78, 0x6d150001,
	0xc78, 0x6c160001,
	0xc78, 0x6b170001,
	0xc78, 0x6a180001,
	0xc78, 0x69190001,
	0xc78, 0x681a0001,
	0xc78, 0x671b0001,
	0xc78, 0x661c0001,
	0xc78, 0x651d0001,
	0xc78, 0x641e0001,
	0xc78, 0x491f0001,
	0xc78, 0x48200001,
	0xc78, 0x47210001,
	0xc78, 0x46220001,
	0xc78, 0x45230001,
	0xc78, 0x44240001,
	0xc78, 0x43250001,
	0xc78, 0x28260001,
	0xc78, 0x27270001,
	0xc78, 0x26280001,
	0xc78, 0x25290001,
	0xc78, 0x242a0001,
	0xc78, 0x232b0001,
	0xc78, 0x222c0001,
	0xc78, 0x212d0001,
	0xc78, 0x202e0001,
	0xc78, 0x0a2f0001,
	0xc78, 0x08300001,
	0xc78, 0x06310001,
	0xc78, 0x05320001,
	0xc78, 0x04330001,
	0xc78, 0x03340001,
	0xc78, 0x02350001,
	0xc78, 0x01360001,
	0xc78, 0x00370001,
	0xc78, 0x00380001,
	0xc78, 0x00390001,
	0xc78, 0x003a0001,
	0xc78, 0x003b0001,
	0xc78, 0x003c0001,
	0xc78, 0x003d0001,
	0xc78, 0x003e0001,
	0xc78, 0x003f0001,
	0xc78, 0x7d400001,
	0xc78, 0x7d410001,
	0xc78, 0x7d420001,
	0xc78, 0x7d430001,
	0xc78, 0x7d440001,
	0xc78, 0x7d450001,
	0xc78, 0x7c460001,
	0xc78, 0x7b470001,
	0xc78, 0x7a480001,
	0xc78, 0x79490001,
	0xc78, 0x784a0001,
	0xc78, 0x774b0001,
	0xc78, 0x764c0001,
	0xc78, 0x754d0001,
	0xc78, 0x744e0001,
	0xc78, 0x734f0001,
	0xc78, 0x72500001,
	0xc78, 0x71510001,
	0xc78, 0x70520001,
	0xc78, 0x6f530001,
	0xc78, 0x6e540001,
	0xc78, 0x6d550001,
	0xc78, 0x6c560001,
	0xc78, 0x6b570001,
	0xc78, 0x6a580001,
	0xc78, 0x69590001,
	0xc78, 0x685a0001,
	0xc78, 0x675b0001,
	0xc78, 0x665c0001,
	0xc78, 0x655d0001,
	0xc78, 0x645e0001,
	0xc78, 0x495f0001,
	0xc78, 0x48600001,
	0xc78, 0x47610001,
	0xc78, 0x46620001,
	0xc78, 0x45630001,
	0xc78, 0x44640001,
	0xc78, 0x43650001,
	0xc78, 0x28660001,
	0xc78, 0x27670001,
	0xc78, 0x26680001,
	0xc78, 0x25690001,
	0xc78, 0x246a0001,
	0xc78, 0x236b0001,
	0xc78, 0x226c0001,
	0xc78, 0x216d0001,
	0xc78, 0x206e0001,
	0xc78, 0x0a6f0001,
	0xc78, 0x08700001,
	0xc78, 0x06710001,
	0xc78, 0x05720001,
	0xc78, 0x04730001,
	0xc78, 0x03740001,
	0xc78, 0x02750001,
	0xc78, 0x01760001,
	0xc78, 0x00770001,
	0xc78, 0x00780001,
	0xc78, 0x00790001,
	0xc78, 0x007a0001,
	0xc78, 0x007b0001,
	0xc78, 0x007c0001,
	0xc78, 0x007d0001,
	0xc78, 0x007e0001,
	0xc78, 0x007f0001,
	0xc78, 0x2e00001e,
	0xc78, 0x2e01001e,
	0xc78, 0x2e02001e,
	0xc78, 0x2e03001e,
	0xc78, 0x2e04001e,
	0xc78, 0x2e05001e,
	0xc78, 0x3006001e,
	0xc78, 0x3407001e,
	0xc78, 0x3908001e,
	0xc78, 0x3c09001e,
	0xc78, 0x3f0a001e,
	0xc78, 0x420b001e,
	0xc78, 0x440c001e,
	0xc78, 0x450d001e,
	0xc78, 0x460e001e,
	0xc78, 0x460f001e,
	0xc78, 0x4710001e,
	0xc78, 0x4811001e,
	0xc78, 0x4912001e,
	0xc78, 0x4a13001e,
	0xc78, 0x4b14001e,
	0xc78, 0x4b15001e,
	0xc78, 0x4c16001e,
	0xc78, 0x4d17001e,
	0xc78, 0x4e18001e,
	0xc78, 0x4f19001e,
	0xc78, 0x4f1a001e,
	0xc78, 0x501b001e,
	0xc78, 0x511c001e,
	0xc78, 0x521d001e,
	0xc78, 0x521e001e,
	0xc78, 0x531f001e,
	0xc78, 0x5320001e,
	0xc78, 0x5421001e,
	0xc78, 0x5522001e,
	0xc78, 0x5523001e,
	0xc78, 0x5624001e,
	0xc78, 0x5725001e,
	0xc78, 0x5726001e,
	0xc78, 0x5827001e,
	0xc78, 0x5828001e,
	0xc78, 0x5929001e,
	0xc78, 0x592a001e,
	0xc78, 0x5a2b001e,
	0xc78, 0x5b2c001e,
	0xc78, 0x5c2d001e,
	0xc78, 0x5c2e001e,
	0xc78, 0x5d2f001e,
	0xc78, 0x5e30001e,
	0xc78, 0x5f31001e,
	0xc78, 0x6032001e,
	0xc78, 0x6033001e,
	0xc78, 0x6134001e,
	0xc78, 0x6235001e,
	0xc78, 0x6336001e,
	0xc78, 0x6437001e,
	0xc78, 0x6438001e,
	0xc78, 0x6539001e,
	0xc78, 0x663a001e,
	0xc78, 0x673b001e,
	0xc78, 0x673c001e,
	0xc78, 0x683d001e,
	0xc78, 0x693e001e,
	0xc78, 0x6a3f001e,
};
