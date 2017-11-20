
#ifndef __PACKET2_H
#define __PACKET2_H

#include "adjust.h"



typedef BYTE CVoiceData[VCSS_FRAME_DATA_SIZE];


struct CPacketHeader
//
// Часть заголовка пакета, общая для всех типов
//
{
    DWORDLONG				nSignature1		:8;		// сигнатура пакета (3 байта)
    DWORDLONG				nSignature2		:8;
    DWORDLONG				nSignature3		:8;
// 3
    DWORDLONG				bData			:1;		// голосовой (1) или управляющий (0) пакет.
    DWORDLONG				nType			:2;		// тип: телефония или радиосвязь, или дополнительный
    DWORDLONG				nId				:12;	// идентификатор отправителя (индивидуальный или групповой).
    DWORDLONG				nSubId			:1;		// номер полукомплекта отправителя.
// 5
    DWORDLONG				nDevId			:1;		// для радиостанции: основная/резервная,
                                                    // для тлф: основное/вспомогательное устройства
    DWORDLONG				bAct			:1;		// при резервировании: этот полукомплект сейчас основной
    DWORDLONG				bSignal			:1;		// признак обнаружения полезного сигнала.
                                                    // Если выставлен bListen (пакеты прослушивающему) - признак активности (соединение или работа в эфир)
    DWORDLONG				bListen			:1;		// признак прослушивания
    DWORDLONG				nHWversion		:1;		// версия железа: 0 - Мега 2, 1 - Мега 3
    DWORDLONG				nRes			:2;		// не используется
    DWORDLONG				bOOS			:1;		// признак выведения из работы
// 6
    DWORDLONG				nCount			:16;	// счетчик.
// 8
};

const unsigned VCSS_P_HEADER_SIZE = 8;

///////////////////////////////////////////////////////////////////////////////
//
//	UltraPacket
//

//	1 - RecordPacket

struct CUltraRecordChannel
{
    unsigned short			nId:12;
    unsigned short			nType:2;
    unsigned short			bRadioMain:1;
    unsigned short			bRes:1;
// 2
} __attribute__((packed));

const unsigned VCSS_P_ULTRA_RECORD_CHANNELS_NUM = 9;

struct CUltraRecord	// +1
{
    DWORD nIP0;
// 4
    DWORD nIP1;
// 8
    BYTE nChannelsNumber;
// 9
    CUltraRecordChannel channels[VCSS_P_ULTRA_RECORD_CHANNELS_NUM];
// 27
} __attribute__((packed));

///////////////////////////////////////////////////////////////////////////////

//	2,3 - InterfaceTerminalDiag

struct CUltraInterfaceTerminalDiagCommonPhoneOutDevVol
{
    WORD	nId				:11;// если 0 (настольный микрофон), ячейка считается незанятой
    WORD	dwOutDev		:4;	// если 0 (настольный микрофон), ячейка считается незанятой
    WORD	dwRes1			:1;
// 2
    BYTE	dwVol			:7;
    BYTE	nRes2			:1;
// 3
} __attribute__((packed));

const unsigned VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM = 5;
const unsigned VCSS_P_ULTRA_TERM_DIAG_COMMON_REPEATS = 5;	// сколько раз повторять бит умолчательных настроек, кнопки "Работа"
struct CUltraInterfaceTerminalDiagCommon	// +1
{
    DWORD	micPhoneActive	:3;	// номер устройства типа VCSS_DEV_* из InterfaceTerminalExchange
    DWORD	micRadioActive	:3;	// эти коды совпадают с VCOutDevs из VCParser.h
    DWORD	bHeadset1Present:1;
    DWORD	bHeadset2Present:1;
// 1
    DWORD	dwMasterVolPhone:7;	// dB без минуса. Если положительное число, то 127 соответствует +1, 126 - +2 и т.д.
    DWORD	dwMasterVolRadio:7;	// в качестве границы использовать -71 dB
    DWORD	dwMechTangentaSlot:4;	// номер слота, на который назначена механическая групповая тангента
    DWORD	bExtraLoud		:1;
    DWORD	bSelfListen		:1;
    DWORD	bDefaultSettings:1;
    DWORD	bWorkBtnPushed	:1;
    DWORD	bOperativeCall	:1;
    DWORD	bFaceIsDying	:1;
// 4
    DWORD	dwVolHeadset1	:7;
    DWORD	dwVolHeadset2	:7;
    DWORD	dwVolHandset	:7;
    DWORD	dwDefPhoneOutDev:4;
    DWORD	dwDefPhoneVol	:7;
// 8
    CUltraInterfaceTerminalDiagCommonPhoneOutDevVol abonents[VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM];
// 23
    DWORD	dwDivert		:12;
    DWORD	dwRingVol		:7;	//+70dB
    DWORD	dwBrightness	:7;
    DWORD	bRxAdd			:1;
    DWORD	bPTTBlocked		:1;
    DWORD	dwRes			:4;
// 27
} __attribute__((packed));

