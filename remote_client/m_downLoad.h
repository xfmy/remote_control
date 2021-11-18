#pragma once


// m_downLoad 对话框

class m_downLoad : public CDialog
{
	DECLARE_DYNAMIC(m_downLoad)

public:
	m_downLoad(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~m_downLoad();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_DOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_str;
	CStatic m_ion_str;
};
