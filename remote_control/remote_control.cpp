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

//int executeCmd(int res);//执行指令

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

bool GetProgramPrivilege() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
    {
        TRACE("打开进程token失败");
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken,TokenElevation,&eve,sizeof eve,&len) == false)
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
    HANDLE hToken = nullptr;
    int ret = LogonUser("Administrator", NULL, NULL, LOGON32_LOGON_BATCH, 
        LOGON32_PROVIDER_DEFAULT, &hToken);
    if (!ret)
    {
        TRACE("登录失败");
        CloseHandle(hToken);
        return;
    }
    STARTUPINFOW si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    WCHAR sPath[MAX_PATH] = L"";
    GetCurrentDirectoryW(MAX_PATH, sPath);
    CStringW strCmd = sPath;
    strCmd += "\\remote_control.exe";
    
    
    ret = CreateProcessWithLogonW(L"Administrator", NULL, NULL,
        LOGON_WITH_PROFILE, NULL, strCmd.GetBuffer(), CREATE_UNICODE_ENVIRONMENT,
        NULL, NULL, &si, &pi);
    ret = GetLastError();
    CloseHandle(hToken);
    if (!ret)
    {
		TRACE("创建失败");
        return;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
int main()
{
    //TODO 转到run函数执行
    if (!GetProgramPrivilege())
    {
        RunAsAdmin();
    }


    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
		else
		{
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
		}
    }
    else
    {
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
