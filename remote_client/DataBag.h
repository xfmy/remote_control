#pragma once
#include <string>
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


//文件信息
typedef struct _DirFileInfo
{
	bool IsInvalid{ true };//是否为有效文件		1有效 0无效
	bool IsDirectory{ true };//是否为目录			1目录 0文件
	bool IsHasNext{ true };//是否还有信息传输	1有   0无
	char fileName[256]{ 0 };
}DFInfo, * PDFInfo;