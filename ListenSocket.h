#pragma once

#include "Head.h"
// CListenSocket ÃüÁîÄ¿±ê

class CListenSocket : public CAsyncSocket
{
public:
	CListenSocket();
	virtual ~CListenSocket();
	virtual void OnAccept(int nErrorCode);
};


