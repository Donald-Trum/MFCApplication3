#include "pch.h"
#include "CClientSocket.h"
#include "MFCApplication3Dlg.h"
#define PORT 4980
void CClientSocket::OnReceive(int nErrorCode)
{
	
	WIN32_FIND_DATA wfd;
	Receive(&wfd, sizeof(wfd));
	CSocket::OnReceive(nErrorCode); 
	CFile file(wfd.cFileName, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
	DWORD dwReadCount = 0;
	while (dwReadCount < wfd.nFileSizeLow)
	{
		char* buffer = new char[1025];
		int nRec = Receive(buffer, 1024);
		buffer[nRec] = '\0';
		file.Write(buffer,
			nRec);
		dwReadCount += nRec;
		delete[] buffer;
	}
	SetFileTime(file.m_hFile, &wfd.ftCreationTime, &wfd.ftLastAccessTime, &wfd.ftLastWriteTime); 
	SetFileAttributes(wfd.cFileName, wfd.dwFileAttributes);
	MessageBox(NULL, _T("文件已收到！"), _T("温馨提示"), MB_OK);
	file.Close();
}






void CClientSocket::OnClose(int nErrorCode)
{
	MessageBox(NULL, _T("服务器关机了亲！"), _T("温馨提示"), MB_OK);
	CSocket::OnClose(nErrorCode);
}


