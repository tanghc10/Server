// SessionSocket.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "SessionSocket.h"
#include "ServerView.h"
#include "MainFrm.h"
#include "afxdb.h"
#include "cJSON.h"
#include <stdlib.h>


// CSessionSocket

CSessionSocket::CSessionSocket()
	:m_strName(_T(""))
{
	// TODO: �ڴ˴���ӹ������

}

CSessionSocket::~CSessionSocket()
{
}


// CSessionSocket ��Ա����

//���ͻ��ر�����ʱ���¼���Ӧ����
void CSessionSocket::OnClose(int nErrorCode)
{
	//��View����ʾ��Ϣ
	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S �û���");
	strTime = strTime + this->m_strName + _T(" �뿪\r\n");
	pView->m_listData.AddString(strTime);
	int ps = pView->m_userList.FindString(0, this->m_strName);
	pView->m_userList.DeleteString(ps);
	pView->m_pSessionList->RemoveAt(pView->m_pSessionList->Find(this));
	//���ķ�������������
	CString str1 = this->Update_ServerLog();
	//֪ͨ�ͻ���ˢ����������
	this->UpDate_ClientUser(str1, "0");
	this->Close();
	//���ٸ��׽���
	delete this;
	CAsyncSocket::OnClose(nErrorCode);
}

//���յ�����ʱ���¼���Ӧ����
void CSessionSocket::OnReceive(int nErrorCode)
{
	//�Ƚ��պʹ�����Ϣͷ
	HEADER head;
	int head_len = sizeof(HEADER);
	char *pHead = NULL;	//���ڽ�����Ϣͷ
	pHead = new char[head_len];
	if (!pHead) {
		TRACE0("CSessionSocket::OnReceive �ڴ����ʧ��");
		return;
	}
	memset(pHead, 0, head_len*sizeof(char)); //����ͷ�ĳ�ʼ��
	Receive(pHead, head_len);
	
	head.type = ((HEADER *)pHead)->type;
	head.nContentLen = ((HEADER *)pHead)->nContentLen;
	memset(head.to_user, 0, sizeof(head.to_user));
	strcpy(head.to_user, ((HEADER *)pHead)->to_user);
	memset(head.from_user, 0, sizeof(head.from_user));
	strcpy(head.from_user, ((HEADER *)pHead)->from_user);
	delete pHead;	//������ɺ��ͷ�ָ��
	pHead = NULL;

	//������Ϣͷ�е���Ϣ���պ���������
	pHead = new char[head.nContentLen];
	if (!pHead) {
		TRACE0("CSessionSocket::OnReceive �ڴ����ʧ��");
		return;
	}
	memset(pHead, 0, head.nContentLen * sizeof(char));
	//�򵥲�����:������ܵ������ݵĳ��Ⱥ���Ϣͷ�еĸ��������ݳ��Ȳ�ͬ�Ļ������Խ��ܣ�ֱ�����
	if (Receive(pHead, head.nContentLen) != head.nContentLen) {
		delete pHead;
		return;
	}

	//������Ϣ���ͣ��ַ�����ͬ�Ĵ�����
	////////////������Ϣ���ͣ���������,���Ҳ�ǺͿͻ��˽��ж�Ӧ�ġ��������MSG_LOGOIN��MSG_SEND�Ƕ���õĳ���������F12����////////////////////
	switch (head.type)
	{
		case MSG_LOGOIN: //��½��Ϣ
			OnLogoIN(pHead, head.nContentLen, head.from_user);
			break;
		case MSG_SEND: //������Ϣ
			OnMSGTranslate(pHead, head.nContentLen, head.to_user, head.from_user);
			break;
		case MSG_REGIST:
			OnUserRegist(head, pHead);
			break;
		default: 
			break;
	}

	delete pHead;
	pHead = NULL;
	CAsyncSocket::OnReceive(nErrorCode);
}

void CSessionSocket::OnLogoIN(char* buff, int nlen,char from_user[20]){
	//CDatabase m_dataBase;  //���ݿ�
	//					   //�������ݿ�
	//m_dataBase.Open(NULL,
	//	false,
	//	false,
	//	_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	//);
	//if (!m_dataBase.IsOpen())
	//{
	//	AfxMessageBox(_T("���ݿ�����ʧ��!"));
	//	return;
	//}
	//CString from(from_user);
	//CString str = _T("insert into socket.users values ('") + from + _T("', '123455', 3, '8888')");
	//m_dataBase.ExecuteSQL(str);
	//m_dataBase.Close();

	//TODO:����û��������Ƿ��Ӧ
	CTime time;
	time = CTime::GetCurrentTime();  //��ȡ����ʱ��
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S �û���");

	CString strTemp(buff);
	strTime = strTime + strTemp + _T(" ��¼\r\n");
	//��¼��־
	//��������NetChatServerDlg��Ŀؼ���ʾ

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	pView->m_listData.AddString(strTime);
	pView->m_userList.AddString(strTemp);
	m_strName = strTemp;
	//���·����б�����Ǹ��·������˵��������� 
	//str1 ���ص��������û��ַ���
	CString str1 = this->Update_ServerLog();
	//�����������пͷ��ˣ�from_user ��Ϊ�˲������Լ��������б�
	//�Լ����Լ�����û�����˼�ɣ���ʵ���Լ���Ҳ���ⲻ����ֻ��Ϊ��ѧϰ��������ôһ������
	this->UpDate_ClientUser(str1, from_user);
}

