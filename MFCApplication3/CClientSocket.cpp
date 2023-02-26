#include "pch.h"
#include "CClientSocket.h"
#include "MFCApplication3Dlg.h"
#define PORT 4980
#include <windows.h>
#include <sddl.h>
#include <iostream>
#include <memory>
#include<ctype.h>
#include<vector>
#include<string>
#include"afxsock.h"
using namespace std;//曾想过用智能指针，弃用

struct sends {//承载发送信息的结构体
	short int type = 0;//用于区分发送的类型：1 发文件；2 发消息 3发文件夹名字 4、5、6用于客户下文件
	short int length = 0;//发送消息的长度，仅会用在发文件内容时，内容不满1024，提示接收端
	WIN32_FIND_DATA wfd;//文件属性，仅用于发文件时的第一个包
	char buffer[1025];//文件内容或消息内容或问件夹相对路径
	char RelativePath[256];//文件的相对路径
};

DWORD dwReadCount = 0;//发文件过程中计数已写入文件的个数
short int state = 0;////判别发文件类型中是发wfd还是发的文件内容
WIN32_FIND_DATA wfd;//文件属性地址留存，便于在函数内修改继承
char* bigbuffer = NULL;//找到了更好的方法，现已弃用
struct sends* sendd;//结构体属性指针地址留存，便于在函数内修改继承
int waitlist = 1;//已弃用，曾怀疑过TCP会乱序
int now = 0;//没用但未弃用，同样曾怀疑过TCP会乱序使用
CString strRelativePath;//相对路径地址留存，便于在函数内修改继承
CFile fie;//文件属性地址留存，便于在函数内修改继承（用于客户端下文件）
CFile* file = &fie;//文件属性指针地址留存，便于在函数内修改继承（用于客户端下文件）
struct sends** senddbuffer = new struct sends* [2048];//早到的sendd的存储地址，已弃用



BOOL CreateDirectoryRecursively(CString strAbsolutePath) {

//输入：文件夹的绝对路径
// 输出：文件夹是否创建成功
// 功能：递归创建该绝对路径上的文件夹

	CString strFolderPath = strAbsolutePath;// 文件夹路径
	CString strNewFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\')); // 获取上一级目录
	if (!PathIsDirectory(strNewFolderPath))
	{
		// 如果上一级目录不存在，递归创建上一级目录
		if (!CreateDirectoryRecursively(strNewFolderPath)) {
			return FALSE;
		}
	}
	BOOL bCreateFolder = CreateDirectory(strFolderPath, NULL); // 创建文件夹
	if (bCreateFolder)
	{
		// 创建文件夹成功
		//AfxMessageBox(_T("文件夹创建成功！文件名是")+ strFolderPath);
		return TRUE;
	}
	else
	{
		// 创建文件夹失败
		AfxMessageBox(_T("文件夹创建失败！") + strFolderPath);
		return FALSE;
	}
}
void chuli(struct sends* sendd, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath, int* now, CFile** file) {
//输入：收到的结构体指针等（其余都是为了能让在函数内的修改下一次调用该函数仍能有所保留）
//功能：根据type不同处理收到的sendd
	if ((sendd->type) % 10 == 2) {	
		wchar_t message[1024] = { 0 };
		//MessageBox(NULL,_T("客户要你陪聊"), _T("温馨提示"), MB_OK);
		CMFCApplication3Dlg* pMainDlg = (CMFCApplication3Dlg*)AfxGetMainWnd();
		memcpy(message, sendd->buffer + 1, 1024);
		//MessageBox(NULL, message, _T("温馨提示"), MB_OK);
		pMainDlg->m_chat.InsertItem(0, _T("金主爸爸"), 0);
		pMainDlg->m_chat.SetItemText(0, 1, message);
	}
	else if ((sendd->type) % 10 == 1) {	// type:1 文件 state=0：
		if (*state == 0) {
			*wfd = sendd->wfd;
			CString strExeDir = _T("C:\\Users\\dell\\Desktop\\MFCC");
			//CString strExeDir = _T("D:\\111");
			int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, NULL, 0);
			wchar_t* wChar = new wchar_t[wLength];
			MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, wChar, wLength);
			CString str(wChar);
			*strRelativePath = str;
			CString nowpath = *strRelativePath;
			// 合并路径
			CString strAbsolutePath = strExeDir + nowpath;
			(*file)->Open(strAbsolutePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
			strAbsolutePath.ReleaseBuffer();
			if (wfd->nFileSizeLow == 0)//处理0kb文件
			{
				(*file)->Close();
				WIN32_FIND_DATA twfd = *wfd;
				SetFileTime((*file)->m_hFile, &twfd.ftCreationTime, &twfd.ftLastAccessTime, &twfd.ftLastWriteTime);
				SetFileAttributes(twfd.cFileName, twfd.dwFileAttributes);
				*state = 0;
			}
			*state = 1;
			*dwReadCount = 0;

		}
		else if (*state == 1) {//文件建立后写入文件内容
			char* buffer = new char[sendd->length + 1];
			buffer = sendd->buffer;
			buffer[sendd->length + 1] = '\0';
			*dwReadCount += sendd->length;
			(*file)->Write(buffer, sendd->length);
			if (*dwReadCount == (*wfd).nFileSizeLow) {
				(*file)->Close();
				WIN32_FIND_DATA twfd = *wfd;
				SetFileTime((*file)->m_hFile, &twfd.ftCreationTime, &twfd.ftLastAccessTime, &twfd.ftLastWriteTime);
				SetFileAttributes(twfd.cFileName, twfd.dwFileAttributes);
				*state = 0;
			}
		}

	}
	else if ((sendd->type) % 10 == 3)	// type3:建文件夹
	{
		int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->buffer, -1, NULL, 0);
		wchar_t* wChar = new wchar_t[wLength];
		MultiByteToWideChar(CP_ACP, 0, sendd->buffer, -1, wChar, wLength);
		CString str(wChar);
		CString strExePath;
		GetModuleFileName(NULL, strExePath.GetBufferSetLength(MAX_PATH), MAX_PATH);
		strExePath.ReleaseBuffer();
		CString strExeDir = _T("C:\\Users\\dell\\Desktop\\MFCC");
		//CString strExeDir = _T("D:\\111");
		*strRelativePath = str;
		// 合并路径
		CString strAbsolutePath;
		strAbsolutePath = strExeDir + *strRelativePath;
		if (PathIsDirectory(strAbsolutePath))
		{
			CString strMsg;
			//strMsg.Format(_T("%s 文件夹已经存在！"), strAbsolutePath);
			//AfxMessageBox(strMsg);
			*now += 10;
			if (*now > 20480)
				*now = 0;
			return;
		}
		if (CreateDirectoryRecursively(strAbsolutePath) == FALSE)
		{
			strAbsolutePath.ReleaseBuffer();
			MessageBox(NULL, str + _T("请注意请注意") + strAbsolutePath, _T("温馨提示"), MB_OK);
			DWORD dwError = GetLastError();
			CString strErrorMsg;
			strErrorMsg.Format(_T("文件夹创建失败，错误代码：%d"), dwError);
			AfxMessageBox(strErrorMsg);
			return;
		}
		//MessageBox(NULL, _T("文件夹建好了，路径是")+ strAbsolutePath, _T("温馨提示"), MB_OK);
	}
	*now += 10;
	if (*now > 20480)
		*now = 0;
}

