
/////////////////////////////////////////////////////////////
////定义协议头 因为直接要传输的类容中有不确定长的的类容
///为了避免浪费空间选择分两部分传输，故定义一个头
/////////////////////////////////////////////////////////////
#pragma once
////////////自定义协议///////////////////

const int MSG_LOGOIN = 0x01; //登录
const int MSG_SEND = 0x02;   //发送消息
const int MSG_CLOSE = 0x03;  //退出
const int MSG_UPDATE = 0x04; //更新信息
const int MSG_REGIST = 0x05;	//注册用户
const int MSG_GETQUE = 0x06;	//获取密保问题
const int MSG_RESET = 0x07;		//重置密码

#pragma pack(push,1)
typedef struct tagHeader {
	int type;//协议类型
	int nContentLen; //将要发送内容的长度
	char to_user[20];//接受者
	char from_user[20];//发送者用户名
}HEADER, *LPHEADER;
#pragma pack(pop)