#include "iocpclass.h"

IOCP_SERVER_CLASS::IOCP_SERVER_CLASS()
{
	IOCP_Server_SetServerIP();
	CheckCPU();
	IOCP_Server_Initialize();
	IOCP_Server_SetWork_and_AcceptThread();
}

IOCP_SERVER_CLASS::~IOCP_SERVER_CLASS()
{
	WSACleanup();
}

void IOCP_SERVER_CLASS::IOCP_Server_SetServerIP()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	PHOSTENT hostInfo;
	char hostName[50];
	char ipaddr[50];

	memset(hostName, 0, sizeof(hostName));
	memset(ipaddr, 0, sizeof(ipaddr));

	int errorResult = gethostname(hostName, sizeof(hostName));
	if (errorResult == 0) {
		hostInfo = gethostbyname(hostName);
		strcpy_s(ipaddr, inet_ntoa(*reinterpret_cast<struct in_addr*>(hostInfo->h_addr_list[0])));
		
	}
	WSACleanup();
	printf("Server's IP : %s \n", ipaddr);

}

void IOCP_SERVER_CLASS::IOCP_Server_Initialize()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_Handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (g_Handle == NULL) {
		// IOCP_SERVER_ErrorQuit(L"IOCP_SERVER_CLASS::IOCP_Server_Initialize", WSAGetLastError());
	}
}

void IOCP_SERVER_CLASS::CheckCPU()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_cpuCount = static_cast<int>(si.dwNumberOfProcessors) * 2;
	printf("CPU Core Count = %d, threads = %d \n", m_cpuCount / 2, m_cpuCount);

}

void IOCP_SERVER_CLASS::IOCP_Server_SetWork_and_AcceptThread()
{
	m_worker_thread.reserve(m_cpuCount);
	for (int i = 0; i < m_cpuCount; ++i) {
		m_worker_thread.push_back(new thread{ &IOCP_SERVER_CLASS::WorkThread, this });
	}

	thread acceptThread{ &IOCP_SERVER_CLASS::AcceptThread, this };
	while (m_Shoutdown) {
		Sleep(1000);
	}

	for (auto thread : m_worker_thread) {
		thread->join();
		delete thread;
	}
	acceptThread.join();
}

void IOCP_SERVER_CLASS::WorkThread()
{
	while ((!m_Shoutdown) == TRUE) {
		DWORD key;
		DWORD iosize; 
		OVERLAP_EX *overlap = nullptr;

		BOOL result = GetQueuedCompletionStatus(g_Handle, &iosize, &key, reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);

		// 접속이 끊겼을 때 처리
		if (result == FALSE || iosize) {
			if (result == FALSE) {
				int err_no = WSAGetLastError();
				IOCP_SERVER_ErrorDisplay("WorkThread, GetQueuedCompletionStatus Error : ", err_no, __LINE__);
			}
			closesocket(m_Sessions[key]->socket());
			// m_Sessions[key]->connection = false;
			printf("[ No. %3u ] Discconected . \n");

			continue;
		}
		else if (OP_SERVER_RECV) {
			// 소켓을 찾는부분에서 lock 걸자.
			
			Packet *buf_ptr = m_Sessions[key]->recv_overlap.iocp_buffer;
			int remained = iosize;
			while (0 < remained) {
				if (0 == m_Sessions[key]->packet_size) { m_clients[key]->packet_size = buf_ptr[0]; }



				int required = m_clients[key]->packet_size - m_clients[key]->previous_size;

				if (remained >= required) {
					memcpy(m_clients[key]->packet_buff + m_clients[key]->previous_size, buf_ptr, required);

					// 아래 함수에서 패킷을 처리하게 된다.
					IOCP_SERVER_ProcessPacket(key, m_clients[key]->packet_buff);

					buf_ptr += required;
					remained -= required;

					m_clients[key]->packet_size = 0;
					m_clients[key]->previous_size = 0;
				}
				else { // 패킷이 완성될 때.
					memcpy(m_clients[key]->packet_buff + m_clients[key]->previous_size, buf_ptr, remained);
					buf_ptr += remained;
					m_clients[key]->previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			int retval = WSARecv(m_Sessions[key]->sock, &m_clients[key]->recv_overlap.wsabuf, 1, NULL, &flags, &m_clients[key]->recv_overlap.original_overlapped, NULL);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) {
					IOCP_SERVER_ErrorDisplay("WorkerThreadStart::WSARecv", err_no, __LINE__);
				}
				continue;
			}
		}

	}

}

