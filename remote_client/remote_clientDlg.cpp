
// remote_clientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "remote_client.h"
#include "remote_clientDlg.h"
#include "afxdialogex.h"
#include "ClientSocket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "SreenMonitor.h"
#include "ClientController.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CremoteclientDlg 对话框



CremoteclientDlg::CremoteclientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTE_CLIENT_DIALOG, pParent)
	, m_ipAddr(0)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//获取CS屏幕分辨率
}

void CremoteclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_IP, m_ipAddr);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_TREE_FILE, m_tree);
	DDX_Control(pDX, IDC_LIST2, m_fileList);
}

BEGIN_MESSAGE_MAP(CremoteclientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_DEMO, &CremoteclientDlg::OnBnClickedButtonDemo)
	ON_BN_CLICKED(IDC_BUTTON_DEMO2, &CremoteclientDlg::OnBnClickedButtonDemo2)
//	ON_NOTIFY(NM_DBLCLK, IDC_TREE_FILE, &CremoteclientDlg::OnNMDblclkTreeFile)
	ON_NOTIFY(NM_CLICK, IDC_TREE_FILE, &CremoteclientDlg::OnNMClickTreeFile)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CremoteclientDlg::OnNMRClickList2)
	ON_COMMAND(ID_EXEC_FILE, &CremoteclientDlg::OnExecFile)
	ON_COMMAND(ID_DEL_FILE, &CremoteclientDlg::OnDelFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CremoteclientDlg::OnDownloadFile)
	ON_COMMAND(ID_getFileSize, &CremoteclientDlg::Ongetfilesize)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_IP, &CremoteclientDlg::OnIpnFieldchangedIpaddressIp)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CremoteclientDlg::OnEnChangeEditPort)
END_MESSAGE_MAP()


// CremoteclientDlg 消息处理程序

BOOL CremoteclientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);//***************************************************************************
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//在此添加额外的初始化代码
	//m_ipAddr = 0x2AC02902;
	m_ipAddr = 0x7F000001;
	m_port = "18086";
	CClientSocket::getObject()->UpData(m_ipAddr, atoi(m_port.GetBuffer()));
	UpdateData(false);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CremoteclientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CremoteclientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CremoteclientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//进入用户代码段

//迁移及问题解决
//1.屏幕监控线程
//2.文件下载线程
//image需要再次进行封装控制层

//屏幕监控线程
//void __cdecl CremoteclientDlg::_threadMonitor(void* th)//th:  CremoteclientDlg对象
//{
//	Sleep(50);
//	CremoteclientDlg* thiz = (CremoteclientDlg*)th;
//	while (! CClientSocket::getObject())
//	{
//		Sleep(1);
//	}
//	CClientController* obj = CClientController::getObject();
//
//	int bufsz = 1024 * 1024 * 10;//10mb
//	char* buf = new char[bufsz];
//	memset(buf, 0, bufsz);
//	UINT index = 0;
//	int res = 0;
//	
//
//	while (true) {
//		if (thiz->isNullMonitor == false)
//		{
//			obj->SendCommandPacket(7);
//			res = obj->recvMes(buf, bufsz, &index, &bag);
//
//			if (res > 0 && bag.cmd == 7)
//			{
//
//				BYTE* data = (BYTE*)bag.m_data.c_str();
//
//				HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
//				if (hMen == NULL)
//				{
//					TRACE("内存不足");
//					Sleep(1);
//					continue;
//				}
//				IStream* iStr = NULL;
//				HRESULT hRet = CreateStreamOnHGlobal(hMen, TRUE, &iStr);
//				if (hRet == S_OK)
//				{
//					ULONG len = 0;
//					iStr->Write(data, bag.m_data.size(), &len);
//					LARGE_INTEGER bg{ 0 };
//					iStr->Seek(bg, STREAM_SEEK_SET, NULL);
//					thiz->imageMonitor.Load(iStr);
//					thiz->isNullMonitor = true;
//				}
//			}
//			Sleep(1);
//			memset(buf, 0, bag.DATA.size());
//			index = 0;
//
//		} Sleep(1);
//	}
//	delete[] buf;
//	_endthread();
//}

