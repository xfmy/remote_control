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

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
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

			//sock->start();
			//int res{};
			/*while ((res = sock->recvMes()) != 666)
			{
				if (res > 0)
				{
					com.executeCmd(res);
				}
			}*/
			

            //TODO 此处应该承接返回值（send返回值）
		}
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
