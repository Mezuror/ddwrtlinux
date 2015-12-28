/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ee_efuse.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RTMP_EFUSE_SUPPORT

#include	"rt_config.h"

/* eFuse registers */
#define EFUSE_CTRL				0x0580
#define EFUSE_DATA0				0x0590
#define EFUSE_DATA1				0x0594
#define EFUSE_DATA2				0x0598
#define EFUSE_DATA3				0x059c

#define EFUSE_CTRL_3290			0x24
#define EFUSE_DATA0_3290		0x28
#define EFUSE_DATA1_3290		0x2c
#define EFUSE_DATA2_3290		0x30
#define EFUSE_DATA3_3290		0x34


#ifdef RT_BIG_ENDIAN
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            SEL_EFUSE:1;
		UINT32            EFSROM_KICK:1;
		UINT32            RESERVED:4;
		UINT32            EFSROM_AIN:10;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_AOUT:6;   
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#else
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            EFSROM_AOUT:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_AIN:10;
		UINT32            RESERVED:4;
		UINT32            EFSROM_KICK:1;
		UINT32            SEL_EFUSE:1;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

UCHAR eFuseReadRegisters(
	IN PRTMP_ADAPTER	pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	OUT	USHORT			*pData);

VOID eFuseReadPhysical( 
	IN PRTMP_ADAPTER	pAd, 
  	IN PUSHORT 			lpInBuffer,
  	IN ULONG 			nInBufferSize,
  	OUT	PUSHORT 		lpOutBuffer,
  	IN ULONG 			nOutBufferSize);

static VOID eFusePhysicalWriteRegisters(
	IN PRTMP_ADAPTER	pAd,	
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	OUT	USHORT			*pData);

static NTSTATUS eFuseWriteRegisters(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	IN USHORT			*pData);

static VOID eFuseWritePhysical( 
	IN PRTMP_ADAPTER	pAd,	
  	INOUT PUSHORT 		lpInBuffer,
	IN ULONG 			nInBufferSize,
  	INOUT PUCHAR 		lpOutBuffer,
  	IN ULONG 			nOutBufferSize);

NTSTATUS eFuseWriteRegistersFromBin(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	IN USHORT			*pData);

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	Note:
	
========================================================================
*/
UCHAR eFuseReadRegisters(
	IN PRTMP_ADAPTER	pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	OUT	USHORT			*pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data;
	UINT32 efuse_ctrl_reg = EFUSE_CTRL;
	
#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		efuse_ctrl_reg = EFUSE_CTRL_3290;
#endif /* defined(RT3290) || defined(RT65xx) */
	
	RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	/*Use the eeprom logical address and covert to address to block number*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 0.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 0;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;
	
	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, efuse_ctrl_reg, data);

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{	
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return 0;
	
		/*rtmp.HwMemoryReadDword(EFUSE_CTRL, (DWORD *) &eFuseCtrlStruc, 4);*/
		RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RTMPusecDelay(2);
			break;
		}	
		RTMPusecDelay(2);
		i++;	
	}

	/*if EFSROM_AOUT is not found in physical address, write 0xffff*/
	if (eFuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
	{
		for(i=0; i<Length/2; i++)
			*(pData+2*i) = 0xffff;
	}
	else
	{
		/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x590-0x59C)*/
#if defined(RT3290) || defined(RT65xx)
		if (IS_RT3290(pAd) || IS_RT65XX(pAd))
			efuseDataOffset =  EFUSE_DATA0_3290 + (Offset & 0xC);
		else
#endif /* defined(RT3290) || defined(RT65xx) */
		efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC);	
		/*data hold 4 bytes data.*/
		/*In RTMP_IO_READ32 will automatically execute 32-bytes swapping*/
		RTMP_IO_READ32(pAd, efuseDataOffset, &data);
		/*Decide the upper 2 bytes or the bottom 2 bytes.*/
		/* Little-endian		S	|	S	Big-endian*/
		/* addr	3	2	1	0	|	0	1	2	3*/
		/* Ori-V	D	C	B	A	|	A	B	C	D*/
		/*After swapping*/
		/*		D	C	B	A	|	D	C	B	A*/
		/*Return 2-bytes*/
		/*The return byte statrs from S. Therefore, the little-endian will return BA, the Big-endian will return DC.*/
		/*For returning the bottom 2 bytes, the Big-endian should shift right 2-bytes.*/
#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));		  
#else
		data = data >> (8*(Offset & 0x3));		
#endif /* RT_BIG_ENDIAN */
		
		NdisMoveMemory(pData, &data, Length);
	}

	return (UCHAR) eFuseCtrlStruc.field.EFSROM_AOUT;
	
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	Note:
	
