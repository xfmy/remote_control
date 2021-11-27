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
	CloseHandle(_lock);//�ر��¼�����
	closesocket(sock);//�ر�����
	closesocket(sock_client);//�رշֻ�
	WSACleanup();//��������
}

int CServerSocket::init(int port)
{
	//�����׽��ֿ�
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	//��ʼ���׽��ֿ�
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		MessageBox(NULL, "��ʼ���׽��ֿ��һ��ʧ��", "����", MB_ICONWARNING);
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		MessageBox(NULL, "��ʼ���׽��ֿ�ڶ���ʧ��", "����", MB_ICONWARNING);
		return-1;
	}
	//�����׽���
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(NULL, "�����׽��ֿ�ʧ��", "����", MB_ICONWARNING);
		TRACE("�����׽��ֿ�ʧ��->%s",GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	//��ip�Ͷ˿�
	if (bind(sock, (sockaddr*)&addr, sizeof addr) == SOCKET_ERROR)
	{
		MessageBox(NULL, "������bind����ֵΪSOCKET_ERROR", "����", MB_ICONWARNING);
		TRACE("bindʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
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
			//AfxMessageBox("recvMes����ֵ�쳣");
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
		TRACE("ִ������ʧ��");
	}
}

bool CServerSocket::start(int count)
{
	//����
	if (listen(sock, count) == SOCKET_ERROR)
	{
		MessageBox(NULL, "������listen����ֵΪSOCKET_ERROR", "����", MB_ICONWARNING);
		TRACE("listenʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return false;
	}
	this->count = count;

	//��������
	int i = 0;
	while (i++ < count)
	{
		sock_client = accept(sock, (sockaddr*)&addr, &addr_len);
		if (sock_client == INVALID_SOCKET)
		{
			AfxMessageBox("��⵽accept����ֵΪINVALID_SOCKET");
			TRACE("acceptʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
			i--;
		}
		UCHAR* pIp = (UCHAR*)&addr.sin_addr.S_un.S_addr;
		USHORT pt;
		memcpy(&pt, (char*)&addr.sin_port + 1, 1);
		memcpy((char*)&pt + 1, (char*)&addr.sin_port, 1);
		TRACE("����˽��ճɹ� �˿ں�%d,ip==%d.%d.%d.%d\n", pt, *pIp, *(pIp + 1), *(pIp + 2), *(pIp + 3));
	}
	//TODO accept,send.recv-----io����---���߳�
	return true;
}

int CServerSocket::recvMes()
{
	//�ͻ����׽��ִ���
	if (sock == -1 || sock == 0)
	{
		return -1;
	}
	int len{};
	static char buf[MAX_BUF]{};//������
	static UINT index = 0;//ÿ�����ݶ����ŵ��±�
	if (index) goto HEAD;
BACK:
		len = recv(sock_client, buf + index, MAX_BUF - index, 0);//�������� 
		if (len <= 0)//�����쳣
		{
			TRACE("recvʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
			return len;
		}
	index += len;//��������(��ӽ��յ����ݳ���)
HEAD:
	int res = databag.parse(buf, index);//����
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
		TRACE("sendʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
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