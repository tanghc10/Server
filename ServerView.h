
// ServerView.h : CServerView 类的接口
//

#pragma once
#include "ListenSocket.h"
#include "ServerDoc.h"
#include "afxwin.h"

class CServerView : public CFormView
{
protected: // 仅从序列化创建
	CServerView();
	DECLARE_DYNCREATE(CServerView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_SERVER_FORM };
#endif

//新加
public:
	CListenSocket *m_pListenSocket;//监听socket指针
	CPtrList *m_pSessionList; //会话Socket链表指针
	CString m_strName;		//连接名称

// 特性
public:
	CServerDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnInitialUpdate(); // 构造后第一次调用

// 实现
public:
	virtual ~CServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listData;
	CListBox m_userList;
	int GivenPort;
};

#ifndef _DEBUG  // ServerView.cpp 中的调试版本
inline CServerDoc* CServerView::GetDocument() const
   { return reinterpret_cast<CServerDoc*>(m_pDocument); }
#endif

