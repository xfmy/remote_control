#pragma once
#define WM_CHILDFRAMEDBCLK WM_USER+1

struct SPoint
{
	CRect client;
	CRect server;
};
typedef struct _MouseEvent
{
	int LR = -1;// 1左键 2右键
	int DUM = -1;//1单机 2双击 4移动
	POINT P{ 0,0 };//坐标
}MouseEvent, * PMouseEvent;


// CSreenMonitor 对话框

class CSreenMonitor : public CDialog
{
	DECLARE_DYNAMIC(CSreenMonitor)

public:
	CSreenMonitor(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSreenMonitor();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SCREEN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 图片控件
	CStatic m_IconAct;
	//将客户端坐标转换成服务端坐标
	//true=控件坐标
	//false屏幕坐标
	CPoint ScreenToRemoteClient(const CPoint& point, bool isbool);
	//坐标类
	SPoint m_Sp;
	//鼠标事件
	MouseEvent m_MouseEvent;
	//坐标初始化
	void initMouse();
	//窗口是否正在显示
	bool isShou = false;
	//int infoGroupSend(int LR,int DUM,)


	//afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//afx_msg void OnStnClickedStaticIcon();
	//afx_msg LRESULT OnChlidFrameDBClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
