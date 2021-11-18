#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
//*******************************************
#define MAX_BUF 4096
//*******************************************



//包的设计类
/*************************************************************************************
*9 + data（长度）
* UINT parse(char* buf, UINT& strSz)
*	参数：（缓冲区，数据大小）
*	功能： 解析包的数据，
*	返回值：0-》解析失败，其他-》buf使用大小
***************************************************************************************/
#pragma pack(push)
#pragma pack(1)
class DataBag
{
public:
	DataBag() :head(0xFEFF), len(0), cmd(0), sum(0) {}
	DataBag(const DataBag& obj);//拷贝构造函数
	DataBag& operator=(const DataBag& obj);//赋值重载运算符
	DataBag(BYTE ncmd, std::string& n_data);
	DataBag(BYTE ncmd);

	const char* package();//封包，返回数据
	UINT parse(char* buf, UINT& strSz);//解析（0：error）
	UINT GetSize();//获取包大小
public:
	WORD head;			//0XFEFF 头部	2字节
	UINT  len;			//长度			2字节
	BYTE cmd;			//命令			1字节
	std::string m_data;	//传输数据
	UINT  sum;			//校验和			4字节

	std::string DATA;	//封装之后的数据
};
#pragma pack(pop)


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
	int init(int ipAddr,int ipPort);//初始化
	//属性
private:
	
	//服务器套接字
	SOCKET sock;
	sockaddr_in addr;
	int addr_len = sizeof addr;

	//单列
	static CClientSocket* myObject;
	static HANDLE _lock;

	static deobj heleper;//自动析构使用

};

//文件信息
typedef struct _DirFileInfo
{
	bool IsInvalid{ true };//是否为有效文件		1有效 0无效
	bool IsDirectory{ true };//是否为目录			1目录 0文件
	bool IsHasNext{ true };//是否还有信息传输	1有   0无
	char fileName[256]{ 0 };
}DFInfo, * PDFInfo;