========================================================================
*/
VOID eFusePhysicalReadRegisters( 
	IN PRTMP_ADAPTER	pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	OUT	USHORT			*pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data;
	UINT32 efuse_ctrl_reg = EFUSE_CTRL;

#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		efuse_ctrl_reg = EFUSE_CTRL_3290;
#endif /* defined(RT3290) || defined(RT65xx) */

	RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
	/*Read in physical view*/
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);	
	RTMP_IO_WRITE32(pAd, efuse_ctrl_reg, data);	

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{	
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;
		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);	
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RTMPusecDelay(2);
			break;
		}
		
		RTMPusecDelay(2);
		i++;	
	}

	/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
	/*Because the size of each EFUSE_DATA is 4 Bytes, the size of address of each is 2 bits.*/
	/*The previous 2 bits is the EFUSE_DATA number, the last 2 bits is used to decide which bytes*/
	/*Decide which EFUSE_DATA to read*/
	/*590:F E D C */
	/*594:B A 9 8 */
	/*598:7 6 5 4*/
	/*59C:3 2 1 0*/
#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		efuseDataOffset =  EFUSE_DATA0_3290 + (Offset & 0xC)  ;
	else
#endif /* defined(RT3290) || defined(RT65xx) */
	efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;	

	RTMP_IO_READ32(pAd, efuseDataOffset, &data);

#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));	
#else
	data = data >> (8*(Offset & 0x3));
#endif /* RT_BIG_ENDIAN */

	NdisMoveMemory(pData, &data, Length);	
	
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
VOID eFuseReadPhysical( 
	IN PRTMP_ADAPTER	pAd, 
  	IN PUSHORT 			lpInBuffer,
  	IN ULONG 			nInBufferSize,
  	OUT	PUSHORT 		lpOutBuffer,
  	IN ULONG 			nOutBufferSize)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	USHORT* pOutBuf = (USHORT*)lpOutBuffer;

	USHORT Offset = pInBuf[0];					/*addr*/
	USHORT Length = pInBuf[1];					/*length*/
	int 		i;
	
	for(i=0; i<Length; i+=2)
	{
		eFusePhysicalReadRegisters(pAd,Offset+i, 2, &pOutBuf[i/2]);	
	} 	
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
NTSTATUS eFuseRead(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	OUT	PUSHORT			pData,
	IN USHORT			Length)
{
	NTSTATUS Status = STATUS_SUCCESS;
	UCHAR	EFSROM_AOUT;
	int	i;
	
	for(i=0; i<Length; i+=2)
	{
		EFSROM_AOUT = eFuseReadRegisters(pAd, Offset+i, 2, &pData[i/2]);
	} 
	return Status;
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
static VOID eFusePhysicalWriteRegisters(
	IN PRTMP_ADAPTER	pAd,	
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	OUT	USHORT			*pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data, eFuseDataBuffer[4];
	UINT32 efuse_ctrl_reg = EFUSE_CTRL;

#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		efuse_ctrl_reg = EFUSE_CTRL_3290;
#endif /* defined(RT3290) || defined(RT65xx) */

	/*Step0. Write 16-byte of data to EFUSE_DATA0-3 (0x590-0x59C), where EFUSE_DATA0 is the LSB DW, EFUSE_DATA3 is the MSB DW.*/

	/*read current values of 16-byte block	*/
	RTMP_IO_READ32(pAd, efuse_ctrl_reg,  &eFuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, efuse_ctrl_reg, data);	

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{	
		RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RTMPusecDelay(2);
			break;
		}
		RTMPusecDelay(2);
		i++;	
	}

	/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		efuseDataOffset = EFUSE_DATA0_3290;
	else
#endif /* defined(RT3290) || defined(RT65xx) */
		efuseDataOffset =  EFUSE_DATA3;
	for(i=0; i< 4; i++)
	{
		RTMP_IO_READ32(pAd, efuseDataOffset, (PUINT32) &eFuseDataBuffer[i]);
#if defined(RT3290) || defined(RT65xx)
		if (IS_RT3290(pAd) || IS_RT65XX(pAd))
			efuseDataOffset += 4;
		else
#endif /* defined(RT3290) || defined(RT65xx) */
			efuseDataOffset -= 4;
	}

	/*Update the value, the offset is multiple of 2, length is 2*/
	efuseDataOffset = (Offset & 0xc) >> 2;
	data = pData[0] & 0xffff;
	/*The offset should be 0x***10 or 0x***00*/
	if((Offset % 4) != 0)
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff) | (data << 16);
	}
	else
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff0000) | data;
	}

	efuseDataOffset =  EFUSE_DATA3;
	for(i=0; i< 4; i++)
	{
		RTMP_IO_WRITE32(pAd, efuseDataOffset, eFuseDataBuffer[i]);
#if defined(RT3290) || defined(RT65xx)
		if (IS_RT3290(pAd) || IS_RT65XX(pAd))
			efuseDataOffset += 4;
		else
#endif /* defined(RT3290) || defined(RT65xx) */
			efuseDataOffset -= 4;		
	}

	/*Step1. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	// TODO: shiang, for below line, windows driver didn't have this read, why we have ??
	RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step2. Write EFSROM_MODE (0x580, bit7:bit6) to 3.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 3;
	
	/*Step3. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical write procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);	
	RTMP_IO_WRITE32(pAd, efuse_ctrl_reg, data);	

	/*Step4. Polling EFSROM_KICK(0x580, bit30) until it become 0 again. It��s done.*/
	i = 0;

	while(i < 500)
	{	
		RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RTMPusecDelay(2);
			break;
		}
		
		RTMPusecDelay(2);	
		i++;	
	}
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	Note:
	
