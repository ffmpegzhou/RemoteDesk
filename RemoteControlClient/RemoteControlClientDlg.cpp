
// RemoteControlClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteControlClient.h"
#include "RemoteControlClientDlg.h"
#include "afxdialogex.h"
#include "Clientsocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CWatchDlg.h"


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


// CRemoteControlClientDlg 对话框



CRemoteControlClientDlg::CRemoteControlClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECONTROLCLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteControlClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, m_server_address);
	DDX_Text(pDX, IDC_EDIT1, m_nPort);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	DDX_Control(pDX, IDC_LIST2, m_list);
}

/*发送命令包
四个参数分别为：
1、命令号；
2、是否自动关闭
3、数据
4、数据长度
*/
int CRemoteControlClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();    //把数据从界面更新到成员变量
	CClientsocket* pClient = CClientsocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort)); //初始化，TODO 返回值处理
	if (!ret)     // if (!a) 等价于 if (a == false)
	{
		AfxMessageBox("网络初始化失败！");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->Send(pack);
	TRACE("Send ret=%d\r\n ", ret);
	int cmd = pClient->DealCommand();
	TRACE("Ack:%d\r\n", cmd);
	if (bAutoClose) pClient->CloseSocket();     //如果AutoClose=1，则自动关闭
	return cmd;
}

//立刻刷新加载当前文件目录(适配删除操作)
/*刷新操作，先删除当前列表内容，发送一次文件, 读取命令2以及当前文件路径到服务器。
得到服务器返回的内容进行更新列表。 */
void CRemoteControlClientDlg::loadfileCurrent()
{
	HTREEITEM htree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(htree);
	m_list.DeleteAllItems();

	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());  //发case2
	PFILEINFO pInfo = (PFILEINFO)CClientsocket::getInstance()->GetPacket().strData.c_str();  /*收*/

	CClientsocket* pClient = CClientsocket::getInstance();
	while (pInfo->HasNext)
	{
		TRACE("[%s] is dir  %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		/*如果不是目录，继续处理*/
		if (!pInfo->IsDirectory)
		{
			m_list.InsertItem(0, pInfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientsocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();
	TRACE("删除并刷新成功了");
}
//消息映射表
BEGIN_MESSAGE_MAP(CRemoteControlClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteControlClientDlg::OnBnClickedBtnTest)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS1, &CRemoteControlClientDlg::OnIpnFieldchangedIpaddress1)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteControlClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CRemoteControlClientDlg::OnNMDblclk)
	//ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CRemoteControlClientDlg::OnTvnSelchangedTree1)
	//ON_NOTIFY(NM_CLICK, IDC_TREE1, &CRemoteControlClientDlg::OnNMClickTree)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CRemoteControlClientDlg::OnOnceClickTree)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CRemoteControlClientDlg::OnLvnItemchangedList2)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CRemoteControlClientDlg::OnTvnSelchangedTree1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CRemoteControlClientDlg::RClickList_inFileBox)
	//ON_COMMAND(IDR_MENU_RUN_RCLC, &CRemoteControlClientDlg::OnIdrMenuRunRclc)
	ON_COMMAND(ID_DOWNLOADFILE, &CRemoteControlClientDlg::OnDownloadFile)//右键菜单=下载文件
	ON_COMMAND(ID_OPENFILE, &CRemoteControlClientDlg::OnOpenFile)//右键菜单=打开文件
	ON_COMMAND(ID_DELETEFILE, &CRemoteControlClientDlg::OnDeleteFile)//右键菜单=删除文件
	//ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteControlClientDlg::OnBnClickedBtnTest) //重复
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteControlClientDlg::OnSendPacket)   //绑定自定义消息到消息映射表
	ON_BN_CLICKED(IDC_BTN_STARTWATCH, &CRemoteControlClientDlg::OnBnClickedBtnStartwatch)

	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteControlClientDlg 消息处理程序
