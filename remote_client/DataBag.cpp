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