void listlizhao(int* now, struct sends** senddbuffer, int* waitlist, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath, CFile** file) {
	//已弃用，曾以为收到的包会乱序，用来梳理收到包的顺序，现在根本不会进入这个函数
	for (int i = 0; i < *waitlist - 1; i++) {
		printf("%d,", senddbuffer[i]->type);
		if (senddbuffer[i]->type > *now && senddbuffer[i]->type < *now + 10) {
			chuli(senddbuffer[i], bigbuffer, state, wfd, dwReadCount, strRelativePath, now, file);
			(*waitlist)--;
			senddbuffer[i] = senddbuffer[*waitlist - 1];
			//senddbuffer = (struct sends**)realloc(senddbuffer, sizeof(struct sends*)*(*waitlist));
			listlizhao(now, senddbuffer, waitlist, bigbuffer, state, wfd, dwReadCount, strRelativePath, file);
			break;
		}
	}
}

void CClientSocket::OnReceive(int nErrorCode)
{
	CSocket::OnReceive(nErrorCode);

	// 读取数据
	char szBuffer[1880];
	int nBytesRead = 0;
	while (nBytesRead < sizeof(szBuffer))//只有缓冲区有1880个数据（一个sendd的size）才用sendd接收（两地TCP连接产生，本地主机不会拥塞）
	{
		int nRead = Receive(szBuffer + nBytesRead, sizeof(szBuffer) - nBytesRead);
		if (nRead == SOCKET_ERROR)
		{
			// 发生错误，处理错误
			int nError = GetLastError();
			MessageBox(NULL, _T("读取缓冲区失败"), _T("温馨提示"), MB_OK);
			return;
		}
		nBytesRead += nRead;
	}

	struct sends* sendd = new struct sends;//接收sendd
	sendd = (struct sends*)szBuffer;

	//以下为 为实现规定要求的 专属type
	if (sendd->type == 4) {//收到服务器please input the...
		MessageBox(NULL, _T("触发隐藏功能，输入“文件路径：路径本径”召唤神龙"), _T("温馨提示"), MB_OK);
	}
	else if (sendd->type == 5) {//收到服务器反馈服务器没有客户输入的文件名
		MessageBox(NULL, _T("隐藏功能关闭，瞎输文件名，神生气了"), _T("温馨提示"), MB_OK);
	}
	else if (sendd->type == 6) {//收到服务器发的文件
		
		sendd->type = 1;
		chuli(sendd, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &now, &file);
		now-=10;
	}



	//以下为非要求的自由发送文件文件夹时会收到的包
	else if (sendd->type <now || sendd->type >now + 10)//按顺序该收到标号为几的包了，因不会乱序，所以必会进入else
	{
		senddbuffer[waitlist - 1] = sendd;//存储提前到达的包
		waitlist++;
		if (waitlist >= 2047) {
			MessageBox(NULL, _T("你网络也太差劲了吧，我无能为力，兄弟自求多福"), _T("温馨提示"), MB_OK);
			return;
		}
		//chuli(sendd,bigbuffer,&state,&wfd,&dwReadCount, &strRelativePath);
		listlizhao(&now, senddbuffer, &waitlist, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &file);//递归处理等待列表中符合要求的sendd
		return;
	}
	else {//处理收到的sendd
		chuli(sendd, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &now, &file);
	}
}

void CClientSocket::OnClose(int nErrorCode)
{
	// 连接断开提醒
	MessageBox(NULL, _T("服务器关机了亲！>>>>"), _T("温馨提示"), MB_OK);
	CSocket::OnClose(nErrorCode);
}
