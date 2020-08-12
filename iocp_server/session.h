#pragma once

#include "stdafx.h"

struct SOCKET_DATA {
	SOCKET socket_;
	SOCKADDR_IN addrInfo_;
};

enum {
	SESSION_TYPE_CLIENT,
};

class Session
{
protected:
	SOCKET_DATA socketData_;
	unsigned int id_;
	int8_t	type_;
	time_t lastHeartbeat_;

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

	int8_t GetSessionType();

	time_t Heartbeat();
	void updateHeartbeat();

};