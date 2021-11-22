
// remote_clientDlg.h: 头文件
//

#pragma once
#include "SreenMonitor.h"
#include "m_downLoad.h"

// CremoteclientDlg 对话框
class CremoteclientDlg : public CDialogEx
{
// 构造
public:
	CremoteclientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTE_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	//视频监控多线程函数
	//static void __cdecl _threadMonitor(void*);
	CImage imageMonitor;
	bool isNullMonitor = false;
	//CSreenMonitor cs;
	afx_msg void OnBnClickedButtonDemo();
	// ip控件绑定变量
	DWORD m_ipAddr;
	// 端口号
	CString m_port;
	// 目录控件
	CTreeCtrl m_tree;
	FILE* pfile;
	CString pathName;

//	CString GetRemotePathName(HTREEITEM hTree);
//	CString GetRemoteFilePathName();
	void deleteSubAllDir(HTREEITEM hTree);
	//m_downLoad m_obj;
	afx_msg void OnBnClickedButtonDemo2();
	//static void WINAPIV _ThreadDoenLoadFunction(void* parametor);
//	afx_msg void OnNMDblclkTreeFile(NMHDR* pNMHDR, LRESULT* pResult);
	// 文件列表
	CListCtrl m_fileList;
	afx_msg void OnNMClickTreeFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickList2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExecFile();
	afx_msg void OnDelFile();
	afx_msg void OnDownloadFile();
	afx_msg void Ongetfilesize();
	afx_msg void OnIpnFieldchangedIpaddressIp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
};
