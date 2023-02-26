#include"CClientSocket.h"
// MFCApplication3Dlg.h: 头文件
//

#pragma once


// CMFCApplication3Dlg 对话框
class CMFCApplication3Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication3Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION3_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CClientSocket* m_pClientSocket;
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedBtnBrowse();
	CString m_strFilePath;
	CProgressCtrl m_pro;
	CListCtrl m_chat;
	afx_msg void OnBnClickedButton1();
	CString m_message;
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	int type=0;
	afx_msg void OnBnClickedButton5();
};
