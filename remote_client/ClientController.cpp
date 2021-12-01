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
	//int a = m_dLoad.Create(IDD_DLG_DOWNLOAD, &dlg);
	return 0;
}

int CClientController::invoke(CWnd* pWnd)
{
	pWnd = &dlg;
	return dlg.DoModal();
}

void CClientController::SendCommandPacket(int cmd, std::string& buf)
{
	DataBag bag(cmd, buf);
	CClientSocket* obj = CClientSocket::getObject();
	//EnterCriticalSection(&obj->RtlSend);
	CClientSocket::getObject()->addPacket(bag);
	//LeaveCriticalSection(&obj->RtlSend);
	//return CClientSocket::getObject()->SendMes(bag);
}

void CClientController::SendCommandPacket(int cmd)
{
	DataBag bag(cmd);
	CClientSocket* obj = CClientSocket::getObject();
	//EnterCriticalSection(&obj->RtlSend);
	CClientSocket::getObject()->addPacket(bag);
	//LeaveCriticalSection(&obj->RtlSend);
	//return CClientSocket::getObject()->SendMes(bag);
}

CClientController* CClientController::getObject()
{
	if (CtlObject == nullptr)
	{
		CtlObject = new CClientController;
		constexpr int a = sizeof(CClientSocket);
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
	//CClientSocket* obj = CClientSocket::getObject();
	CClientController* obj = CClientController::getObject();
	//ULONGLONG fileSize = *(ULONGLONG*)obj->databag.m_data.c_str();//文件大小

	CremoteclientDlg* pCD = (CremoteclientDlg*)parametor;

	int getsz = 0;
	std::string pathName(pCD->pathName);
	//DataBag bag1(4, pathName);
	//obj->SendMes(bag1);
	obj->SendCommandPacket(4, pathName);
	//char* buf = new char[100 * 1024];
	//memset(buf, 0, 100 * 1024);
	//UINT index = 0;
	//memset(&bag1, 0, sizeof bag1);
	//while (obj->recvMes(buf, 100 * 1024, &index, &bag1) == 4)
	std::string res;
	while((res = obj->RecvCommand(4)).size() != 0)
	{
		//if (bag1.m_data.size() == 0)
		//{
		//	AfxMessageBox("文件打开失败");
		//	break;
		//}
		size_t len = fwrite(res.c_str(), 1, res.size(), pCD->pfile);
		getsz += res.size();
		if (getsz == obj->fileSize) {
			AfxMessageBox("文件下载完毕");
			break;
		}
	}
	CClientController::getObject()->m_dLoad.ShowWindow(SW_HIDE);
	fclose(pCD->pfile);
	//delete buf;
	_endthread();
}


void __cdecl CClientController::_threadMonitor(void* th)
{
	Sleep(50);
	
	CremoteclientDlg* thiz = (CremoteclientDlg*)th;
	CClientController* obj = CClientController::getObject();
	while (!CClientSocket::getObject())
	{
		Sleep(1);
	}
	DWORD time = GetCurrentTime();
	while (obj->m_screenMonitor.isShou) {
		DataBag bag(7);

		if (GetCurrentTime() - time < 50)
		{
			Sleep(50 - (GetCurrentTime() - time));
		}
		time = GetCurrentTime();
		
		obj->SendCommandPacket(7);
		std::string res = obj->RecvCommand(7);
		if (res.size() > 0)
		{
			BYTE* data = (BYTE*)res.c_str();
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
				iStr->Write(data, res.size(), &len);

				LARGE_INTEGER bg{ 0 };
				iStr->Seek(bg, STREAM_SEEK_SET, NULL);

				thiz->imageMonitor.Load(iStr);
				iStr->Release();
				if (obj->m_screenMonitor.isShou){
					CRect rect;
					obj->m_screenMonitor.m_IconAct.GetWindowRect(rect);
					//dig->imageMonitor.BitBlt(m_IconAct.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
					CDC* dc = obj->m_screenMonitor.m_IconAct.GetDC();
					thiz->imageMonitor.StretchBlt(dc->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
					obj->m_screenMonitor.m_IconAct.ReleaseDC(dc);
					obj->m_screenMonitor.m_IconAct.InvalidateRect(NULL);
				}
			}
			thiz->imageMonitor.Destroy();
			GlobalFree(hMen);
		}
		Sleep(30);
	}
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

std::string CClientController::RecvCommand(int nCmd)
{
	return CClientSocket::getObject()->GetResultInfo(nCmd);
		//return CClientSocket::getObject()->recvMes();
}

//void __cdecl CClientController::_threadDialogDomel(void*arg) {
//	//CClientController::getObject()->m_screenMonitor.DoModal();
//	//ctl->m_dLoad.Create(IDD_DLG_DOWNLOAD, this);
//	CClientController::getObject()->m_screenMonitor.Create(IDD_DIALOG_SCREEN);
//	CClientController::getObject()->m_screenMonitor.ShowWindow(SW_SHOW);
//	while (true)
//	{
//
//	}
//	_endthread();
//}

//const std::string& CClientController::getResult()
//{
//	return CClientSocket::getObject()->databag.m_data;
//}
