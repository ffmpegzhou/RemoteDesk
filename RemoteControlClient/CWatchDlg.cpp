// CWatchDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteControlClient.h"
#include "afxdialogex.h"
#include "CWatchDlg.h"
#include "RemoteControlClientDlg.h"

// CWatchDlg 对话框

IMPLEMENT_DYNAMIC(CWatchDlg, CDialogEx)

CWatchDlg::CWatchDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_WATCH, pParent)
{

}

CWatchDlg::~CWatchDlg()
{
}

void CWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	//ON_BN_CLICKED(IDC_BUTTON2, &CWatchDlg::OnBnClickedButton2)
	//ON_BN_CLICKED(IDC_BUTTON3, &CWatchDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CWatchDlg 消息处理程序

//参数的初始化
BOOL CWatchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 50,NULL);    //加一个50ms定时器
	return TRUE;  // return TRUE unless you set the focus to a control
				// 异常: OCX 属性页应返回 FALSE
}

/*函数说明：MFC定时器，该函数定义在图传界面的CWatchDialg类里，
其作用用于定时从m_image中读取图片数据，让远程桌面成为动图。
此时，通过父窗口指针得到父窗口的成员m_image,也就是图传线程得到的数据。
设置到CStatic的对象m_picture中，让其显示。需要InvalidateRect通知进行重绘,否则无法显示下一帧
*/
void CWatchDlg::OnTimer(UINT_PTR nIDEvent)
{
	 
	if (nIDEvent==0)
	{
		CRemoteControlClientDlg* pParent = (CRemoteControlClientDlg*)GetParent();
		if (pParent->isFull())
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParent->getImage().BitBlt(m_picture.GetDC()->GetSafeHdc(),0,0,SRCCOPY);   //从左上角(0,0)以直接位复制的方式绘制
			pParent->getImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(),0,0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);  //
			pParent->getImage().Destroy();
			pParent->setImagestatus();
			
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CWatchDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CWatchDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CWatchDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
}
