// SessionSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "SessionSocket.h"
#include "ServerView.h"
#include "MainFrm.h"
#include "ServerInfo.h"
#include "afxdb.h"
#include "cJSON.h"
#include <stdlib.h>


// CSessionSocket

CSessionSocket::CSessionSocket()
	:m_strName(_T(""))
{
	// TODO: 在此处添加构造代码

}

CSessionSocket::~CSessionSocket()
{
}

// CSessionSocket 成员函数

//当客户关闭连接时的事件响应函数
void CSessionSocket::OnClose(int nErrorCode)
{
	if (m_strName.Compare(_T("")) == 0) {
		return;
	}
	//在View里显示信息
	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S 用户：");
	strTime = strTime + this->m_strName + _T(" 离开\r\n");
	pView->m_listData.AddString(strTime);

	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}

	CString str = _T("SELECT * FROM socket.users WHERE name = '") + this->m_strName + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long cnt = m_recordset->GetRecordCount();
	if (cnt != 0) {
		CString str1 = _T("update socket.users set isOnline = 'N' where name='") + this->m_strName + _T("'");
		m_dataBase.ExecuteSQL(str1);
		str1 = _T("update socket.users set ip = '0.0.0.0' where name='") + this->m_strName + _T("'");
		m_dataBase.ExecuteSQL(str1);
		str1 = _T("update socket.users set port = '0' where name='") + this->m_strName + _T("'");
		m_dataBase.ExecuteSQL(str1);
	}
	m_recordset->Close();
	m_dataBase.Close();

	int ps = pView->m_userList.FindString(0, this->m_strName);
	pView->m_userList.DeleteString(ps);
	pView->m_pSessionList->RemoveAt(pView->m_pSessionList->Find(this));
	//更改服务器在线名单
	CString str1 = this->Update_ServerLog();
	//通知客户端刷新在线名单
	this->UpDate_ClientUser(str1, "0");
	this->Close();
	//销毁该套接字
	delete this;
	CSocket::OnClose(nErrorCode);
}

//接收到数据时的事件响应函数
void CSessionSocket::OnReceive(int nErrorCode)
{
	//先接收和处理消息头
	HEADER head;
	int head_len = sizeof(HEADER);
	char *pHead = NULL;	//用于接收消息头
	pHead = new char[head_len];
	if (!pHead) {
		TRACE0("CSessionSocket::OnReceive 内存分配失败");
		return;
	}
	memset(pHead, 0, head_len*sizeof(char)); //接受头的初始化
	Receive(pHead, head_len);
	
	head.type = ((HEADER *)pHead)->type;
	head.nContentLen = ((HEADER *)pHead)->nContentLen;
	memset(head.to_user, 0, sizeof(head.to_user));
	strcpy(head.to_user, ((HEADER *)pHead)->to_user);
	memset(head.from_user, 0, sizeof(head.from_user));
	strcpy(head.from_user, ((HEADER *)pHead)->from_user);
	delete pHead;	//接受完成后释放指针
	pHead = NULL;

	//根据消息头中的信息接收后续的数据
	pHead = new char[head.nContentLen];
	if (!pHead) {
		TRACE0("CSessionSocket::OnReceive                        分配失败");
		return;
	}
	memset(pHead, 0, head.nContentLen * sizeof(char));
	//简单差错检验:如果接受到的数据的长度和信息头中的给出的数据长度不同的话不予以接受，直接清空
	if (Receive(pHead, head.nContentLen) != head.nContentLen) {
		delete pHead;
		return;
	}

	//根据消息类型，分发给不同的处理函数
	////////////根据消息类型，处理数据,这个也是和客户端进行对应的。。下面的MSG_LOGOIN，MSG_SEND是定义好的常量，可以F12看看////////////////////
	switch (head.type)
	{
		case MSG_LOGIN: //登陆消息
			OnLogoIN(pHead, head.nContentLen, head.from_user);
			break;
		case MSG_SEND: //发送消息
			OnMSGTranslate(pHead, head);
			break;
		case MSG_REGIST:
			OnUserRegist(head, pHead);
			break;
		case MSG_GETQUE:
			GetQuestion(head);
			break;
		case MSG_RESET:
			OnUserReset(head, pHead);
			break;
		case MSG_GETIP:
			OnGetIP(head, pHead);
			break;
		default: 
			break;
	}

	delete pHead;
	pHead = NULL;
	CSocket::OnReceive(nErrorCode);
}