========================================================================
*/
static NTSTATUS eFuseWriteRegisters(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	IN USHORT			*pData)
{
	USHORT	i,Loop=0, StartBlock=0, EndBlock=0;
	USHORT	eFuseData;
	USHORT	LogicalAddress, BlkNum = 0xffff;
	UCHAR	EFSROM_AOUT;

	USHORT addr,tmpaddr, InBuf[3], tmpOffset;
	USHORT buffer[8];
	BOOLEAN		bWriteSuccess = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters Offset=%x, pData=%x\n", Offset, *pData));
	/*set start block and end block number, start from tail of mapping table*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1; 
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END; 
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1; 
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START; 
	}
	/*Step 0. find the entry in the mapping table*/
	/*The address of EEPROM is 2-bytes alignment.*/
	/*The last bit is used for alignment, so it must be 0.*/
	tmpOffset = Offset & 0xfffe;
	EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData);
	if( EFSROM_AOUT == 0x3f)
	{	/*find available logical address pointer	*/
		/*the logical address does not exist, find an empty one*/
		/*from the first address of block 45=16*45=0x2d0 to the last address of block 47*/
		/*==>48*16-3(reserved)=2FC*/
		for (i=StartBlock; i >= EndBlock; i-=2)
		{
			/*Retrive the logical block nubmer form each logical address pointer*/
			/*It will access two logical address pointer each time.*/
			eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
			/*To avoid the odd byte problem, ex. We read the 21|20 bytes and if 21 is the */
			/* end byte. Then, the EFUSE_USAGE_MAP_END which is 21 is not equal to*/
			/* i which is 20. Therefore, this 21th byte could be used.*/
			/*Otherwise, if 20 is the stop byte, i which is 20 is equal EFUSE_USAGE_MAP_END.*/
			/* It means the 21th byte could not be used.*/
			if(( (LogicalAddress >> 8) & 0xff) == 0)
			{/*Not used logical address pointer*/
				if (i != pAd->chipCap.EFUSE_USAGE_MAP_END)
				{		
					BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START+1;	
					break;
				}				
				
			}
			
			if( (LogicalAddress & 0xff) == 0)
			{/*Not used logical address pointer*/
				if (i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
				{
					BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
					break;
				}
			}
			
		}
	}
	else
	{
		BlkNum = EFSROM_AOUT;
	}	

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

	if(BlkNum == 0xffff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
		return FALSE;
	}	

	/*Step 1. Save data of this block	which is pointed by the avaible logical address pointer*/
	/* read and save the original block data*/
	for(i =0; i<8; i++)
	{
		addr = BlkNum * 0x10 ;
		
		InBuf[0] = addr+2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;	
		
		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

		buffer[i] = InBuf[2];
	}

	/*Step 2. Update the data in buffer, and write the data to Efuse*/
	buffer[ (Offset >> 1) % 8] = pData[0];

	do
	{	Loop++;
		/*Step 3. Write the data to Efuse*/
		if(!bWriteSuccess)
		{
			for(i =0; i<8; i++)
			{
				addr = BlkNum * 0x10 ;
				
				InBuf[0] = addr+2*i;
				InBuf[1] = 2;
				InBuf[2] = buffer[i];	
				
				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);		
			}
		}
		else
		{
				addr = BlkNum * 0x10 ;
				
				InBuf[0] = addr+(Offset % 16);
				InBuf[1] = 2;
				InBuf[2] = pData[0];	
				
				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);	
		}
	
		/*Step 4. Write mapping table*/
		addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;

		tmpaddr = addr;

		if(addr % 2 != 0)
			addr = addr -1; 
		InBuf[0] = addr;
		InBuf[1] = 2;

		/*convert the address from 10 to 8 bit ( bit7, 6 = parity and bit5 ~ 0 = bit9~4), and write to logical map entry*/
		tmpOffset = Offset;
		tmpOffset >>= 4;
		tmpOffset |= ((~((tmpOffset & 0x01) ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
		tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

		/* write the logical address*/
		if(tmpaddr%2 != 0) 	
			InBuf[2] = tmpOffset<<8;	
		else          
			InBuf[2] = tmpOffset;

		eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

		/*Step 5. Compare data if not the same, invalidate the mapping entry, then re-write the data until E-fuse is exhausted*/
		bWriteSuccess = TRUE;
		for(i =0; i<8; i++)
		{
			addr = BlkNum * 0x10 ;
			
			InBuf[0] = addr+2*i;
			InBuf[1] = 2;
			InBuf[2] = 0x0;	
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

			if(buffer[i] != InBuf[2])
			{
				bWriteSuccess = FALSE;
				break;
			}	
		}

		/*Step 6. invlidate mapping entry and find a free mapping entry if not succeed*/
		if (!bWriteSuccess)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess BlkNum = %d\n", BlkNum));

			/* the offset of current mapping entry*/
			addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;			

			/*find a new mapping entry*/
			BlkNum = 0xffff;
			for (i=StartBlock; i >= EndBlock; i-=2)
			{
				eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				if(( (LogicalAddress >> 8) & 0xff) == 0)
				{
					if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
					{
						BlkNum = i+1-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}	
				}

				if( (LogicalAddress & 0xff) == 0)
				{
					if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
					{
						BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess and allocate new BlkNum = %d\n", BlkNum));	
			if(BlkNum == 0xffff)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
				return FALSE;
			}

			/*invalidate the original mapping entry if new entry is not found*/
			tmpaddr = addr;

			if(addr % 2 != 0)
				addr = addr -1; 
			InBuf[0] = addr;
			InBuf[1] = 2;		
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);				

			/* write the logical address*/
			if(tmpaddr%2 != 0) 
			{
				/* Invalidate the high byte*/
				for (i=8; i<15; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}		
			}	
			else
			{
				/* invalidate the low byte*/
				for (i=0; i<8; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}					
			}
			eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0);	
		}	
	}	
	while (!bWriteSuccess&&Loop<2);	
	if(!bWriteSuccess)
		DBGPRINT(RT_DEBUG_ERROR,("Efsue Write Failed!!\n"));
	return TRUE;
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
static VOID eFuseWritePhysical( 
	IN PRTMP_ADAPTER	pAd,	
  	INOUT PUSHORT 		lpInBuffer,
	IN ULONG 			nInBufferSize,
  	INOUT PUCHAR 		lpOutBuffer,
  	IN ULONG 			nOutBufferSize)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	int 		i;
	/*USHORT* pOutBuf = (USHORT*)ioBuffer;*/
	USHORT Offset = pInBuf[0];					/* addr*/
	USHORT Length = pInBuf[1];					/* length*/
	USHORT* pValueX = &pInBuf[2];				/* value ...		*/

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWritePhysical Offset=0x%x, length=%d\n", Offset, Length));

	{
		/* Little-endian		S	|	S	Big-endian*/
		/* addr	3	2	1	0	|	0	1	2	3*/
		/* Ori-V	D	C	B	A	|	A	B	C	D*/
		/* After swapping*/
		/*		D	C	B	A	|	D	C	B	A*/
		/* Both the little and big-endian use the same sequence to write  data.*/
		/* Therefore, we only need swap data when read the data.*/
		for (i=0; i<Length; i+=2)
		{
			eFusePhysicalWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);	
		}	
	}
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
NTSTATUS eFuseWrite(  
   	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUSHORT			pData,
	IN	USHORT			length)
{
	int i;
	USHORT* pValueX = (PUSHORT) pData;				/*value ...		*/
	PUSHORT OddWriteByteBuf;
/*	OddWriteByteBuf=(PUSHORT)kmalloc(sizeof(USHORT)*2, MEM_ALLOC_FLAG);*/
	os_alloc_mem(NULL, (UCHAR **)&OddWriteByteBuf, sizeof(USHORT)*2);
	/* The input value=3070 will be stored as following*/
	/* Little-endian		S	|	S	Big-endian*/
	/* addr			1	0	|	0	1	*/
	/* Ori-V			30	70	|	30	70	*/
	/* After swapping*/
	/*				30	70	|	70	30*/
	/* Casting*/
	/*				3070	|	7030 (x)*/
	/* The swapping should be removed for big-endian*/
	if (OddWriteByteBuf == NULL)
		return FALSE;
	if((Offset%2)!=0)
	{
		length+=2;
		Offset-=1;
		eFuseRead(pAd,Offset,OddWriteByteBuf,2);
		eFuseRead(pAd,Offset+2,(OddWriteByteBuf+1),2);
		*OddWriteByteBuf&=0x00ff;
		*OddWriteByteBuf|=((*pData)&0xff)<<8;
		*(OddWriteByteBuf+1)&=0xff00;
		*(OddWriteByteBuf+1)|=(*pData&0xff00)>>8;
		pValueX=OddWriteByteBuf;
		
	}
	
	for(i=0; i<length; i+=2)
	{
		eFuseWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);	
	}
