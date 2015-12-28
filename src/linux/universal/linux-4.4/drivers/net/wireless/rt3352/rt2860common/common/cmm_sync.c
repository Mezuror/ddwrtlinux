/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-09-01      modified for rt2561/2661
*/
#include "rt_config.h"

/*BaSizeArray follows the 802.11n definition as MaxRxFactor.  2^(13+factor) bytes. When factor =0, it's about Ba buffer size =8.*/
UCHAR BaSizeArray[4] = {8,16,32,64};

#ifdef P2P_SUPPORT
extern UCHAR	WILDP2PSSID[];
extern UCHAR	WILDP2PSSIDLEN;
#endif /* P2P_SUPPORT */

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;

/* 
	==========================================================================
	Description:
		Update StaCfg->ChannelList[] according to 1) Country Region 2) RF IC type,
		and 3) PHY-mode user selected.
		The outcome is used by driver when doing site survey.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID BuildChannelList(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR i, j, index=0, num=0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PUCHAR pChannelList;
	PUCHAR pChannelListFlag;

	NdisZeroMemory(pAd->ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	/* if not 11a-only mode, channel list starts from 2.4Ghz band*/
	if ((pAd->CommonCfg.PhyMode != PHY_11A) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.PhyMode != PHY_11AN_MIXED) && (pAd->CommonCfg.PhyMode != PHY_11N_5G)
#endif /* DOT11_N_SUPPORT */
	)
	{
		for (i = 0; i < Country_Region_GroupNum_2GHZ; i++)
		{
			if ((pAd->CommonCfg.CountryRegion & 0x7f) ==
				Country_Region_ChDesc_2GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_2GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;
				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("CountryRegion=%d not support", pAd->CommonCfg.CountryRegion));
			return;		
		}

		if (num > 0)
		{
			os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

			if (!pChannelList)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelList failed\n", __FUNCTION__));
				return;
			}

			os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

			if (!pChannelListFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelListFlag failed\n", __FUNCTION__));
				os_free_mem(NULL, pChannelList);
				return;	
			}

			for (i = 0; i < num; i++)
			{
				pChannelList[i] = GetChannel_2GHZ(pChDesc, i);
				pChannelListFlag[i] = GetChannelFlag(pChDesc, i);
			}

			for (i = 0; i < num; i++)
			{
				for (j = 0; j < MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel)
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
						pAd->ChannelList[index + i].Flags = pChannelListFlag[i];
				}

				pAd->ChannelList[index+i].MaxTxPwr = 20;
			}

			index += num;

			os_free_mem(NULL, pChannelList);
			os_free_mem(NULL, pChannelListFlag);
		}
		bRegionFound = FALSE;
		num = 0;
	}

	if ((pAd->CommonCfg.PhyMode == PHY_11A) || (pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED) 
#ifdef DOT11_N_SUPPORT
		|| (pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11AN_MIXED) 
		|| (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)
#endif /* DOT11_N_SUPPORT */
	)
	{
		for (i = 0; i < Country_Region_GroupNum_5GHZ; i++)
		{
			if ((pAd->CommonCfg.CountryRegionForABand & 0x7f) ==
				Country_Region_ChDesc_5GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_5GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;
				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("CountryRegionABand=%d not support", pAd->CommonCfg.CountryRegionForABand));
			return;
		}

		if (num > 0)
		{
			UCHAR RadarCh[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
			UCHAR q=0;
#endif /* DFS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

			if (!pChannelList)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelList failed\n", __FUNCTION__));
				return;
			}

			os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

			if (!pChannelListFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelListFlag failed\n", __FUNCTION__));
				os_free_mem(NULL, pChannelList);
				return;
			}

			for (i = 0; i < num; i++)
			{
				pChannelList[i] = GetChannel_5GHZ(pChDesc, i);
				pChannelListFlag[i] = GetChannelFlag(pChDesc, i);
			}

#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
			for (i = 0; i < num; i++)
			{

				 if((pAd->CommonCfg.bIEEE80211H == 0)|| ((pAd->CommonCfg.bIEEE80211H == 1) && (pAd->CommonCfg.RadarDetect.RDDurRegion != FCC)))			 	
                                {
										pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
										pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
                                        q++;
                                }
/*Based on the requiremnt of FCC, some channles could not be used anymore when test DFS function.*/
                                else if ((pAd->CommonCfg.bIEEE80211H == 1) && (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC) && (pAd->CommonCfg.bDFSIndoor == 1))
                                {
                                        if((GetChannel_5GHZ(pChDesc, i) < 116) || (GetChannel_5GHZ(pChDesc, i) > 128))
                                        {
												pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
												pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
                                                q++;
                                        }
                                }

                                else if ((pAd->CommonCfg.bIEEE80211H == 1) && (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC) && (pAd->CommonCfg.bDFSIndoor == 0))
                                {
                                        if((GetChannel_5GHZ(pChDesc, i) < 100) || (GetChannel_5GHZ(pChDesc, i) > 140) )
                                        {
												pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
												pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
                                                q++;
                                        }
                                }

			}

			num = q;
#endif /* DFS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			for (i=0; i<num; i++)
			{
				for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel)
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
						pAd->ChannelList[index + i].Flags = pChannelListFlag[i];
				}

				for (j=0; j<15; j++)
				{
					if (pChannelList[i] == RadarCh[j])
						pAd->ChannelList[index+i].DfsReq = TRUE;
				}
				pAd->ChannelList[index+i].MaxTxPwr = 20;
			}
			index += num;

			os_free_mem(NULL, pChannelList);
			os_free_mem(NULL, pChannelListFlag);
		}
	}

	pAd->ChannelListNum = index;	
	DBGPRINT(RT_DEBUG_TRACE,("country code=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n", 
		pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryRegionForABand, pAd->RfIcType, pAd->CommonCfg.PhyMode, pAd->ChannelListNum));