/*类CDialog中的初始化成员函数
（虚函数）。相当于对对话框进行初始化处理*/
BOOL CRemoteControlClientDlg::OnInitDialog()
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

	// 设置此对话框的图标。 当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	/* 此处注意ip地址的转换，比如 192.168.101.17是0XC0A86511
								192.168.101.18是0xC0A86512
								127.0.0.1     是0x7F000001*/
	UpdateData();
	m_nPort = _T("9527");
	m_server_address =0x7F000001 ;  //16进制表示的被控端虚拟机ip地址。
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);  //隐藏窗口并将活动状态传递给其它窗口
	m_imageIsFull = false;   //图像缓存，初始化为0
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteControlClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  这将由框架自动完成。初始化失败

void CRemoteControlClientDlg::OnPaint()
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
HCURSOR CRemoteControlClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//发包测试网络
void CRemoteControlClientDlg::OnBnClickedBtnTest()
{
	SendCommandPacket(1981);
}



void CRemoteControlClientDlg::OnIpnFieldchangedIpaddress1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

/* “查看文件信息”按钮的点击事件，显示根目录
先拿到磁盘分区（case1），如果识别到分隔符","，就把此磁盘信息加入列表中
以追加的方式，把磁盘信息加入根目录*/
void CRemoteControlClientDlg::OnBnClickedBtnFileinfo()
{

	int ret = SendCommandPacket(1);
	if (ret == -1)
	{
		AfxMessageBox(_T("命令处理失败！"));
		return;
	}
	CClientsocket* pClient = CClientsocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();  //清空文件树
	for (size_t i = 0; i < drivers.size(); i++)
	{/**/
		if (drivers[i] == ',')
			/*InsertItem是用于MFC中CListCtrl控件加入列表项的函数*/
		{
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);   //
			m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	if (dr.size()>0)
	{
		dr += ":";
		HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);   //
		m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
	}
}


