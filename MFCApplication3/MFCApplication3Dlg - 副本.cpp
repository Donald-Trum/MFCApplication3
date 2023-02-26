
// MFCApplication3Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication3.h"
#include "MFCApplication3Dlg.h"
#include "afxdialogex.h"
#include"CClientSocket.h"
#define PORT 4980
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCApplication3Dlg 对话框



CMFCApplication3Dlg::CMFCApplication3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION3_DIALOG, pParent)
	, m_strFilePath(_T(""))
	, m_message(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strFilePath);
	DDX_Control(pDX, IDC_PROGRESS1, m_pro);
	DDX_Control(pDX, IDC_LIST3, m_chat);
	DDX_Text(pDX, IDC_EDIT2, m_message);
}

BEGIN_MESSAGE_MAP(CMFCApplication3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT1, &CMFCApplication3Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BTN_SEND, &CMFCApplication3Dlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CMFCApplication3Dlg::OnBnClickedBtnBrowse)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, &CMFCApplication3Dlg::OnLvnItemchangedList3)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication3Dlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT2, &CMFCApplication3Dlg::OnEnChangeEdit2)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 消息处理程序

BOOL CMFCApplication3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 创建进度条
	m_pro.SetPos(0);
	//创建对象
	m_pClientSocket = new CClientSocket;
	CString strMsg;
	//创建套接字 0 1-65535，1024
	if (FALSE == m_pClientSocket->Create(SOCK_STREAM)) {
		strMsg.Format(_T("创建套接字失败,错误编号：%d"), GetLastError());
		MessageBox(strMsg, _T("温馨提示"), MB_OK);
		delete m_pClientSocket;
		m_pClientSocket = NULL;
		//关闭对话框
		EndDialog(IDOK);
		return TRUE;
	}
	//连接服务器

	if (FALSE == m_pClientSocket->Connect(_T("192.168.75.1"),PORT)) {
		strMsg.Format(_T("创建服器失败,错误编号：%d"), GetLastError());
		MessageBox(strMsg, _T("温馨提示"), MB_OK);
		delete m_pClientSocket;
		m_pClientSocket = NULL;
		//关闭对话框
		EndDialog(IDOK);
		return TRUE;
	}
	CString str[] = { TEXT("IP"),TEXT("说了") };

	for (int i = 0; i < 2; i++)
	{
		//设置表头  参数：1索引，2内容，3对齐方式，4宽度
		m_chat.InsertColumn(i, str[i], LVCFMT_LEFT, 300);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication3Dlg::OnPaint()
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
HCURSOR CMFCApplication3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication3Dlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CMFCApplication3Dlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMFCApplication3Dlg::OnBnClickedBtnSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_strFilePath.IsEmpty()) {
		MessageBox(_T("笨蛋，你还没选文件呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	sendd[num].type = 1;
	//获取文件属性
	WIN32_FIND_DATA wfd;
	HANDLE hFinder = FindFirstFile(m_strFilePath, &wfd);
	FindClose(hFinder);
	sendd[num].wfd = wfd;
	m_pro.SetPos(0);
	//先发文件属性
	m_pClientSocket->Send(&(sendd[num]), sizeof(sendd[0]));
	num += 1;
	//再发文件内容
	CFile file;
	if (FALSE == file.Open(m_strFilePath, CFile::modeRead | CFile::typeBinary)) {
		MessageBox(_T("笨蛋，文件打不开！"), _T("温馨提示"), MB_OK);
		return;

	}
	DWORD dwReadCount = 0;
	//循环发送
	while (dwReadCount < wfd.nFileSizeLow)
	{
		
		int nRead = file.Read(sendd[num].buffer, 1024);
		sendd[num].buffer[nRead] = '\0';
		sendd[num].type = 3;
		sendd[num].longth = nRead;
		//发送
		m_pClientSocket->Send(&(sendd[num]), sizeof(sendd[0]));
		dwReadCount += nRead;
		m_pro.SetPos((dwReadCount+0.0)/( wfd.nFileSizeLow+0.0)* 100);
		num += 1;
	}
	//关闭文件
	MessageBox(_T("🤫！搞定了，别声张"), _T("温馨提示"), MB_OK);
	file.Close();
	sendd[num].type = 4;
	m_pClientSocket->Send(&(sendd[num]), sizeof(sendd[0]));
	num += 1;
	m_pro.SetPos(0);
}


void CMFCApplication3Dlg::OnBnClickedBtnBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE);
	if (IDCANCEL == dlg.DoModal())//弹出
		return;
	m_strFilePath = dlg.GetPathName();
	UpdateData(FALSE);
}




void CMFCApplication3Dlg::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类
	sendd[num].type = 0;
	if (m_pClientSocket) {
		m_pClientSocket->Send(&(sendd[num]), sizeof(sendd[0]));
	}
	num += 1;
	CDialogEx::PostNcDestroy();
}


void CMFCApplication3Dlg::OnLvnItemchangedList3(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	*pResult = 0;
}


void CMFCApplication3Dlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_message.IsEmpty()) {
		MessageBox(_T("笨蛋，不说话别占资源！"), _T("温馨提示"), MB_OK);
	}
	else{
		sendd[num].type = 2;
		memcpy(sendd[num].buffer + 1, m_message.GetBuffer(0), m_message.GetLength()*2);
		//sendd[num].message = m_message;
		CString strMsg;
		//strMsg.Format(_T("%s%d"), sendd[num].message,sendd[num].type);
		//MessageBox(strMsg, _T("温馨提示"), MB_OK);
		m_pClientSocket->Send(&(sendd[num]), sizeof(sendd[0]));
		m_message = "";
		num += 1;
	}
	
}


void CMFCApplication3Dlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
