#pragma once
#include "Head.h"
// CSessionSocket 命令目标

class CSessionSocket : public CAsyncSocket
{
public:
	CSessionSocket();
	virtual ~CSessionSocket();
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
public:
	void OnLogoIN(char* buff, int nlen, char from_user[20]);
	void UpDate_ClientUser(CString strUserInfo, char from_user[20]);
	void OnMSGTranslate(char* buff, int nlen, char to_user[20], char from_user[20]); //转发消息给其他用户
	void OnUserRegist(HEADER head, char *buf);
	void Answer_Login(int flag, char *from_user);
	void GetQuestion(HEADER head);
	void OnUserReset(HEADER head, char *buf);
	void Answer_Reset(int flag, HEADER head);
	CString Update_ServerLog();
private:
	BOOL WChar2MByte(LPCWSTR srcBuff, LPSTR destBuff, int nlen);
public:
	SOCKADDR_IN sockAddr;
	CString m_strName; //连接名称
};


