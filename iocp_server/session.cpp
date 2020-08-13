#include "stdafx.h"
#include "Session.h"

Session::Session()
{
	type_ = SESSION_TYPE_CLIENT;
	this->updateHeartbeat();
}

Session::~Session()
{
	this->onClose();
}

bool Session::setSocketOpt()
{
	// 세션 시간 적용하면서 소켓옵션 설정.
	tcp_keepalive keepAliveSet = { 0 }, returned = { 0 };
	keepAliveSet.onoff = 1;
	keepAliveSet.keepalivetime = 3000; // 3초 세션유지 시간.
	keepAliveSet.keepaliveinterval = 3000; // 응답없으면 3초후에 다시 보냄.

	DWORD dwByte;
	if (WSAIoctl(socketData_.socket_, SIO_KEEPALIVE_VALS, &keepAliveSet, sizeof(keepAliveSet), &returned, sizeof(returned), &dwByte, NULL, NULL)) {
		return false;
	}

}

bool Session::onAccept(SOCKET socket, SOCKADDR_IN addrInfo)
{
	// 데이터 세팅하고, 소켓옵션 설정.
	socketData_.socket_ = socket;
	int addrLen;
	getpeername(socketData_.socket_, (struct sockaddr*)&socketData_.addrInfo_, &addrLen);
	socketData_.addrInfo_ = addrInfo;
	if (!this->setSocketOpt())
	{
		return false;
	}
	return true;
}

void Session::onClose(bool force)
{
	// 세션매니저에서 force = true이면 매니저에서 강제종료.
	// 그렇지않으면 매니저에서 세션 종료.
	if (force) {
		SessionManageMent.forceCloseSession(this);
	}
	else
	{
		SessionManageMent.closeSession(this);
	}
}


SOCKET& Session::socket()
{
	return socketData_.socket_;
}

int Session::GetSessionId()
{
	return id_;
}

void Session::setSessionID(unsigned int id)
{
	id_ = id;
}

int8_t Session::GetSessionType()
{
	return type_;
}

time_t Session::Heartbeat()
{
	return lastHeartbeat_;
}

void Session::updateHeartbeat()
{
	lastHeartbeat_ = system_clock::to_time_t(system_clock::now());
}
