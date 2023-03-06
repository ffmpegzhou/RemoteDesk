#pragma once
#include "afxdialogex.h"


// CWatchdTest 对话框

class CWatchdTest : public CDialogEx
{
	DECLARE_DYNAMIC(CWatchdTest)

public:
	CWatchdTest(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchdTest();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum {
		IDD = IDD_DIALOG_TESTPIC
};
#endif

public:
	int m_nObjWidth{-1}; //目标屏幕的宽
	int m_nObjHeight{-1};//目标屏幕的高
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CPoint UserPoint2RemoteScreenPoint( CPoint& point,bool isScreen=false);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_pictest;   //图形数据 测试
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	virtual void OnOK();
	void ClickLock();
	afx_msg void OnBnClickedBtnlock();
	afx_msg void OnBnClickedBtnunloc();
};