#ifdef DBG	
	for (i=0;i<pAd->ChannelListNum;i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE,("BuildChannel # %d :: Pwr0 = %d, Pwr1 =%d, Flags = %x\n ", 
									 pAd->ChannelList[i].Channel, 
									 pAd->ChannelList[i].Power, 
									 pAd->ChannelList[i].Power2, 
									 pAd->ChannelList[i].Flags));
	}
#endif
}

/* 
	==========================================================================
	Description:
		This routine return the first channel number according to the country 
		code selection and RF IC selection (signal band or dual band). It is called
		whenever driver need to start a site survey of all supported channels.
	Return:
		ch - the first channel number of current country code setting

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
UCHAR FirstChannel(
	IN PRTMP_ADAPTER pAd)
{
	return pAd->ChannelList[0].Channel;
}

/* 
	==========================================================================
	Description:
		This routine returns the next channel number. This routine is called
		during driver need to start a site survey of all supported channels.
	Return:
		next_channel - the next channel number valid in current country code setting.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR NextChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR channel)
{
	int i;
	UCHAR next_channel = 0;
#ifdef P2P_SUPPORT
	UCHAR	CurrentChannel = channel;

	if (pAd->MlmeAux.ScanType == SCAN_P2P_SEARCH)
	{
		if (IS_P2P_LISTEN(pAd))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Error !! P2P Discovery state machine has change to Listen state during scanning !\n"));
			return next_channel;
		}	

		for (i = 0; i < (pAd->P2pCfg.P2pProprietary.ListenChanelCount - 1); i++)
		{
			if (CurrentChannel == pAd->P2pCfg.P2pProprietary.ListenChanel[i])
				next_channel = pAd->P2pCfg.P2pProprietary.ListenChanel[i+1];				
		}
		P2P_INC_CHA_INDEX(pAd->P2pCfg.P2pProprietary.ListenChanelIndex, pAd->P2pCfg.P2pProprietary.ListenChanelCount);
		if (next_channel == CurrentChannel)
		{
			DBGPRINT(RT_DEBUG_INFO, ("SYNC -  next_channel equals to CurrentChannel= %d\n", next_channel));
			DBGPRINT(RT_DEBUG_INFO, ("SYNC -  ListenChannel List : %d  %d  %d\n", pAd->P2pCfg.P2pProprietary.ListenChanel[0], pAd->P2pCfg.P2pProprietary.ListenChanel[1], pAd->P2pCfg.P2pProprietary.ListenChanel[2]));

			next_channel = 0;
		}

		if (!INFRA_ON(pAd) && (next_channel == 0))
			pAd->CommonCfg.Channel = pAd->P2pCfg.ListenChannel;
		DBGPRINT(RT_DEBUG_INFO, ("SYNC - P2P Scan return channel = %d.    Listen Channel = %d.\n", next_channel, pAd->CommonCfg.Channel));

		return next_channel;
	}
#endif /* P2P_SUPPORT */
			
	for (i = 0; i < (pAd->ChannelListNum - 1); i++)
	{
		if (channel == pAd->ChannelList[i].Channel)
		{
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			/* Only scan effected channel if this is a SCAN_2040_BSS_COEXIST*/
			/* 2009 PF#2: Nee to handle the second channel of AP fall into affected channel range.*/
			if ((pAd->MlmeAux.ScanType == SCAN_2040_BSS_COEXIST) && (pAd->ChannelList[i+1].Channel >14))
			{
				channel = pAd->ChannelList[i+1].Channel;
				continue;
			}
			else
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			{
				/* Record this channel's idx in ChannelList array.*/
			next_channel = pAd->ChannelList[i+1].Channel;
			break;
	}
		}
		
	}
	return next_channel;
}

