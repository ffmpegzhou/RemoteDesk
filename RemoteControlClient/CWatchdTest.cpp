// CWatchdTest.cpp: 实现文件
//

#include "pch.h"
#include "RemoteControlClient.h"
#include "afxdialogex.h"
#include "CWatchdTest.h"
#include "RemoteControlClientDlg.h"


// CWatchdTest 对话框
//初始化目标屏幕的宽高
IMPLEMENT_DYNAMIC(CWatchdTest, CDialogEx)

CWatchdTest::CWatchdTest(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	int m_nObjWidth = -1; 
	int m_nObjHeight = -1;
}


CWatchdTest::~CWatchdTest()
{
}

void CWatchdTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_pictest, m_pictest);
}


BEGIN_MESSAGE_MAP(CWatchdTest, CDialogEx)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDD_DIALOG_TESTPIC, &CWatchdTest::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTNLOCK, &CWatchdTest::OnBnClickedBtnlock)
	ON_BN_CLICKED(IDC_BTNUNLOC, &CWatchdTest::OnBnClickedBtnunloc)
END_MESSAGE_MAP()


// CWatchdTest 消息处理程序
	// TODO:  在此添加额外的初始化

/*鼠标坐标转换函数。
* 首先构造一个矩形类，(如果是全局坐标)就把它转换成客户区域本地坐标；
* 再用GetWindowRect把本地转为远程服务端的坐标
* 此处（x,y）与本地是等比例缩放关系
*/
CPoint CWatchdTest::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen)
{
	CRect clientRect;
	if (isScreen)ScreenToClient(&point);
	TRACE("x=%d y=%d \r\n", point.x, point.y);
	m_pictest.GetWindowRect(clientRect);
	TRACE("client width=%d client height=%d \r\n", clientRect.Width(), clientRect.Height());
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	//int width = 1024, height = 769;
	return CPoint(point.x * m_nObjWidth / width0, point.y * m_nObjHeight / height0);
}

BOOL CWatchdTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetTimer(0, 37, NULL);			 //加定时器
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



/*函数说明：MFC定时器，该函数定义在图传界面的CWatchDialg类里，
其作用用于定时从m_image中读取图片数据，让远程桌面成为动图。
此时，通过父窗口指针得到父窗口的成员m_image,也就是图传线程得到的数据。
设置到CStatic的对象m_picture中，让其显示。
(变量m_image在关闭后未及时释放,导致监视界面反复打开导致引发异常崩溃的bug)
解决方案是；
*/
void CWatchdTest::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		if (pParent->isFull())
		{
			CRect rect;
			m_pictest.GetWindowRect(rect);
			//pParent->getImage().BitBlt(m_picture.GetDC()->GetSafeHdc(),0,0,SRCCOPY);   //从左上角(0,0)以直接位复制的方式绘制
			if (m_nObjWidth == -1)
			{
				m_nObjWidth = pParent->getImage().GetWidth();
			}
			if (m_nObjHeight == -1)
			{
				m_nObjHeight = pParent->getImage().GetHeight();
			}
			pParent->getImage().StretchBlt(
				m_pictest.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_pictest.InvalidateRect(NULL);  //刷新
			pParent->getImage().Destroy();
			pParent->setImagestatus();
			TRACE("图像绘制完成\r\n");
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

/*以下鼠标事件先转换, 之后封装。
注意需要确保没有收到数据之前没有鼠标事件发送过去*/

/*鼠标左键双击*/
void CWatchdTest::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 0;
		Mouse_event.nAction = 2;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessageA(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

/*鼠标左键按下*/
void CWatchdTest::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))         
	{
		TRACE("x=%d y=%d \r\n", point.x, point.y);
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d y=%d \r\n", point.x, point.y);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 0;
		Mouse_event.nAction = 2;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
		
	}
CDialogEx::OnLButtonDown(nFlags, point);
}

/*鼠标左键弹起*/
void CWatchdTest::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 0;
		Mouse_event.nAction = 3;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

/*鼠标右键双击*/
void CWatchdTest::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 1;
		Mouse_event.nAction = 1;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
	CDialogEx::OnRButtonDblClk(nFlags, point);
}

/*鼠标右键按下*/
void CWatchdTest::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 1;
		Mouse_event.nAction = 2;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}

/*鼠标右键弹起*/
void CWatchdTest::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 1;
		Mouse_event.nAction = 3;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}

	CDialogEx::OnRButtonUp(nFlags, point);
}

/*鼠标移动*/
void CWatchdTest::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 8;
		Mouse_event.nAction = 0;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

/*自定义点击事件（单击）*/
void CWatchdTest::OnStnClickedWatch()
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint point;
		GetCursorPos(&point);
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		MOUSEEV Mouse_event;
		/* 封装*/
		Mouse_event.ptXY = remote;
		Mouse_event.nButton = 0;
		Mouse_event.nAction = 0;
		/* 发送*/
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&Mouse_event);
	}
}

void CWatchdTest::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


/*锁机按钮响应事件*/
void CWatchdTest::OnBnClickedBtnlock()
{
	CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
	pParent->SendMessageA(WM_SEND_PACKET, 7 << 1 | 1);
}

/*解锁按钮响应事件*/
void CWatchdTest::OnBnClickedBtnunloc()
{
	CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
	pParent->SendMessageA(WM_SEND_PACKET, 8 << 1 | 1);
}
