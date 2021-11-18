// m_downLoad.cpp: 实现文件
//

#include "pch.h"
#include "remote_client.h"
#include "m_downLoad.h"
#include "afxdialogex.h"


// m_downLoad 对话框

IMPLEMENT_DYNAMIC(m_downLoad, CDialogEx)

m_downLoad::m_downLoad(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_DOWNLOAD, pParent)
	, m_str(_T(""))
{

}

m_downLoad::~m_downLoad()
{
}

void m_downLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_TEXT, m_str);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_ion_str);
}


BEGIN_MESSAGE_MAP(m_downLoad, CDialog)
END_MESSAGE_MAP()


// m_downLoad 消息处理程序