//链接按钮按下
void CremoteclientDlg::OnBnClickedButtonDemo()
{
	UpdateData();
	CClientController* obj = CClientController::getObject();
	int res = CClientSocket::getObject()->init();
	if (res == 0) {
		(CButton*)GetDlgItem(IDC_BUTTON_DEMO)->EnableWindow(false);
		//TODO 启动屏幕监控 处理
		_beginthread(CClientController::_threadMonitor, 0, this);
		//TODO 处理
		obj->m_screenMonitor.initMouse();
		obj->m_screenMonitor.DoModal();
	}
}

//目录控件初始化
void CremoteclientDlg::OnBnClickedButtonDemo2()
{
	CClientController* obj = CClientController::getObject();
	obj->SendCommandPacket(1);
	if (obj->RecvCommand() == 1) {
		m_tree.DeleteAllItems();
		for (int i = 0,j =  obj->getResult().size();i < j;i++)
		{
			if (obj->getResult()[i] != '.')
			{
				CString str;
				str = obj->getResult()[i];
				str += ":";
				HTREEITEM htr = m_tree.InsertItem(str.GetBuffer(),TVI_ROOT,TVI_LAST);
			}
		}
	}
}

//文件下载线程
//void WINAPIV CremoteclientDlg::_ThreadDoenLoadFunction(void* parametor) //TODO 此线程应该移到控制层
//{
//	CClientController* obj = CClientController::getObject();
//	ULONGLONG fileSize = *(ULONGLONG*)obj->getResult().c_str();//文件大小
//
//	CremoteclientDlg* pCD = (CremoteclientDlg*)parametor;
//
//	int getsz = 0;
//	std::string pathName(pCD->pathName);
//	obj->SendCommandPacket(4, pathName);
//	
//	//DataBag bag1(4, pathName);
//	//obj->SendMes(bag1);
//	char* buf = new char[100 * 1024];
//	memset(buf, 0, 100 * 1024);
//	UINT index = 0;
//
//	while (obj->recvMes(buf,100*1024,&index,&bag1) == 4)
//	{
//		if (bag1.m_data.size() == 0)
//		{
//			AfxMessageBox("文件打开失败");
//			break;
//		}
//		size_t len = fwrite(bag1.m_data.c_str(), 1, bag1.m_data.size(), pCD->pfile);
//		getsz += bag1.m_data.size();
//		if (getsz == fileSize) {
//			AfxMessageBox("文件下载完毕");
//			break;
//		}
//	}
//	CClientController::getObject()->m_dLoad.ShowWindow(SW_HIDE);
//	fclose(pCD->pfile);
//	//delete parametor;
//	delete buf;
//	_endthread();
//}
//CString CremoteclientDlg::GetRemotePathName(HTREEITEM hTree)
//{
//	CString strpath, str;
//	do 
//	{
//		str = m_tree.GetItemText(hTree);
//		strpath = str + "\\" + strpath;
//		hTree = m_tree.GetParentItem(hTree);
//	} while (hTree != NULL);
//	return strpath;
//}
//CString CremoteclientDlg::GetRemoteFilePathName()
//{
//	int fileSelectName = m_fileList.GetSelectionMark();
//	CString fileName = m_fileList.GetItemText(fileSelectName, 0);
//	CString pathName = GetRemotePathName(m_tree.GetSelectedItem());
//	pathName += fileName;
//	return pathName;
//}