struct CUltraInterfaceTerminalDiagRadioSlot
{
    WORD	dwSlotVol			: 7;	// смотри выше про громкость
    WORD	dwSlotOutDev		: 4;	// номер устройства типа VCSS_DEV_* из InterfaceTerminalExchange
                                        // эти коды совпадают с VCOutDevs из VCParser.h. 0 - слот не используется
    WORD	bSlotBSS			: 1;
    WORD	bSlotLOTS			: 1;
    WORD	bSlotAllTx			: 1;
    WORD	nSlotRetranslation	: 2;	// смотри eCouplingMode в RSGroup
    // 2
} __attribute__((packed));

struct CUltraInterfaceTerminalDiagRadio
{
    CUltraInterfaceTerminalDiagRadioSlot m_slots[VCSS_MAX_RADIOS_ON_WP];
    // 24
    BYTE	dwRes1;
// 25
    BYTE	dwRes2;
// 26
    BYTE	dwRes3;
// 27
} __attribute__((packed));

//	4 - PermanentDiag

struct CUltraPermanentDiagTerminalE1
{
    DWORD	dwLineStatus;
//	4
    DWORD	bInCAS	: 1;
    DWORD	bOutCAS	: 1;
    DWORD	bChannelNoReady : 1;
    DWORD	bRes2	: 29;
//	8
    char	nRes1[18];
//	26
}__attribute__((packed));

union CUltraPermanentDiagBody
{
    CUltraPermanentDiagTerminalE1 permanentDiagTerminalE1;
//	26
}__attribute__((packed));

enum CControlPacketUltraPermanentDiagTypes
{
    VCSS_PERMANENT_DIAG_TERMINAL_E1		= 1
};

struct CUltraPermanentDiag
{
    BYTE			nPermanentDiagType;
//	1
    CUltraPermanentDiagBody body;
//	27
}__attribute__((packed));
///////////////////////////////////////////////////////////////////////////////
const unsigned VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM	= 6;
struct CUltraTerminalExtensionListen
{
    unsigned short narListenId[VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM];
//	12
    unsigned char res[14];
//	26
}__attribute__((packed));

union CUltraTerminalExtensionsBody
{
    CUltraTerminalExtensionListen	terminalExtensionsListen;
//	26
}__attribute__((packed));

enum CControlPacketUltraTerminalExtensionsTypes
{
    VCS_TERMINAL_EXTENSION_LISTEN		= 1
};

struct CUltraTerminalExtensions
{
    BYTE			nExtensionType;
//	1
    CUltraTerminalExtensionsBody body;
//	27
}__attribute__((packed));
///////////////////////////////////////////////////////////////////////////////
#define VCSS_DIVERT_GROUP_SIZE		12
struct CUltraDivertGroups
{
    BYTE		nTotalAliases;
    WORD		nDivertDst		: 12;
    WORD		nRes			: 4;
//	3
    WORD		Aliases[VCSS_DIVERT_GROUP_SIZE];
//	27
}__attribute__((packed));
///////////////////////////////////////////////////////////////////////////////
#define VCSS_LOG_SIP_LOAD			2048
struct CUltraLogSip
{
    WORD		bWasProcessed	: 1;
    WORD		bIn				: 1;	// 0 - out, 1 - in
    WORD		nRes1			: 14;
//	2
    WORD		nDataSize;
//	4
//
//	27
}__attribute__((packed));
///////////////////////////////////////////////////////////////////////////////

union CUltraPacketBody	// +1
{
    CUltraRecord						record;
    CUltraInterfaceTerminalDiagCommon	interaceTerminalDiagCommon;
    CUltraInterfaceTerminalDiagRadio	interaceTerminalDiagRadio;
    CUltraPermanentDiag					permanentDiag;
    CUltraTerminalExtensions			terminalExtensions;
    CUltraDivertGroups					divertGroups;
    CUltraLogSip						logSIP;
// 27
}__attribute__((packed));

