
#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::myObject = nullptr;
HANDLE CClientSocket::_lock = CreateEvent(nullptr, false, true, NULL);
CClientSocket::deobj CClientSocket::heleper;


std::string GetErrorInfo(int wsaErrorNumber) {
	std::string info;
	LPVOID lpMsgBuf = nullptr;
	FormatMessageA(
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



CClientSocket::CClientSocket()
{
	nIp = ADDR_ANY;
	nPort = 0;
	for (int i = 1;i <= 11;i++)
	{
		m_mapAck.insert(std::pair<int, std::list<DataBag>>(i, std::list<DataBag>()));
		InitializeCriticalSection(&RtlRecv[i-1]);
	}
	InitializeCriticalSection(&RtlSend);
}

CClientSocket::~CClientSocket()
{
	CloseHandle(_lock);//关闭事件对象
	closesocket(sock);//关闭主机
	WSACleanup();//清理网络
	sock = INVALID_SOCKET;
	DeleteCriticalSection(&RtlSend);
	for (int i = 0; i < 11; i++)
	{
		DeleteCriticalSection(&RtlRecv[i - 1]);
	}
}

CClientSocket::CClientSocket(const CClientSocket& obj)
{

}

int CClientSocket::init()
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
	if (sock == NULL)
	{
		MessageBox(NULL, "创建套接字库失败", "警告", MB_ICONWARNING);
		TRACE("创建套接字库失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	addr.sin_port = htons(nPort);
	addr.sin_addr.S_un.S_addr = htonl(nIp);
	addr.sin_family = AF_INET;
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("ip地址无效");
		return -1;
	}
	if (connect(sock, (sockaddr*)&addr, sizeof addr) < 0) {
		AfxMessageBox("链接失败");
		TRACE("connect error cmd=%d,result=%s", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	return 0;
}

void CClientSocket::UpData(int ip, int port)
{
	nIp = ip;
	nPort = port;
}

void WINAPIV CClientSocket::_threadSendPacketFun(void* ary)
{
	CClientSocket* thiz = (CClientSocket*)ary;
	while (thiz->sock != INVALID_SOCKET)
	{
		if (thiz->m_sendBag.size() != 0)
		{
			TRACE("发包cmd：%d\n", thiz->m_sendBag.front().cmd);
			EnterCriticalSection(&thiz->RtlSend);
			int res = thiz->SendMes(thiz->m_sendBag.front());
			TRACE("发包结果%d\n", res);
			thiz->m_sendBag.pop_front();
			LeaveCriticalSection(&thiz->RtlSend);
		}
		else Sleep(1);
	}
	_endthread();
}

#include <memory>
void WINAPIV CClientSocket::_threadRecvPacketFun(void* ary)
{
	CClientSocket* thiz = (CClientSocket*)ary;
	//将缓冲区设置为100mb
	constexpr int maxSize = 1024 * 1024 * 100;
	std::unique_ptr<char> ptr{ new char[maxSize] };
	memset(ptr.get(), 0, maxSize);
	UINT index = 0;
	int res = -1;
	while (thiz->sock != INVALID_SOCKET)
	{
		//1-11
		DataBag bag;
		TRACE("开始收报\n");
		res = thiz->recvMes(ptr.get(), maxSize, &index, &bag);
		TRACE("收报结束index=%d\n",index);
		if (res > 0) {
			TRACE("收报内容%d=%s\n",bag.cmd,bag.m_data.c_str());
			//EnterCriticalSection(&thiz->RtlRecv);
			auto it = thiz->m_mapAck.find(res);
			if (it != thiz->m_mapAck.end())
			{
				EnterCriticalSection(&thiz->RtlRecv[res - 1]);
				it->second.push_back(bag);
				LeaveCriticalSection(&thiz->RtlRecv[res - 1]);
			}
			//LeaveCriticalSection(&thiz->RtlRecv);
		}
	}
	ptr.reset();
	_endthread();
}

void CClientSocket::addPacket(DataBag& bag)
{
	EnterCriticalSection(&RtlSend);
	m_sendBag.push_back(bag);
	LeaveCriticalSection(&RtlSend);
}

CClientSocket* CClientSocket::getObject()
{
	if (myObject == NULL)
	{
		WaitForSingleObject(_lock, INFINITE);
		if (myObject == NULL)
		{
			myObject = new CClientSocket;
			return myObject;
		}
	}
	return myObject;
}

//1.自定义缓冲区
//2.缓冲区最大值
//3.索引
//4.数据包
int CClientSocket::recvMes(char* buf, int maxSize, UINT* index, DataBag* bag)
{
	//客户端套接字错误
	if (sock == -1 || sock == 0)
	{
		return -1;
	}
	int len{};
	if (*index) goto HEAD;
BACK:
	len = recv(sock, buf + *index, maxSize - *index, 0);//接收数据 
	if (len <= 0)//接收异常
	{
		TRACE("recv失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return len;
	}
	*index += len;//更新索引(添加接收的数据长度)
HEAD:
	int res = bag->parse(buf, *index);//解析
	if (res == -1)
	{
		goto BACK;
	}
	else if (res == -2)
	{
		return -2;
	}
	return bag->cmd;
}
//int CClientSocket::recvMes()
//{
//	//客户端套接字错误
//	if (sock == -1 || sock == 0)
//	{
//		return -1;
//	}
//	int len{};
//	static char buf[MAX_BUF]{};//缓冲区
//	static UINT index = 0;//每次数据读入存放的下标
//	if (index) goto HEAD;
//BACK:
//	len = recv(sock, buf + index, MAX_BUF - index, 0);//接收数据 
//	if (len <= 0)//接收异常
//	{
//		TRACE("recv失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
//		return len;
//	}
//	index += len;//更新索引(添加接收的数据长度)
//HEAD:
//	int res = databag.parse(buf, index);//解析
//	if (res == -1)
//	{
//		goto BACK;
//	}
//	else if (res == -2)
//	{
//		return -2;
//	}
//	return databag.cmd;
//}

int CClientSocket::SendMes(DataBag& bag)
{
	int res = send(sock, bag.DATA.c_str(), bag.GetSize(), 0);
	if (res <= 0)
	{
		TRACE("send失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
	}
	return res; 
}
std::string CClientSocket::GetResultInfo(int nCmd)
{
	std::string res;
	//取包加锁，收包解锁
	std::list<DataBag>& bag = m_mapAck.find(nCmd)->second;
	while (bag.size() == 0)
	{
		Sleep(1);
	}
	EnterCriticalSection(&RtlRecv[nCmd - 1]);
	res = bag.front().m_data;
	bag.pop_front();
	LeaveCriticalSection(&RtlRecv[nCmd - 1]);

	//if (bag.size() != 0) {
	//	//EnterCriticalSection(&RtlRecv);等待
	//	res = bag.front().m_data;
	//	bag.pop_front();
	//	//EnterCriticalSection(&RtlRecv);等待
	//}
	//else {
	//	while (true)
	//	{

	//	}
	//			
	//}


	//auto it = m_mapAck.find(nCmd);
	//if (it != m_mapAck.end())
	//{
	//	while (it->second.size() == 0)
	//	{
	//		Sleep(50);
	//	}
	//	res = it->second.front().m_data;
	//	it->second.pop_front();
	//}
	//EnterCriticalSection(&RtlRecv);
	//LeaveCriticalSection(&RtlRecv);
	return res;
}

//void CClientSocket::iocp() {
//	//创建iocp
//	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);
//	//添加监听socket
//	//CreateIoCompletionPort(sock, iocpHandle, 指针, 0);
//	//等待完成事件
//	GetQueuedCompletionStatus(iocpHandle,)
//}
