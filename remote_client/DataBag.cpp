#include "pch.h"
#include "DataBag.h"

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
		this->sum += (BYTE) * (str + i);
	}
	this->package();
}
DataBag::DataBag(BYTE ncmd) :head(0xFEFF), cmd(ncmd), len(sizeof cmd + sizeof sum), sum(ncmd)
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