enum CControlPacketUltraTypes
{
    VCSS_TYPE_ULTRA_REC								= 1,
    VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON	= 2,
    VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO	= 3,
    VCSS_TYPE_ULTRA_PERMANENT_DIAG					= 4,
    VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS				= 5,
    VCSS_TYPE_ULTRA_DIVERT_GROUPS					= 6,
    VCSS_TYPE_ULTRA_LOG_SIP							= 11
};

struct CControlPacketT0
{
    BYTE					nUltraType;
// 1
    CUltraPacketBody body;
// 28
} __attribute__((packed));

//
//	UltraPacket
//
///////////////////////////////////////////////////////////////////////////////

struct CControlPacketT1
//
// Заголовок управляющего пакета для телефонии.
//
{
    DWORDLONG				nLevel 			:7;   // уровень (для графика)
    DWORDLONG				nListenId		:12;  // кого слушаем
    DWORDLONG				nGroupSign		:1;   // добавить/удалить идентификатор дин. группы
    DWORDLONG				nGroup 			:12;  // идентификатор динамической группы
// 4
//
// Поле адреса назначения
//
    DWORDLONG				nDestId			:12;
//
//
// Поле команд и атрибутов запрашиваемого соединения.
//
    DWORDLONG				nCallType		:2;		// тип вызова: оперативное, прямое, косвенное.
    DWORDLONG				bIntr			:1;		// признак вторжения.
    DWORDLONG				bHalfDuplex		:1;		// дуплекс (0) или полудуплекс (1).
// 6
    DWORDLONG				nConnType		:3;		// тип соединения.

    DWORDLONG				nCmd			:5;		// команда.
// 7
    DWORDLONG				nPriority		:4;   // приоритет абонента
    DWORDLONG				bBusy			:1;		// признак занятости
    DWORDLONG				nRes3			:3;   // не используется
// 8
//
// Поле номера для внешней сети (двоично/десятичное).
//
    DWORDLONG				dwlENA;
// 16
    DWORDLONG				nIP0			:32;  // IP-адреса отправителя пакета
// 20
    DWORDLONG				nIP1			:32;
// 24
    DWORD						nHWstate	:16;	// состояние железа, интерпретируется в зависимости от nHWversion
                                                    // для nHWversion = 0: 1 соответствует ошибке в плате,
                                                    // правый бит - правая плата (порты 0,8,16,24)
                                                    // для nHWversion = 1: пока в разработке
                                                    ////////////////////////////////////////////////////////////
                                                    // для рабочего места информация о нажатии мех.тангент
                                                    // младший байт - 8 настольных тангент
                                                    // затем 2 бита - нажатия на гарнитурах
                                                    // 3-й бит - нажатие тангенты ТТ, 4-й бит - поднятие ТТ
                                                    ///////////////////////////////////////////////////////////
    DWORD					nBusy			:12;	// идентификатор получателя признака занятости
    DWORD					bNeedEC		:1;	// необходимость эхокомпенсатора
    DWORD					bCallQueue		:1;	// р/м имеет очередь вызовов и ему можно посылать вызов,
                                                // даже если оно занято
    DWORD					bInDivert		:1;	// у нас включен Divert - на нас делать Divert нельзя
    DWORD					nRes2			:1;	// не используется
//28
} __attribute__((packed));

enum
{
    VCSS_PCMD_NONE			= 0,
    VCSS_PCMD_OUTGOINGCALL,	// 1
    VCSS_PCMD_INCOMINGCALL,	// 2
    VCSS_PCMD_PICKCALL,		// 3
    VCSS_PCMD_CONNECTED,	// 4
    VCSS_PCMD_HOLD,			// 5
    VCSS_PCMD_SUSPEND,		// 6
    VCSS_PCMD_INTERCEPT,	// 7
    VCSS_PCMD_TRANSFER,		// 8
    VCSS_PCMD_DIVERT,		// 9
    VCSS_PCMD_EXT_DISCONNECT// 10
};

