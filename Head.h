
/////////////////////////////////////////////////////////////
////����Э��ͷ ��Ϊֱ��Ҫ������������в�ȷ�����ĵ�����
///Ϊ�˱����˷ѿռ�ѡ��������ִ��䣬�ʶ���һ��ͷ
/////////////////////////////////////////////////////////////
#pragma once
////////////�Զ���Э��///////////////////

const int MSG_LOGOIN = 0x01; //��¼
const int MSG_SEND = 0x02;   //������Ϣ
const int MSG_CLOSE = 0x03;  //�˳�
const int MSG_UPDATE = 0x04; //������Ϣ
const int MSG_REGIST = 0x05;	//ע���û�
const int MSG_GETQUE = 0x06;	//��ȡ�ܱ�����
const int MSG_RESET = 0x07;		//��������

#pragma pack(push,1)
typedef struct tagHeader {
	int type;//Э������
	int nContentLen; //��Ҫ�������ݵĳ���
	char to_user[20];//������
	char from_user[20];//�������û���
}HEADER, *LPHEADER;
#pragma pack(pop)