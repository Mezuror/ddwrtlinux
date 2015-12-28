/*
 * ths8200 - Texas Instruments THS8200 video encoder driver
 *
 * Copyright 2013 Cisco Systems, Inc. and/or its affiliates.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef THS8200_REGS_H
#define THS8200_REGS_H

/* Register offset macros */
#define THS8200_VERSION				0x02
#define THS8200_CHIP_CTL			0x03
#define THS8200_CSC_R11				0x04
#define THS8200_CSC_R12				0x05
#define THS8200_CSC_R21				0x06
#define THS8200_CSC_R22				0x07
#define THS8200_CSC_R31				0x08
#define THS8200_CSC_R32				0x09
#define THS8200_CSC_G11				0x0a
#define THS8200_CSC_G12				0x0b
#define THS8200_CSC_G21				0x0c
#define THS8200_CSC_G22				0x0d
#define THS8200_CSC_G31				0x0e
#define THS8200_CSC_G32				0x0f
#define THS8200_CSC_B11				0x10
#define THS8200_CSC_B12				0x11
#define THS8200_CSC_B21				0x12
#define THS8200_CSC_B22				0x13
#define THS8200_CSC_B31				0x14
#define THS8200_CSC_B32				0x15
#define THS8200_CSC_OFFS1			0x16
#define THS8200_CSC_OFFS12			0x17
#define THS8200_CSC_OFFS23			0x18
#define THS8200_CSC_OFFS3			0x19
#define THS8200_TST_CNTL1			0x1a
#define THS8200_TST_CNTL2			0x1b
#define THS8200_DATA_CNTL			0x1c
#define THS8200_DTG1_Y_SYNC1_LSB		0x1d
#define THS8200_DTG1_Y_SYNC2_LSB		0x1e
#define THS8200_DTG1_Y_SYNC3_LSB		0x1f
#define THS8200_DTG1_CBCR_SYNC1_LSB		0x20
#define THS8200_DTG1_CBCR_SYNC2_LSB		0x21
#define THS8200_DTG1_CBCR_SYNC3_LSB		0x22
#define THS8200_DTG1_Y_SYNC_MSB			0x23
#define THS8200_DTG1_CBCR_SYNC_MSB		0x24
#define THS8200_DTG1_SPEC_A			0x25
#define THS8200_DTG1_SPEC_B			0x26
#define THS8200_DTG1_SPEC_C			0x27
#define THS8200_DTG1_SPEC_D_LSB			0x28
#define THS8200_DTG1_SPEC_D1			0x29
#define THS8200_DTG1_SPEC_E_LSB			0x2a
#define THS8200_DTG1_SPEC_DEH_MSB		0x2b
#define THS8200_DTG1_SPEC_H_LSB			0x2c
#define THS8200_DTG1_SPEC_I_MSB			0x2d
#define THS8200_DTG1_SPEC_I_LSB			0x2e
#define THS8200_DTG1_SPEC_K_LSB			0x2f
#define THS8200_DTG1_SPEC_K_MSB			0x30
#define THS8200_DTG1_SPEC_K1			0x31
#define THS8200_DTG1_SPEC_G_LSB			0x32
#define THS8200_DTG1_SPEC_G_MSB			0x33
#define THS8200_DTG1_TOT_PIXELS_MSB		0x34
#define THS8200_DTG1_TOT_PIXELS_LSB		0x35
#define THS8200_DTG1_FLD_FLIP_LINECNT_MSB	0x36
#define THS8200_DTG1_LINECNT_LSB		0x37
#define THS8200_DTG1_MODE			0x38
#define THS8200_DTG1_FRAME_FIELD_SZ_MSB		0x39
#define THS8200_DTG1_FRAME_SZ_LSB		0x3a
#define THS8200_DTG1_FIELD_SZ_LSB		0x3b
#define THS8200_DTG1_VESA_CBAR_SIZE		0x3c
#define THS8200_DAC_CNTL_MSB			0x3d
#define THS8200_DAC1_CNTL_LSB			0x3e
#define THS8200_DAC2_CNTL_LSB			0x3f
#define THS8200_DAC3_CNTL_LSB			0x40
#define THS8200_CSM_CLIP_GY_LOW			0x41
#define THS8200_CSM_CLIP_BCB_LOW		0x42
#define THS8200_CSM_CLIP_RCR_LOW		0x43
#define THS8200_CSM_CLIP_GY_HIGH		0x44
#define THS8200_CSM_CLIP_BCB_HIGH		0x45
#define THS8200_CSM_CLIP_RCR_HIGH		0x46
#define THS8200_CSM_SHIFT_GY			0x47
#define THS8200_CSM_SHIFT_BCB			0x48
#define THS8200_CSM_SHIFT_RCR			0x49
#define THS8200_CSM_GY_CNTL_MULT_MSB		0x4a
#define THS8200_CSM_MULT_BCB_RCR_MSB		0x4b
#define THS8200_CSM_MULT_GY_LSB			0x4c
#define THS8200_CSM_MULT_BCB_LSB		0x4d
#define THS8200_CSM_MULT_RCR_LSB		0x4e
#define THS8200_CSM_MULT_RCR_BCB_CNTL		0x4f
#define THS8200_CSM_MULT_RCR_LSB		0x4e
#define THS8200_DTG2_BP1_2_MSB			0x50
#define THS8200_DTG2_BP3_4_MSB			0x51
#define THS8200_DTG2_BP5_6_MSB			0x52
#define THS8200_DTG2_BP7_8_MSB			0x53
#define THS8200_DTG2_BP9_10_MSB			0x54
#define THS8200_DTG2_BP11_12_MSB		0x55
#define THS8200_DTG2_BP13_14_MSB		0x56
#define THS8200_DTG2_BP15_16_MSB		0x57
#define THS8200_DTG2_BP1_LSB			0x58
#define THS8200_DTG2_BP2_LSB			0x59
#define THS8200_DTG2_BP3_LSB			0x5a
#define THS8200_DTG2_BP4_LSB			0x5b
#define THS8200_DTG2_BP5_LSB			0x5c
#define THS8200_DTG2_BP6_LSB			0x5d
#define THS8200_DTG2_BP7_LSB			0x5e
#define THS8200_DTG2_BP8_LSB			0x5f
#define THS8200_DTG2_BP9_LSB			0x60
#define THS8200_DTG2_BP10_LSB			0x61
#define THS8200_DTG2_BP11_LSB			0x62
#define THS8200_DTG2_BP12_LSB			0x63
#define THS8200_DTG2_BP13_LSB			0x64
#define THS8200_DTG2_BP14_LSB			0x65
#define THS8200_DTG2_BP15_LSB			0x66
#define THS8200_DTG2_BP16_LSB			0x67
#define THS8200_DTG2_LINETYPE1			0x68
#define THS8200_DTG2_LINETYPE2			0x69
#define THS8200_DTG2_LINETYPE3			0x6a
#define THS8200_DTG2_LINETYPE4			0x6b
#define THS8200_DTG2_LINETYPE5			0x6c
#define THS8200_DTG2_LINETYPE6			0x6d
#define THS8200_DTG2_LINETYPE7			0x6e
#define THS8200_DTG2_LINETYPE8			0x6f
#define THS8200_DTG2_HLENGTH_LSB		0x70
#define THS8200_DTG2_HLENGTH_LSB_HDLY_MSB	0x71
#define THS8200_DTG2_HLENGTH_HDLY_LSB		0x72
#define THS8200_DTG2_VLENGTH1_LSB		0x73
#define THS8200_DTG2_VLENGTH1_MSB_VDLY1_MSB	0x74
#define THS8200_DTG2_VDLY1_LSB			0x75
#define THS8200_DTG2_VLENGTH2_LSB		0x76
#define THS8200_DTG2_VLENGTH2_MSB_VDLY2_MSB	0x77
#define THS8200_DTG2_VDLY2_LSB			0x78
#define THS8200_DTG2_HS_IN_DLY_MSB		0x79
#define THS8200_DTG2_HS_IN_DLY_LSB		0x7a
#define THS8200_DTG2_VS_IN_DLY_MSB		0x7b
#define THS8200_DTG2_VS_IN_DLY_LSB		0x7c
#define THS8200_DTG2_PIXEL_CNT_MSB		0x7d
#define THS8200_DTG2_PIXEL_CNT_LSB		0x7e
#define THS8200_DTG2_LINE_CNT_MSB		0x7f
#define THS8200_DTG2_LINE_CNT_LSB		0x80
#define THS8200_DTG2_CNTL			0x82
#define THS8200_CGMS_CNTL_HEADER		0x83
#define THS8200_CGMS_PAYLOAD_MSB		0x84
#define THS8200_CGMS_PAYLOAD_LSB		0x85
#define THS8200_MISC_PPL_LSB			0x86
#define THS8200_MISC_PPL_MSB			0x87
#define THS8200_MISC_LPF_MSB			0x88
#define THS8200_MISC_LPF_LSB			0x89

#endif /* THS8200_REGS_H */
