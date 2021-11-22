#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
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
	CClientSocket(const CClientSocket& obj) {}
	CClientSocket& operator=(const CClientSocket& obj) {}


public:
	//����
	static CClientSocket* getObject();//����ģʽ����ȡΨһ����
	DataBag databag;
	//����
	int recvMes(char* _buf,  int maxSize, UINT* _index , DataBag* bag );//������Ϣ(-1�ͻ����׽��ִ���)(����:recv�����쳣����ֵ)(0�������쳣)(result>0,���ղ���������)
	int recvMes();
	int SendMes(DataBag& bag);
	int init();//��ʼ��
	void UpData(int ip, int port);
	//����
private:
	int nIp;
	int nPort;
	//�������׽���
	SOCKET sock;
	sockaddr_in addr;
	int addr_len = sizeof addr;

	//����
	static CClientSocket* myObject;
	static HANDLE _lock;

	static deobj heleper;//�Զ�����ʹ��

};
