
// ServerView.h : CServerView ��Ľӿ�
//

#pragma once
#include "ListenSocket.h"
#include "ServerDoc.h"
#include "afxwin.h"

class CServerView : public CFormView
{
protected: // �������л�����
	CServerView();
	DECLARE_DYNCREATE(CServerView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_SERVER_FORM };
#endif

//�¼�
public:
	CListenSocket *m_pListenSocket;//����socketָ��
	CPtrList *m_pSessionList; //�ỰSocket����ָ��
	CString m_strName;		//��������

// ����
public:
	CServerDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnInitialUpdate(); // ������һ�ε���

// ʵ��
public:
	virtual ~CServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listData;
	CListBox m_userList;
	int GivenPort;
};

#ifndef _DEBUG  // ServerView.cpp �еĵ��԰汾
inline CServerDoc* CServerView::GetDocument() const
   { return reinterpret_cast<CServerDoc*>(m_pDocument); }
#endif

