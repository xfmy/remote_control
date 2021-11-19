#pragma once
#include "ClientSocket.h"
#include "remote_clientDlg.h"
#include "SreenMonitor.h"
#include "m_downLoad.h"
#include <map>
#include "resource.h"

#define WM_SEND_PACKET			(WM_USER + 1)
#define WM_SEND_DATA			(WM_USER + 2)
#define WM_SHOW_DOWNLOAD		(WM_USER + 3)
#define WM_SHOW_SREENMONITER	(WM_USER + 4)
#define WM_SEND_MESSAGE			(WM_USER + 1000)

//UINT        message;
//WPARAM      wParam;
//LPARAM      lParam;
class CClientController
{
	CClientController();
	~CClientController() = default;

	//单列销毁
	class deobj
	{
	public:
		deobj() {
			CClientController::getObject();
		}
		~deobj() {	//删除单列
			if (CClientController::myObject != nullptr)
			{
				delete CClientController::myObject;
				CClientController::myObject = nullptr;
			}
		}
	};

public:
	//初始化
	int init();
	//启动
	int invoke(CWnd* pWnd);

	static CClientController* getObject();
private:
	static CClientController* myObject;
	typedef LRESULT (CClientController::* LPFUN)(UINT,WPARAM, LPARAM);
	static unsigned __stdcall ThreadEntry(void*);
	//-1自定义消息没找到
	void ThreadFun();
	LRESULT SendMSG(MSG& msg);

private:
	LRESULT OnSendPacket(UINT, WPARAM, LPARAM);
	LRESULT OnSendData(UINT, WPARAM, LPARAM);
	LRESULT OnShowDownLoad(UINT, WPARAM, LPARAM);
	LRESULT OnShowScreenMoniter(UINT, WPARAM, LPARAM);

private:
	static std::map<UINT, LPFUN> m_mapFun;
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

	CremoteclientDlg dlg;
	m_downLoad m_dLoad;
	CSreenMonitor m_screenMonitor;
};
CClientController* CClientController::myObject = nullptr;