/*	kfree(OddWriteByteBuf);*/
	os_free_mem(NULL, OddWriteByteBuf);
	return TRUE;
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
INT set_eFuseGetFreeBlockCount_Proc(  
   	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT free_num = 0;
	
	if (pAd->bUseEfuse == FALSE)
		return FALSE;
	
	eFuseGetFreeBlockCount(pAd, &free_num);
	DBGPRINT(RT_DEBUG_OFF, ("efuseFreeNumber = %d\n", free_num));
	return TRUE;
}


INT set_eFusedump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	USHORT InBuf[3];
	INT i = 0;
	
	if (pAd->bUseEfuse == FALSE)
		return FALSE;

	for (i = 0; i < pAd->chipCap.EFUSE_USAGE_MAP_END/2; i++)
	{
		InBuf[0] = 2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;	

		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);
		
		if (i%4 == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\nBlock %x:", i/8));
		DBGPRINT(RT_DEBUG_OFF, ("%04x ", InBuf[2]));
	}
	return TRUE;
}


INT	set_eFuseLoadFromBin_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PSTRING					src;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osfsInfo;
	INT 						retval, memSize;
	PSTRING					buffer, memPtr;
	INT						TotalByte= 0,ReadedByte=0,CompareBuf=1;
	USHORT					*PDATA;
	USHORT					DATA;
	
	memSize = 128 + MAX_EEPROM_BIN_FILE_SIZE + sizeof(USHORT) * 8;
