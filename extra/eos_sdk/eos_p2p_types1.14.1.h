// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#pragma pack(push, 8)

/** The most recent version of the EOS_P2P_SendPacket API. */
#define EOS_P2P_SENDPACKET_API_002 2

/**
 * Structure containing information about the data being sent and to which player
 */
EOS_STRUCT(EOS_P2P_SendPacketOptions002, (
	/** API Version: Set this to EOS_P2P_SENDPACKET_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is sending this packet */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the Peer you would like to send a packet to */
	EOS_ProductUserId RemoteUserId;
	/** The socket ID for data you are sending in this packet */
	const EOS_P2P_SocketId* SocketId;
	/** Channel associated with this data */
	uint8_t Channel;
	/** The size of the data to be sent to the RemoteUser */
	uint32_t DataLengthBytes;
	/** The data to be sent to the RemoteUser */
	const void* Data;
	/** If false and we do not already have an established connection to the peer, this data will be dropped */
	EOS_Bool bAllowDelayedDelivery;
	/** Setting to control the delivery reliability of this packet */
	EOS_EPacketReliability Reliability;
));

#pragma pack(pop)
