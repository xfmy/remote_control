#include "pch.h"
#include "Cmd.h"
#include "pch.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include "remote_control.h"
#include <atlimage.h>
#include "ServerSocket.h"


//获取磁盘分区信息 cmd=1
int CCommand::GetDiskInfo(std::list<DataBag>& m_list, DataBag& bag) {
	std::string result;
	for (int i = 1; i <= 26; i++)
	{
		if (_chdrive(i) == 0) {
			if (result.size() != 0)
				result += '.';
			result += ('A' + i - 1);
		}
	}
	m_list.push_back(DataBag(1, result));
	//DataBag bag(1, result);
	//int res = CServerSocket::getObject()->SendMes(bag);
	//if (res == SOCKET_ERROR)
	//{
	//	TRACE("GetDiskInfo(获取磁盘信息函数抛出异常 返回值-》)", res);
	//}
	return 0;
}


//获取目录文件信息 cmd=2

void CCommand::SendTo(DFInfo& info, std::list<DataBag>&m_list) {
	std::string str;
	str.append((char*)&info, sizeof info);
	m_list.push_back(DataBag(2, str));
}


int CCommand::GetDirFileInfo(std::list<DataBag>& m_list, DataBag& bag)
{
	_finddata_t fd;
	int fileNameSize = bag.m_data.size();
	const char* fileName = bag.m_data.c_str();

	intptr_t han;
	if (_chdir(fileName) != 0 || (han = _findfirst("*", &fd)) == -1)
	{
		DFInfo info{ false,true,false };
		memcpy_s(info.fileName, sizeof info.fileName, fd.name, strlen(fd.name));
		SendTo(info,m_list);
		TRACE("cmd=2:%s 无访问权限或没有找到任何文件", info.fileName);
		return -1;
	}
	DFInfo info;
	do
	{
		info.IsDirectory = ((fd.attrib & _A_SUBDIR) != 0);
		memset(info.fileName, 0, 256);
		memcpy_s(info.fileName, sizeof info.fileName, fd.name, strlen(fd.name));
		SendTo(info,m_list);
	} while (!_findnext(han, &fd));
	info.IsInvalid = false;
	info.IsHasNext = false;
	SendTo(info,m_list);
	return 0;
}

//cmd=3
int CCommand::runFile(std::list<DataBag>& m_list, DataBag& bag)
{
	int res = (int)ShellExecuteA(NULL, NULL, bag.m_data.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if (res < 32)
	{
		std::string str = "文件执行失败";
		m_list.push_back(DataBag(3, str));
		TRACE("cmd=3:文件执行失败");
		return -1;
	}
	std::string str = "文件执行成功";
	m_list.push_back(DataBag(3, str));
	return 1;
}
//cmd=4
int CCommand::downLoadFile(std::list<DataBag>& m_list, DataBag& bag)
{
	FILE* fd;
	errno_t err = fopen_s(&fd, bag.m_data.c_str(), "rb");
	if (err != 0 || (fd == NULL)) {
		//forwardMes(4, "文件打开失败");
		m_list.push_back(DataBag(4));
		TRACE("%s %s", bag.m_data.c_str(), "文件打开失败");
		return -1;
	}
	size_t sz{};
	do {//此处可以改成一次读1kb，读2次 while判断条件修改
		char fileFormat[3072]{};
		sz = fread(fileFormat, 1, 3072, fd);

		std::string str(fileFormat, sz);
		m_list.push_back(DataBag(4,str));
	} while (sz == 3072);
	fclose(fd);
	return 0;
}
//cmd=5
int CCommand::GetFileSize(std::list<DataBag>& m_list, DataBag& bag)
{
	ULARGE_INTEGER uliFileSize;
	uliFileSize.LowPart = GetCompressedFileSizeA(bag.m_data.c_str(), &uliFileSize.HighPart);
	//printf("%lld bytes\n%4.2f KB\n%4.2f MB\n%4.2f GB\n", uliFileSize.QuadPart, (float)uliFileSize.QuadPart / 1024, (float)uliFileSize.QuadPart / (1024 * 1024), (float)uliFileSize.QuadPart / (1024 * 1024 * 1024));
	// 64-bit file size is now in uliFileSize.QuadPart
	std::string str((const char*)&uliFileSize.QuadPart, sizeof(unsigned long long));
	m_list.push_back(DataBag(5,str));
	return 0;
}

//cmd=6

int CCommand::corsurEvent(std::list<DataBag>& m_list, DataBag& bag)
{
	MouseEvent = *(PMouseEvent)bag.m_data.c_str();
	int count = MouseEvent.DUM + MouseEvent.LR * 10;
	switch (count)
	{
	case 10:
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 11://左键按下
		//SetCursorPos(ME.P.x, ME.P.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 12://左键弹起
		//SetCursorPos(ME.P.x, ME.P.y);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 21://右键按下
		//SetCursorPos(ME.P.x, ME.P.y);
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 22://右键弹起
		//SetCursorPos(ME.P.x, ME.P.y);
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 4://鼠标移动到指定地点
		SetCursorPos(MouseEvent.P.x, MouseEvent.P.y);
		break;
	default:
		break;
	}
	return 0;
}

//cmd=7

int CCommand::screenMonitor(std::list<DataBag>& m_list, DataBag& bag)
{
	CImage Screen;
	HDC hScerrn = ::GetDC(NULL);
	int nBitperpixel = GetDeviceCaps(hScerrn, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScerrn, HORZRES);
	int nHeight = GetDeviceCaps(hScerrn, VERTRES);
	Screen.Create(nWidth, nHeight, nBitperpixel);
	BitBlt(Screen.GetDC(), 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hScerrn, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScerrn);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMem == NULL)
	{
		return -1;
	}
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMem, true, &pStream);
	if (ret == S_OK)
	{
		Screen.Save("C:\\Users\\马云\\Desktop\\bit1.png", Gdiplus::ImageFormatPNG);
		Screen.Save(pStream, Gdiplus::ImageFormatPNG);
		LARGE_INTEGER bg{ 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		const char* pData = (const char*)GlobalLock(hMem);
		size_t nSize = GlobalSize(hMem);
		std::string str(pData, nSize);
		m_list.push_back(DataBag(7,str));
		GlobalUnlock(hMem);
	}
	pStream->Release();
	GlobalFree(hMem);
	Screen.ReleaseDC();
	return 0;
}