/*	memPtr = kmalloc(memSize, MEM_ALLOC_FLAG);*/
	os_alloc_mem(NULL, (UCHAR **)&memPtr, memSize);
	if (memPtr == NULL)
		return FALSE;

	NdisZeroMemory(memPtr, memSize);
	src = memPtr; /* kmalloc(128, MEM_ALLOC_FLAG);*/
	buffer = src + 128;		/* kmalloc(MAX_EEPROM_BIN_FILE_SIZE, MEM_ALLOC_FLAG);*/
	PDATA = (USHORT*)(buffer + MAX_EEPROM_BIN_FILE_SIZE);	/* kmalloc(sizeof(USHORT)*8,MEM_ALLOC_FLAG);*/
	
 	if(strlen(arg)>0)
		NdisMoveMemory(src, arg, strlen(arg));
	else
		NdisMoveMemory(src, EEPROM_DEFULT_BIN_FILE, strlen(BIN_FILE_PATH));
	DBGPRINT(RT_DEBUG_OFF, ("FileName=%s\n",src));

	RtmpOSFSInfoChange(&osfsInfo, TRUE);

	srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(srcf)) 
	{
		DBGPRINT_ERR(("--> Error opening file %s\n", src));
		retval = FALSE;
		goto recoverFS;
	}
	else 
	{
		/* The object must have a read method*/
		while(RtmpOSFileRead(srcf, &buffer[TotalByte], 1)==1)
		{
          		TotalByte++;
			if(TotalByte>MAX_EEPROM_BIN_FILE_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error reading file %s, file size too large[>%d]\n", src, MAX_EEPROM_BIN_FILE_SIZE));
				retval = FALSE;
				goto closeFile;
			}
		}

		retval = RtmpOSFileClose(srcf);
		if (retval)
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error closing file %s\n", src));
  	}

	
	RtmpOSFSInfoChange(&osfsInfo, FALSE);

	for(ReadedByte=0;ReadedByte<TotalByte;ReadedByte++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%02X ",buffer[ReadedByte]&0xff));
		if((ReadedByte+1)%2==0)
			PDATA[ReadedByte/2%8]=((buffer[ReadedByte]<<8)&0xff00)|(buffer[ReadedByte-1]&0xff);
		if(ReadedByte%16==0)
		{
			CompareBuf=buffer[ReadedByte]&0xff;
		
		}
		else
		{
			CompareBuf&=(buffer[ReadedByte]&0xff);
			if((ReadedByte+1)%16==0)
			{
				DBGPRINT(RT_DEBUG_TRACE, (" result=%02X,blk=%02x\n",CompareBuf,ReadedByte/16));

				if(CompareBuf!=0xff)
					eFuseWriteRegistersFromBin(pAd,(USHORT)ReadedByte-15, 16, PDATA);
				else
				{
					if(eFuseReadRegisters(pAd,ReadedByte, 2,(PUSHORT)&DATA)!=0x3f)
						eFuseWriteRegistersFromBin(pAd,(USHORT)ReadedByte-15, 16, PDATA);
				}
				/*
				for(l=0;l<8;l++)
					printk("%04x ",PDATA[l]);
				printk("\n");
				*/
				NdisZeroMemory(PDATA,16);
			}
		}
	}
	
	return TRUE;

closeFile:
	if (srcf)
		if (RtmpOSFileClose(srcf) != 0)
			retval = FALSE;

recoverFS:
	RtmpOSFSInfoChange(&osfsInfo, FALSE);
	

	if (memPtr)
/*		kfree(memPtr);*/
		os_free_mem(NULL, memPtr);
	
	return retval;
}


