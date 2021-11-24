#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <list>
#include <map>
#include "DataBag.h"

//*******************************************
#define MAX_BUF 4096
//*******************************************

class CClientSocket
{
	//��������
	class deobj
	{
	public:
		deobj() {
			CClientSocket::getObject();
		}
		~deobj() {	//ɾ������
			if (CClientSocket::myObject != nullptr)
			{
				delete CClientSocket::myObject;
				CClientSocket::myObject = nullptr;
			}
		}
	};

	CClientSocket();
	~CClientSocket();
	CClientSocket(const CClientSocket& obj);
	CClientSocket& operator=(const CClientSocket& obj) {}


public:
	RTL_CRITICAL_SECTION RtlSend;
	RTL_CRITICAL_SECTION RtlRecv[11];
	//����
	static CClientSocket* getObject();//����ģʽ����ȡΨһ����
	DataBag databag;
	//����
	int recvMes(char* _buf,  int maxSize, UINT* _index , DataBag* bag );//������Ϣ(-1�ͻ����׽��ִ���)(����:recv�����쳣����ֵ)(0�������쳣)(result>0,���ղ���������)
	//int recvMes();
	int SendMes(DataBag& bag);
	int init();//��ʼ��
	void UpData(int ip, int port);
	//�������ݰ��߳�
	static void WINAPIV _threadSendPacketFun(void*);
	//�������ݰ��߳�
	static void WINAPIV _threadRecvPacketFun(void*);
	//�����Ҫ���͵İ�
	void addPacket(DataBag& bag);
	//��ȡ���صİ� 
	std::string GetResultInfo(int nCmd);

	//����
private:
	int nIp;//ip��ַ
	int nPort;//�˿�
	//�������׽���
	SOCKET sock = 0;
	sockaddr_in addr;
	int addr_len = sizeof addr;

	//����
	static CClientSocket* myObject;
	static HANDLE _lock;

	static deobj heleper;//�Զ�����ʹ��

	//��������
	std::list<DataBag> m_sendBag;
	
	//�հ�
	std::map<int, std::list<DataBag>> m_mapAck;
	
};
