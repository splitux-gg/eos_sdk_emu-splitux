// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

enum { k_iRTCDataCallbackBase = 28000 };
// next free callback_id: k_iMetricsCallbackBase + 4

#define EOS_RTCData_SendDataOptions					   EOS_RTCData_SendDataOptions001
#define EOS_RTCData_AddNotifyParticipantUpdatedOptions EOS_RTCData_AddNotifyParticipantUpdatedOptions001
#define EOS_RTCData_AddNotifyDataReceivedOptions	   EOS_RTCData_AddNotifyDataReceivedOptions001
#define EOS_RTCData_UpdateSendingOptions               EOS_RTCData_UpdateSendingOptions001
#define EOS_RTCData_UpdateReceivingOptions             EOS_RTCData_UpdateReceivingOptions001

#include "eos_rtc_data_types1.16.4.h"

#define EOS_METRICS_BEGINPLAYERSESSION_API_LATEST		   EOS_RTCDATA_SENDDATA_API_001
#define EOS_RTCDATA_ADDNOTIFYPARTICIPANTUPDATED_API_LATEST EOS_RTCDATA_ADDNOTIFYPARTICIPANTUPDATED_API_001
#define EOS_RTCDATA_ADDNOTIFYDATARECEIVED_API_LATEST	   EOS_RTCDATA_ADDNOTIFYDATARECEIVED_API_001
#define EOS_RTCDATA_UPDATESENDING_API_LATEST			   EOS_RTCDATA_UPDATESENDING_API_001
#define EOS_RTCDATA_UPDATERECEIVING_API_LATEST             EOS_RTCDATA_UPDATERECEIVING_API_001