NTSTATUS eFuseWriteRegistersFromBin(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT 			Offset, 
	IN USHORT 			Length, 
	IN USHORT			*pData)
{
	USHORT	i,StartBlock=0,EndBlock=0;
	USHORT	eFuseData;
	USHORT	LogicalAddress, BlkNum = 0xffff;
	UCHAR	EFSROM_AOUT,Loop=0;
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	USHORT	efuseDataOffset;
	UINT32	data,tempbuffer;
	USHORT addr,tmpaddr, InBuf[3], tmpOffset;
	UINT32 buffer[4];
	BOOLEAN		bWriteSuccess = TRUE;
	BOOLEAN		bNotWrite=TRUE;
	BOOLEAN		bAllocateNewBlk=TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin Offset=%x, pData=%04x:%04x:%04x:%04x\n", Offset, *pData,*(pData+1),*(pData+2),*(pData+3)));
	/*set start block and end block number, start from tail of mapping table*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1; 
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END; 
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1; 
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START; 
	}
	
	do
	{
	/*Step 0. find the entry in the mapping table*/
	/*The address of EEPROM is 2-bytes alignment.*/
	/*The last bit is used for alignment, so it must be 0.*/
	Loop++;
	tmpOffset = Offset & 0xfffe;
	EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData);
	
	if( EFSROM_AOUT == 0x3f)
	{	/*find available logical address pointer	*/
		/*the logical address does not exist, find an empty one*/
		/*from the first address of block 45=16*45=0x2d0 to the last address of block 47*/
		/*==>48*16-3(reserved)=2FC*/
		bAllocateNewBlk=TRUE;
		for (i=StartBlock; i>=EndBlock; i-=2)
		{
			/*Retrive the logical block nubmer form each logical address pointer*/
			/*It will access two logical address pointer each time.*/
			eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
			if(( (LogicalAddress >> 8) & 0xff) == 0)
			{
				if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
				{
					BlkNum = i+1-pAd->chipCap.EFUSE_USAGE_MAP_START;
					break;
				}	
			}

			if( (LogicalAddress & 0xff) == 0)
			{
				if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
				{
					BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
					break;
				}
			}
			
		}
	}
	else
	{
		bAllocateNewBlk=FALSE;
		BlkNum = EFSROM_AOUT;
	}	

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

	if(BlkNum == 0xffff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
		return FALSE;
	}	
	/*Step 1.1.0*/
	/*If the block is not existing in mapping table, create one */
	/*and write down the 16-bytes data to the new block*/
	if(bAllocateNewBlk)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate New Blk\n"));
		efuseDataOffset =  EFUSE_DATA3;
		for(i=0; i< 4; i++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Allocate New Blk, Data%d=%04x%04x\n",3-i,pData[2*i+1],pData[2*i]));
			tempbuffer=((pData[2*i+1]<<16)&0xffff0000)|pData[2*i];


			RTMP_IO_WRITE32(pAd, efuseDataOffset,tempbuffer);			
			efuseDataOffset -= 4;		

		}

		/*Step1.1.1. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);
		eFuseCtrlStruc.field.EFSROM_AIN = BlkNum* 0x10 ;

		/*Step1.1.2. Write EFSROM_MODE (0x580, bit7:bit6) to 3.*/
		eFuseCtrlStruc.field.EFSROM_MODE = 3;
		
		/*Step1.1.3. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical write procedure.*/
		eFuseCtrlStruc.field.EFSROM_KICK = 1;

		NdisMoveMemory(&data, &eFuseCtrlStruc, 4);	
		
		RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);	

		/*Step1.1.4. Polling EFSROM_KICK(0x580, bit30) until it become 0 again. It��s done.*/
		i = 0;
		while(i < 100)
		{	
			RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

			if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
				break;
			
			RTMPusecDelay(2);	
			i++;	
		}
		
	}
	else
	{	/*Step1.2.*/
		/*If the same logical number is existing, check if the writting data and the data */
		/*saving in this block are the same.*/
		/*read current values of 16-byte block	*/
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

		/*Step1.2.0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
		eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

		/*Step1.2.1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
		eFuseCtrlStruc.field.EFSROM_MODE = 0;

		/*Step1.2.2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
		eFuseCtrlStruc.field.EFSROM_KICK = 1;

		NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
		RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);	

		/*Step1.2.3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
		i = 0;
		while(i < 500)
		{	
			RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

			if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
				break;
			RTMPusecDelay(2);
			i++;	
		}

		/*Step1.2.4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
		efuseDataOffset =  EFUSE_DATA3;		
		for(i=0; i< 4; i++)
		{
			RTMP_IO_READ32(pAd, efuseDataOffset, (PUINT32) &buffer[i]);
			efuseDataOffset -=  4;		
		}
		/*Step1.2.5. Check if the data of efuse and the writing data are the same.*/
		for(i =0; i<4; i++)
		{
			tempbuffer=((pData[2*i+1]<<16)&0xffff0000)|pData[2*i];
			DBGPRINT(RT_DEBUG_TRACE, ("buffer[%d]=%x,pData[%d]=%x,pData[%d]=%x,tempbuffer=%x\n",i,buffer[i],2*i,pData[2*i],2*i+1,pData[2*i+1],tempbuffer));
			
			if(((buffer[i]&0xffff0000)==(pData[2*i+1]<<16))&&((buffer[i]&0xffff)==pData[2*i]))
				bNotWrite&=TRUE;
			else
			{
				bNotWrite&=FALSE;
				break;
			}
		}
		if(!bNotWrite)
		{
		printk("The data is not the same\n");
		
			for(i =0; i<8; i++)
			{
				addr = BlkNum * 0x10 ;
				
				InBuf[0] = addr+2*i;
				InBuf[1] = 2;
				InBuf[2] = pData[i];	
				
				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);		
			}
			
		}
		else
			return TRUE;
	     }
	


		/*Step 2. Write mapping table*/
		addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;

		tmpaddr = addr;

		if(addr % 2 != 0)
			addr = addr -1; 
		InBuf[0] = addr;
		InBuf[1] = 2;

		/*convert the address from 10 to 8 bit ( bit7, 6 = parity and bit5 ~ 0 = bit9~4), and write to logical map entry*/
		tmpOffset = Offset;
		tmpOffset >>= 4;
		tmpOffset |= ((~((tmpOffset & 0x01) ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
		tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

		/* write the logical address*/
		if(tmpaddr%2 != 0) 	
			InBuf[2] = tmpOffset<<8;	
		else          
			InBuf[2] = tmpOffset;

		eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

		/*Step 3. Compare data if not the same, invalidate the mapping entry, then re-write the data until E-fuse is exhausted*/
		bWriteSuccess = TRUE;
		for(i =0; i<8; i++)
		{
			addr = BlkNum * 0x10 ;
			
			InBuf[0] = addr+2*i;
			InBuf[1] = 2;
			InBuf[2] = 0x0;	
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);
			DBGPRINT(RT_DEBUG_TRACE, ("addr=%x, buffer[i]=%x,InBuf[2]=%x\n",InBuf[0],pData[i],InBuf[2]));
			if(pData[i] != InBuf[2])
			{
				bWriteSuccess = FALSE;
				break;
			}	
		}

		/*Step 4. invlidate mapping entry and find a free mapping entry if not succeed*/
		
		if (!bWriteSuccess&&Loop<2)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin::Not bWriteSuccess BlkNum = %d\n", BlkNum));
		
			/* the offset of current mapping entry*/
			addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;			

			/*find a new mapping entry*/
			BlkNum = 0xffff;
			for (i=StartBlock; i>=EndBlock; i-=2)
			{
				/*Retrive the logical block nubmer form each logical address pointer*/
				/*It will access two logical address pointer each time.*/
				eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				if(( (LogicalAddress >> 8) & 0xff) == 0)
				{
					if(i !=pAd->chipCap.EFUSE_USAGE_MAP_END)
					{
						BlkNum = i+1-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}	
				}

				if( (LogicalAddress & 0xff) == 0)
				{
					if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
					{
						BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}
				}
			
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin::Not bWriteSuccess new BlkNum = %d\n", BlkNum));	
			if(BlkNum == 0xffff)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin: out of free E-fuse space!!!\n"));
				return FALSE;
			}

			/*invalidate the original mapping entry if new entry is not found*/
			tmpaddr = addr;

			if(addr % 2 != 0)
				addr = addr -1; 
			InBuf[0] = addr;
			InBuf[1] = 2;		
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);				

			/* write the logical address*/
			if(tmpaddr%2 != 0) 
			{
				/* Invalidate the high byte*/
				for (i=8; i<15; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}		
			}	
			else
			{
				/* invalidate the low byte*/
				for (i=0; i<8; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}					
			}
			eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0);	
		}	
		
	}
	while(!bWriteSuccess&&Loop<2);

	return TRUE;
}