/* 
	==========================================================================
	Description:
		This routine is for Cisco Compatible Extensions 2.X 
		Spec31. AP Control of Client Transmit Power
	Return:
		None
	Note:
	   Required by Aironet dBm(mW)
		   0dBm(1mW),   1dBm(5mW), 13dBm(20mW), 15dBm(30mW),
		  17dBm(50mw), 20dBm(100mW)

	   We supported 
		   3dBm(Lowest), 6dBm(10%), 9dBm(25%), 12dBm(50%),
		  14dBm(75%),   15dBm(100%)

		The client station's actual transmit power shall be within +/- 5dB of
		the minimum value or next lower value.
	==========================================================================
 */
VOID ChangeToCellPowerLimit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         AironetCellPowerLimit)
{
	/*
		valud 0xFF means that hasn't found power limit information
		from the AP's Beacon/Probe response
	*/
	if (AironetCellPowerLimit == 0xFF)
		return;  
	
	if (AironetCellPowerLimit < 6) /*Used Lowest Power Percentage.*/
		pAd->CommonCfg.TxPowerPercentage = 6; 
	else if (AironetCellPowerLimit < 9)
		pAd->CommonCfg.TxPowerPercentage = 10;
	else if (AironetCellPowerLimit < 12)
		pAd->CommonCfg.TxPowerPercentage = 25;
	else if (AironetCellPowerLimit < 14)
		pAd->CommonCfg.TxPowerPercentage = 50;
	else if (AironetCellPowerLimit < 15)
		pAd->CommonCfg.TxPowerPercentage = 75;
	else
		pAd->CommonCfg.TxPowerPercentage = 100; /*else used maximum*/

	if (pAd->CommonCfg.TxPowerPercentage > pAd->CommonCfg.TxPowerDefault)
		pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;
	
}

CHAR	ConvertToRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi,
	IN UCHAR			RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	/* Rssi equals to zero should be an invalid value*/
	if (Rssi == 0)
		return -99;

	LNAGain = GET_LNA_GAIN(pAd);
    if (pAd->LatchRfRegs.Channel > 14)
    {
        if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
    }
    else
    {
        if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
    }
	
    return (-12 - RssiOffset - LNAGain - Rssi);
}

CHAR	ConvertToSnr(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR				Snr)	
{
	return ((0xeb - Snr) * 3) / 16 ;

}

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID ScanNextChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR OpMode) 
{
	HEADER_802_11   Hdr80211;
	PUCHAR          pOutBuffer = NULL;
	NDIS_STATUS     NStatus;
	ULONG           FrameLen = 0;
	UCHAR           SsidLen = 0, ScanType = pAd->MlmeAux.ScanType, BBPValue = 0;
#ifdef CONFIG_STA_SUPPORT
	USHORT          Status;
/*	PHEADER_802_11  pHdr80211;  no use*/
#endif /* CONFIG_STA_SUPPORT */
	UINT			ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;
	BOOLEAN			ScanPending = FALSE;


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (MONITOR_ON(pAd))
			return;
	}

	ScanPending = ((pAd->StaCfg.bImprovedScan) && (pAd->StaCfg.ScanChannelCnt>=7));
