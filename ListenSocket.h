#pragma once

// CListenSocket ÃüÁîÄ¿±ê

class CListenSocket : public CSocket
{
public:
	CListenSocket();
	virtual ~CListenSocket();
	virtual void OnAccept(int nErrorCode);
};