int rtmp_ee_efuse_read16(
	IN RTMP_ADAPTER *pAd, 
	IN USHORT Offset,
	OUT USHORT *pValue)
{
	eFuseReadRegisters(pAd, Offset, 2, pValue);
	return (*pValue);
}


int rtmp_ee_efuse_write16(
	IN RTMP_ADAPTER *pAd, 
	IN USHORT Offset, 
	IN USHORT data)
{
	eFuseWrite(pAd,Offset ,&data, 2);
	return 0;
}

INT rtmp_ee_write_to_efuse(
	IN PRTMP_ADAPTER 	pAd)
{
	INT read_byte = 0, cmp_buf = 1;
	USHORT data, *pdata;
		
	pdata = (USHORT*)(pAd->EEPROMImage + MAX_EEPROM_BIN_FILE_SIZE);	
	
	for (read_byte = 0; read_byte < MAX_EEPROM_BIN_FILE_SIZE; read_byte++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%02X ",pAd->EEPROMImage[read_byte] & 0xff));
		
		if ((read_byte+1)%2 == 0)
			pdata[read_byte/2%8] = ((pAd->EEPROMImage[read_byte] << 8) & 0xff00) | (pAd->EEPROMImage[read_byte-1] & 0xff);
		
		if (read_byte%16 == 0)
			cmp_buf = pAd->EEPROMImage[read_byte] & 0xff;
		else
		{
			cmp_buf &= (pAd->EEPROMImage[read_byte] & 0xff);
			if ((read_byte+1)%16 == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Result=%02X, blk=%02X\n", cmp_buf, read_byte/16));

				if (cmp_buf != 0xff)
					eFuseWriteRegistersFromBin(pAd, (USHORT)read_byte - 15, 16, pdata);
				else if(eFuseReadRegisters(pAd, read_byte, 2, (PUSHORT)&data) != 0x3f)
						eFuseWriteRegistersFromBin(pAd, (USHORT)read_byte - 15, 16, pdata);
				
				NdisZeroMemory(pdata, 16);
			}
		}
	}

	return TRUE;
}


