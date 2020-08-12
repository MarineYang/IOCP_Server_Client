#pragma once
// 수많은 세션들이 아주 빈번하게 추가와 삭제가 이루어지므로
// 세션들을 관리하는 매니저가 필요하다.

#include "stdafx.h"
#include "Session.h"


#define SESSION_CAPACITY (5000)
#define SessionManageMent SessionManager::GetInstance()

class SessionManager : public Singleton<SessionManager>
{
	list<Session*> sessionList_;
	int sessiontCount_;
	int maxConnCount_;

	mutex mutex_;
	int8_t sessionSeed_;

public:
	SessionManager(int maxConnCount = SESSION_CAPACITY);
	~SessionManager();

	int8_t createSessionID();
	bool addSession(Session *session);

	list<Session*>& GetSessionList();
	bool closeSession(Session *session);
	void forceCloseSession(Session *session);
	Session *session(int8_t id);
};