#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket::myObject = nullptr;
HANDLE CServerSocket::_lock = CreateEvent(nullptr, false, true, NULL);
CServerSocket::deobj CServerSocket::heleper;


CServerSocket::CServerSocket()
{
	
}

CServerSocket::~CServerSocket()
{
	CloseHandle(_lock);//关闭事件对象
	closesocket(sock);//关闭主机
	closesocket(sock_client);//关闭分机
	WSACleanup();//清理网络
}

int CServerSocket::init(int port)
{
	//加载套接字库
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	//初始化套接字库
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		MessageBox(NULL, "初始化套接字库第一步失败", "警告", MB_ICONWARNING);
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		MessageBox(NULL, "初始化套接字库第二部失败", "警告", MB_ICONWARNING);
		return-1;
	}
	//创建套接字
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(NULL, "创建套接字库失败", "警告", MB_ICONWARNING);
		TRACE("创建套接字库失败->%s",GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	//绑定ip和端口
	if (bind(sock, (sockaddr*)&addr, sizeof addr) == SOCKET_ERROR)
	{
		MessageBox(NULL, "监听到bind返回值为SOCKET_ERROR", "警告", MB_ICONWARNING);
		TRACE("bind失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	return 0;
}

CServerSocket* CServerSocket::getObject()
{
	if (myObject == NULL)
	{
		WaitForSingleObject(_lock, INFINITE);
		if (myObject == NULL)
		{
			myObject = new CServerSocket;
			return myObject;
		}
	}
	return myObject;
}

int CServerSocket::Run(SOCK_CALL_FUNCTION call_back, void* arg, int port)
{
	if (init(port)) return -1;

	if(start() == false) return -2;
	std::list<DataBag> m_list;
	m_callBack = call_back;
	m_arg = arg;
	int res = 0;
	while (res >= 0)
	{
		res = recvMes();
		if (res > 0) {
			//AfxMessageBox("recvMes返回值异常");
			m_callBack(m_arg, res, m_list, databag);

			while (m_list.size() != 0)
			{
				SendMes(m_list.front());
				m_list.pop_front();
			}
		}
	}
	return 0;
}

void CServerSocket::RunCommand(void* arg, int nCmd, std::list<DataBag>& m_list, DataBag& bag)
{
	CCommand* thiz = (CCommand*)arg;
	int res = thiz->executeCmd(nCmd, m_list, bag);
	if (res != 0)
	{
		TRACE("执行命令失败");
	}
}

bool CServerSocket::start(int count)
{
	//监听
	if (listen(sock, count) == SOCKET_ERROR)
	{
		MessageBox(NULL, "监听到listen返回值为SOCKET_ERROR", "警告", MB_ICONWARNING);
		TRACE("listen失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return false;
	}
	this->count = count;

	//接收请求
	int i = 0;
	while (i++ < count)
	{
		sock_client = accept(sock, (sockaddr*)&addr, &addr_len);
		if (sock_client == INVALID_SOCKET)
		{
			AfxMessageBox("检测到accept返回值为INVALID_SOCKET");
			TRACE("accept失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
			i--;
		}
		UCHAR* pIp = (UCHAR*)&addr.sin_addr.S_un.S_addr;
		USHORT pt;
		memcpy(&pt, (char*)&addr.sin_port + 1, 1);
		memcpy((char*)&pt + 1, (char*)&addr.sin_port, 1);
		TRACE("服务端接收成功 端口号%d,ip==%d.%d.%d.%d\n", pt, *pIp, *(pIp + 1), *(pIp + 2), *(pIp + 3));
	}
	//TODO accept,send.recv-----io复用---多线程
	return true;
}

int CServerSocket::recvMes()
{
	//客户端套接字错误
	if (sock == -1 || sock == 0)
	{
		return -1;
	}
	int len{};
	static char buf[MAX_BUF]{};//缓冲区
	static UINT index = 0;//每次数据读入存放的下标
	if (index) goto HEAD;
BACK:
		len = recv(sock_client, buf + index, MAX_BUF - index, 0);//接收数据 
		if (len <= 0)//接收异常
		{
			TRACE("recv失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
			return len;
		}
	index += len;//更新索引(添加接收的数据长度)
HEAD:
	int res = databag.parse(buf, index);//解析
	if (res == -1)
	{
		goto BACK;
	}
	else if (res == -2)
	{
		return -2;
	}
	return databag.cmd;
}

int CServerSocket::SendMes(DataBag& bag)
{
	int res = send(sock_client, bag.DATA.c_str(), bag.GetSize(), 0);
	if (res <= 0)
	{
		TRACE("send失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
	}
	return res;
}


std::string GetErrorInfo(int wsaErrorNumber) {
	std::string info;
	LPVOID lpMsgBuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrorNumber,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	info = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return info;
}