//cmd=8
void CCommand::ThreadStart() {
	CRect rect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));//获取屏幕的大小
	isLock = true;
	//CRect rect(0, 0, 100, 100);
	dlg.Create(IDD_DIALOG_lock, NULL);
	dlg.MoveWindow(&rect);//修改大小
	rect.left = 0;
	rect.top = 0;
	rect.bottom = 1;
	rect.right = 1;
	ClipCursor(&rect);//限制鼠标范围
	ShowCursor(false);//不显示鼠标
	dlg.ShowWindow(SW_SHOW);//窗口显示
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//控制窗口大小不受改变
	::ShowWindow(::FindWindow("Shell_TrayWnd", NULL), SW_HIDE);
	while (isLock) {
		int a = 100;
		for (int i = 0; i < a; i++)
		{
			i += a;
			Sleep(500);
		}
		Sleep(500);
	}
	/**
	MSG msg;
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		ClipCursor(&rect);//限制鼠标范围
		if (msg.message == WM_KEYDOWN)
		{
			TRACE("msg:%08X wparam %08X lp:%08X\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x41)
			{
				break;
			}
		}
	}
	*/
	ShowCursor(true);//显示鼠标
	::ShowWindow(::FindWindow("Shell_trayWnd", NULL), SW_SHOW);
	dlg.DestroyWindow();
}
unsigned __stdcall CCommand::lock_function(void* thiz) {
	CCommand* th = (CCommand*)thiz;
	th->ThreadStart();
	_endthreadex(0);
	return 0;
}
int CCommand::lockMachine(std::list<DataBag>& m_list, DataBag& bag)
{
	unsigned pid;
	if (dlg.m_hWnd == NULL || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
		uintptr_t id = _beginthreadex(NULL, 0, &CCommand::lock_function, this, 0, &pid);
	}
	return 0;
}

//cmd=9
int CCommand::unlockMachine(std::list<DataBag>& m_list, DataBag& bag)
{
	isLock = false;
	return 0;
}
//cmd=10 
int CCommand::DeFile(std::list<DataBag>& m_list, DataBag& bag)
{
	BOOL res = DeleteFileA(bag.m_data.c_str());
	if (res)//如果函数成功，则返回值非零。
	{
		std::string info = "文件删除成功";
		m_list.push_back(DataBag(10,info));
	}
	else
	{
		std::string info = GetErrorInfo(GetLastError());
		m_list.push_back(DataBag(10,info));
	}
	return 0;
}

//cmd = 11
int CCommand::GetScreenRatio(std::list<DataBag>& m_list, DataBag& bag)
{
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	std::string info;
	info.append((const char*)&x, 4);
	info.append((const char*)&y, 4);
	m_list.push_back(DataBag(11,info));
	return 0;
}

CCommand::CCommand()
{
	struct
	{
		int cmd;
		CALLFUN callFun;
	}data[] = {
		{1,&CCommand::GetDiskInfo},
		{2,&CCommand::GetDirFileInfo},
		{3,&CCommand::runFile},
		{4,&CCommand::downLoadFile},
		{5,&CCommand::GetFileSize},
		{6,&CCommand::corsurEvent},
		{7,&CCommand::screenMonitor},
		{8,&CCommand::lockMachine},
		{9,&CCommand::unlockMachine},
		{10,&CCommand::DeFile},
		{11,&CCommand::GetScreenRatio},
		{-1,NULL},
	};
	for (int i = 0; -1 != data[i].cmd; i++)
	{
		m_Function.insert(std::pair<int, CALLFUN>(data[i].cmd, data[i].callFun));
	}
}
//#include <map>
int CCommand::executeCmd(int res, std::list<DataBag>& m_list, DataBag& bag)
{
	std::map<int, CALLFUN>::iterator it = m_Function.find(res);
	if (it == m_Function.end())
	{
		return -1;
	}
	return (this->*it->second)(m_list, bag);
}
