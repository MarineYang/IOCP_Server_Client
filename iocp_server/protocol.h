#pragma once

#define PORT 9001
#define MAX_BUF_SIZE 256	
#define MAX_USER 500

// packet[1] operation
#define DISCONNECTED 0

// iocp buf operation
#define OP_SERVER_RECV 1
#define OP_SERVER_SEND 2

// process protocol
enum PacketProtocolType_Server_ProcessPacketFunction {
	TEST = 1,		// 받은 패킷 그대로 돌려주기용. ( 보낸 내용이 그대로 돌아오지 않는다면, 클라나 서버에 문제가 있다는 뜻 )
	HEARTBEAT,		// 클라이언트 세션 생명주기.

};
using Packet = unsigned char;