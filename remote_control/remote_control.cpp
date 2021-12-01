// remote_control.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "remote_control.h"
#include "ServerSocket.h"//服务器套接字类
#include "Cmd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void RunAsAdmin();
bool GetProgramPrivilege();
bool init();
int main()
{
    //管理员权限处理
    if (!GetProgramPrivilege()) RunAsAdmin();

	if (init() == 1) return 1;

	CCommand com;
	CServerSocket* sock = CServerSocket::getObject();
	int res = sock->Run(&CServerSocket::RunCommand, &com);
	switch (res)
	{
	case -1:
		AfxMessageBox("初始化出现异常，构造结束");
		break;
	case -2:
		AfxMessageBox("监听异常");
		break;
	default:
		break;
	}
    return 0;
}

bool GetProgramPrivilege() {
	HANDLE hToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TRACE("打开进程token失败");
		return false;
	}
	TOKEN_ELEVATION eve;
	DWORD len = 0;
	if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof eve, &len) == false)
	{
		TRACE("获取tolen信息失败");
		return false;
	}
	CloseHandle(hToken);
	if (len == sizeof eve)
	{
		return eve.TokenIsElevated;
	}
	TRACE("长度不对");
	return false;
}
void RunAsAdmin()
{
	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	WCHAR sPath[MAX_PATH] = L"";
	GetModuleFileNameW(NULL, sPath, MAX_PATH);
	int ret = CreateProcessWithLogonW(L"Administrator", NULL, NULL,
		LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT,
		NULL, NULL, &si, &pi);
	if (!ret)
	{
		TRACE("创建失败");
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	exit(0);
}

bool init() {
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule != nullptr)
	{
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
	}
	else
	{
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}
	return nRetCode;
}