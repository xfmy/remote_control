#pragma once
#include <string>
//���������
/*************************************************************************************
*9 + data�����ȣ�
* UINT parse(char* buf, UINT& strSz)
*	�������������������ݴ�С��
*	���ܣ� �����������ݣ�
*	����ֵ��0-������ʧ�ܣ�����-��bufʹ�ô�С
***************************************************************************************/
#pragma pack(push)
#pragma pack(1)
class DataBag
{
public:
	DataBag() :head(0xFEFF), len(0), cmd(0), sum(0) {}
	DataBag(const DataBag& obj);//�������캯��
	DataBag& operator=(const DataBag& obj);//��ֵ���������
	DataBag(BYTE ncmd, std::string& n_data);
	DataBag(BYTE ncmd);

	const char* package();//�������������
	UINT parse(char* buf, UINT& strSz);//������0��error��
	UINT GetSize();//��ȡ����С
public:
	WORD head;			//0XFEFF ͷ��	2�ֽ�
	UINT  len;			//����			2�ֽ�
	BYTE cmd;			//����			1�ֽ�
	std::string m_data;	//��������
	UINT  sum;			//У���			4�ֽ�

	std::string DATA;	//��װ֮�������
};
#pragma pack(pop)


//�ļ���Ϣ
typedef struct _DirFileInfo
{
	bool IsInvalid{ true };//�Ƿ�Ϊ��Ч�ļ�		1��Ч 0��Ч
	bool IsDirectory{ true };//�Ƿ�ΪĿ¼			1Ŀ¼ 0�ļ�
	bool IsHasNext{ true };//�Ƿ�����Ϣ����	1��   0��
	char fileName[256]{ 0 };
}DFInfo, * PDFInfo;