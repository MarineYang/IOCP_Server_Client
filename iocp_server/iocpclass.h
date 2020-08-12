#pragma once
#include "stdafx.h"
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

using SESSION_INFO = struct Client_INFO {
	SOCKET sock;
	unsigned int id;
	char ipAddr[16];
	bool connection;
	OVERLAP_EX recv_overlap;
	int packet_size;
	int previous_size;
	Packet packet_buff[MAX_BUF_SIZE];
};

class IOCP_SERVER_CLASS
{
private:
	HANDLE g_Handle;
	int m_cpuCount;

	vector<thread*> m_worker_thread;
	bool m_Shoutdown{ false };

	vector<SESSION_INFO*> m_clients;
	unsigned int m_playerindex{ UINT_MAX };

public:
	IOCP_SERVER_CLASS();
	~IOCP_SERVER_CLASS();

	void IOCP_Server_SetServerIP();
	void IOCP_Server_Initialize();
	void IOCP_Server_SetWork_and_AcceptThread();
	void CheckCPU();
	void WorkThread();
	void AcceptThread();
	void SendPacket(unsigned int id, const Packet *packet);
	void IOCP_SERVER_ErrorDisplay(string msg, int err_no, int line);
	void IOCP_SERVER_ErrorQuit(LPCWSTR msg, int err_no);
	void IOCP_SERVER_ProcessPacket(const unsigned int& id, const Packet buf[]);


};