struct CControlPacketT2
//
// Заголовок управляющего пакета для радиосвязи, сторона радиостанции.
//
{
    DWORDLONG				bRxEnable	:1;		// приемник доступен.
    DWORDLONG				bTxEnable	:1;		// передатчик доступен.

    DWORDLONG				nRxMS		:2;		// основной (01)/резервный (10)/оба (11) приемники включены.
    DWORDLONG				bTxMS		:1;		// основной (1)/резервный (0) передатчик включен.

    DWORDLONG				bSquelsh	:1;		// прием.

    DWORDLONG				nFreqInt	:16;	// частота, на которой работает станция.
    DWORDLONG				nFreqFract	:10;
// 4
    DWORDLONG				nOwner		:12;	// id передающего р/м.
    DWORDLONG				bDegradation:1;		// нельзя удалять
    DWORDLONG				nRes0       :3;		// не используется

    DWORDLONG				nCmd        :5;		// исполняемая в данный момент команда

    DWORDLONG				nLevel		:7;		// уровень для графика
    DWORDLONG				nRes1      	:4;		// состояние бинарных входов (для диагностики)
// 8
    char					cName[8];			// имя станции (для отображения на колонке)
// 16
    DWORDLONG				nIP0		:32;  // IP-адреса отправителя пакета
// 20
    DWORDLONG				nIP1		:32;
// 24
    DWORD					nHWstate	:16;	// состояние железа, интерпретируется в зависимости от nHWversion
                                                // для nHWversion = 0: 1 соответствует ошибке в плате,
                                                // правый бит - правая плата (порты 0,8,16,24)
                                                // для nHWversion = 1: пока в разработке
                                                ////////////////////////////////////////////////////////////
                                                // для интерфейса Тангента+
                                                // младший байт, младший бит - неисправность линии
                                                // младший байт, следующий бит - неисправность р/ст

#ifdef VCSS_BRANCH_ONE
  DWORD           nRes2       :16;
#endif //VCSS_BRANCH_ONE

#ifdef VCSS_BRANCH_TWO
  // VMBx:
  DWORD           nQOS        :8;   // качество сигнала
  // :VMBx
  DWORD           nRes2       :8;
#endif //VCSS_BRANCH_TWO
//28
} __attribute__((packed));


struct CRadioChannelCmd
//
// Управляющая информация для одного радиоканала.
//
{
    BYTE						nId			:8;		// идентификатор выбранного радиоканала.
// 1
    BYTE						bTransmit	:1;		// ТАНГЕНТА.

    BYTE						nRxMS		:2;		// включить основной (01)/резервный (10)/оба(11) приемника.
    BYTE						bTxMS		:1;		// включить основной (1)/резервный (0) передатчик.

    BYTE						nCmd		:1;		// получатель команды
    BYTE						bRx			:1;		// признак включенного на р/м приема - чтобы не гонять зря пакеты
    BYTE						bTx			:1;		// признак включенного на передачу - для документирования операций на РМ
    BYTE						bEmergency	:1;
// 2
} __attribute__((packed));

struct CControlPacketT3
//
// Заголовок управляющего пакета для радиосвязи, сторона терминала.
//
{
    DWORD						bPriority		:1;		// приоритет доступа к радиосвязи.

    DWORD						nCmd				:5;		// команда одному из каналов.
    DWORD						nData				:26;	// данные к команде (напр. новая частота)
// 4
    CRadioChannelCmd	Channels[VCSS_MAX_RADIOS_ON_WP];	// 2x12=24 bytes.
// 28
};

const unsigned VCSS_P_CTRL_SIZE = 36;
const unsigned VCSS_P_LOG_SIP_MAX_SIZE = VCSS_LOG_SIP_LOAD + VCSS_P_CTRL_SIZE;

enum
{
    VCSS_RCMD_NONE = 0,
    VCSS_RCMD_FREQ,		// установить частоту р/ст (в nData (nFreqInt << 10) | nFreqFract)
    VCSS_RCMD_CHANNEL,	// выбрать частотный канал р/ст (в nData номер канала)
    VCSS_RCMD_SELCAL,	// послать SELCAL (код в nData, см SELCAL.h)
    VCSS_RCMD_ICOM,		// частотный интерком: признак того, что идет работа частотного интеркома
    VCSS_RCMD_COUPLING,	// уведомление того, что идет передача по причине каплинга
};

struct CVoicePacket2
//
// Голосовой пакет.
//
{
    CPacketHeader			m_Header;					// общий заголовок.
    CVoiceData				m_Data;						// голосовые данные.
};

const unsigned VCSS_P_VOICE_SIZE = 72;

struct CControlPacket2
//
// Управляющий пакет.
//
{
    CPacketHeader			m_Header;					// общий заголовок.

    union
    {
        CControlPacketT0		t0;
        CControlPacketT1		t1;
        CControlPacketT2		t2;
        CControlPacketT3		t3;
    }
        m_Ctrl;														// специальный заголовок.
} __attribute__((packed));

#endif //__PACKET2_H











