// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)



/**
 * EOS_UserInfo_CopyUserInfo is used to immediately retrieve user information for an account id, cached by a previous call to EOS_UserInfo_QueryUserInfo.
 * The following types are used to work with the API.
 */

 /** The most recent version of the EOS_UserInfo_CopyUserInfo API. */
#define EOS_USERINFO_COPYUSERINFO_API_002 2

/** A structure that contains the user information. These structures are created by EOS_UserInfo_CopyUserInfo and must be passed to EOS_UserInfo_Release. */
EOS_STRUCT(EOS_UserInfo002, (
	/** API Version: Set this to EOS_USERINFO_COPYUSERINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Account ID of the user */
	EOS_EpicAccountId UserId;
	/** The name of the owner's country. This may be null */
	const char* Country;
	/** The display name. This may be null */
	const char* DisplayName;
	/** The ISO 639 language code for the user's preferred language. This may be null */
	const char* PreferredLanguage;
	/** A nickname/alias for the target user assigned by the local user. This may be null */
	const char* Nickname;
));

/**
 * Input parameters for the EOS_UserInfo_CopyUserInfo function.
 */
EOS_STRUCT(EOS_UserInfo_CopyUserInfoOptions001, (
	/** API Version: Set this to EOS_USERINFO_COPYUSERINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Account ID of the local player requesting the information */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Account ID of the player whose information is being retrieved */
	EOS_EpicAccountId TargetUserId;
));

/** The most recent version of the EOS_UserInfo_ExternalUserInfo struct. */
#define EOS_USERINFO_EXTERNALUSERINFO_API_001 1

/**
 * Contains information about a single external user info.
 */
EOS_STRUCT(EOS_UserInfo_ExternalUserInfo001, (
	/** API Version: Set this to EOS_USERINFO_EXTERNALUSERINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The type of the external account */
	EOS_EExternalAccountType AccountType;
	/** The ID of the external account. Can be null */
	const char* AccountId;
	/** The display name of the external account. Can be null */
	const char* DisplayName;
));

#pragma pack(pop)