#endif /* CONFIG_STA_SUPPORT */

#ifdef RALINK_ATE
	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

	if ((pAd->MlmeAux.Channel == 0) || ScanPending) 
	{
		if ((pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef CONFIG_STA_SUPPORT
			&& (INFRA_ON(pAd) || ADHOC_ON(pAd)
#ifdef P2P_SUPPORT
				|| (OpMode == OPMODE_AP))
#else
				|| (pAd->OpMode == OPMODE_AP))
#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
			)
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			BBPValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 40MHz channel %d, Total BSS[%02d]\n",pAd->CommonCfg.CentralChannel, pAd->ScanTab.BssNr));
		}
		else
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to channel %d, Total BSS[%02d]\n",pAd->CommonCfg.Channel, pAd->ScanTab.BssNr));
		}
		
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
		if (OpMode == OPMODE_STA)
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
		{

			/*
				If all peer Ad-hoc clients leave, driver would do LinkDown and LinkUp.
				In LinkUp, CommonCfg.Ssid would copy SSID from MlmeAux. 
				To prevent SSID is zero or wrong in Beacon, need to recover MlmeAux.SSID here.
			*/
			if (ADHOC_ON(pAd))
			{
				NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pAd->MlmeAux.SsidLen = pAd->CommonCfg.SsidLen;
				NdisMoveMemory(pAd->MlmeAux.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
			}
		
			
			/*
				To prevent data lost.
				Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
				Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done 
			*/
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
			{
				RTMPSendNullFrame(pAd, 
								  pAd->CommonCfg.TxRate, 
								  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
								  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
				DBGPRINT(RT_DEBUG_TRACE, ("%s -- Send null frame\n", __FUNCTION__));
			}

			/* keep the latest scan channel, could be 0 for scan complete, or other channel*/
			pAd->StaCfg.LastScanChannel = pAd->MlmeAux.Channel;

			pAd->StaCfg.ScanChannelCnt = 0;

			/* Suspend scanning and Resume TxData for Fast Scanning*/
			if ((pAd->MlmeAux.Channel != 0) &&
				(pAd->StaCfg.bImprovedScan))	/* it is scan pending*/
			{
				pAd->Mlme.SyncMachine.CurrState = SCAN_PENDING;
				Status = MLME_SUCCESS;
				DBGPRINT(RT_DEBUG_WARN, ("bFastRoamingScan ~~~~~~~~~~~~~ Get back to send data ~~~~~~~~~~~~~\n"));

				RTMPResumeMsduTransmission(pAd);
			}
			else
			{
				pAd->StaCfg.BssNr = pAd->ScanTab.BssNr;
				pAd->StaCfg.bImprovedScan = FALSE;

				pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
				Status = MLME_SUCCESS;
				MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);

#ifdef P2P_SUPPORT
				if (!((pAd->MlmeAux.ScanType == SCAN_P2P_SEARCH) || (pAd->MlmeAux.ScanType == SCAN_P2P)))
#endif /* P2P_SUPPORT */
				{
					RTMPSendWirelessEvent(pAd, IW_SCAN_COMPLETED_EVENT_FLAG, NULL, BSS0, 0);
#ifdef WPA_SUPPLICANT_SUPPORT
					RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_SCAN, -1, NULL, NULL, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */
				}
			}

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
			RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
		if (OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
		{
			pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
			RTMPResumeMsduTransmission(pAd);

			/* iwpriv set auto channel selection*/
			/* scanned all channels*/
			if (pAd->ApCfg.bAutoChannelAtBootup==TRUE)
			{
			    pAd->CommonCfg.Channel = SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;
#ifdef DOT11_N_SUPPORT
				N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
				APStop(pAd);
				APStartUp(pAd);
			}

			if (!((pAd->CommonCfg.Channel > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE)))
			{
				AsicEnableBssSync(pAd);
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef P2P_SUPPORT
			if ((ScanType == SCAN_P2P) || (ScanType == SCAN_P2P_SEARCH))
				P2PSetListenTimer(pAd, 0);
#endif /* P2P_SUPPORT */
	} 
	else 
	{
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
		if (OpMode == OPMODE_STA)
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
		{
			/* BBP and RF are not accessible in PS mode, we has to wake them up first*/
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
				AsicForceWakeup(pAd, TRUE);

			/* leave PSM during scanning. otherwise we may lost ProbeRsp & BEACON*/
			if (pAd->StaCfg.Psm == PWR_SAVE)
				RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
		}
#endif /* CONFIG_STA_SUPPORT */

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, TRUE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
		if (OpMode == OPMODE_STA)
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
		{
			if (pAd->MlmeAux.Channel > 14)
			{
				if ((pAd->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pAd->MlmeAux.Channel))
				{
					ScanType = SCAN_PASSIVE;
					ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
				}
			}

#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier*/
			/* carrier detection*/
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			{
				ScanType = SCAN_PASSIVE;
				ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
			}
#endif /* CARRIER_DETECTION_SUPPORT */ 
		}

#endif /* CONFIG_STA_SUPPORT */

		/* Check if channel if passive scan under current regulatory domain */
		if (CHAN_PropertyCheck(pAd, pAd->MlmeAux.Channel, CHANNEL_PASSIVE_SCAN) == TRUE)
			ScanType = SCAN_PASSIVE;

		/* We need to shorten active scan time in order for WZC connect issue*/
		/* Chnage the channel scan time for CISCO stuff based on its IAPP announcement*/
		if (ScanType == FAST_SCAN_ACTIVE)
			RTMPSetTimer(&pAd->MlmeAux.ScanTimer, FAST_ACTIVE_SCAN_TIME);
		else /* must be SCAN_PASSIVE or SCAN_ACTIVE*/
		{
#ifdef CONFIG_STA_SUPPORT
			pAd->StaCfg.ScanChannelCnt++;
#endif /* CONFIG_STA_SUPPORT */
			if ((pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED) 
#ifdef DOT11_N_SUPPORT
				|| (pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED)
#endif /* DOT11_N_SUPPORT */
			)
			{
#ifdef CONFIG_AP_SUPPORT
				if (pAd->ApCfg.bAutoChannelAtBootup)
				{
					/* wait 400 ms*/
					if (OpMode == OPMODE_AP)
					RTMPSetTimer(&pAd->MlmeAux.APScanTimer, AUTO_CHANNEL_SEL_TIMEOUT);
				}
				else
#endif /* CONFIG_AP_SUPPORT */
				{
					if (pAd->MlmeAux.Channel > 14)
					{
#ifdef CONFIG_AP_SUPPORT
						if (OpMode == OPMODE_AP)
							RTMPSetTimer(&pAd->MlmeAux.APScanTimer, ScanTimeIn5gChannel);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
						if (OpMode == OPMODE_STA)
						RTMPSetTimer(&pAd->MlmeAux.ScanTimer, ScanTimeIn5gChannel);
#endif /* CONFIG_STA_SUPPORT */
					}
					else
					{
#ifdef CONFIG_AP_SUPPORT
						if (OpMode == OPMODE_AP)
							RTMPSetTimer(&pAd->MlmeAux.APScanTimer, MIN_CHANNEL_TIME);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
						if (OpMode == OPMODE_STA)
						RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MIN_CHANNEL_TIME);
#endif /* CONFIG_STA_SUPPORT */
					}
				}
			}
			else
			{
#ifdef CONFIG_AP_SUPPORT
				if (pAd->ApCfg.bAutoChannelAtBootup)
				{
					/* wait 400 ms*/
					if (OpMode == OPMODE_AP)
					RTMPSetTimer(&pAd->MlmeAux.APScanTimer, AUTO_CHANNEL_SEL_TIMEOUT);
				}
				else
#endif /* CONFIG_AP_SUPPORT */
				{
#ifdef CONFIG_AP_SUPPORT
					if (OpMode == OPMODE_AP)
						RTMPSetTimer(&pAd->MlmeAux.APScanTimer, MAX_CHANNEL_TIME);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
					if (OpMode == OPMODE_STA)
					RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MAX_CHANNEL_TIME);
#endif /* CONFIG_STA_SUPPORT */
				}
			}
		}
		if ((ScanType == SCAN_ACTIVE)
			|| (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
#ifdef P2P_SUPPORT
			|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#else
			|| ((ScanType == SCAN_WSC_ACTIVE) && (pAd->OpMode == OPMODE_STA))
#endif /* P2P_SUPPORT */
#endif /* WSC_STA_SUPPORT */
#ifdef WSC_AP_SUPPORT
#ifdef P2P_SUPPORT
			|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP))
#endif /* P2P_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			|| (ScanType == SCAN_2040_BSS_COEXIST)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef P2P_SUPPORT
			|| (ScanType == SCAN_P2P) 
			|| (ScanType == SCAN_P2P_SEARCH)
#endif /* P2P_SUPPORT */
			)
		{
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
			if (NStatus != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - ScanNextChannel() allocate memory fail\n"));
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
				if (OpMode == OPMODE_STA)
#else
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
				{
					pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
					Status = MLME_FAIL_NO_RESOURCE;
					MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
				}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
				if (OpMode == OPMODE_AP)
#else
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
					pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif /* CONFIG_AP_SUPPORT */
				return;
			}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			if (ScanType == SCAN_2040_BSS_COEXIST)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - SCAN_2040_BSS_COEXIST !! Prepare to send Probe Request\n"));
			}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			
			/* There is no need to send broadcast probe request if active scan is in effect.*/
			SsidLen = 0;
			if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
#ifdef P2P_SUPPORT
				|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#else
				|| ((ScanType == SCAN_WSC_ACTIVE) && (pAd->OpMode == OPMODE_STA))
#endif /* P2P_SUPPORT */
#endif /* WSC_STA_SUPPORT */
				)
				SsidLen = pAd->MlmeAux.SsidLen;

#ifdef P2P_SUPPORT
			if ((pAd->MlmeAux.ScanType == SCAN_P2P) || (pAd->MlmeAux.ScanType == SCAN_P2P_SEARCH) ||
				((pAd->MlmeAux.ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP) && (P2P_CLI_ON(pAd))))
			{
				PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
				UCHAR		SupRate[MAX_LEN_OF_SUPPORTED_RATES];
				/*UCHAR		SupRateIe = IE_SUPP_RATES;*/
				UCHAR		SupRateLen = 0;
				/* JANTEMP */
				/* Use Wildword SSID */

				SsidLen = WILDP2PSSIDLEN;
				SupRate[0]	= 0x8C;    /* 6 mbps, in units of 0.5 Mbps, basic rate */
				SupRate[1]	= 0x12;    /* 9 mbps, in units of 0.5 Mbps */
				SupRate[2]	= 0x98;    /* 12 mbps, in units of 0.5 Mbps, basic rate */
				SupRate[3]	= 0x24;    /* 18 mbps, in units of 0.5 Mbps */
				SupRate[4]	= 0xb0;    /* 24 mbps, in units of 0.5 Mbps, basic rate */
				SupRate[5]	= 0x48;    /* 36 mbps, in units of 0.5 Mbps */
				SupRate[6]	= 0x60;    /* 48 mbps, in units of 0.5 Mbps */
				SupRate[7]	= 0x6c;    /* 54 mbps, in units of 0.5 Mbps */
				SupRateLen	= 8;
				/* P2P scan must use P2P mac address. */
				MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
									pP2PCtrl->CurrentAddress,
									BROADCAST_ADDR);

				MakeOutgoingFrame(pOutBuffer,				&FrameLen,
									sizeof(HEADER_802_11),	&Hdr80211,
									1,						&SsidIe,
									1,						&SsidLen,
									SsidLen,					&WILDP2PSSID[0],
									1,						&SupRateIe,
									1,						&SupRateLen,
									SupRateLen, 			SupRate, 
									END_OF_ARGS);
			}
			else
