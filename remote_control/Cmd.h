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
* 1.获取磁盘分区
* 2.获取文件目录信息
* 3.指定文件执行
* 4.下载文件->一次传输1024字节，小于1024表示这是最后一个包
* 5.获取文件大小
* 6.鼠标事件
* 7.屏幕监控
* 8.加锁
* 9.解锁
* 10.删除文件
* 11.获取屏幕分辨率
*********************************************************/
typedef struct _MouseEvent
{
	int LR = -1;// 1左键 2右键 
	int DUM = -1;//1按下 2弹起 3移动
	POINT P{ 0,0 };//坐标
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