//删除目录控件下整个子目录
void CremoteclientDlg::deleteSubAllDir(HTREEITEM hTree) {
	HTREEITEM sub = NULL;
	do 
	{
		sub = m_tree.GetChildItem(hTree);
		if(sub != NULL) m_tree.DeleteItem(sub);
	} while (sub != NULL);
}
//void CremoteclientDlg::OnNMDblclkTreeFile(NMHDR* pNMHDR, LRESULT* pResult)
//{
//	*pResult = 0;
//	CPoint point;
//	GetCursorPos(&point);
//	m_tree.ScreenToClient(&point);
//	HTREEITEM htr = m_tree.HitTest(point, 0);
//	if (htr == NULL)
//	{
//		return;
//	}
//	//if (m_tree.GetChildItem(htr) == NULL)//如果该项是文件（没有子目录）
//	//{
//	//	return;
//	//}
//	deleteSubAllDir(htr);
//	m_fileList.DeleteAllItems();
//	std::string strPath = GetRemotePathName(htr);
//	DataBag bag(2, strPath);
//	CClientSocket* obj = CClientSocket::getObject();
//	obj->SendMes(bag);
//	DFInfo info;
//	info.IsHasNext = true;
//	while (info.IsHasNext) {
//		if (obj->recvMes() == 2) {
//			info = *(PDFInfo)obj->databag.m_data.c_str();
//			if(!info.IsInvalid) continue;
//			if (CString(info.fileName) == "." || CString(info.fileName) == "..")
//				continue;
//			//HTREEITEM sub = m_tree.InsertItem(info.fileName, htr, TVI_LAST);
//			if (info.IsDirectory) {
//				//文件夹处理
//				m_tree.InsertItem(info.fileName, htr, TVI_LAST);
//				//m_tree.InsertItem("",sub, TVI_LAST);
//			}
//			else m_fileList.InsertItem(0, info.fileName);
//			TRACE("**************recv***%s****\n", info.fileName);
//		}
//		else break;
//	}
//	
//}

//左键单击目录控件
void CremoteclientDlg::OnNMClickTreeFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	CPoint point;
	GetCursorPos(&point);
	m_tree.ScreenToClient(&point);
	HTREEITEM htr = m_tree.HitTest(point, 0);
	if (htr == NULL)
	{
		return;
	}
	//清空
	deleteSubAllDir(htr);//删除目录
	m_fileList.DeleteAllItems();//删除所有图标

	CClientController* obj = CClientController::getObject();
	std::string strPath = obj->GetRemotePathName(htr);
	//DataBag bag(2, strPath);
	//CClientSocket* obj = CClientSocket::getObject();
	//obj->SendMes(bag);
	obj->SendCommandPacket(2, strPath);

	DFInfo info;
	info.IsHasNext = true;
	//m_fileList.InsertItem(0, "");//插入一个空（mfc控件检测不到第一个的右击）
	while (info.IsHasNext) {
		if (obj->RecvCommand() == 2) {
			info = *(PDFInfo)obj->getResult().c_str();
			if (!info.IsInvalid) continue;
			if (CString(info.fileName) == "." || CString(info.fileName) == "..")
				continue;
			//HTREEITEM sub = m_tree.InsertItem(info.fileName, htr, TVI_LAST);
			//判断是不是目录，分别放入目录控件和文件控件
			if (info.IsDirectory) {
				//文件夹处理
				m_tree.InsertItem(info.fileName, htr, TVI_LAST);
				//m_tree.InsertItem("",sub, TVI_LAST);
			}
			else m_fileList.InsertItem(0, info.fileName);
			TRACE("**************recv***%s****\n", info.fileName);
		}
		else break;
	}
	
}

//右键单击（右键选择菜单）
void CremoteclientDlg::OnNMRClickList2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CPoint point,subPoint;
	GetCursorPos(&point);
	subPoint = point;
	m_fileList.ScreenToClient(&subPoint);
	int htr = m_fileList.HitTest(subPoint, 0);
	if (htr < 0)//无选中
	{
		return;
	}
	CMenu FMenu;
	FMenu.LoadMenu(IDR_MENU1);
	CMenu* pMenu = FMenu.GetSubMenu(0);
	if (pMenu != NULL)
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
}
//文件执行
void CremoteclientDlg::OnExecFile()
{
	CClientController* obj = CClientController::getObject();
	std::string pathName = obj->GetRemoteFilePathName();
	obj->SendCommandPacket(3, pathName);
	if (obj->RecvCommand() == 3)
		AfxMessageBox(obj->getResult().c_str());
}

//文件删除
void CremoteclientDlg::OnDelFile()
{
	CClientController* obj = CClientController::getObject();
	std::string pathName = obj->GetRemoteFilePathName();
	obj->SendCommandPacket(10, pathName);
	if (obj->RecvCommand() == 10) {
		AfxMessageBox(obj->getResult().c_str());
		m_fileList.DeleteItem(m_fileList.GetSelectionMark());
	}
}

