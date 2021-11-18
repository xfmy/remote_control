
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
}

CClientSocket::~CClientSocket()
{
	CloseHandle(_lock);//关闭事件对象
	closesocket(sock);//关闭主机
	WSACleanup();//清理网络
}

int CClientSocket::init(int ipAddr, int ipPort)
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
	addr.sin_port = htons(ipPort);
	addr.sin_addr.S_un.S_addr = htonl(ipAddr);
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
int CClientSocket::recvMes()
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
	len = recv(sock, buf + index, MAX_BUF - index, 0);//接收数据 
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

int CClientSocket::SendMes(DataBag& bag)
{
	int res = send(sock, bag.DATA.c_str(), bag.GetSize(), 0);
	if (res <= 0)
	{
		TRACE("send失败->%s", GetErrorInfo(WSAGetLastError()).c_str());
	}
	return res;
}


DataBag::DataBag(const DataBag& obj)
{
	head = obj.head;
	len = obj.len;
	cmd = obj.cmd;
	m_data = obj.m_data;
	sum = obj.sum;
}

DataBag& DataBag::operator=(const DataBag& obj)
{
	if (&obj == this) return *this;

	head = obj.head;
	len = obj.len;
	cmd = obj.cmd;
	m_data = obj.m_data;
	sum = obj.sum;

	return *this;
}

DataBag::DataBag(BYTE ncmd, std::string& n_data) :head(0xFEFF)
{
	this->len = (UINT)n_data.size() + (sizeof ncmd) + (sizeof sum);
	this->cmd = ncmd;
	this->m_data = n_data;
	this->sum = ncmd;
	const BYTE* str = (const BYTE*)this->m_data.c_str();
	UINT strzs = this->m_data.size();
	for (UINT i = 0; i < strzs; i++)
	{
		this->sum += (BYTE)*(str + i);
	}
	this->package();
}
DataBag::DataBag(BYTE ncmd) :head(0xFEFF), cmd(ncmd),len(sizeof cmd + sizeof sum),sum(ncmd)
{
	m_data.clear();
	this->package();
}


const char* DataBag::package()
{
	DATA.clear();
	DATA.append((const char*)&head, sizeof head);
	DATA.append((const char*)&len, sizeof len);
	DATA.append((const char*)&cmd, sizeof cmd);
	DATA.append(m_data.c_str(), m_data.size());
	DATA.append((char*)&sum, sizeof sum);
	return DATA.c_str();
}


//返回值：使用的长度--strsz 缓存区有效大小
UINT DataBag::parse(char* buf, UINT& strSz)//buf：数据 strsz：数据长度
{
	UINT i = 0;//索引

	//寻找包头
	for (; i < strSz; i++)
	{
		if (0xFEFF == *(WORD*)(buf + i)) {
			head = 0xFEFF;
			i += sizeof head;//i->len
			break;
		}
	}
	if (i + sizeof len + sizeof cmd + sizeof sum > strSz)//长度不够
	{
		return -1;
	}
	len = *(UINT*)(buf + i);//获取len
	i += sizeof len;//i->cmd
	if (len < 5 || (len + sizeof sum) > strSz)//len过小或者 理论长度》实际长度
	{
		return -1;
	}
	cmd = *(BYTE*)(buf + i);//获取cmd指令
	i += sizeof cmd;//i-》数据或者和校验
	if (len > 5)//解析数据
	{
		m_data.clear();
		m_data.append(buf + i, len - sizeof cmd - sizeof sum);
		i += len - sizeof cmd - sizeof sum;//i->sum
	}
	sum = *(UINT*)(buf + i);//获取和校验
	UINT sun{};
	for (UINT j = i + sizeof sum - len; j < i; j++)//和校验()
	{
		sun += (BYTE)buf[j];
	}
	if (sun != sum)
	{
		return -2;
	}
	i += sizeof sum;//i->数据包的最后一个字节
	//数据迁移
	for (UINT j = 0; j < strSz - i; j++)
	{
		buf[j] = buf[i + j];
	}
	//memset(buf + i, 0, strSz - len - 4);//迁移后对数据进行清理
	memset(buf + strSz - i, 0, i);//迁移后对数据进行清理
	strSz -= i;//更新缓冲区实际存储大小
	return i;//i==整个包的长度==使用了i字节
}

UINT DataBag::GetSize()
{
	return len + (sizeof head) + (sizeof len);
}