void CSessionSocket::UpDate_ClientUser(CString strUserInfo, char from_user[20]) {
	HEADER head;
	head.type = MSG_UPDATE;
	head.nContentLen = strUserInfo.GetLength() + 1;
	memset(head.from_user, 0, sizeof(head.from_user));
	strcpy(head.from_user, from_user);
	char *pSend = new char[head.nContentLen];
	memset(pSend, 0, head.nContentLen * sizeof(char));
	//��WideCharToMultiByte����ת��
	if (!WChar2MByte(strUserInfo.GetBuffer(0), pSend, head.nContentLen)) {
		AfxMessageBox(_T("�ַ�ת��ʧ��"));
		delete pSend;
		return;
	}
	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();
	while (ps != NULL)
	{
		CSessionSocket *pTemp = (CSessionSocket *)pView->m_pSessionList->GetNext(ps);
		pTemp->Send((char *)&head, sizeof(head));
		pTemp->Send(pSend, head.nContentLen);
	}
	delete pSend;
}

BOOL CSessionSocket::WChar2MByte(LPCWSTR srcBuff, LPSTR destBuff, int nlen)
{
	int n = 0;
	n = WideCharToMultiByte(CP_OEMCP, 0, srcBuff, -1, destBuff, 0, 0, FALSE);
	if (n<nlen)
		return FALSE;

	WideCharToMultiByte(CP_OEMCP, 0, srcBuff, -1, destBuff, nlen, 0, FALSE);

	return TRUE;
}

CString CSessionSocket::Update_ServerLog() {
	CString strUserInfo = _T("");
	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();
	while (ps != NULL) {
		CSessionSocket *pTemp = (CSessionSocket *)pView->m_pSessionList->GetNext(ps);
		strUserInfo += pTemp->m_strName + _T("#");	//��'#'���ָ�
	}

	return strUserInfo;
}

//ת����Ϣ
void CSessionSocket::OnMSGTranslate(char* buff, int nlen, char to_user[20], char from_user[20])
{
	//����ͷ����Ϣ��׼������
	HEADER head;
	head.type = MSG_SEND;
	head.nContentLen = nlen;
	strcpy(head.to_user, to_user);
	strcpy(head.from_user, from_user);

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //ȡ�ã������û��Ķ���
	CString str(buff);
	int i = strcmp(head.to_user, "������");
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//ֻ����2���ˣ� һ���Ƿ���������Ϣ���˺ͽ���������Ϣ���ˡ�
		//���������������Ϣ�����ǡ�Ⱥ�ġ���ô�ͷ��������û���ʵ��Ⱥ�ĺ�һ��һ�ؼ������ڴ�
		if (pTemp->m_strName == head.to_user || pTemp->m_strName == head.from_user || i == 0)
		{
			pTemp->Send(&head, sizeof(HEADER));  //�ȷ���ͷ��
			pTemp->Send(buff, nlen);			//Ȼ�󷢲�����
		}
	}
}

void CSessionSocket::OnUserRegist(HEADER head, char *buf) {
	cJSON *json_root = NULL;
	json_root = cJSON_Parse(buf);
	char *name = cJSON_GetObjectItem(json_root, "username")->valuestring;
	char *password = cJSON_GetObjectItem(json_root, "password")->valuestring;
	char *question = cJSON_GetObjectItem(json_root, "question")->valuestring;
	char *answer = cJSON_GetObjectItem(json_root, "answer")->valuestring;
	CDatabase m_dataBase;  //���ݿ�
						   //�������ݿ�
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("���ݿ�����ʧ��!"));
		return;
	}
	CString Name(name);
	CString Password(password);
	CString Question(question);
	CString Answer(answer);

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	m_strName = Name;
	CString str = _T("SELECT * FROM socket.users WHERE name = '") + Name + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long num = m_recordset->GetRecordCount();
	char Msg;
	if (num == 0) {
		Msg = '1';
		CString str = _T("insert into socket.users values ('") + Name + _T("', '") + Password + _T("', '") + Question + _T("', '") + Answer + _T("')");
		m_dataBase.ExecuteSQL(str);
		CTime time;
		time = CTime::GetCurrentTime();  //��ȡ����ʱ��
		CString strTime = time.Format("%Y-%m-%d %H:%M:%S �û���");
		CString str1 = strTime + _T("�û� ") + Name + _T(" ע�����");
		pView->m_listData.AddString(str1);
	}
	else {
		Msg = '0';
	}
	m_recordset->Close();
	m_dataBase.Close();
	//����ͷ����Ϣ��׼������
	HEADER _head;
	_head.type = MSG_REGIST;
	_head.nContentLen = sizeof(char);
	strcpy(_head.to_user, name);
	strcpy(_head.from_user, "Server");
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //ȡ�ã������û��Ķ���
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//ֻ����2���ˣ� һ���Ƿ���������Ϣ���˺ͽ���������Ϣ���ˡ�
		//���������������Ϣ�����ǡ�Ⱥ�ġ���ô�ͷ��������û���ʵ��Ⱥ�ĺ�һ��һ�ؼ������ڴ�
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //�ȷ���ͷ��
			pTemp->Send(&Msg, sizeof(char));	//Ȼ�󷢲�����
		}
	}
}