#endif /* P2P_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				/*IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
				if (OpMode == OPMODE_AP)
				{
					MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, 
#ifdef P2P_SUPPORT
										pAd->ApCfg.MBSSID[0].Bssid,
#endif /* P2P_SUPPORT */
										pAd->ApCfg.MBSSID[0].Bssid);
				}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				/*IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
				if (OpMode == OPMODE_STA)
				{
					MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, 
#ifdef P2P_SUPPORT
										pAd->CurrentAddress,
#endif /* P2P_SUPPORT */
										BROADCAST_ADDR);
				}
#endif /* CONFIG_STA_SUPPORT */

				MakeOutgoingFrame(pOutBuffer,               &FrameLen,
								  sizeof(HEADER_802_11),    &Hdr80211,
								  1,                        &SsidIe,
								  1,                        &SsidLen,
								  SsidLen,			        pAd->MlmeAux.Ssid,
								  1,                        &SupRateIe,
								  1,                        &pAd->CommonCfg.SupRateLen,
								  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate, 
								  END_OF_ARGS);

				if (pAd->CommonCfg.ExtRateLen)
				{
					ULONG Tmp;
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
									  1,                                &ExtRateIe,
									  1,                                &pAd->CommonCfg.ExtRateLen,
									  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate, 
									  END_OF_ARGS);
					FrameLen += Tmp;
				}
			}
