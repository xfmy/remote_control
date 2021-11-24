// SreenMonitor.cpp: 实现文件
//

#include "pch.h"
#include "remote_client.h"
#include "SreenMonitor.h"
#include "afxdialogex.h"
#include "remote_clientDlg.h"
#include "ClientSocket.h"
#include "ClientController.h"
// CSreenMonitor 对话框

IMPLEMENT_DYNAMIC(CSreenMonitor, CDialog)

CSreenMonitor::CSreenMonitor(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_SCREEN, pParent)
{

}

CSreenMonitor::~CSreenMonitor()
{
}

void CSreenMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ICON, m_IconAct);
}


BEGIN_MESSAGE_MAP(CSreenMonitor, CDialog)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//ON_STN_CLICKED(IDC_STATIC_ICON, &CSreenMonitor::OnStnClickedStaticIcon)
	//ON_MESSAGE(WM_CHILDFRAMEDBCLK, OnChlidFrameDBClick)   // 映射添加到这里
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON1, &CSreenMonitor::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSreenMonitor::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSreenMonitor 消息处理程序


CPoint CSreenMonitor::ScreenToRemoteClient(const CPoint& point,bool isbool = true)
{
	CPoint obj = point;
	if(isbool)
		this->ClientToScreen(&obj);
	m_IconAct.ScreenToClient(&obj);
	RECT rect;
	m_IconAct.GetClientRect(&rect);//1260 638相对于00
	//m_IconAct.GetWindowRect(&rect);//相对于屏幕
	//rect.bottom//高度
	//rect.right//宽度
	return CPoint((int)((float)obj.x / ((float)rect.right / (float)m_Sp.server.Width())), (int)((float)obj.y / ((float)rect.bottom / (float)m_Sp.server.Height())));
}

void CSreenMonitor::initMouse()
{
	//获取CS屏幕分辨率
	CClientController* obj = CClientController::getObject();
	obj->SendCommandPacket(11);
	std::string res = obj->RecvCommand(11);
	int Sx = *(int*)res.c_str();
	int Sy = *(int*)(res.c_str() + 4);

	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	m_Sp.server.SetRect(0, 0, Sx, Sy);
	m_Sp.client.SetRect(0, 0, x, y);
}

void CSreenMonitor::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0) {
		//CremoteclientDlg* dig = (CremoteclientDlg*)GetParent();
		/*CClientController::getObject()->dlg*/
		CremoteclientDlg* dig = &CClientController::getObject()->dlg;
		if (dig->isNullMonitor)
		{
			CRect rect;
			m_IconAct.GetWindowRect(rect);
			//dig->imageMonitor.BitBlt(m_IconAct.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			dig->imageMonitor.StretchBlt(m_IconAct.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_IconAct.InvalidateRect(NULL);
			dig->imageMonitor.Destroy();
			dig->isNullMonitor = false;
		}
	}
	CDialog::OnTimer(nIDEvent);
}


BOOL CSreenMonitor::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(0, 35, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
/**
typedef struct _MouseEvent
{
	int LR = -1;// 1左键 2右键
	int DUM = -1;//1单机 2双击 4移动
	POINT P{ 0,0 };//坐标
}MouseEvent, * PMouseEvent;
*/

//光标移动
void CSreenMonitor::OnMouseMove(UINT nFlags, CPoint _point)
{
	m_MouseEvent.LR = 0;
	m_MouseEvent.DUM = 4;
	m_MouseEvent.P = ScreenToRemoteClient(_point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6, val);
	CDialog::OnMouseMove(nFlags, _point);
}

//左键双击
void CSreenMonitor::OnLButtonDblClk(UINT nFlags, CPoint _point)
{
	m_MouseEvent.LR = 1;
	m_MouseEvent.DUM = 0;
	m_MouseEvent.P = ScreenToRemoteClient(_point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6, val);
	CDialog::OnLButtonDblClk(nFlags, _point);
}

//左键按下
void CSreenMonitor::OnLButtonDown(UINT nFlags, CPoint _point)
{
	m_MouseEvent.LR = 1;
	m_MouseEvent.DUM = 1;
	m_MouseEvent.P = ScreenToRemoteClient(_point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6, val);

	CDialog::OnLButtonDown(nFlags, _point);
}

//左键弹起
void CSreenMonitor::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_MouseEvent.LR = 1;
	m_MouseEvent.DUM = 2;
	m_MouseEvent.P = ScreenToRemoteClient(point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6, val);

	CDialog::OnLButtonUp(nFlags, point);
}

//右键弹起
void CSreenMonitor::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_MouseEvent.LR = 2;
	m_MouseEvent.DUM = 2;
	m_MouseEvent.P = ScreenToRemoteClient(point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6,val);

	CDialog::OnRButtonUp(nFlags, point);
}

//左键单机
//void CSreenMonitor::OnStnClickedStaticIcon()
//{
//	CPoint point;
//	GetCursorPos(&point);
//	m_MouseEvent.P = ScreenToRemoteClient(point,false);
//	m_MouseEvent.LR = 1;
//	m_MouseEvent.DUM = 1;
//	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
//	DataBag bag(6, val);
//	CClientSocket::getObject()->SendMes(bag);
//}

//右键按下
void CSreenMonitor::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_MouseEvent.LR = 2;
	m_MouseEvent.DUM = 1;
	m_MouseEvent.P = ScreenToRemoteClient(point);
	std::string val((const char*)&m_MouseEvent, sizeof m_MouseEvent);
	CClientController::getObject()->SendCommandPacket(6,val);

	CDialog::OnRButtonDown(nFlags, point);
}


void CSreenMonitor::OnBnClickedButton1()
{
	CClientController::getObject()->SendCommandPacket(9);
}


void CSreenMonitor::OnBnClickedButton2()
{
	CClientController::getObject()->SendCommandPacket(9);
}
