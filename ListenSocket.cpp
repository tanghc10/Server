// ListenSocket.cpp : ʵ���ļ�
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


// CListenSocket ��Ա����


void CListenSocket::OnAccept(int nErrorCode)
{
	CServerView* pView = (CServerView*)
		((CMainFrame*)AfxGetApp()->m_pMainWnd)
		->GetActiveView();

	CSessionSocket* pNewSocket = new CSessionSocket();

	// ��������,�õ��Ựsocket,
	int len = sizeof(SOCKADDR);
	if (Accept(*pNewSocket, (SOCKADDR *)&(pNewSocket->sockAddr), &len))
	{
		//Ϊ�µõ��ĻỰsocket�����첽ѡ���¼�
		pNewSocket->AsyncSelect(FD_READ | FD_CLOSE);

		//���µõ��ĻỰsocket�ӵ��Ựsocket�����ĩβ
		pView->m_pSessionList->AddTail(pNewSocket);
	}
	else
		delete pNewSocket;
	CAsyncSocket::OnAccept(nErrorCode);

}