void CSessionSocket::OnLogoIN(char* buff, int nlen,char from_user[20]){
	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}
	CString from(from_user);
	m_strName = from;
	CString str = _T("SELECT * FROM socket.users WHERE name = '") + from + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long num = m_recordset->GetRecordCount();

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	if (num == 0) {
		Answer_Login(0, from_user, pView->GivenPort);
		return;
	}
	CString tempStr;
	LPCTSTR lpctStr = (LPCTSTR)_T("password");
	m_recordset->GetFieldValue(lpctStr, tempStr);
	m_recordset->Close();
	m_dataBase.Close();
	cJSON *json_root = NULL;
	json_root = cJSON_Parse(buff);
	char *name = cJSON_GetObjectItem(json_root, "username")->valuestring;
	char *password = cJSON_GetObjectItem(json_root, "password")->valuestring;
	CString Name(name);
	CString Password(password);
	if (tempStr.Compare(Password) != 0) {
		Answer_Login(0, from_user, pView->GivenPort);
		return;
	}
	else {
		Answer_Login(1, from_user, pView->GivenPort);
	}

	//TODO:检查用户与密码是否对应
	CTime time;
	time = CTime::GetCurrentTime();  //获取现在时间
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S 用户：");

	strTime = strTime + from + _T(" 登录\r\n");
	//记录日志
	//将内容在NetChatServerDlg里的控件显示


	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}

	CString strTmp = _T("SELECT * FROM socket.users WHERE name = '") + from + _T("'");
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, strTmp);
	long cnt = m_recordset->GetRecordCount();
	if (cnt != 0) {
		CString str1 = _T("update socket.users set isOnline = 'Y' where name='") + from + _T("'");
		m_dataBase.ExecuteSQL(str1);
	}
	/*更新数据库中用户的ip*/
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		if (pTemp->m_strName == from)
		{
			SOCKADDR_IN sock = pTemp->sockAddr;
			char * ip = inet_ntoa(sock.sin_addr);
			CString IP(ip);
			CString ListenPort;
			ListenPort.Format(_T("%d"), pView->GivenPort);
			pView->GivenPort += 2;
			CString str1 = _T("update socket.users set ip = '")+ IP + _T("' where name='") + from + _T("'");
			m_dataBase.ExecuteSQL(str1);
			str1 = _T("update socket.users set port = '") + ListenPort + _T("' where name='") + from + _T("'");
			m_dataBase.ExecuteSQL(str1);
			break;
		}
	}

	/*检查是否有离线消息*/
	CString findOffline = _T("SELECT * FROM socket.offline_msg WHERE toUser = '") + from + _T("'");
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, findOffline);
	long offlineCnt = m_recordset->GetRecordCount();
	if (offlineCnt != 0) {
		m_recordset->MoveLast();
		CString lastMsg;
		LPCTSTR lpctStr = (LPCTSTR)_T("message");
		m_recordset->GetFieldValue(lpctStr, lastMsg);
		m_recordset->MoveFirst();
		CString message("");
		CString TolMes("");
		while (message.Compare(lastMsg) != 0) {
			m_recordset->GetFieldValue(lpctStr, message);
			TolMes += message;
			m_recordset->MoveNext();
		}
		SendOfflineMsg(from_user, TolMes);
		CString safeDelete = _T("SET SQL_SAFE_UPDATES = 0;");
		m_dataBase.ExecuteSQL(safeDelete);
		CString str = _T("delete from socket.offline_msg where toUser = '") + from + _T("'");
		m_dataBase.ExecuteSQL(str);
		safeDelete = _T("SET SQL_SAFE_UPDATES = 1;");
		m_dataBase.ExecuteSQL(safeDelete);
	}

	m_recordset->Close();
	m_dataBase.Close();

	pView->m_listData.AddString(strTime);
	pView->m_userList.AddString(from);
	CString str1 = this->Update_ServerLog();
	this->UpDate_ClientUser(str1, from_user);
}

