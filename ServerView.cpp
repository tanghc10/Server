
// ServerView.cpp : CServerView 类的实现
//

#include "stdafx.h"
#include "afxwin.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Server.h"
#endif

#include "ServerDoc.h"
#include "ServerView.h"
#include "ListenSocket.h"
#include "ServerInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CServerView

IMPLEMENT_DYNCREATE(CServerView, CFormView)

BEGIN_MESSAGE_MAP(CServerView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CServerView 构造/析构

CServerView::CServerView()
	: CFormView(IDD_SERVER_FORM)
{
	GivenPort = 6000;
}

CServerView::~CServerView()
{
}

void CServerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listData);
	DDX_Control(pDX, IDC_LIST2, m_userList);
}

BOOL CServerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CFormView::PreCreateWindow(cs);
}

void CServerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	// 创建套接字
	m_pListenSocket = new CListenSocket();
	if (!m_pListenSocket)
	{
		AfxMessageBox(_T("动态创建服务器套接字出错!"));
		return ;
	}

	if (m_pListenSocket->Create(Server_port, SOCK_STREAM) == FALSE)
	{
		AfxMessageBox(_T("创建套接字失败!"), MB_OK | MB_ICONEXCLAMATION);
		m_pListenSocket->Close();
		return;
	}

	m_pSessionList = new CPtrList();
	if (!m_pSessionList)
	{
		AfxMessageBox(_T("创建会话表失败!"));
	}

	// 侦听成功，等待连接请求
	if (m_pListenSocket->Listen() == FALSE)
	{
		if (m_pListenSocket->GetLastError() == WSAEWOULDBLOCK)
		{
			AfxMessageBox(_T("网络侦听失败!"));//因为vs2005默认使用的是unicode字符编码集，而unicode要占2byte,通常的字符只占1byte,所以导致无法转换，故需要加上 _T("") 或 L"" 进行转换。
			m_pListenSocket->Close();
			return;
		}
	}
	AfxMessageBox(_T("服务器已启动!"));
}

void CServerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CServerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CServerView 诊断

#ifdef _DEBUG
void CServerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CServerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CServerDoc* CServerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CServerDoc)));
	return (CServerDoc*)m_pDocument;
}
#endif //_DEBUG


// CServerView 消息处理程序