/*展开指定目录节点*/
CString CRemoteControlClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTemp;
	do
	{/*判断是否是根目录,如果不是，则一直加\\，遍历*/
		strTemp = m_Tree.GetItemText(hTree);
		strRet = strTemp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

/*开启图传的线程函数*/
void CRemoteControlClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteControlClientDlg* thiz = (CRemoteControlClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

/*图片传输线程的主要处理部分。获取监控画面的步骤和SendScreen()逻辑相反，是把文件流写入到全局堆区。
图传线程的主要步骤：
1、用sendMessage发送命令包6
2、得到图传数据包，解析得到数据：pClient->GetPacket();
3、申请全局堆区GlobalAlloc();
4、建立内存流IStream* pStream;
5、创建内存流到全局内存堆区的通道CreateStreamOnHGlobal()并将图片数据包通过通道写入到全局堆区pStream->Write();
6、通过读取通道得到图片数据Load(pStream)并写入CImage对象m_image.
7、最后SendMessage到CwatchDialg中的回调函数中

需要在加载之前释放之前的句柄，否则可能存在异步问题引发异常：点击 “开始监控”按钮就开了一个线程，
关闭后再次重复点击但线程句柄又未及时释放，两个监控线程同时写入Cimage冲突，导致程序崩溃.。
改进：
解决方法是加一个标志bool m_isClosed监控画面是否关闭，如果没关闭才继续更新数据到缓存。
在监视按钮响应函数OnBnClickedBtnStartwatch()开始前后设置此标志，
最后最好设置WaitForSingleObject()函数来等待50ms直到线程彻底结束。

*/
void CRemoteControlClientDlg::threadWatchData()
{
	Sleep(1);
	CClientsocket* pClient = NULL;
	do
	{
		pClient = CClientsocket::getInstance();
	} while (pClient == NULL);
	while(!m_isClosed)
	{
		if (m_imageIsFull == false) //更新数据（图像）到缓存
		{
			int ret = SendMessageA(WM_SEND_PACKET, 6 << 1 | 1);
			if (ret == 6)
			{
				//int cmd = pClient->DealCommand();//此处取数据重复
				//if (cmd==6)
				{
					BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
					HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
					if (hMem == NULL)
					{
						TRACE("内存不足！");
						Sleep(1);
						continue;
					}
					IStream* pStream = NULL;
					HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
					if (hRet == S_OK)
					{
						ULONG length = 0;
						pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
						LARGE_INTEGER begin{ 0 };
						pStream->Seek(begin, STREAM_SEEK_SET, NULL); //跳转到缓存流的开头
						if ((HBITMAP)m_image!=nullptr)m_image.Destroy();
						m_image.Load(pStream);
						m_imageIsFull = true;
						TRACE("图片从缓存写入成功\r\n");
					}
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}
}

/*开始下载的线程函数*/
void CRemoteControlClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteControlClientDlg* thiz = (CRemoteControlClientDlg*)arg;
	thiz->threadDownoadFile();
	_endthread();
}

/*下载文件
SendMessageA将指定的消息发送到一个或多个窗口的消息队列。此函数为指定的窗口调用窗口程序，
直到窗口程序处理完消息再返回。
这里的位运算逻辑是将命令号向左移1位，然后和0做或运算用于取整，实现了参数的拆分。*/
void CRemoteControlClientDlg::threadDownoadFile()
{
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	CFileDialog dlg(FALSE, NULL,		//显示 Windows文件对话框,并允许用户浏览文件和目录
		strFile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() == IDOK) //模态对话框)
	{
		FILE* pFile = fopen(dlg.GetPathName(), "wb +");  //读写方式打开或新建一个二进制文件, 允许读和写
		if (pFile == nullptr)
		{
			AfxMessageBox("无权限保存该文件,或文件无法创建！");
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		HTREEITEM hSelected = m_Tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		TRACE("%s\r\n", LPCTSTR(strFile));
		CClientsocket* pClient = CClientsocket::getInstance();
		do
		{
			//int ret = SendCommandPacket(4, false, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
			int ret = SendMessageA(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);
			if (ret < 0)
			{
				AfxMessageBox("执行下载失败！");

				TRACE("执行下载失败：ret= %d \r\n", ret);
				EndWaitCursor();
				return;
			}

			long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
			if (nLength == 0)
			{
				AfxMessageBox("文件长度为0或无法读取！");
				return;
			}
			long long  nCount = 0;
			while (nCount < nLength)
			{
				ret = pClient->DealCommand();
				if (ret < 0)
				{
					AfxMessageBox("传输失败！");
					TRACE("传输失败：ret= %d \r\n", ret);
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
		} while (false);
		fclose(pFile);
		pClient->CloseSocket();
	}
	m_dlgStatus.ShowWindow(SW_HIDE);
	EndWaitCursor();
	MessageBox(_T("下载完成"), _T("完成"));
}


/*删除目录树中重复的子节点.
如果发现子节点不为空就删掉，避免了双击后文件树不停重复增长。*/
void CRemoteControlClientDlg::DeleteTreeChildrenItem(HTREEITEM htree)
{
	HTREEITEM hSub = NULL;
	do
	{
		hSub = m_Tree.GetChildItem(htree);
		if (hSub != NULL) m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}


/*加载显示文件目录信息*/
void CRemoteControlClientDlg::LoadFileInfo()
{
	/*拿到当前全局鼠标指针位置通过遍历树获取当前路径*/
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);

	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)
		return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)//如果是文件,就直接返回
		return;
	DeleteTreeChildrenItem(hTreeSelected);
	m_list.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientsocket::getInstance()->GetPacket().strData.c_str();
	/*插入目录信息,需考虑一开始就是空文件夹的情况（fdata不存在）*/
	CClientsocket* pClient = CClientsocket::getInstance();
	int  count = 0;
	while (pInfo->HasNext)
	{
		TRACE("[%s] is dir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory)
		{/*过滤掉根目录和上级目录，防止反复递归*/
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..")
			{
				int cmd = pClient->DealCommand();
				TRACE("Ack:%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientsocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else/*如果是文件*/
		{
			m_list.InsertItem(0, pInfo->szFileName);
		}
		count++;
		int cmd = pClient->DealCommand();
		TRACE("Ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientsocket::getInstance()->GetPacket().strData.c_str();
	}
	TRACE("LoadFileInfo count:%d\r\n", count);
	pClient->CloseSocket();
}

void CRemoteControlClientDlg::OnTvnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult)  //?功能
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}



/*在文件树双击鼠标的处理*/
void CRemoteControlClientDlg::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}

//void CRemoteControlClientDlg::OnNMClickTree(NMHDR* pNMHDR, LRESULT* pResult)
//{
//	*pResult = 0;
//	LoadFileInfo();
//}

/*在文件树单击鼠标的处理*/
void CRemoteControlClientDlg::OnOnceClickTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}

/* 在右侧文件框显示文件*/
void CRemoteControlClientDlg::OnLvnItemchangedList2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


/* 右键在文件列表框单击,弹出文件操作菜单*/
/*跟踪鼠标位置，通过TrackPopupMenu函数弹出菜单*/
void CRemoteControlClientDlg::RClickList_inFileBox(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_list.ScreenToClient(&ptList);
	int ListSelected = m_list.HitTest(ptList);
	if (ListSelected < 0)return;    //没点中则返回
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RUN_RCLC);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup != NULL)
	{
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
}



/*右键菜单：下载文件
函数GetSelectionMark()返回鼠标焦点所在行数，取得文件名；
获取该文件路径，发命令包，以二进制形式打开，取长度：
count<nlength的前提下持续写入；*/
void CRemoteControlClientDlg::OnDownloadFile()
{
	_beginthread(CRemoteControlClientDlg::threadEntryForDownFile, 0, this);
	BeginWaitCursor();   //此时设置光标形状是等待状态
	m_dlgStatus.m_info.SetWindowText(_T("命令执行中"));
	m_dlgStatus.ShowWindow(SW_SHOW);    //显示窗口
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();     //激活指定的窗口

}

/*右键菜单：删除文件*/
void CRemoteControlClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	//int ret = SendCommandPacket(9, true, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	int ret = SendMessageA(WM_SEND_PACKET, 9 << 1 | 1, (LPARAM)(LPCSTR)strFile);
	if (ret < 0)
	{
		AfxMessageBox("删除文件命令执行失败!");
		return;
	}
	loadfileCurrent();
	TRACE("删除成功");
}

/*右键菜单：打开文件
取到指定节点的路径,取得列表中选中的项和文件名
然后发包*/
void CRemoteControlClientDlg::OnOpenFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	int ret = SendCommandPacket(3, TRUE, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	//int ret = SendMessageA(WM_SEND_PACKET, 3 << 1 | 1, (LPARAM)(LPCSTR)strFile);
	if (ret < 0)
	{
		AfxMessageBox("打开文件命令执行失败!");
		//return;
	}
}


//自定义消息响应函数（重点）
/*需要用位运算合并参数
原型：int ret = SendCommandPacket(4, false,
	(BYTE*)(LPCTSTR)strFile, strFile.GetLength());

（一般情况使用LPARAM传递地址，而WPARAM传递其他参数，如常量、窗口或控件的句柄）
位运算可以实现这两个变量的合并和拆分
*/
LRESULT CRemoteControlClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	int cmd = wParam >> 1;
	switch (cmd) 
	{
		/*文件下载*/
	case 4:
	{
		CString strFile = (LPCTSTR)lParam;
		ret = SendCommandPacket(cmd, wParam & 1,
			(BYTE*)(LPCTSTR)strFile, strFile.GetLength());
	}
			break;
	/*鼠标操作*/
	case 5:
	{
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MouseEvent));
	}
			break;
	/*截屏*/
	case 6:
	case 7:
	case 8:
	{
		ret = SendCommandPacket(cmd, wParam & 1);
	}
			break;
	default:
		ret = -1;
	}
	return ret;
}

#include "CWatchdTest.h"

/*监控按钮的点击响应函数
加一个标志位bool m_isClosed监控画面是否关闭，如果没关闭才继续更新数据到缓存。
在开始前后设置此标志，最后设置WaitForSingleObject函数来等待*/
void CRemoteControlClientDlg::OnBnClickedBtnStartwatch()
{
	m_isClosed = false;
	CWatchdTest dlg(this);
	HANDLE hThread=(HANDLE)_beginthread(CRemoteControlClientDlg::threadEntryForWatchData, 0, this);
	//GetDlgItem(IDC_BTN_STARTWATCH)->EnableWindow(FALSE);  //点击后禁用窗口，防止新对话框打开后点击事件多次生效
	dlg.DoModal();  //模态对话框
	m_isClosed = true;
	WaitForSingleObject(hThread, 500);
}


/*定时器,需配合初始化函数OnInitDialog()中的settimer*/
void CRemoteControlClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
