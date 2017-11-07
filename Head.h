
/////////////////////////////////////////////////////////////
////定义协议头 因为直接要传输的类容中有不确定长的的类容
////为了避免浪费空间选择分两部分传输，故定义一个头
/////////////////////////////////////////////////////////////
#pragma once
////////////自定义协议///////////////////

const int MSG_LOGIN = 0x01;	//登录
const int MSG_SEND = 0x02;		//发送消息
const int MSG_CLOSE = 0x03;		//退出
const int MSG_UPDATE = 0x04;	//更新信息
const int MSG_REGIST = 0x05;	//注册用户
const int MSG_GETQUE = 0x06;	//获取密保问题
const int MSG_RESET = 0x07;		//重置密码
const int MSG_GETIP = 0x08;	//获取聊天对象IP
const int MSG_OFFLINE = 0x09;	//离线消息

static char* _CMD = "cmd";
static char* _PORT = "port";
static char* _QUESTION = "question";
static char* _NAME = "name";
static char* _STATUS = "status";
static char* _ISONLINE = "isOnline";
static char* _IP = "ip";
static char* _TOUSER = "touser";
static char* _OFFLINEMSG = "offlineMsg";

#pragma pack(push,1)
typedef struct tagHeader {
	int type;//协议类型
	int nContentLen; //将要发送内容的长度
	char to_user[20];//接受者
	char from_user[20];//发送者用户名
}HEADER, *LPHEADER;
#pragma pack(pop)