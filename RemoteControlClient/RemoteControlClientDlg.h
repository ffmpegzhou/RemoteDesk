
// RemoteControlClientDlg.h: 头文件
//
//#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "Clientsocket.h"
#include "StatusDlg.h"

#define WM_SEND_PACKET (WM_USER + 1)   //自定义消息，在“下载文件处”发送数据包

// CRemoteControlClientDlg 对话框
//^b*[^:b#/]+.*$ ，正则表达式统计行数
class CRemoteControlClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteControlClientDlg(CWnd* pParent = nullptr);	// 标准构造函数
	
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECONTROLCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	/*图传的静态线程函数主要做框架，而threadWatchData()专注于实现功能、逻辑
	缓存可作为他的成员变量，十分方便*/
	static void threadEntryForWatchData(void* arg);
	void threadWatchData();
	static void threadEntryForDownFile(void* arg);
	void threadDownoadFile();
	void DeleteTreeChildrenItem(HTREEITEM htree);
	void LoadFileInfo();
	bool m_isClosed;  //监视窗口是否关闭

//1:查看分区；
// 2：查看指定目录文件
	//3：打开文件
	//4:下载文件
	//5：鼠标操作
	//6:发送屏幕截图
	//7:锁机
	// 8：解锁
	// 9：删除文件
	// 1981：连接测试
	//返回值是命令号。如小于0则为错误。
	int SendCommandPacket(int nCmd, bool bAutoClose=true, BYTE* pData = NULL, size_t nLength = 0);
	void loadfileCurrent(); //删除后立即刷新加载当前文件目录



// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CString GetPath(HTREEITEM hTree);
	CImage m_image;   //图像缓存数据
	bool m_imageIsFull;   //标志位，代表是否有图片缓存，TRUE代表有
public:
	bool isFull() const    
	{
		return m_imageIsFull;
	}
	CImage& getImage()
	{
		return m_image;
	}
	void setImagestatus(bool isFull = false)  //默认初始图像为空
	{
		m_imageIsFull = isFull;
	}
//	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnIpnFieldchangedIpaddress1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBtnFileinfo();        //显示文件目录的按钮
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult); //双击展开树
	//afx_msg void OnTvnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOnceClickTree(NMHDR* pNMHDR, LRESULT* pResult);//单击展开树
	// 显示文件
	CListCtrl m_list;
	afx_msg void OnLvnItemchangedList2(NMHDR* pNMHDR, LRESULT* pResult);//在右侧框显示文件
	afx_msg void OnTvnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void RClickList_inFileBox(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnIdrMenuRunRclc();
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnOpenFile();
	//afx_msg void OntestfileDown();
	afx_msg void OnBnClickedBtnTest();
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);   //自定义消息响应函数
	afx_msg void OnBnClickedBtnStartwatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
