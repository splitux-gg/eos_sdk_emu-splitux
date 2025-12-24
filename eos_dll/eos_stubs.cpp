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

// Stub implementations for missing EOS SDK functions
// These allow games using newer SDK versions to load without crashing

#include "common_includes.h"

// ============================================================================
// RTCAudio Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyParticipantUpdated(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyParticipantUpdatedOptions* Options, void* ClientData, const EOS_RTCAudio_OnParticipantUpdatedCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyParticipantUpdated(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioDevicesChanged(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioDevicesChangedOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioDevicesChangedCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioDevicesChanged(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioInputState(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioInputStateOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioInputStateCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioInputState(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioOutputState(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioOutputStateOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioOutputStateCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioOutputState(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeSend(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioBeforeSendOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioBeforeSendCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeSend(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeRender(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioBeforeRenderOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioBeforeRenderCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeRender(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SendAudio(EOS_HRTCAudio Handle, const EOS_RTCAudio_SendAudioOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateSending(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateSendingOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateSendingCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_UpdateSendingCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateReceiving(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateReceivingOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateReceivingCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_UpdateReceivingCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateSendingVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateSendingVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateSendingVolumeCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_UpdateSendingVolumeCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateReceivingVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateReceivingVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateReceivingVolumeCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_UpdateReceivingVolumeCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateParticipantVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateParticipantVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateParticipantVolumeCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_UpdateParticipantVolumeCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioInputSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetAudioInputSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioOutputSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetAudioOutputSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetInputDeviceSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetInputDeviceSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetOutputDeviceSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetOutputDeviceSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioInputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioInputDevicesCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioOutputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioOutputDevicesCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetInputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetInputDevicesCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetOutputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetOutputDevicesCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_CopyInputDeviceInformationByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_CopyInputDeviceInformationByIndexOptions* Options, EOS_RTCAudio_InputDeviceInformation** OutInputDeviceInformation)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_CopyOutputDeviceInformationByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_CopyOutputDeviceInformationByIndexOptions* Options, EOS_RTCAudio_OutputDeviceInformation** OutOutputDeviceInformation)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(const EOS_RTCAudio_AudioInputDeviceInfo*) EOS_RTCAudio_GetAudioInputDeviceByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioInputDeviceByIndexOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(const EOS_RTCAudio_AudioOutputDeviceInfo*) EOS_RTCAudio_GetAudioOutputDeviceByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioOutputDeviceByIndexOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_InputDeviceInformation_Release(EOS_RTCAudio_InputDeviceInformation* InputDeviceInformation)
{
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_OutputDeviceInformation_Release(EOS_RTCAudio_OutputDeviceInformation* OutputDeviceInformation)
{
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_QueryInputDevicesInformation(EOS_HRTCAudio Handle, const EOS_RTCAudio_QueryInputDevicesInformationOptions* Options, void* ClientData, const EOS_RTCAudio_OnQueryInputDevicesInformationCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_QueryInputDevicesInformationCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_QueryOutputDevicesInformation(EOS_HRTCAudio Handle, const EOS_RTCAudio_QueryOutputDevicesInformationOptions* Options, void* ClientData, const EOS_RTCAudio_OnQueryOutputDevicesInformationCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAudio_QueryOutputDevicesInformationCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_RegisterPlatformUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_RegisterPlatformUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UnregisterPlatformUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_UnregisterPlatformUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_RegisterPlatformAudioUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_RegisterPlatformAudioUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UnregisterPlatformAudioUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_UnregisterPlatformAudioUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// RTC Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyDisconnected(EOS_HRTC Handle, const EOS_RTC_AddNotifyDisconnectedOptions* Options, void* ClientData, const EOS_RTC_OnDisconnectedCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyDisconnected(EOS_HRTC Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyParticipantStatusChanged(EOS_HRTC Handle, const EOS_RTC_AddNotifyParticipantStatusChangedOptions* Options, void* ClientData, const EOS_RTC_OnParticipantStatusChangedCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyParticipantStatusChanged(EOS_HRTC Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyRoomStatisticsUpdated(EOS_HRTC Handle, const EOS_RTC_AddNotifyRoomStatisticsUpdatedOptions* Options, void* ClientData, const EOS_RTC_OnRoomStatisticsUpdatedCallback CompletionDelegate)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyRoomStatisticsUpdated(EOS_HRTC Handle, EOS_NotificationId NotificationId)
{
}

// ============================================================================
// RTCAdmin Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_QueryJoinRoomToken(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_QueryJoinRoomTokenOptions* Options, void* ClientData, const EOS_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAdmin_QueryJoinRoomTokenCompleteCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByIndex(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByIndexOptions* Options, EOS_RTCAdmin_UserToken** OutUserToken)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByUserId(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByUserIdOptions* Options, EOS_RTCAdmin_UserToken** OutUserToken)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_UserToken_Release(EOS_RTCAdmin_UserToken* UserToken)
{
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_Kick(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_KickOptions* Options, void* ClientData, const EOS_RTCAdmin_OnKickCompleteCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAdmin_KickCompleteCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_SetParticipantHardMute(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_SetParticipantHardMuteOptions* Options, void* ClientData, const EOS_RTCAdmin_OnSetParticipantHardMuteCompleteCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_RTCAdmin_SetParticipantHardMuteCompleteCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// Audio Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_HAudioInputStream) EOS_Audio_CreateNewInputStream(EOS_HAudio Handle, const EOS_Audio_CreateNewInputStreamOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(EOS_HAudioOutputStream) EOS_Audio_CreateNewOutputStream(EOS_HAudio Handle, const EOS_Audio_CreateNewOutputStreamOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(void) EOS_Audio_DestroyInputStream(EOS_HAudio Handle, EOS_HAudioInputStream Stream)
{
}

EOS_DECLARE_FUNC(void) EOS_Audio_DestroyOutputStream(EOS_HAudio Handle, EOS_HAudioOutputStream Stream)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_GetInputDeviceInfo(EOS_HAudio Handle, const EOS_Audio_GetInputDeviceInfoOptions* Options, EOS_Audio_InputDeviceInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_GetOutputDeviceInfo(EOS_HAudio Handle, const EOS_Audio_GetOutputDeviceInfoOptions* Options, EOS_Audio_OutputDeviceInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_GetInputStreamInfo(EOS_HAudio Handle, EOS_HAudioInputStream Stream, EOS_Audio_InputStreamInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_GetOutputStreamInfo(EOS_HAudio Handle, EOS_HAudioOutputStream Stream, EOS_Audio_OutputStreamInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Audio_IsInputStreamDeviceDisconnected(EOS_HAudio Handle, EOS_HAudioInputStream Stream)
{
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Audio_IsInputStreamSilent(EOS_HAudio Handle, EOS_HAudioInputStream Stream)
{
    return EOS_TRUE;
}

EOS_DECLARE_FUNC(void) EOS_Audio_QueryInputDevices(EOS_HAudio Handle, const EOS_Audio_QueryInputDevicesOptions* Options, void* ClientData, const EOS_Audio_OnQueryInputDevicesCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Audio_QueryInputDevicesCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Audio_QueryOutputDevices(EOS_HAudio Handle, const EOS_Audio_QueryOutputDevicesOptions* Options, void* ClientData, const EOS_Audio_OnQueryOutputDevicesCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Audio_QueryOutputDevicesCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_RegisterUser(EOS_HAudio Handle, const EOS_Audio_RegisterUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_UnregisterUser(EOS_HAudio Handle, const EOS_Audio_UnregisterUserOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Audio_SetNotifyDevicesChanged(EOS_HAudio Handle, const EOS_Audio_SetNotifyDevicesChangedOptions* Options, void* ClientData, const EOS_Audio_OnDevicesChangedCallback Callback)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Audio_RemoveNotifyDevicesChanged(EOS_HAudio Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_SetFeatureEnabledForInputStream(EOS_HAudio Handle, EOS_HAudioInputStream Stream, const EOS_Audio_SetFeatureEnabledForInputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_StartInputStream(EOS_HAudio Handle, EOS_HAudioInputStream Stream, const EOS_Audio_StartInputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_StopInputStream(EOS_HAudio Handle, EOS_HAudioInputStream Stream, const EOS_Audio_StopInputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_StartOutputStream(EOS_HAudio Handle, EOS_HAudioOutputStream Stream, const EOS_Audio_StartOutputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_StopOutputStream(EOS_HAudio Handle, EOS_HAudioOutputStream Stream, const EOS_Audio_StopOutputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Audio_EnableCommunicationsModeOutputDevices(EOS_HAudio Handle, const EOS_Audio_EnableCommunicationsModeOutputDevicesOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// BroadcastAudio Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_HBroadcastAudioInputStream) EOS_BroadcastAudio_CreateNewInputStream(EOS_HBroadcastAudio Handle, const EOS_BroadcastAudio_CreateNewInputStreamOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(EOS_HBroadcastAudioOutputStream) EOS_BroadcastAudio_CreateNewOutputStream(EOS_HBroadcastAudio Handle, const EOS_BroadcastAudio_CreateNewOutputStreamOptions* Options)
{
    return nullptr;
}

EOS_DECLARE_FUNC(void) EOS_BroadcastAudio_DestroyInputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream)
{
}

EOS_DECLARE_FUNC(void) EOS_BroadcastAudio_DestroyOutputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioOutputStream Stream)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_GetInputStreamInfo(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream, EOS_BroadcastAudio_InputStreamInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_GetOutputStreamInfo(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioOutputStream Stream, EOS_BroadcastAudio_OutputStreamInfo* OutInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(float) EOS_BroadcastAudio_GetCurrentGainLevel(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream)
{
    return 1.0f;
}

EOS_DECLARE_FUNC(float) EOS_BroadcastAudio_GetCurrentMicAmplitude(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream)
{
    return 0.0f;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_PushPacketToOutputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioOutputStream Stream, const EOS_BroadcastAudio_PushPacketToOutputStreamOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_SetEncoderSettings(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream, const EOS_BroadcastAudio_SetEncoderSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_SetMicProcessingSettings(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream, const EOS_BroadcastAudio_SetMicProcessingSettingsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_StartInputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_StopInputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioInputStream Stream)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_StartOutputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioOutputStream Stream)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_BroadcastAudio_StopOutputStream(EOS_HBroadcastAudio Handle, EOS_HBroadcastAudioOutputStream Stream)
{
    return EOS_EResult::EOS_Success;
}