#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				ULONG	Tmp;
				UCHAR	HtLen;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
#ifdef RT_BIG_ENDIAN
				HT_CAPABILITY_IE HtCapabilityTmp;
#endif
				if (pAd->bBroadComHT == TRUE)
				{
					HtLen = pAd->MlmeAux.HtCapabilityLen + 4;
#ifdef RT_BIG_ENDIAN
					NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
					*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
					}
#else				
					*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &WpaIe,
									1,                                &HtLen,
									4,                                &BROADCOM[0],
									pAd->MlmeAux.HtCapabilityLen,     &HtCapabilityTmp, 
									END_OF_ARGS);
#else
					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &WpaIe,
									1,                                &HtLen,
									4,                                &BROADCOM[0],
									pAd->MlmeAux.HtCapabilityLen,     &pAd->MlmeAux.HtCapability, 
									END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
				}
				else				
				{
					HtLen = sizeof(HT_CAPABILITY_IE);
#ifdef RT_BIG_ENDIAN
					NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, SIZE_HT_CAP_IE);
					*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
					}
#else				
					*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &HtCapIe,
									1,                                &HtLen,
									HtLen,                            &HtCapabilityTmp, 
									END_OF_ARGS);
#else
					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &HtCapIe,
									1,                                &HtLen,
									HtLen,                            &pAd->CommonCfg.HtCapability, 
									END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
				}
				FrameLen += Tmp;