//***************************************************************************************************************重构
#include <Mmsystem.h>
#pragma comment( lib,"winmm.lib" )
void CremoteclientDlg::OnDownloadFile()
{
	CClientController* obj = CClientController::getObject();
	std::string pathName = obj->GetRemoteFilePathName();//获取完整文件名
	
	this->pathName = pathName.c_str();
	
	//获取文件保存路径
	CFileDialog dia(false, "*", m_fileList.GetItemText(m_fileList.GetSelectionMark(), 0).GetBuffer(), 
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, NULL, this);
	if (dia.DoModal() != IDOK)
	{
		return;
	}

	//创建并打开文件
	errno_t err	= fopen_s(&this->pfile,dia.GetPathName().GetBuffer(), "wb+");
	if (this->pfile == NULL)
	{
		return;
	}
	if (err != 0 || (this->pfile == NULL)) {
		AfxMessageBox("文件打开失败");
		TRACE("%s", "文件打开失败");
		return;
	}

	CClientController::getObject()->SendCommandPacket(5, pathName);
	if (obj->RecvCommand() == 5) { //先获取文件大小
		ULONGLONG fileSize = *(ULONGLONG*)obj->getResult().c_str();//文件大小
		char fs[20]{};
		_ultoa_s((ULONG)fileSize, fs, 20, 10);
		CString info = "文件正在下载中，总字节：";
		info += fs;
		CClientController* ctl = CClientController::getObject();

		ctl->m_dLoad.Create(IDD_DLG_DOWNLOAD, this);
		ctl->m_dLoad.m_ion_str.SetWindowTextA(info);
		ctl->m_dLoad.m_ion_str.UpdateData(FALSE);
		UpdateData(FALSE);
		CClientController::getObject()->m_dLoad.ShowWindow(SW_SHOW);
		//m_obj.ShowWindow(SW_SHOW);
		ctl->m_dLoad.SetActiveWindow();

		_beginthread(&CClientController::_ThreadDoenLoadFunction, 0, this);
	}
		//_beginthread(CremoteclientDlg::_ThreadDoenLoadFunction,0,(void*)mys);
		/*
		ULONGLONG fileSize = *(ULONGLONG*)obj->databag.m_data.c_str();//文件大小
		ULONGLONG getsz = 0;
		DataBag bag1(4, pathName);
		obj->SendMes(bag1);
		//DWORD t = timeGetTime();
		while (obj->recvMes() == 4)
		{
			if (obj->databag.m_data.size() == 0)
			{
				AfxMessageBox("文件打开失败");
				break;
			}
			fwrite(obj->databag.m_data.c_str(), 1, obj->databag.m_data.size(), pFile);
			getsz += obj->databag.m_data.size();
			//if(obj->databag.m_data.size() == 3072)
			//	fwrite(obj->databag.m_data.c_str(), 3072, 1, pFile);
			//else fwrite(obj->databag.m_data.c_str(), 1, obj->databag.m_data.size(), pFile);

			//getsz += obj->databag.m_data.size();
			if (getsz == fileSize) {
				//t = (timeGetTime() - t)/1000;
				//char ct[5]{};
				//_ultoa_s(t, ct, 5);
				//CString str = "文件下载完毕,下载时间：";
				//str += ct;
				AfxMessageBox("文件下载完毕");
				break;
			}
		}
		*/
	
	//fclose(pFile);
}

//获取文件大小
void CremoteclientDlg::Ongetfilesize()
{
	CClientController* obj = CClientController::getObject();
	std::string pathName = obj->GetRemoteFilePathName();
	obj->SendCommandPacket(5, pathName);
		if (obj->RecvCommand() == 5){
		ULONGLONG fileSz = *(ULONGLONG*)obj->getResult().c_str();
		char str[20]{};
		_ultoa_s((ULONG)fileSz, str, 20, 10);
		strcat_s(str, 20, "byte");
		AfxMessageBox(str);
	}		
}

//ip控件修改
void CremoteclientDlg::OnIpnFieldchangedIpaddressIp(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	*pResult = 0;
	UpdateData(FALSE);
	CClientSocket::getObject()->UpData(m_ipAddr, atoi(m_port.GetBuffer()));
}

//端口控件修改
void CremoteclientDlg::OnEnChangeEditPort()
{
	UpdateData(FALSE);
	CClientSocket::getObject()->UpData(m_ipAddr, atoi(m_port.GetBuffer()));
}
