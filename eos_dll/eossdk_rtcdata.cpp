/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "eossdk_rtcdata.h"
#include "eossdk_platform.h"

namespace sdk
{

EOSSDK_RTCData::EOSSDK_RTCData()
{
    GetCB_Manager().register_frame(this);
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_RTCData::~EOSSDK_RTCData()
{}

EOS_NotificationId EOSSDK_RTCData::AddNotifyDataReceived(const EOS_RTCData_AddNotifyDataReceivedOptions* Options, const EOS_RTCData_OnDataReceivedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new  FrameResult);
    EOS_RTCData_DataReceivedCallbackInfo& lurci = res->CreateCallback<EOS_RTCData_DataReceivedCallbackInfo>((CallbackFunc)NotificationFn);

    return GetCB_Manager().add_notification(this, res);

}


///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_RTCData::CBRunFrame()
{
    GLOBAL_LOCK();
    return true;
}

bool EOSSDK_RTCData::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;;
}

void EOSSDK_RTCData::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
    case EOS_RTCData_ParticipantUpdatedCallbackInfo::k_iCallback:
    {
        EOS_RTCData_ParticipantUpdatedCallbackInfo& callback = res->GetCallback<EOS_RTCData_ParticipantUpdatedCallbackInfo>();
        // Free resources
    }
    break;

    case EOS_RTCData_DataReceivedCallbackInfo::k_iCallback:
    {
        EOS_RTCData_DataReceivedCallbackInfo& callback = res->GetCallback<EOS_RTCData_DataReceivedCallbackInfo>();
        // Free resources
    }
    break;

    case EOS_RTCData_UpdateSendingCallbackInfo::k_iCallback:
    {
        EOS_RTCData_UpdateSendingCallbackInfo& callback = res->GetCallback<EOS_RTCData_UpdateSendingCallbackInfo>();
        // Free resources
    }
    break;

    case EOS_RTCData_UpdateReceivingCallbackInfo::k_iCallback:
    {
        EOS_RTCData_UpdateReceivingCallbackInfo& callback = res->GetCallback<EOS_RTCData_UpdateReceivingCallbackInfo>();
        // Free resources
    }
    break;

    }
}

}