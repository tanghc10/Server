// ListenSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ListenSocket.h"
#include "ServerView.h"
#include "SessionSocket.h"
#include "MainFrm.h"


// CListenSocket

CListenSocket::CListenSocket()
{
}

CListenSocket::~CListenSocket()
{
}


// CListenSocket 成员函数


void CListenSocket::OnAccept(int nErrorCode)
{
	CServerView* pView = (CServerView*)
		((CMainFrame*)AfxGetApp()->m_pMainWnd)
		->GetActiveView();

	CSessionSocket* pNewSocket = new CSessionSocket();

	// 接受连接,得到会话socket,
	int len = sizeof(SOCKADDR);
	if (Accept(*pNewSocket, (SOCKADDR *)&(pNewSocket->sockAddr), &len))
	{
		//为新得到的会话socket设置异步选择事件
		pNewSocket->AsyncSelect(FD_READ | FD_CLOSE);

		//将新得到的会话socket加到会话socket链表的末尾
		pView->m_pSessionList->AddTail(pNewSocket);
	}
	else
		delete pNewSocket;
	CAsyncSocket::OnAccept(nErrorCode);

}
