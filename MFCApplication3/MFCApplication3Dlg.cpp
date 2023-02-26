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
bool HasSubFolders(const CString& folderPath)
//输入：文件夹路径
//输出：路径下是否还含有子文件夹
//功能：递归查询当前路径下是否有子文件夹
{
	CFileFind finder;
	CString strFilePath = folderPath + _T("\\*.*");

	// 开始查找文件和文件夹
	BOOL bWorking = finder.FindFile(strFilePath);
	bool bHasSubFolders = false;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// 如果是文件夹且不是点或点点，递归查找子文件夹
		if (finder.IsDirectory() && !finder.IsDots())
		{
			bHasSubFolders = true;
			CString strSubFolderPath = folderPath + _T("\\") + finder.GetFileName();
			if (HasSubFolders(strSubFolderPath))
			{
				return true;
			}
		}
	}

	return bHasSubFolders;
}


void TraverseFolder1(const CString& folderPath, CClientSocket* m_pClientSocket, CString Orgpa,int *type)
{
//输入：文件夹路径，ClientSocket指针，原始父文件夹路径，待发送sendd的序号（已弃用）
//功能：查询待发送文件夹里的所有文件夹，如果该文件夹下无子文件夹，则发送该文件夹相对于原父文件夹的相对路径
	CFileFind finder;
	CString lastFolderName;
	CString strFilePath = folderPath + _T("\\*.*");
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	// 开始查找文件和文件夹
	BOOL bWorking = finder.FindFile(strFilePath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// 如果是文件夹，继续递归查找子文件夹
		if (finder.IsDirectory() && !finder.IsDots())
		{
			CString strFolderName = finder.GetFileName();
			// 递归查找子文件夹
			CString strSubFolderPath = folderPath + _T("\\") + strFolderName;
			TraverseFolder1(strSubFolderPath,m_pClientSocket,Orgpa,type);
		}
	}
	// 如果没有子文件夹了，说明当前文件夹是最底层的文件夹
	if (!HasSubFolders(folderPath))
	{
		lastFolderName = folderPath;
		CString temp = Orgpa.Right(Orgpa.GetLength() - Orgpa.ReverseFind('\\')-1);
		CString strRelativepath = _T("\\")+temp+folderPath.Right(folderPath.GetLength() - Orgpa.GetLength());
        struct sends* sendd = new struct sends;
		sendd->type = *type+3;
		*type += 10;
		if (*type > 20480)
		{
			*type = 0;
		}
		int length = WideCharToMultiByte(CP_ACP, 0, strRelativepath, -1, NULL, 0, NULL, NULL);
		if (length <= 1025) {
			WideCharToMultiByte(CP_ACP, 0, strRelativepath, -1,sendd->buffer, length, NULL, NULL);
		}
		else {
			MessageBox(NULL,_T("这文件名也太长了，你有病吧"), _T("温馨提示"), MB_OK);
			return;
		}
		int nsend=m_pClientSocket->Send(sendd, sizeof(struct sends));
		delete sendd;
		
	}

}
void TraverseFolder2(CString path,CClientSocket* m_pClientSocket, CString Orgpath,int*type)
{
//输入：文件夹路径，ClientSocket指针，原始父文件夹路径，待发送sendd的序号（已弃用）
//功能：递归查找路径下的文件并发送
	CFileFind finder;
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	BOOL bWorking = finder.FindFile(path + _T("\\*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		
		if (finder.IsDots())
			continue;
		if (finder.IsDirectory())
		{
			CString folderName = finder.GetFileName();
			CString folderPath = path + _T("\\")+ folderName;
			TraverseFolder2(folderPath, m_pClientSocket, Orgpath,type);
		}
		else
		{
			CString filePath = finder.GetFilePath();
			CFile file;

			if (FALSE == file.Open(filePath, CFile::modeRead | CFile::typeBinary)) {
				MessageBox(NULL,_T("笨蛋，文件打不开！"), _T("温馨提示"), MB_OK);
				return;

			}
			struct sends* sendd = new struct sends;
			sendd->type = *type+1;
			*type += 10;
			if (*type > 20480)
			{
				*type = 0;
			}
			//获取文件属性
			WIN32_FIND_DATA wfd;
			HANDLE hFinder = FindFirstFile(filePath, &wfd);
			FindClose(hFinder);
			sendd->wfd = wfd;
			CString te = Orgpath.Right(Orgpath.GetLength() - Orgpath.ReverseFind('\\') - 1);
			CString temp = _T("\\") + te+filePath.Right(filePath.GetLength() - Orgpath.GetLength());
			int length = WideCharToMultiByte(CP_ACP, 0, temp, -1, NULL, 0, NULL, NULL);//中文路径乱码处理
			if (length <= 256) {
				WideCharToMultiByte(CP_ACP, 0, temp, -1,sendd->strRelativePath, length, NULL, NULL);
			}
			else {
				MessageBox(NULL,_T("这文件名也太长了，你有病吧"), _T("温馨提示"), MB_OK);
				return;
			}
			//先发文件属性;
			int nsend=m_pClientSocket->Send((sendd), sizeof(struct sends));
			//再发文件内容
			DWORD dwReadCount = 0;
			//循环发送
			while (dwReadCount < wfd.nFileSizeLow)
			{
				int nRead = file.Read(sendd->buffer, 1024);
				sendd->buffer[nRead] = '\0';
				sendd->longth = nRead;
				sendd->type = *type + 1;
				*type += 10;
				if (*type > 20480)
				{
					*type = 0;
				}
				//发送
				int nsend=m_pClientSocket->Send((sendd), sizeof(struct sends));
				dwReadCount += nRead;
			}
			//关闭文件
			file.Close();
			delete sendd;
		}
	}
}


CMFCApplication3Dlg::CMFCApplication3Dlg(CWnd* pParent /*=nullptr*/)//控件绑定变量初始化
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

BEGIN_MESSAGE_MAP(CMFCApplication3Dlg, CDialogEx)//map响应函数和用户行为
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND, &CMFCApplication3Dlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CMFCApplication3Dlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication3Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCApplication3Dlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 消息处理程序

BOOL CMFCApplication3Dlg::OnInitDialog()//对话框初始化
{
	CDialogEx::OnInitDialog();
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

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

	if (FALSE == m_pClientSocket->Connect(_T("10.10.227.164"),PORT)) {
	//sif (FALSE == m_pClientSocket->Connect(_T("192.168.75.1"), PORT)) {
		strMsg.Format(_T("创建服器失败,错误编号：%d"), GetLastError());
		MessageBox(strMsg, _T("温馨提示"), MB_OK);
		delete m_pClientSocket;
		m_pClientSocket = NULL;
		//关闭对话框
		EndDialog(IDOK);
		return TRUE;
	}
	//初始化聊天列表
	CString str[] = { TEXT("who"),TEXT("说了") };

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

void CMFCApplication3Dlg::OnPaint()//绘制对话框
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
void CMFCApplication3Dlg::OnBnClickedBtnSend()//按下send按钮后发送文件
{
	UpdateData(TRUE);//更新文本编辑框绑定的m_strFilePath的值
	if (m_strFilePath.IsEmpty()) {
		//如果文件路径空的则不发并提示
		MessageBox(_T("笨蛋，你还没选文件呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	CFile file;
	if (FALSE == file.Open(m_strFilePath, CFile::modeRead | CFile::typeBinary)) {
		//如果文件打不开则不发并提示
		MessageBox(_T("笨蛋，文件打不开！"), _T("温馨提示"), MB_OK);
		return;

	}
	struct sends* sendd = new struct sends;
	//获取文件属性
	WIN32_FIND_DATA wfd;
	HANDLE hFinder = FindFirstFile(m_strFilePath, &wfd);
	FindClose(hFinder);
	sendd->wfd = wfd;
	CString Name = wfd.cFileName;
	int length = WideCharToMultiByte(CP_ACP, 0, _T("\\")+Name, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, _T("\\") + Name, -1, sendd->strRelativePath, length, NULL, NULL);
	m_pro.SetPos(0);
	//先发文件属性
	sendd->type = type + 1;
	int nsend=m_pClientSocket->Send((sendd), sizeof(struct sends));
	type += 10;
	if (type > 20480)
		type = 0;
	//再发文件内容
	DWORD dwReadCount = 0;
	//循环发送
	while (dwReadCount < wfd.nFileSizeLow)
	{
		int nRead = file.Read(sendd->buffer, 1024);
		sendd->type = type + 1;
		sendd->buffer[nRead] = '\0';
		sendd->longth = nRead;
		//发送
		int nsend=m_pClientSocket->Send((sendd), sizeof(struct sends));
		dwReadCount += nRead;
		m_pro.SetPos((dwReadCount+0.0)/( wfd.nFileSizeLow+0.0)* 100);
		type += 10; 
		if (type > 20480)
			type = 0;
	}
	//关闭文件
	MessageBox(_T("🤫！搞定了，别声张"), _T("温馨提示"), MB_OK);
	file.Close();
	delete sendd;
	m_pro.SetPos(0);
}


void CMFCApplication3Dlg::OnBnClickedBtnBrowse()
{
	// 按下浏览键，弹出文件浏览框，false为另存为框
	CFileDialog dlg(TRUE);
	if (IDCANCEL == dlg.DoModal())//弹出
		return;
	m_strFilePath = dlg.GetPathName();
	UpdateData(FALSE);//变量给文本编辑框赋值
}


void CMFCApplication3Dlg::OnBnClickedButton1()
{
	// 按下发消息键，先判别是否是规定消息，如果是，则按要求发送给服务器固定消息并回显，如不是，则正常聊天发送消息并将自己发的消息写在聊天列表里
	UpdateData(TRUE);//更新文本编辑框绑定的变量
	if (m_message.IsEmpty()) {
		MessageBox(_T("笨蛋，不说话别占资源！"), _T("温馨提示"), MB_OK);
	}
	else if (m_message == _T("我要下文件")) {//走流程
		struct sends* sendd = new struct sends;
		sendd->type = 4;
		m_pClientSocket->Send(sendd, sizeof(struct sends));
		m_chat.InsertItem(0, _T("帅比本人"), 0);
		m_chat.SetItemText(0, 1, m_message+_T("（触发神秘功能）"));
		m_message = "";
		delete sendd;
		UpdateData(FALSE);
	}
	else if ((m_message.Mid(0,5)==_T("文件路径："))){//走流程，检测是否发送文件路径
		CString a = m_message.Mid(0, 4);
		struct sends* sendd = new struct sends;
		sendd->type = 5;
		CString tempp = _T("\\") + m_message.Mid(5, m_message.GetLength());
		int length = WideCharToMultiByte(CP_ACP, 0, tempp, -1, NULL, 0, NULL, NULL);
		if (length <= 256) {
			WideCharToMultiByte(CP_ACP, 0, tempp, -1, sendd->strRelativePath, length, NULL, NULL);
		}
		else {
			MessageBox(_T("这文件名也太长了，你有病吧"), _T("温馨提示"), MB_OK);
			m_chat.InsertItem(0, _T("帅比本人"), 0);
			m_chat.SetItemText(0, 1, _T("说了句废话，呵~"));
			m_message = "";
			delete sendd;
			UpdateData(FALSE);
			return;
		}
		m_pClientSocket->Send(sendd, sizeof(struct sends));
		m_chat.InsertItem(0, _T("帅比本人"), 0);
		m_chat.SetItemText(0, 1, m_message);
		m_message = "";
		delete sendd;
		UpdateData(FALSE);
	}
	else{//正常聊天
		
		struct sends* sendd = new struct sends;
		sendd->type = type+2;
		memcpy(sendd->buffer + 1, m_message.GetBuffer(0), m_message.GetLength()*2);
		CString strMsg;
		m_pClientSocket->Send(sendd, sizeof(struct sends));//发送自己发的话
		type += 10;
		if (type > 20480)
			type = 0;
		m_chat.InsertItem(0, _T("帅比本人"), 0);//本地聊天框插入自己发的话
		m_chat.SetItemText(0, 1, m_message);
		m_message = "";
		delete sendd;
		UpdateData(FALSE);
		
	}
	
}



void CMFCApplication3Dlg::OnBnClickedButton5()
{
	//按下发文件夹按钮
	UpdateData(TRUE);//更新消息编辑框绑定的变量
	if (m_strFilePath.IsEmpty()) {
		MessageBox(_T("笨蛋，你还没选文件呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	CFile file;
	CString startPath = m_strFilePath;
	TraverseFolder1(startPath, m_pClientSocket, startPath,&type);//发文件夹
	TraverseFolder2(startPath, m_pClientSocket, startPath,&type);//发文件
	m_strFilePath = _T("");//恢复现场
	UpdateData(FALSE);
}
