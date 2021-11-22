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
	//单列销毁
	class deobj
	{
	public:
		deobj() {
			CClientSocket::getObject();
		}
		~deobj() {	//删除单列
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
	//属性
	static CClientSocket* getObject();//单列模式，获取唯一对象
	DataBag databag;
	//方法
	int recvMes(char* _buf,  int maxSize, UINT* _index , DataBag* bag );//接收消息(-1客户端套接字错误)(其他:recv接收异常返回值)(0包解析异常)(result>0,接收并解析正常)
	int recvMes();
	int SendMes(DataBag& bag);
	int init();//初始化
	void UpData(int ip, int port);
	//属性
private:
	int nIp;
	int nPort;
	//服务器套接字
	SOCKET sock;
	sockaddr_in addr;
	int addr_len = sizeof addr;

	//单列
	static CClientSocket* myObject;
	static HANDLE _lock;

	static deobj heleper;//自动析构使用

};
