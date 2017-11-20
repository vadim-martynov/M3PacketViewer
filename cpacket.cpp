#include "cpacket.h"


CPacket::CPacket()
{

}

void CPacket::UnPackHeader (CPacketHeader* pHeader, const unsigned char* pBuff)
{

    //pHeader->nSignature1  = (quint64)pBuff[0];
    pHeader->nSignature1  = pBuff[0];
    pHeader->nSignature2  = pBuff[1];
    pHeader->nSignature3  = pBuff[2];

    pHeader->bData = (pBuff[3] >> 7) & 1;
    pHeader->nType = (pBuff[3] >> 5) & 3;

    unsigned int byte1 = (pBuff[3]) & 31;
    unsigned int byte2 = (pBuff[4] >> 1) & 127;
    pHeader->nId = (byte1 << 7) + byte2;

    pHeader->nSubId = pBuff[4] & 1;

    pHeader->nDevId = (pBuff[5] >> 7) & 1;
    pHeader->bAct = (pBuff[5] >> 6) & 1;
    pHeader->bSignal = (pBuff[5] >> 5) & 1;
    pHeader->bListen = (pBuff[5] >> 4) & 1;
    pHeader->nHWversion = (pBuff[5] >> 3) & 1;
    pHeader->nRes = (pBuff[5] >> 1) & 3;
    pHeader->bOOS = (pBuff[5]) & 1;
    byte1 = pBuff[6];
    byte2 = pBuff[7];

    pHeader->nCount = (byte1 << 8) + byte2;

}