void CSessionSocket::Answer_Login(int flag, char *from_user, int GivenPort) {

	CString str;
	if (flag == 0) {
		str = _T("{\"cmd\":0}");
	}
	else {
		str.Format(_T("{\"cmd\":1, \"port\":%d}"), GivenPort);
	}

	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, -1, data, len, NULL, NULL);

	HEADER _head;
	_head.type = MSG_LOGIN;
	_head.nContentLen = len + 1;
	memset(_head.to_user, 0, sizeof(_head.to_user));
	strcpy(_head.to_user, from_user);
	memset(_head.from_user, 0, sizeof(_head.from_user));
	strcpy(_head.from_user, "Server");


	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, len + 1);	//然后发布内容
		}
	}
}

void CSessionSocket::UpDate_ClientUser(CString strUserInfo, char from_user[20]) {
	HEADER head;
	head.type = MSG_UPDATE;
	head.nContentLen = strUserInfo.GetLength() + 1;
	memset(head.from_user, 0, sizeof(head.from_user));
	strcpy(head.from_user, from_user);
	char *pSend = new char[head.nContentLen];
	memset(pSend, 0, head.nContentLen * sizeof(char));
	//用WideCharToMultiByte进行转化
	if (!WChar2MByte(strUserInfo.GetBuffer(0), pSend, head.nContentLen)) {
		AfxMessageBox(_T("字符转换失败"));
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

	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return _T("");
	}
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	CString str = _T("SELECT * FROM socket.users");
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	/*long num = m_recordset->GetRecordCount();
	short cnt = m_recordset->GetODBCFieldCount();*/
	m_recordset->MoveLast();
	CString lastname;
	LPCTSTR lpctStr = (LPCTSTR)_T("name");
	m_recordset->GetFieldValue(lpctStr, lastname);
	m_recordset->MoveFirst();
	CString name("");
	CString status("");
	CString strUserInfo("");
	while (name.Compare(lastname) != 0) {
		LPCTSTR getName = (LPCTSTR)_T("name");
		m_recordset->GetFieldValue(getName, name);
		LPCTSTR getStatus = (LPCTSTR)_T("isOnline");
		m_recordset->GetFieldValue(getStatus, status);
		CString json = _T("{\"name\":\"") + name + _T("\",\"status\":\"") + status + _T("\"}");
		strUserInfo = strUserInfo + json + _T("#");
		m_recordset->MoveNext();
	}
	m_recordset->Close();
	m_dataBase.Close();

	return strUserInfo;
}

void CSessionSocket::OnMSGTranslate(char* buff, HEADER head)
{
	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}
	CString toUser(head.to_user);
	CString fromUser(head.from_user);
	CString Msg(buff);
	CString str = _T("insert into socket.offline_msg values ('") + toUser + _T("', '") + fromUser + _T("', '") + Msg + _T("')");
	m_dataBase.ExecuteSQL(str);
	m_dataBase.Close();
}

void CSessionSocket::OnUserRegist(HEADER head, char *buf) {
	cJSON *json_root = NULL;
	json_root = cJSON_Parse(buf);
	char *name = cJSON_GetObjectItem(json_root, "username")->valuestring;
	char *password = cJSON_GetObjectItem(json_root, "password")->valuestring;
	char *question = cJSON_GetObjectItem(json_root, "question")->valuestring;
	char *answer = cJSON_GetObjectItem(json_root, "answer")->valuestring;

	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
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
	CString strTemp;
	if (num == 0) {
		strTemp = _T("{\"cmd\":1}");
		CString str = _T("insert into socket.users values ('") + Name + _T("', '") + Password + _T("', '") + Question + _T("', '") + Answer + _T("', 'N', '0.0.0.0', '0')");
		m_dataBase.ExecuteSQL(str);
		CTime time;
		time = CTime::GetCurrentTime();  //获取现在时间
		CString strTime = time.Format("%Y-%m-%d %H:%M:%S 用户：");
		CString str1 = strTime + Name + _T(" 注册完成");
		pView->m_listData.AddString(str1);
	}
	else {
		strTemp = _T("{\"cmd\":0}");
	}
	m_recordset->Close();
	m_dataBase.Close();
	//建立头部信息，准备发送

	int len = WideCharToMultiByte(CP_ACP, 0, strTemp, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, strTemp, -1, data, len, NULL, NULL);
	HEADER _head;
	_head.type = MSG_REGIST;
	_head.nContentLen = len + 1;
	strcpy(_head.to_user, name);
	strcpy(_head.from_user, "Server");
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, len + 1);	//然后发布内容
		}
	}
}

