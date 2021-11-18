
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
	CloseHandle(_lock);//�ر��¼�����
	closesocket(sock);//�ر�����
	WSACleanup();//��������
}

int CClientSocket::init(int ipAddr, int ipPort)
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
	if (sock == NULL)
	{
		MessageBox(NULL, "�����׽��ֿ�ʧ��", "����", MB_ICONWARNING);
		TRACE("�����׽��ֿ�ʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return -1;
	}
	addr.sin_port = htons(ipPort);
	addr.sin_addr.S_un.S_addr = htonl(ipAddr);
	addr.sin_family = AF_INET;
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("ip��ַ��Ч");
		return -1;
	}
	if (connect(sock, (sockaddr*)&addr, sizeof addr) < 0) {
		AfxMessageBox("����ʧ��");
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

//1.�Զ��建����
//2.���������ֵ
//3.����
//4.���ݰ�
int CClientSocket::recvMes(char* buf, int maxSize, UINT* index, DataBag* bag)
{
	//�ͻ����׽��ִ���
	if (sock == -1 || sock == 0)
	{
		return -1;
	}
	int len{};
	if (*index) goto HEAD;
BACK:
	len = recv(sock, buf + *index, maxSize - *index, 0);//�������� 
	if (len <= 0)//�����쳣
	{
		TRACE("recvʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
		return len;
	}
	*index += len;//��������(��ӽ��յ����ݳ���)
HEAD:
	int res = bag->parse(buf, *index);//����
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
	len = recv(sock, buf + index, MAX_BUF - index, 0);//�������� 
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

int CClientSocket::SendMes(DataBag& bag)
{
	int res = send(sock, bag.DATA.c_str(), bag.GetSize(), 0);
	if (res <= 0)
	{
		TRACE("sendʧ��->%s", GetErrorInfo(WSAGetLastError()).c_str());
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


//����ֵ��ʹ�õĳ���--strsz ��������Ч��С
UINT DataBag::parse(char* buf, UINT& strSz)//buf������ strsz�����ݳ���
{
	UINT i = 0;//����

	//Ѱ�Ұ�ͷ
	for (; i < strSz; i++)
	{
		if (0xFEFF == *(WORD*)(buf + i)) {
			head = 0xFEFF;
			i += sizeof head;//i->len
			break;
		}
	}
	if (i + sizeof len + sizeof cmd + sizeof sum > strSz)//���Ȳ���
	{
		return -1;
	}
	len = *(UINT*)(buf + i);//��ȡlen
	i += sizeof len;//i->cmd
	if (len < 5 || (len + sizeof sum) > strSz)//len��С���� ���۳��ȡ�ʵ�ʳ���
	{
		return -1;
	}
	cmd = *(BYTE*)(buf + i);//��ȡcmdָ��
	i += sizeof cmd;//i-�����ݻ��ߺ�У��
	if (len > 5)//��������
	{
		m_data.clear();
		m_data.append(buf + i, len - sizeof cmd - sizeof sum);
		i += len - sizeof cmd - sizeof sum;//i->sum
	}
	sum = *(UINT*)(buf + i);//��ȡ��У��
	UINT sun{};
	for (UINT j = i + sizeof sum - len; j < i; j++)//��У��()
	{
		sun += (BYTE)buf[j];
	}
	if (sun != sum)
	{
		return -2;
	}
	i += sizeof sum;//i->���ݰ������һ���ֽ�
	//����Ǩ��
	for (UINT j = 0; j < strSz - i; j++)
	{
		buf[j] = buf[i + j];
	}
	//memset(buf + i, 0, strSz - len - 4);//Ǩ�ƺ�����ݽ�������
	memset(buf + strSz - i, 0, i);//Ǩ�ƺ�����ݽ�������
	strSz -= i;//���»�����ʵ�ʴ洢��С
	return i;//i==�������ĳ���==ʹ����i�ֽ�
}

UINT DataBag::GetSize()
{
	return len + (sizeof head) + (sizeof len);
}
