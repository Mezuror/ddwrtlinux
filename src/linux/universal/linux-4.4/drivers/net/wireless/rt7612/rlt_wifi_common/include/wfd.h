/* 

  This file is provided under a dual BSD/GPLv2 license.  When using or 
  redistributing this file, you may do so under either license.

  GPL LICENSE SUMMARY

  Copyright(c) 2005-2011 Ralink Technology Corporation.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
    Ralink Technology Corporation
    5F, No.5, Tai-Yuen 1st St., Jhubei City,
    HsinChu Hsien 30265, Taiwan, R.O.C.


  BSD LICENSE 

  Copyright(c) 2005-2011 Ralink Technology Corporation. All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

    * Redistributions of source code must retain the above copyright 
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright 
      notice, this list of conditions and the following disclaimer in 
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Intel Corporation nor the names of its 
      contributors may be used to endorse or promote products derived 
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/*
	Module Name:
	wfd.h
	
	Revision History:
	Who 			When			What
	--------		----------		----------------------------------------------
	
*/


#ifndef	__WFD_H__
#define	__WFD_H__

#ifdef WFD_SUPPORT

#include "rtmp_type.h"

INT Set_WfdEnable_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

#ifdef RT_CFG80211_SUPPORT
INT Set_WfdInsertIe_Proc
(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PSTRING 		arg);
#endif /* RT_CFG80211_SUPPORT */

INT Set_WfdDeviceType_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT Set_WfdCouple_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT Set_WfdSessionAvailable_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT Set_WfdCP_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT	Set_WfdRtspPort_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT	Set_WfdMaxThroughput_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT Set_WfdLocalIp_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

INT Set_PeerRtspPort_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg);

VOID WfdMakeWfdIE(
	IN	PRTMP_ADAPTER	pAd,
	IN 	ULONG			WfdIeBitmap,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen);

ULONG InsertWfdSubelmtTlv(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			SubId,
	IN PUCHAR			pInBuffer,
	IN PUCHAR			pOutBuffer,
	IN UINT				Action);

VOID WfdParseSubElmt(
	IN PRTMP_ADAPTER 	pAd, 
	IN PWFD_ENTRY_INFO	pWfdEntryInfo,
	IN VOID 				*Msg, 
	IN ULONG 			MsgLen);

VOID WfdCfgInit(
	IN PRTMP_ADAPTER pAd);

#endif /* WFD_SUPPORT */
#endif /* __WFD_H__ */

