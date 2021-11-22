#include "pch.h"
#include "ClientController.h"

CClientController* CClientController::CtlObject = nullptr;
CClientController::deobj CClientController::DeObj;

typedef LRESULT(CClientController::* LPFUN)(UINT, WPARAM, LPARAM);
std::map<UINT, LPFUN> CClientController::m_mapFun;

CClientController::CClientController()
	:m_threadPid(-1), 
	m_dLoad(&dlg),
	m_threadHandle(INVALID_HANDLE_VALUE),
	m_screenMonitor(&dlg)
{
	
}

int CClientController::init()
{
	struct
	{
		UINT _int;
		LPFUN _fun;
	}data[]{
		{WM_SEND_PACKET,&CClientController::OnSendPacket},
		{WM_SEND_DATA,&CClientController::OnSendData},
		{WM_SHOW_DOWNLOAD,&CClientController::OnShowDownLoad},
		{WM_SHOW_SREENMONITER,&CClientController::OnShowScreenMoniter},
		{(UINT)-1,NULL},
	};
	for (int i = 0; data[i]._int != -1; i++)
	{
		m_mapFun.insert(std::pair<UINT, LPFUN>(data[i]._int, data[i]._fun));
	}
	

	m_threadHandle = (HANDLE)_beginthreadex(0, 0,
		&CClientController::ThreadEntry, this, 0, &m_threadPid);
	//int res = m_screenMonitor.Create(IDD_DIALOG_SCREEN, &dlg);

	return 0;
}

int CClientController::invoke(CWnd* pWnd)
{
	pWnd = &dlg;
	return dlg.DoModal();
}

int CClientController::SendCommandPacket(int cmd, std::string& buf)
{
	DataBag bag(cmd, buf);
	return CClientSocket::getObject()->SendMes(bag);
}

int CClientController::SendCommandPacket(int cmd)
{
	DataBag bag(cmd);
	return CClientSocket::getObject()->SendMes(bag);
}

CClientController* CClientController::getObject()
{
	if (CtlObject == nullptr)
	{
		CtlObject = new CClientController;
	}
	return CtlObject;
}


unsigned __stdcall CClientController::ThreadEntry(void* th)
{
	CClientController* thiz = (CClientController*)th;
	thiz->ThreadFun();
	_endthreadex(0);
	return 0;
}
//SendMessage
void CClientController::ThreadFun()
{
	MSG msg;
	while (::GetMessageA(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			HANDLE handle = (HANDLE)msg.lParam;
			MSGINFO* info = (MSGINFO*)msg.wParam;
			auto value = m_mapFun.find(msg.message);
			if (value != m_mapFun.end())
			{
				info->res = (this->*value->second)(info->msg.message,
					info->msg.wParam, info->msg.lParam);
			}
			else info->res = -1;
			SetEvent(handle);
		}
		else
		{
			auto value = m_mapFun.find(msg.message);
			if (value != m_mapFun.end())
				(this->*value->second)(msg.message,
					msg.wParam, msg.lParam);
		}
	}
}

LRESULT CClientController::SendMSG(MSG& msg)
{
	MSGINFO info(msg);
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)	return -2;
	PostThreadMessage(m_threadPid, WM_SEND_MESSAGE,
		(WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
	return info.res;
}
CString CClientController::GetRemotePathName(HTREEITEM hTree)
{
	CString strpath, str;
	do
	{
		str = dlg.m_tree.GetItemText(hTree);
		strpath = str + "\\" + strpath;
		hTree = dlg.m_tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strpath;
}
CString CClientController::GetRemoteFilePathName()
{
	int fileSelectName = dlg.m_fileList.GetSelectionMark();
	CString fileName = dlg.m_fileList.GetItemText(fileSelectName, 0);
	CString pathName = GetRemotePathName(dlg.m_tree.GetSelectedItem());
	pathName += fileName;
	return pathName;
}

LRESULT CClientController::OnSendPacket(UINT message,
	WPARAM wParam, LPARAM lParam)
{
	return CClientSocket::getObject()->SendMes(*(DataBag*)lParam);
}

LRESULT CClientController::OnSendData(UINT message,
	WPARAM wParam, LPARAM lParam)
{
	//TODO SendData
	return LRESULT();
}

LRESULT CClientController::OnShowDownLoad(UINT message,
	WPARAM wParam, LPARAM lParam)
{
	return m_dLoad.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowScreenMoniter(UINT message,
	WPARAM wParam, LPARAM lParam)
{
	return m_screenMonitor.DoModal();
}