#ifdef DOT11N_DRAFT3
				if ((pAd->MlmeAux.Channel <= 14) && (pAd->CommonCfg.bBssCoexEnable == TRUE))
				{
					ULONG		Tmp;
					HtLen = 1;
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
									  1,					&ExtHtCapIe,
									  1,					&HtLen,
									  1,          			&pAd->CommonCfg.BSSCoexist2040.word, 
									  END_OF_ARGS);

					FrameLen += Tmp;
				}
#endif /* DOT11N_DRAFT3 */
			}
#endif /* DOT11_N_SUPPORT */

#ifdef WSC_STA_SUPPORT
#ifdef P2P_SUPPORT
			if (OpMode == OPMODE_STA)
#else
			if (pAd->OpMode == OPMODE_STA)
#endif /* P2P_SUPPORT */
			{
				BOOLEAN bHasWscIe = FALSE;
				/* 
					Append WSC information in probe request if WSC state is running
				*/
				if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
					(pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
					(pAd->StaCfg.WscControl.bWscTrigger == TRUE))
					bHasWscIe = TRUE;
#ifdef WSC_V2_SUPPORT
				else if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
					(pAd->StaCfg.WscControl.WscV2Info.bEnableWpsV2))
					bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */

#ifdef P2P_SUPPORT
				/* 
					P2pMakeProbeReqIE will build WSC IE for P2P, 
					it is not good to append normal WSC IE into P2P probe request frame here.
				*/
				if ((pAd->MlmeAux.ScanType == SCAN_P2P) || (pAd->MlmeAux.ScanType == SCAN_P2P_SEARCH) ||
					((pAd->MlmeAux.ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP) && (P2P_CLI_ON(pAd))))
					bHasWscIe = FALSE;
