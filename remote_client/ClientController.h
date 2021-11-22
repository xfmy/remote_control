#pragma once
#include <map>
#include "ClientSocket.h"

#include "m_downLoad.h"
#include "resource.h"

#define WM_SEND_PACKET			(WM_USER + 1)
#define WM_SEND_DATA			(WM_USER + 2)
#define WM_SHOW_DOWNLOAD		(WM_USER + 3)
#define WM_SHOW_SREENMONITER	(WM_USER + 4)
#define WM_SEND_MESSAGE			(WM_USER + 1000)

#include "remote_clientDlg.h"
#include "SreenMonitor.h"
class CClientController
{
	CClientController();
	~CClientController() = default;

	//单列销毁
	class deobj
	{
	public:
		deobj() {
			//CClientController::getObject();
		}
		~deobj() {	//删除单列
			if (CClientController::CtlObject != nullptr)
			{
				delete CClientController::CtlObject;
				CClientController::CtlObject = nullptr;
			}
		}
	};

public:
	CremoteclientDlg dlg;
	m_downLoad m_dLoad;
	CSreenMonitor m_screenMonitor;
	//获取控件组合文件名
	CString GetRemotePathName(HTREEITEM hTree);
	CString GetRemoteFilePathName();
	//初始化
	int init();
	//启动
	int invoke(CWnd* pWnd);
	//向数据转发至网络层
	int SendCommandPacket(int cmd,std::string& buf);
	int SendCommandPacket(int cmd);
	int RecvCommand();
	//recvMes(buf,100*1024,&index,&bag1)
	//recvMes(char* buf, int maxSize, UINT* index, DataBag* bag)
	int RecvCommand(char* buf, int maxSize, UINT* index, DataBag* bag);
	int GetImage();
	const std::string& getResult();
	//获取单列
	static CClientController* getObject();
	static void WINAPIV _ThreadDoenLoadFunction(void* parametor);
	static void __cdecl _threadMonitor(void*);
private:
	typedef LRESULT(CClientController::* LPFUN)(UINT, WPARAM, LPARAM);
	static std::map<UINT, LPFUN> m_mapFun;
	static CClientController* CtlObject;
	LRESULT SendMSG(MSG& msg);
	static unsigned __stdcall ThreadEntry(void*);
	//-1自定义消息没找到
	void ThreadFun();

	LRESULT OnSendPacket(UINT, WPARAM, LPARAM);
	LRESULT OnSendData(UINT, WPARAM, LPARAM);
	LRESULT OnShowDownLoad(UINT, WPARAM, LPARAM);
	LRESULT OnShowScreenMoniter(UINT, WPARAM, LPARAM);

private:
	static deobj DeObj;
	HANDLE m_threadHandle;
	unsigned m_threadPid;
	typedef struct _MsgInfo 
	{
		LRESULT res;
		MSG		msg;
		_MsgInfo(MSG& _msg) {
			res = 0;
			msg = _msg;
		}
	}MSGINFO;
	
};

