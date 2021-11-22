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

void WINAPIV CClientController::_ThreadDoenLoadFunction(void* parametor)
{
	CClientSocket* obj = CClientSocket::getObject();
	ULONGLONG fileSize = *(ULONGLONG*)obj->databag.m_data.c_str();//文件大小

	CremoteclientDlg* pCD = (CremoteclientDlg*)parametor;

	int getsz = 0;
	std::string pathName(pCD->pathName);
	DataBag bag1(4, pathName);
	obj->SendMes(bag1);

	char* buf = new char[100 * 1024];
	memset(buf, 0, 100 * 1024);
	UINT index = 0;
	memset(&bag1, 0, sizeof bag1);
	while (obj->recvMes(buf, 100 * 1024, &index, &bag1) == 4)
	{
		if (bag1.m_data.size() == 0)
		{
			AfxMessageBox("文件打开失败");
			break;
		}
		size_t len = fwrite(bag1.m_data.c_str(), 1, bag1.m_data.size(), pCD->pfile);
		getsz += bag1.m_data.size();
		if (getsz == fileSize) {
			AfxMessageBox("文件下载完毕");
			break;
		}
	}
	CClientController::getObject()->m_dLoad.ShowWindow(SW_HIDE);
	fclose(pCD->pfile);
	delete buf;
	_endthread();
}

void __cdecl CClientController::_threadMonitor(void* th)
{
	Sleep(50);
	CremoteclientDlg* thiz = (CremoteclientDlg*)th;
	while (!CClientSocket::getObject())
	{
		Sleep(1);
	}
	CClientSocket* obj = CClientSocket::getObject();
	int bufsz = 1024 * 1024 * 10;//10mb
	char* buf = new char[bufsz];
	memset(buf, 0, bufsz);
	UINT index = 0;
	int res = 0;
	

	while (true) {
		if (thiz->isNullMonitor == false)
		{
			DataBag bag(7);
			obj->SendMes(bag);
			res = obj->recvMes(buf, bufsz, &index, &bag);

			if (res > 0 && bag.cmd == 7)
			{

				BYTE* data = (BYTE*)bag.m_data.c_str();

				HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
				if (hMen == NULL)
				{
					TRACE("内存不足");
					Sleep(1);
					continue;
				}
				IStream* iStr = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMen, TRUE, &iStr);
				if (hRet == S_OK)
				{
					ULONG len = 0;
					iStr->Write(data, bag.m_data.size(), &len);
					LARGE_INTEGER bg{ 0 };
					iStr->Seek(bg, STREAM_SEEK_SET, NULL);
					thiz->imageMonitor.Load(iStr);
					thiz->isNullMonitor = true;
				}
			}
			Sleep(1);
			memset(buf, 0, bag.DATA.size());
			index = 0;

		} Sleep(1);
	}
	delete[] buf;
	_endthread();
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

int CClientController::RecvCommand()
{
	return CClientSocket::getObject()->recvMes();
}

const std::string& CClientController::getResult()
{
	return CClientSocket::getObject()->databag.m_data;
}