#endif /* P2P_SUPPORT */

				if (bHasWscIe)
				{
					UCHAR		*pWscBuf = NULL, WscIeLen = 0;
					ULONG 		WscTmpLen = 0;

					os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
					if (pWscBuf != NULL)
					{
						NdisZeroMemory(pWscBuf, 512);
						WscBuildProbeReqIE(pAd, STA_MODE, pWscBuf, &WscIeLen);

						MakeOutgoingFrame(pOutBuffer + FrameLen,              &WscTmpLen,
										WscIeLen,                             pWscBuf,
										END_OF_ARGS);

						FrameLen += WscTmpLen;
						os_free_mem(NULL, pWscBuf);
					}
					else
						DBGPRINT(RT_DEBUG_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
				}
			}

#endif /* WSC_STA_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
			if (
#ifdef P2P_SUPPORT
			(OpMode == OPMODE_STA) &&
#else
			(pAd->OpMode == OPMODE_STA) &&
#endif /* P2P_SUPPORT */
				(pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
				(pAd->StaCfg.WpsProbeReqIeLen != 0))
			{
				ULONG 		WpsTmpLen = 0;
				
				MakeOutgoingFrame(pOutBuffer + FrameLen,              &WpsTmpLen,
								pAd->StaCfg.WpsProbeReqIeLen,	pAd->StaCfg.pWpsProbeReqIe,
								END_OF_ARGS);

				FrameLen += WpsTmpLen;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef P2P_SUPPORT
			if ((pAd->MlmeAux.ScanType == SCAN_P2P) || (pAd->MlmeAux.ScanType == SCAN_P2P_SEARCH) ||
				((pAd->MlmeAux.ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP) && (P2P_CLI_ON(pAd))))
			{
				ULONG	P2PIeLen;
				PUCHAR	ptr;
				ptr = pOutBuffer + FrameLen;
				P2pMakeProbeReqIE(pAd, ptr, &BBPValue);
				FrameLen += BBPValue;
				
				/* Put P2P IE to the last. */
				ptr = pOutBuffer + FrameLen;
				P2pMakeP2pIE(pAd, SUBTYPE_PROBE_REQ, ptr, &P2PIeLen);

				FrameLen += P2PIeLen;
			}
#endif /* P2P_SUPPORT */

			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
			if (OpMode == OPMODE_STA)
#else
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
			{
				
				/*
					To prevent data lost.
					Send an NULL data with turned PSM bit on to current associated AP when SCAN in the channel where
					associated AP located.
				*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
					(INFRA_ON(pAd)) &&
					(pAd->CommonCfg.Channel == pAd->MlmeAux.Channel))
				{
					RTMPSendNullFrame(pAd, 
								  pAd->CommonCfg.TxRate, 
								  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
								  PWR_SAVE);
					DBGPRINT(RT_DEBUG_TRACE, ("ScanNextChannel():Send PWA NullData frame to notify the associated AP!\n"));
				}
			}
#endif /* CONFIG_STA_SUPPORT */

			MlmeFreeMemory(pAd, pOutBuffer);
		}

		/* For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse*/
		
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
		if (OpMode == OPMODE_STA)
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
			pAd->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
				if (OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
			pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
#endif /* CONFIG_AP_SUPPORT */

	}
}
#endif



#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
extern int DetectOverlappingPeriodicRound;

VOID Handle_BSS_Width_Trigger_Events(
	IN PRTMP_ADAPTER pAd) 
{
	ULONG Now32;
	
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
		(pAd->CommonCfg.Channel <=14))
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("Rcv BSS Width Trigger Event: 40Mhz --> 20Mhz \n"));
        NdisGetSystemUpTime(&Now32);
		pAd->CommonCfg.LastRcvBSSWidthTriggerEventsTime = Now32;
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = TRUE;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;	
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
        DetectOverlappingPeriodicRound = 31;
	}
}
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN ScanRunning(
		IN PRTMP_ADAPTER pAd)
{
	BOOLEAN	rv = FALSE;

#ifdef P2P_SUPPORT
		rv = ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) ? TRUE : FALSE);

		if (rv == FALSE)
			rv = ((pAd->Mlme.SyncMachine.CurrState == SCAN_LISTEN) ? TRUE : FALSE);
#else
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			rv = ((pAd->Mlme.SyncMachine.CurrState == SCAN_LISTEN) ? TRUE : FALSE);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			rv = ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) ? TRUE : FALSE);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* P2P_SUPPORT */

	return rv;
}