void CSessionSocket::GetQuestion(HEADER head) {
	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}
	CString from(head.from_user);

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	CTime time;
	time = CTime::GetCurrentTime();  //获取现在时间
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S 用户：");
	strTime = strTime + from + _T(" 获取密保问题\r\n");
	pView->m_listData.AddString(strTime);

	m_strName = from;
	CString str = _T("SELECT * FROM socket.users WHERE name = '") + from + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long num = m_recordset->GetRecordCount();
	CString question("");
	CString str1;
	if (num != 0) {
		LPCTSTR lpctStr = (LPCTSTR)_T("question");
		m_recordset->GetFieldValue(lpctStr, question);
		str1 = _T("{\"cmd\":1,\"question\":\"") + question + _T("\"}");
	}
	else {
		str1 = _T("{\"cmd\":0}");
	}
	int len = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str1, -1, data, len, NULL, NULL);
	m_recordset->Close();
	m_dataBase.Close();

	HEADER _head;
	_head.type = MSG_GETQUE;
	memset(_head.from_user, 0, sizeof(_head.from_user));
	strcpy(_head.from_user, "Server");
	memset(_head.to_user, 0, sizeof(_head.to_user));
	strcpy(_head.to_user, head.from_user);
	_head.nContentLen = strlen(data);

	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, strlen(data));	//然后发布内容
		}
	}
}

void CSessionSocket::OnUserReset(HEADER head, char *buf) {
	cJSON *json_root = NULL;
	json_root = cJSON_Parse(buf);
	char *name = cJSON_GetObjectItem(json_root, "username")->valuestring;
	char *password = cJSON_GetObjectItem(json_root, "password")->valuestring;
	char *answer = cJSON_GetObjectItem(json_root, "answer")->valuestring;

	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}
	CString Name(name);
	CString Password(password);
	CString Answer(answer);

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	m_strName = Name;
	CString str = _T("SELECT * FROM socket.users WHERE name = '") + Name + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long cnt = m_recordset->GetRecordCount();
	if (cnt == 0) {
		Answer_Reset(0, head);
		return;
	}
	CString m_answer;
	LPCTSTR lpctStr = (LPCTSTR)_T("answer");
	m_recordset->GetFieldValue(lpctStr, m_answer);
	if (m_answer.Compare(Answer) != 0) {
		Answer_Reset(0, head);
		return;
	}
	CString str1 = _T("update socket.users set password='")+ Password + _T("' where name='") + Name + _T("'");
	m_dataBase.ExecuteSQL(str1);
	Answer_Reset(1, head);
	//TODO:检查用户与密码是否对应

	CTime time;
	time = CTime::GetCurrentTime();  //获取现在时间
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S 用户：");
	strTime = strTime + Name + _T(" 修改密码\r\n");
	pView->m_listData.AddString(strTime);

	m_recordset->Close();
	m_dataBase.Close();
}

void CSessionSocket::Answer_Reset(int flag, HEADER head) {
	CString tempStr;
	if (flag == 1) {
		tempStr = _T("{\"cmd\":1}");
	}
	else {
		tempStr = _T("{\"cmd\":0}");
	}
	int len = WideCharToMultiByte(CP_ACP, 0, tempStr, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, tempStr, -1, data, len, NULL, NULL);

	HEADER _head;
	_head.type = MSG_RESET;
	_head.nContentLen = len + 1;
	memset(_head.to_user, 0, sizeof(_head.to_user));
	strcpy(_head.to_user, head.from_user);
	memset(_head.from_user, 0, sizeof(_head.from_user));
	strcpy(_head.from_user, "Server");

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, len + 1);	//然后发布内容
		}
	}
}