void IOCP_SERVER_CLASS::AcceptThread()
{

	// 소켓 연결할때 세션 연동.
	// 세션에 버퍼를 만들자.
	int retval{ 0 };

	// socket() - IPv4 ( AF_INET )
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) {
		int err_no = WSAGetLastError();
		IOCP_SERVER_ErrorQuit(L"socket()", err_no);
	};

	// bind()
	struct sockaddr_in serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	retval = ::bind(listen_sock, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		IOCP_SERVER_ErrorQuit(L"socket()", err_no);
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		IOCP_SERVER_ErrorQuit(L"socket()", err_no);
	}

	while (TRUE == (!m_Shoutdown)) {
		// accept()
		//struct sockaddr_in clientaddr;
		//int addrlen = sizeof(clientaddr);
		//SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		//if (INVALID_SOCKET == client_sock) {
		//	int err_no = WSAGetLastError();
		//	IOCP_SERVER_ErrorDisplay("Accept::WSAAccept", err_no, __LINE__);
		//	while (true);
		//}

		///* DB 관련 login 기능이 여기에 추가되어야 한다. 로그인이 번호가 제대로 맞으면 통과, 아니면 클라이언트 연결을 끊는다. */

		//m_playerindex += 1;
		//printf("[ No. %3u ] Client IP = %s, Port = %d is Connected\n", m_playerindex, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_Handle, m_playerindex, 0);

		//SESSION_INFO *user = new SESSION_INFO;

		//user->sock = client_sock;
		//user->connection = true;
		//user->id = m_playerindex;
		//user->packet_size = 0;
		//user->previous_size = 0;
		//memset(&user->recv_overlap.original_overlapped, 0, sizeof(user->recv_overlap.original_overlapped));
		//user->recv_overlap.ioType = IO_WRITE;
		//user->recv_overlap.wsabuf.buf = reinterpret_cast<char*>(&user->recv_overlap.iocp_buffer);
		//user->recv_overlap.wsabuf.len = sizeof(user->recv_overlap.iocp_buffer);

		//// 추가로 데이터가 필요한 경우라면, PLAYER_INFO 구조체 내용을 수정하고,
		//// 추가로 초기화 값이나 불러오는 내용을 대입하여 넣어주어야 한다.

		//m_clients.push_back(move(user));

		//// 클라이언트에서 응답오길 기다리기
		//DWORD flags{ 0 };
		//retval = WSARecv(client_sock, &m_clients[m_playerindex]->recv_overlap.wsabuf, 1, NULL, &flags, &m_clients[m_playerindex]->recv_overlap.original_overlapped, NULL);
		//if (SOCKET_ERROR == retval) {
		//	int err_no = WSAGetLastError();
		//	if (ERROR_IO_PENDING != err_no) {
		//		IOCP_SERVER_ErrorDisplay("Accept::WSARecv", err_no, __LINE__);
		//	}
		//}
	}
}

void IOCP_SERVER_CLASS::SendPacket(unsigned int id, const Packet *packet)
{
	OVERLAP_EX* overlap = new OVERLAP_EX;
	memset(overlap, 0, sizeof(OVERLAP_EX));
	overlap->ioType = IO_READ;
	overlap->wsabuf.buf = reinterpret_cast<char*>(overlap->ioType);
	overlap->wsabuf.len = packet[0];
	memcpy(overlap->iocp_buffer, packet, packet[0]);

	DWORD flags{ 0 };
	int returnValue = WSASend(m_clients[id]->sock, &overlap->wsabuf, 1, NULL, flags, &overlap->original_overlapped, NULL);
	if (returnValue == SOCKET_ERROR) {
		if (WSAGetLastError() == ERROR_IO_PENDING) {
			IOCP_SERVER_ErrorDisplay("SendPacket, WSASend Error : ", WSAGetLastError(), __LINE__);
			while (true);
		}
	}
}


void IOCP_SERVER_CLASS::IOCP_SERVER_ErrorDisplay(string msg, int err_no, int line)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	std::printf("[ %s - %d ]", msg, line);
	wprintf(L"에러 %s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void IOCP_SERVER_CLASS::IOCP_SERVER_ErrorQuit(LPCWSTR msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (wchar_t*) msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

