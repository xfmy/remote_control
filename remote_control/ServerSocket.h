#pragma once
#include "pch.h"
#include "framework.h"
#include "Cmd.h"
#include <list>
#include "DataBag.h"

typedef void(*SOCK_CALL_FUNCTION)(void*,int,std::list<DataBag>&,DataBag&);

class CServerSocket
{
	//��������
	class deobj
	{
	public:
		deobj() {
			CServerSocket::getObject();}
		~deobj() {	//ɾ������
			if (CServerSocket::myObject != nullptr)
			{
				delete CServerSocket::myObject;
				CServerSocket::myObject = nullptr;
			}
		}
	};

	CServerSocket();
	~CServerSocket();
	CServerSocket(const CServerSocket& obj){}
	CServerSocket& operator=(const CServerSocket& obj){}


public:
//����
	static CServerSocket* getObject();//����ģʽ����ȡΨһ����
	
//����
	int Run(SOCK_CALL_FUNCTION call_back,void* arg,int port = 18086);
	static void RunCommand(void* arg, int nCmd, std::list<DataBag>&, DataBag&);

//����
private:
	SOCK_CALL_FUNCTION m_callBack;
	void* m_arg;
	//���ݰ�
	DataBag databag;
	//�������׽���
	SOCKET sock;
	sockaddr_in addr;
	int addr_len = sizeof addr;
	//�ͻ����׽���
	SOCKET sock_client;

	//����
	static CServerSocket* myObject;
	static HANDLE _lock;

	int count;//listen ��������
	static deobj heleper;//�Զ�����ʹ��

//����
private:
	
	bool start(int count = 1);//����+����
	int recvMes();//������Ϣ(-1�ͻ����׽��ִ���)(����:recv�����쳣����ֵ)(0�������쳣)(result>0,���ղ���������)
	int SendMes(DataBag& bag);
	int init(int port = 18086);//��ʼ��
};

std::string GetErrorInfo(int wsaErrorNumber);