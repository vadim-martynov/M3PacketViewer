
#ifndef __CONST_H
#define __CONST_H

#include "adjust.h"
#include "common_header.h"

#ifdef MEG3
const unsigned VCSS_VERSION_MAJOR			= 3;
#endif

#ifdef KSRPAMEG4
const unsigned VCSS_VERSION_MAJOR			= 4;
#endif

#ifdef VCSS_BRANCH_ONE
const unsigned VCSS_VERSION_MINOR			= 1;
const unsigned VCSS_VERSION_FIX				= 18;
#endif //VCSS_BRANCH_ONE

#ifdef VCSS_BRANCH_TWO
#ifdef MEG3
const unsigned VCSS_VERSION_MINOR			= 6;
const unsigned VCSS_VERSION_FIX				= 99;
#endif
#ifdef KSRPA
const unsigned VCSS_VERSION_MINOR			= 0;
const unsigned VCSS_VERSION_FIX				= 18;
#endif
#endif //VCSS_BRANCH_TWO

const unsigned VCSS_MAX_TYPES				= 3;			// 0 is wrong!
const unsigned VCSS_MAX_IDS_PHONE			= 4096;   // space for aliases!
const unsigned VCSS_MAX_IDS_RADIO			= 255;
const unsigned VCSS_MAX_ID_PHONE			= 1024;
const unsigned VCSS_MAX_ID_RADIO			= 255;
const unsigned VCSS_STAT_ALIAS_MASK			= 0x800;
//	Минимальный номер статического алиаса
const unsigned VCSS_STAT_ALIAS_START_NUM	= VCSS_STAT_ALIAS_MASK;
//	Общее число статических алиасов
const unsigned VCSS_STAT_ALIAS_ALL_NUM		= VCSS_MAX_IDS_PHONE-VCSS_STAT_ALIAS_START_NUM;

////AnT!
const unsigned VCSS_MAX_ALARM_CALL_BUTTONS   = 6;
const unsigned VCSS_MAX_ALARM_CALL_ABONENTS  = 32;
/////

const unsigned VCSS_SIGNATURE_1				= 'V';
const unsigned VCSS_SIGNATURE_2				= 'C';
const unsigned VCSS_SIGNATURE_3				= '4';

const int VCSS_FRAME_DATA_TIME				= IO_PERIOD;
const int VCSS_FRAME_DATA_SIZE				= IO_PERIOD*SAMPLE_FREQ;
const int VCSS_DISKRET_ON_FRAME				= IO_PERIOD/2;

const int VCSS_E1_BYTE_SYNC   = 0;        // местоположение синхробайта во фрейме E1
const int VCSS_E1_BYTE_CTRL   = 16;       // местоположение байта данных во фрейме E1

/*const int VCSS_TIME_FOR_SLEEP	= 1;
const int VCSS_SLEEP_ATTEMPTS	= 8;
const int VCSS_MAX_SLEEP		= 50;*/

const unsigned VCSS_DEF_CONTROL_DIVIDE	= 4;
const unsigned VCSS_DEF_UPDATE_DIVIDE	= 1;
const unsigned VCSS_DEF_ITERM_DIAG_DIVIDE = 32;	// делитель для отправки диагностикиинтерфейс терминала
                                                // относительно отправки управляющих пакетов - 1024мс.
const unsigned VCSS_DEF_DIVERT_GROUP_DIVIDE = 32;	// делитель для отправки пакетов о переводе
                                                    // групповых номеров - 1024мс
const unsigned VCSS_DEF_ALARM_ABONENT_STATE_DIVIDE = 32;	// делитель для отправки состояния абонентов аварийного оповещения
                                                            // относительно отправки управляющих пакетов - 1024мс.

const unsigned VCSS_TYPE_ULTRA			= 0;
const unsigned VCSS_TYPE_PHONE			= 1;
const unsigned VCSS_TYPE_RADIO			= 2;
const unsigned VCSS_TYPE_TERMINAL		= 3;

const unsigned VCSS_CALLTYPE_INDIRECT	= 1;
const unsigned VCSS_CALLTYPE_DIRECT		= 2;
const unsigned VCSS_CALLTYPE_OPERATIVE	= 3;

const unsigned VCSS_CONNTYPE_SIMPLE		= 1;
const unsigned VCSS_CONNTYPE_UNSELECT	= 2;
const unsigned VCSS_CONNTYPE_CIRCULAR	= 4;
const unsigned VCSS_CONNTYPE_CONF_NEW	= 3;
const unsigned VCSS_CONNTYPE_DPC		= 5;
const unsigned VCSS_CONNTYPE_IM_CONFMASTER_MASK	= 0x08;

const unsigned VCSS_MAX_RADIOS_ON_WP	= 12;

const unsigned VCSS_CODE_IO				= 1;
const unsigned VCSS_CODE_NET			= 2;
const unsigned VCSS_CODE_SWITCH			= 3;
const unsigned VCSS_CODE_IFACE			= 4;

const short 	IP_PORT_DATA 			= 10000;
const short 	IP_PORT_CTRL 			= 10001;

#ifdef VCSS_USE_MCAST_VOICE
const unsigned	IP_MCAST_RADIO_START_ADDR	= (234<<24)+(16<<16)+(1<<8);	// HostOrder, прибавлять id р/ст
const unsigned	IP_MCAST_PHONE_START_ADDR	= (234<<24)+(16<<16)+(10<<8);	// HostOrder, прибавлять id абонента
#endif

const unsigned VCSS_ALARM_CALL_OVERALL_ABONENTS_NUM	= 64*3;

typedef BYTE CVoiceFrame[VCSS_FRAME_DATA_SIZE];
typedef short CVoiceRaw[VCSS_FRAME_DATA_SIZE];
typedef DWORDLONG CBinaryFrame;

/// delay

const unsigned VCSS_DELAY_MAX_FRAMES	= 375;	// 3000 ms

/// delay

#endif //__CONST_H