void CPacket::UnPackControl ( CControlPacket2* pPacket, const unsigned char* pBuff)
{
	UnPackHeader ( &pPacket->m_Header, pBuff );

	switch ( pPacket->m_Header.nType )
	{
	case VCSS_TYPE_ULTRA:
		{
			pPacket->m_Ctrl.t0.nUltraType = pBuff[0 + sizeof(CPacketHeader)];
			switch(pPacket->m_Ctrl.t0.nUltraType)
			{
			case VCSS_TYPE_ULTRA_REC:
				{
					pPacket->m_Ctrl.t0.body.record.nIP0=0;
					pPacket->m_Ctrl.t0.body.record.nIP1=0;
					for (size_t i=0; i<4; ++i)
					{
						pPacket->m_Ctrl.t0.body.record.nIP0 += ((unsigned int)pBuff[1+sizeof(CPacketHeader)+i])<<(i*8);
						pPacket->m_Ctrl.t0.body.record.nIP1 += ((unsigned int)pBuff[5+sizeof(CPacketHeader)+i])<<(i*8);
					}

					pPacket->m_Ctrl.t0.body.record.nChannelsNumber=pBuff[9+sizeof(CPacketHeader)];

					for (size_t i=0; i<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; ++i)
					{
						const unsigned int byte1=pBuff[10+sizeof(CPacketHeader)+i*2];
						const unsigned int byte2=pBuff[11+sizeof(CPacketHeader)+i*2];
						pPacket->m_Ctrl.t0.body.record.channels[i].nId= (byte1<<4) | (byte2>>4)&0xF;
						pPacket->m_Ctrl.t0.body.record.channels[i].nType=(byte2>>2)&0x03;
						pPacket->m_Ctrl.t0.body.record.channels[i].bRadioMain=(byte2>>1)&0x01;
						pPacket->m_Ctrl.t0.body.record.channels[i].bRes=byte2&0x01;
					}
					break;
				}
			case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
				{
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micPhoneActive		=	(pBuff[1+sizeof(CPacketHeader)]>>5) & 0x07;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micRadioActive		=	(pBuff[1+sizeof(CPacketHeader)]>>2) & 0x07;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset1Present		=	(pBuff[1+sizeof(CPacketHeader)]>>1) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset2Present		=	(pBuff[1+sizeof(CPacketHeader)]   ) & 0x01;

					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolPhone		=	(pBuff[2+sizeof(CPacketHeader)]>>1) & 0x7F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolRadio		=	(pBuff[2+sizeof(CPacketHeader)]<<6) & 0x40 |
																								(pBuff[3+sizeof(CPacketHeader)]>>2) & 0x3F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMechTangentaSlot	=	(pBuff[3+sizeof(CPacketHeader)]<<2) & 0x0C |
																								(pBuff[4+sizeof(CPacketHeader)]>>6) & 0x03;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bExtraLoud			=	(pBuff[4+sizeof(CPacketHeader)]>>5) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bSelfListen			=	(pBuff[4+sizeof(CPacketHeader)]>>4) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bDefaultSettings		=	(pBuff[4+sizeof(CPacketHeader)]>>3) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bWorkBtnPushed		=	(pBuff[4+sizeof(CPacketHeader)]>>2) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bOperativeCall		=	(pBuff[4+sizeof(CPacketHeader)]>>1) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bFaceIsDying			=	(pBuff[4+sizeof(CPacketHeader)]   ) & 0x01;

					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset1		=	(pBuff[5+sizeof(CPacketHeader)]>>1) & 0x7F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset2		=	(pBuff[5+sizeof(CPacketHeader)]<<6) & 0x40 |
																								(pBuff[6+sizeof(CPacketHeader)]>>2) & 0x3F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHandset			=	(pBuff[6+sizeof(CPacketHeader)]<<5) & 0x60|
																								(pBuff[7+sizeof(CPacketHeader)]>>3) & 0x1F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneOutDev		=	(pBuff[7+sizeof(CPacketHeader)]<<1) & 0x0E |
																								(pBuff[8+sizeof(CPacketHeader)]>>7) & 0x01;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneVol		=	(pBuff[8+sizeof(CPacketHeader)]   ) & 0x7F;

					for (size_t i=0; i<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; ++i)
					{
						const unsigned int byte1=pBuff[9+sizeof(CPacketHeader)+sizeof(CUltraInterfaceTerminalDiagCommonPhoneOutDevVol)*i  ];
						const unsigned int byte2=pBuff[9+sizeof(CPacketHeader)+sizeof(CUltraInterfaceTerminalDiagCommonPhoneOutDevVol)*i+1];
						const unsigned int byte3=pBuff[9+sizeof(CPacketHeader)+sizeof(CUltraInterfaceTerminalDiagCommonPhoneOutDevVol)*i+2];
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nId	=	(byte1 << 3) | (byte2>>5)&0x07;

						pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwOutDev	= (byte2>>1)&0x0F;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwRes1	= (byte2   )&0x01;

						pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwVol	= (byte3>>1)&0x7F;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nRes2	= (byte3   )&0x01;
					}

					const unsigned int byte1=pBuff[24+sizeof(CPacketHeader)];
					const unsigned int byte2=pBuff[25+sizeof(CPacketHeader)];
					const unsigned int byte3=pBuff[26+sizeof(CPacketHeader)];
					const unsigned int byte4=pBuff[27+sizeof(CPacketHeader)];
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDivert				=	(byte1 << 4) | (byte2>>4)&0x0F;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwRingVol			=	((byte2&0x0F) << 3) | (byte3 >> 5)&0x07;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwBrightness			=	((byte3&0x1F) << 2) | (byte4 >> 6)&0x03;
					pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bRxAdd				=	( byte4 >> 5 ) & 0x01;

					break;
				}
			case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
				{
					for (size_t i=0; i<VCSS_MAX_RADIOS_ON_WP; ++i)
					{
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotVol		=	(pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)]>>1) & 0x7F;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotOutDev	=	(pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)]<<3) & 0x08 |
								(pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)+1]>>5) & 0x07;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotBSS		= (pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)+1]>>4) & 0x01;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotLOTS		= (pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)+1]>>3) & 0x01;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotAllTx		= (pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)+1]>>2) & 0x01;
						pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].nSlotRetranslation		= (pBuff[1+sizeof(CPacketHeader)+i*sizeof(CUltraInterfaceTerminalDiagRadioSlot)+1]   ) & 0x03;
					}
					break;
				}
			case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
				{
					pPacket->m_Ctrl.t0.body.permanentDiag.nPermanentDiagType=pBuff[1+sizeof(CPacketHeader)];
					switch (pPacket->m_Ctrl.t0.body.permanentDiag.nPermanentDiagType)
					{
						case VCSS_PERMANENT_DIAG_TERMINAL_E1:
						{
                            unsigned int byte1=pBuff[2+sizeof(CPacketHeader)];
                            unsigned int byte2=pBuff[3+sizeof(CPacketHeader)];
                            unsigned int byte3=pBuff[4+sizeof(CPacketHeader)];
                            unsigned int byte4=pBuff[5+sizeof(CPacketHeader)];
                            pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.dwLineStatus	= (byte1<<24)|(byte2<<16)|(byte3<<8)|byte4;

                            byte1=pBuff[6+sizeof(CPacketHeader)];
                            byte2=pBuff[7+sizeof(CPacketHeader)];
                            byte3=pBuff[8+sizeof(CPacketHeader)];
                            byte4=pBuff[9+sizeof(CPacketHeader)];
                            pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bInCAS	=(byte1>>7)&0x01;
                            pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bOutCAS	=(byte1>>6)&0x01;
                            pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bChannelNoReady = (byte1>>5)&0x01;
                            pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bRes2	=((byte1&0x1F)<<24)|(byte2<<16)|(byte3<<8)|byte4;
                            break;
                        }
					}
					break;
				}
			case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
				{
					pPacket->m_Ctrl.t0.body.terminalExtensions.nExtensionType=pBuff[1+sizeof(CPacketHeader)];
					switch(pPacket->m_Ctrl.t0.body.terminalExtensions.nExtensionType)
					{
						case VCS_TERMINAL_EXTENSION_LISTEN:
						{
							for (size_t i=0; i<VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM; ++i)
							{
								unsigned short byte1=pBuff[2+sizeof(CPacketHeader)+i*2];
								unsigned short byte2=pBuff[2+sizeof(CPacketHeader)+i*2+1];
								pPacket->m_Ctrl.t0.body.terminalExtensions.body.terminalExtensionsListen.narListenId[i]=(byte1<<8)|byte2;
							}
							break;
						}
					}
					break;
				}
			case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
				{
					const unsigned int byte1=pBuff[1+sizeof(CPacketHeader)];
					const unsigned int byte2=pBuff[2+sizeof(CPacketHeader)];
					const unsigned int byte3=pBuff[3+sizeof(CPacketHeader)];

					pPacket->m_Ctrl.t0.body.divertGroups.nTotalAliases	= byte1;
					pPacket->m_Ctrl.t0.body.divertGroups.nDivertDst		= (byte2<<4)&0x0FF0 | (byte3>>4)&0x000F;
					pPacket->m_Ctrl.t0.body.divertGroups.nRes			= (byte3   )&0x000F;

					for (size_t i=0; i<VCSS_DIVERT_GROUP_SIZE; ++i)
					{
						const unsigned int byte1Alias=pBuff[4+sizeof(CPacketHeader)+i*2  ];
						const unsigned int byte2Alias=pBuff[4+sizeof(CPacketHeader)+i*2+1];
						pPacket->m_Ctrl.t0.body.divertGroups.Aliases[i] = (byte1Alias<<8)&0xFF00 | (byte2Alias)&0x00FF;
					}
					break;
				}
            case VCSS_TYPE_ULTRA_LOG_SIP:
                {
                    pPacket->m_Ctrl.t0.body.logSIP.bWasProcessed	= pBuff[1+sizeof(CPacketHeader)]	&0x01;
                    pPacket->m_Ctrl.t0.body.logSIP.bIn				=(pBuff[1+sizeof(CPacketHeader)]>>1)&0x01;

                    const unsigned int byte1=pBuff[3+sizeof(CPacketHeader)];
                    const unsigned int byte2=pBuff[4+sizeof(CPacketHeader)];
                    pPacket->m_Ctrl.t0.body.logSIP.nDataSize = (byte1 << 8) | byte2;
                    break;
                }
            }
			break;
		}
	case VCSS_TYPE_PHONE:
		{
			pPacket->m_Ctrl.t1.nLevel = (pBuff[0 + sizeof(CPacketHeader)] >> 1) & 127;

			unsigned int byte1 = pBuff[0 + sizeof(CPacketHeader)] & 1;
			unsigned int byte2 = pBuff[1 + sizeof(CPacketHeader)];
			unsigned int byte3 = (pBuff[2 + sizeof(CPacketHeader)] >> 5) & 7;

			pPacket->m_Ctrl.t1.nListenId = (byte1 << 11) + (byte2 << 3) + byte3;
			pPacket->m_Ctrl.t1.nGroupSign = (pBuff[2 + sizeof(CPacketHeader)] >> 4) & 1;

			byte1 = (pBuff[2 + sizeof(CPacketHeader)]) & 15;
			byte2 = (pBuff[3 + sizeof(CPacketHeader)]);

			pPacket->m_Ctrl.t1.nGroup = (byte1 << 8) + byte2;

			byte1 = (pBuff[4 + sizeof(CPacketHeader)]);
			byte2 = (pBuff[5 + sizeof(CPacketHeader)] >> 4) & 15;

			pPacket->m_Ctrl.t1.nDestId = byte2 + (byte1 << 4);
			pPacket->m_Ctrl.t1.nCallType = (pBuff[5 + sizeof(CPacketHeader)] >> 2) & 3;
			pPacket->m_Ctrl.t1.bIntr = (pBuff[5 + sizeof(CPacketHeader)] >> 1) & 1;
			pPacket->m_Ctrl.t1.bHalfDuplex = (pBuff[5 + sizeof(CPacketHeader)]) & 1;
			pPacket->m_Ctrl.t1.nConnType = (pBuff[6 + sizeof(CPacketHeader)] >> 5) & 7;
			pPacket->m_Ctrl.t1.nCmd = (pBuff[6 + sizeof(CPacketHeader)]) & 31;

			pPacket->m_Ctrl.t1.nPriority = (pBuff[7 + sizeof(CPacketHeader)] >> 4) & 15;
			pPacket->m_Ctrl.t1.bBusy = (pBuff[7 + sizeof(CPacketHeader)] >> 3) & 1;
			pPacket->m_Ctrl.t1.nRes3 = (pBuff[7 + sizeof(CPacketHeader)]) & 7;

			pPacket->m_Ctrl.t1.dwlENA = 0;
			for (int i = 0; i < 8; i++)
			{
				unsigned char byte = pBuff[8 + sizeof(CPacketHeader) + i];
				pPacket->m_Ctrl.t1.dwlENA += ((DWORDLONG)byte << ((7 - i) * 8));
			}

			pPacket->m_Ctrl.t1.nIP0 = 0;
			for (int i = 0; i < 4; i++)
			{
				unsigned long byte = pBuff[16 + sizeof(CPacketHeader) + i];
				pPacket->m_Ctrl.t1.nIP0 += (byte << (i * 8));
			}

			pPacket->m_Ctrl.t1.nIP1 = 0;
			for (int i = 0; i < 4; i++)
			{
				unsigned long byte = pBuff[20 + sizeof(CPacketHeader) + i];
				pPacket->m_Ctrl.t1.nIP1 += (byte << (i * 8));
			}

			byte1 = pBuff[24 + sizeof(CPacketHeader)];
			byte2 = pBuff[25 + sizeof(CPacketHeader)];

			pPacket->m_Ctrl.t1.nHWstate = (byte1 << 8) + byte2;

			byte1 = pBuff[26 + sizeof(CPacketHeader)];
			byte2 = (pBuff[27 + sizeof(CPacketHeader)] >> 4) & 15;
			pPacket->m_Ctrl.t1.nBusy = (byte1 << 4) + byte2;

			pPacket->m_Ctrl.t1.bNeedEC = (pBuff[27 + sizeof(CPacketHeader)] >> 3) & 1;
			pPacket->m_Ctrl.t1.bCallQueue = (pBuff[27 + sizeof(CPacketHeader)] >> 2) & 1;
			pPacket->m_Ctrl.t1.bInDivert = (pBuff[27 + sizeof(CPacketHeader)] >> 1) & 1;

			pPacket->m_Ctrl.t1.nRes2 = (pBuff[27 + sizeof(CPacketHeader)]) & 1;
			break;
		}

	case VCSS_TYPE_RADIO:
		{
			pPacket->m_Ctrl.t2.bRxEnable = (pBuff[0 + sizeof(CPacketHeader)] >> 7) & 1;

			pPacket->m_Ctrl.t2.bTxEnable = (pBuff[0 + sizeof(CPacketHeader)] >> 6) & 1;

			pPacket->m_Ctrl.t2.nRxMS = (pBuff[0 + sizeof(CPacketHeader)] >> 4) & 3;

			pPacket->m_Ctrl.t2.bTxMS = (pBuff[0 + sizeof(CPacketHeader)] >> 3) & 1;

			pPacket->m_Ctrl.t2.bSquelsh = (pBuff[0 + sizeof(CPacketHeader)] >> 2) & 1;

			unsigned int byte1 = (pBuff[0 + sizeof(CPacketHeader)]) & 3;

			unsigned int byte2 = (pBuff[1 + sizeof(CPacketHeader)]);

			unsigned int byte3 = (pBuff[2 + sizeof(CPacketHeader)] >> 2) & 63;

			pPacket->m_Ctrl.t2.nFreqInt = (byte1 << 14) + (byte2 << 6) + (byte3);

			byte1 = (pBuff[2 + sizeof(CPacketHeader)]) & 3;

			byte2 = (pBuff[3 + sizeof(CPacketHeader)]);

			pPacket->m_Ctrl.t2.nFreqFract = (byte1 << 8) + byte2;

			byte1 = (pBuff[4 + sizeof(CPacketHeader)]);

			byte2 = (pBuff[5 + sizeof(CPacketHeader)] >> 4) & 15;

			pPacket->m_Ctrl.t2.nOwner = (byte1 << 4) + byte2;

			pPacket->m_Ctrl.t2.bDegradation = (pBuff[5 + sizeof(CPacketHeader)] >> 3) & 1;

			pPacket->m_Ctrl.t2.nRes0 = (pBuff[5 + sizeof(CPacketHeader)]) & 7;

			pPacket->m_Ctrl.t2.nCmd = (pBuff[6 + sizeof(CPacketHeader)] >> 3) & 31;

			byte1 = pBuff[6 + sizeof(CPacketHeader)] & 7;

			byte2 = (pBuff[7 + sizeof(CPacketHeader)] >> 4) & 15;

			pPacket->m_Ctrl.t2.nLevel = (byte1 << 4) + byte2;

			pPacket->m_Ctrl.t2.nRes1 = (pBuff[7 + sizeof(CPacketHeader)]) & 15;

			for (int i = 0; i < 8; i++)
			{
				pPacket->m_Ctrl.t2.cName[i] = pBuff[8 + sizeof(CPacketHeader) + i];

			}

			pPacket->m_Ctrl.t2.nIP0 = 0;
			for (int i = 0; i < 4; i++)
			{
				unsigned long byte = pBuff[16 + sizeof(CPacketHeader) + i];
				pPacket->m_Ctrl.t2.nIP0 += (byte << (i * 8));
			}

			pPacket->m_Ctrl.t2.nIP1 = 0;
			for (int i = 0; i < 4; i++)
			{
				unsigned long byte = pBuff[20 + sizeof(CPacketHeader) + i];
				pPacket->m_Ctrl.t2.nIP1 += (byte << (i * 8));
			}

			byte1 = pBuff[24 + sizeof(CPacketHeader)];
			byte2 = pBuff[25 + sizeof(CPacketHeader)];

			pPacket->m_Ctrl.t2.nHWstate = (byte1 << 8) + byte2;

			pPacket->m_Ctrl.t2.nQOS = pBuff[26 + sizeof(CPacketHeader)];

			pPacket->m_Ctrl.t2.nRes2 = pBuff[27 + sizeof(CPacketHeader)];

			break;
		}

	case VCSS_TYPE_TERMINAL:
		{
			pPacket->m_Ctrl.t3.bPriority = (pBuff[0 + sizeof(CPacketHeader)] >> 7) & 1;

			pPacket->m_Ctrl.t3.nCmd = (pBuff[0 + sizeof(CPacketHeader)] >> 2) & 31;

			unsigned int byte1 = pBuff[0 + sizeof(CPacketHeader)] & 3;
			unsigned int byte2 = pBuff[1 + sizeof(CPacketHeader)];
			unsigned int byte3 = pBuff[2 + sizeof(CPacketHeader)];
			unsigned int byte4 = pBuff[3 + sizeof(CPacketHeader)];

			pPacket->m_Ctrl.t3.nData = (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4;

			int nOffset = 4 + sizeof(CPacketHeader);
			for (int i = 0; i < VCSS_MAX_RADIOS_ON_WP; i++)
			{
				pPacket->m_Ctrl.t3.Channels[i].nId = pBuff[nOffset];

				pPacket->m_Ctrl.t3.Channels[i].bTransmit = (pBuff[nOffset + 1] >> 7) & 1;

				pPacket->m_Ctrl.t3.Channels[i].nRxMS = (pBuff[nOffset + 1] >> 5) & 3;

				pPacket->m_Ctrl.t3.Channels[i].bTxMS = (pBuff[nOffset + 1] >> 4) & 1;

				pPacket->m_Ctrl.t3.Channels[i].nCmd = (pBuff[nOffset + 1] >> 3) & 1;

				pPacket->m_Ctrl.t3.Channels[i].bRx = (pBuff[nOffset + 1] >> 2) & 1;

				pPacket->m_Ctrl.t3.Channels[i].bTx = (pBuff[nOffset + 1] >> 1) & 1;

                pPacket->m_Ctrl.t3.Channels[i].bEmergency = (pBuff[nOffset + 1]) & 1;

				nOffset += 2;
			}










			break;
		}
	};
}



CPacketHeader CPacket::UnPackHeaderD(const unsigned char *pBuff)
{
        CPacketHeader header;
        header.nSignature1  = (quint64)pBuff[0];
        header.nSignature2  = pBuff[1];
        header.nSignature3  = pBuff[2];

        header.bData = (pBuff[3] >> 7) & 1;
        header.nType = (pBuff[3] >> 5) & 3;

        unsigned int byte1 = (pBuff[3]) & 31;
        unsigned int byte2 = (pBuff[4] >> 1) & 127;
        header.nId = (byte1 << 7) + byte2;

        header.nSubId = pBuff[4] & 1;

        header.nDevId = (pBuff[5] >> 7) & 1;
        header.bAct = (pBuff[5] >> 6) & 1;
        header.bSignal = (pBuff[5] >> 5) & 1;
        header.bListen = (pBuff[5] >> 4) & 1;
        header.nHWversion = (pBuff[5] >> 3) & 1;
        header.nRes = (pBuff[5] >> 1) & 3;
        header.bOOS = (pBuff[5]) & 1;
        byte1 = pBuff[6];
        byte2 = pBuff[7];

        header.nCount = (byte1 << 8) + byte2;

        return header;
}
