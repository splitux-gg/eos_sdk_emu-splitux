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

#include "eossdk_rtc_admin.h"
#include "eossdk_platform.h"

namespace sdk
{

EOSSDK_RTCAdmin::EOSSDK_RTCAdmin()
{
    GetCB_Manager().register_frame(this);
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_RTCAdmin::~EOSSDK_RTCAdmin()
{}


///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_RTCAdmin::CBRunFrame()
{
    GLOBAL_LOCK();
    return true;
}

bool EOSSDK_RTCAdmin::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;;
}

void EOSSDK_RTCAdmin::FreeCallback(pFrameResult_t res)
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