void CSessionSocket::OnGetIP(HEADER head, char *buf) {
	cJSON *json_root = NULL;
	json_root = cJSON_Parse(buf);
	char *touser = cJSON_GetObjectItem(json_root, "touser")->valuestring;
	CDatabase m_dataBase;  //数据库
						   //连接数据库
	m_dataBase.Open(NULL,
		false,
		false,
		_T("ODBC;server=127.0.0.1;DSN=My_odbc;UID=root;PWD=tanghuichuan1997")
	);
	if (!m_dataBase.IsOpen())
	{
		AfxMessageBox(_T("数据库连接失败!"));
		return;
	}
	CString to_user(touser);

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	CString from_user(head.from_user);
	CString showMsg = _T("用户 ") + from_user + _T(" 请求与用户 ") + to_user + _T("通信");
	pView->m_listData.AddString(showMsg);

	CString str = _T("SELECT * FROM socket.users WHERE name = '") + from_user + _T("'");
	CRecordset *m_recordset;
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	CString LocalIP;
	LPCTSTR getLocal = (LPCTSTR)_T("ip");
	m_recordset->GetFieldValue(getLocal, LocalIP);

	str = _T("SELECT * FROM socket.users WHERE name = '") + to_user + _T("'");
	m_recordset = new CRecordset(&m_dataBase);
	m_recordset->Open(AFX_DB_USE_DEFAULT_TYPE, str);
	long cnt = m_recordset->GetRecordCount();
	if (cnt == 0) {
		Answer_Reset(0, head);
		return;
	}
	CString isOnline;
	LPCTSTR lpctStr = (LPCTSTR)_T("isOnline");
	m_recordset->GetFieldValue(lpctStr, isOnline);
	CString Online("Y");
	CString SendMsg;
	if (isOnline.Compare(Online) == 0) {
		CString IP, Port;
		LPCTSTR lpctStr = (LPCTSTR)_T("ip");
		m_recordset->GetFieldValue(lpctStr, IP);
		lpctStr = (LPCTSTR)_T("port");
		m_recordset->GetFieldValue(lpctStr, Port);
		SendMsg = _T("{\"isOnline\":\"") + isOnline + _T("\", \"ip\":\"") + IP + _T("\", \"port\": \"") + Port + _T("\", \"touser\":\"") + to_user + _T("\",\"localIP\":\"") + LocalIP + _T("\"}");
	}
	else {
		CString IP(Server_IP);
		CString Port;
		Port.Format(_T("%d"), Server_port);
		SendMsg = _T("{\"isOnline\":\"") + isOnline + _T("\", \"ip\":\"") + IP + _T("\", \"port\": \"") + Port + _T("\", \"touser\":\"") + to_user + _T("\",\"localIP\":\"") + LocalIP + _T("\"}");
	}
	m_recordset->Close();
	m_dataBase.Close();

	int len = WideCharToMultiByte(CP_ACP, 0, SendMsg, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, SendMsg, -1, data, len, NULL, NULL);

	HEADER _head;
	_head.type = MSG_GETIP;
	_head.nContentLen = len + 1;
	memset(_head.to_user, 0, sizeof(_head.to_user));
	strcpy(_head.to_user, head.from_user);
	memset(_head.from_user, 0, sizeof(_head.from_user));
	strcpy(_head.from_user, "Server");
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == _head.to_user)
		{
			pTemp->Send(&_head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, len + 1);	//然后发布内容
		}
	}
}

void CSessionSocket::SendOfflineMsg(char *toUser, CString msg) {
	CString sendStr = _T("{\"offlineMsg\":\"") + msg + _T("\"}");
	int len = WideCharToMultiByte(CP_ACP, 0, sendStr, -1, NULL, 0, NULL, NULL);
	char *data = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, sendStr, -1, data, len, NULL, NULL);

	HEADER head;
	head.type = MSG_OFFLINE;
	head.nContentLen = len + 1;
	memset(head.to_user, 0, sizeof(head.to_user));
	strcpy(head.to_user, toUser);
	memset(head.from_user, 0, sizeof(head.from_user));
	strcpy(head.from_user, "Server");

	CServerView* pView = (CServerView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	POSITION ps = pView->m_pSessionList->GetHeadPosition();  //取得，所有用户的队列
	while (ps != NULL)
	{
		CSessionSocket* pTemp = (CSessionSocket*)pView->m_pSessionList->GetNext(ps);
		//只发送2个人， 一个是发送聊天消息的人和接收聊天消息的人。
		//如果，接收聊天消息的人是“群聊”那么就发送所有用户，实现群聊和一对一关键就在于此
		if (pTemp->m_strName == head.to_user)
		{
			pTemp->Send(&head, sizeof(HEADER));  //先发送头部
			pTemp->Send(data, len + 1);	//然后发布内容
		}
	}
}