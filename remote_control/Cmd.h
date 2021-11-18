#pragma once
#pragma once
#include "pch.h"
#include "LockInfoDialog.h"
#include "ServerSocket.h"
#include <map>
#include "DataBag.h"
#include <list>
//#include "framework.h"
/*********************************************************
*							CMD
* 1.��ȡ���̷���
* 2.��ȡ�ļ�Ŀ¼��Ϣ
* 3.ָ���ļ�ִ��
* 4.�����ļ�->һ�δ���1024�ֽڣ�С��1024��ʾ�������һ����
* 5.��ȡ�ļ���С
* 6.����¼�
* 7.��Ļ���
* 8.����
* 9.����
* 10.ɾ���ļ�
* 11.��ȡ��Ļ�ֱ���
*********************************************************/
typedef struct _MouseEvent
{
	int LR = -1;// 1��� 2�Ҽ� 
	int DUM = -1;//1���� 2���� 3�ƶ�
	POINT P{ 0,0 };//����
}*PMouseEvent;


class CCommand {
public:
	CCommand();
	~CCommand() = default;
	int executeCmd(int res,std::list<DataBag>& m_list, DataBag& bag);
protected:
	typedef int(CCommand::* CALLFUN)(std::list<DataBag>&, DataBag&);
	std::map<int, CALLFUN> m_Function;
	_MouseEvent MouseEvent;
	CLockInfoDialog dlg;
	bool isLock = true;
protected:
	int GetDiskInfo(std::list<DataBag>& , DataBag&);//1
	int GetDirFileInfo(std::list<DataBag>&, DataBag&);//2
	int runFile(std::list<DataBag>&, DataBag&);//3
	int downLoadFile(std::list<DataBag>&, DataBag&);//4 
	int GetFileSize(std::list<DataBag>&, DataBag&);//5
	int corsurEvent(std::list<DataBag>&, DataBag&);//6
	int screenMonitor(std::list<DataBag>&, DataBag&);//7
	int lockMachine(std::list<DataBag>&, DataBag&);//8
	int unlockMachine(std::list<DataBag>&, DataBag&);//9
	int DeFile(std::list<DataBag>&, DataBag&);//10
	int GetScreenRatio(std::list<DataBag>&, DataBag&);//11

	void SendTo(DFInfo& info, std::list<DataBag>& m_list);
	static unsigned __stdcall lock_function(void*);
	void ThreadStart();
};