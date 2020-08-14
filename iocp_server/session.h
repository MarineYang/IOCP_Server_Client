#pragma once

#include "stdafx.h"

struct SOCKET_DATA {
	SOCKET socket_;
	SOCKADDR_IN addrInfo_;
};


typedef enum IO_TYPE
{
	IO_READ,
	IO_WRITE,
};
using OVERLAP_EX = struct Overlap_ex {
	OVERLAPPED original_overlapped;
	// int operation;
	IO_TYPE ioType; // operation
	WSABUF wsabuf;
	Packet iocp_buffer[MAX_BUF_SIZE];
};

//enum {
//	SESSION_TYPE_CLIENT,
//};

class Session
{
protected:
	SOCKET_DATA socketData_;
	unsigned int id_;
	int8_t	type_;
	time_t lastHeartbeat_;
	OVERLAP_EX recv_overlap;
	int packet_size;
	int previous_size;

	bool setSocketOpt();

public:
	Session();
	virtual ~Session();
	virtual bool onAccept(SOCKET socket, SOCKADDR_IN addrInfo);
	virtual void onSend(size_t transferSize) = 0;
	virtual void sendPacket(Packet *packet) = 0;
	virtual void onRecv(size_t transferSize) = 0;
	virtual void onClose(bool force = false);
	

	SOCKET&	socket();

	int GetSessionId();
	void setSessionID(unsigned int id);

	OVERLAP_EX& GetOverlapped();
	int GetPacketSize();
	int GetPreviousSize();

	int8_t GetSessionType();

	time_t Heartbeat();
	void updateHeartbeat();


	void SetPacketSize(int size);
	void SetPreviosSize(int size);


};