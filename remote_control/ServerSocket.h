#pragma once
#include "pch.h"
#include "framework.h"
#include "Cmd.h"
#include <list>
#include "DataBag.h"

typedef void(*SOCK_CALL_FUNCTION)(void*,int,std::list<DataBag>&,DataBag&);

class CServerSocket
{
	//单列销毁
	class deobj
	{
	public:
		deobj() {
			CServerSocket::getObject();}
		~deobj() {	//删除单列
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
//属性
	static CServerSocket* getObject();//单列模式，获取唯一对象
	
//方法
	int Run(SOCK_CALL_FUNCTION call_back,void* arg,int port = 18086);
	static void RunCommand(void* arg, int nCmd, std::list<DataBag>&, DataBag&);

//属性
private:
	SOCK_CALL_FUNCTION m_callBack;
	void* m_arg;
	//数据包
	DataBag databag;
	//服务器套接字
	SOCKET sock;
	sockaddr_in addr;
	int addr_len = sizeof addr;
	//客户端套接字
	SOCKET sock_client;

	//单列
	static CServerSocket* myObject;
	static HANDLE _lock;

	int count;//listen 监听数量
	static deobj heleper;//自动析构使用

//方法
private:
	
	bool start(int count = 1);//监听+接收
	int recvMes();//接收消息(-1客户端套接字错误)(其他:recv接收异常返回值)(0包解析异常)(result>0,接收并解析正常)
	int SendMes(DataBag& bag);
	int init(int port = 18086);//初始化
};

std::string GetErrorInfo(int wsaErrorNumber);