VOID eFuseGetFreeBlockCount(
	IN PRTMP_ADAPTER 	pAd, 
	INOUT PUINT 			EfuseFreeBlock)
{
	
	USHORT i=0, StartBlock=0, EndBlock=0;
	USHORT	LogicalAddress;
	USHORT	FirstFreeBlock = 0xffff, LastFreeBlock = 0xffff;

	if (!pAd->bUseEfuse)
	{
		DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount Only supports efuse Mode\n"));
		return ;
	}
	*EfuseFreeBlock = 0;
	/* find first free block*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1; 
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START; 
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1; 
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END; 
	}

	for (i = StartBlock; i <= EndBlock; i+=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
		
		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				FirstFreeBlock = i;
				break;
			}
		}		

		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				FirstFreeBlock = i+1;
				break;
			}	
		}			
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, FirstFreeBlock= 0x%x\n", FirstFreeBlock));

	/*if not find, return free block number = 0*/
	if(FirstFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;	
	}
	for (i = EndBlock; i >= StartBlock; i-=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				
		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				LastFreeBlock = i+1;
				break;
			}	
		}

		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				LastFreeBlock = i;
				break;
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, LastFreeBlock= 0x%x\n", LastFreeBlock));

	/*if not find last free block, return free block number = 0, this should not happen since we have checked first free block number previously*/
	if(LastFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;	
	}

	/* return total free block number, last free block number must >= first free block number*/
	if(LastFreeBlock < FirstFreeBlock)
	{
		*EfuseFreeBlock = 0;	
	}
	else
	{
		*EfuseFreeBlock = LastFreeBlock - FirstFreeBlock + 1;
	}
	
	DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount is %d\n",*EfuseFreeBlock));
}

INT eFuse_init(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT free_blk = 0;

	DBGPRINT(RT_DEBUG_OFF, ("NVM is efuse and its size =%x[%x-%x]\n",
		pAd->chipCap.EFUSE_USAGE_MAP_SIZE, pAd->chipCap.EFUSE_USAGE_MAP_START, pAd->chipCap.EFUSE_USAGE_MAP_END));
	
	eFuseGetFreeBlockCount(pAd, &free_blk);
	
	/************************************************************************************
  	  *	If the number of the used block is less than 5, we assume the efuse is not well-calibrated. 	*
  	  *   Thus, in order to bring up the interface successfully, force to use EEPROM buffer mode.			*
  	  ***********************************************************************************/
	if (free_blk > (pAd->chipCap.EFUSE_USAGE_MAP_SIZE - 5))
	{
		DBGPRINT(RT_DEBUG_OFF, ("NVM is efuse and the information is too less to bring up the interface\n"));
		DBGPRINT(RT_DEBUG_OFF, ("Load EEPROM buffer from BIN, and force to use BIN buffer mode\n"));

		pAd->bUseEfuse = FALSE;
		rtmp_ee_load_from_bin(pAd);

		/* Forse to use BIN eeprom buffer mode */
		pAd->E2pAccessMode = E2P_BIN_MODE;
		RtmpChipOpsEepromHook(pAd, pAd->infType);
	}

	return 0;
}


INT efuse_probe(
	IN RTMP_ADAPTER 	*pAd)
{
	UINT32 efuse_ctrl, ctrl_reg;
	
	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	pAd->bUseEfuse = FALSE;

#if defined(RT3290) || defined(RT65xx)
	if (IS_RT3290(pAd) || IS_RT65XX(pAd))
		ctrl_reg = EFUSE_CTRL_3290;
	else
#endif /* defined(RT3290) || defined(RT65xx) */
		ctrl_reg = EFUSE_CTRL;

	RTMP_IO_READ32(pAd, ctrl_reg, &efuse_ctrl);
	pAd->bUseEfuse = ((efuse_ctrl & 0x80000000) == 0x80000000) ? 1 : 0;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s::bUseEfuse = %d\n", __FUNCTION__, pAd->bUseEfuse));
	return 0;
}


#ifdef RALINK_ATE
INT Set_LoadEepromBufferFromEfuse_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	UINT free_blk = 0;
	
	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM buffer from efuse, and change to BIN buffer mode\n"));	

		/* If the number of the used block is less than 5, assume the efuse is not well-calibrated, and force to use buffer mode */
		eFuseGetFreeBlockCount(pAd, &free_blk);
		if (free_blk > (pAd->chipCap.EFUSE_USAGE_MAP_SIZE - 5))
			return FALSE;
		
		NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
		eFuseRead(pAd, 0, (PUSHORT)&pAd->EEPROMImage[0], MAX_EEPROM_BIN_FILE_SIZE);		

		/* Change to BIN eeprom buffer mode */
		pAd->E2pAccessMode = E2P_BIN_MODE;
		RtmpChipOpsEepromHook(pAd, pAd->infType);
		return TRUE;
	}
}

INT set_eFuseBufferModeWriteBack_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	
	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s::Write EEPROM buffer back to eFuse\n", __FUNCTION__));	
		Set_EepromBufferWriteBack_Proc(pAd, "1");
		return TRUE;
	}
}
#endif /* RALINK_ATE */
#endif /* RTMP_EFUSE_SUPPORT */

