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
using namespace std;//�����������ָ�룬����

struct sends {//���ط�����Ϣ�Ľṹ��
	short int type = 0;//�������ַ��͵����ͣ�1 ���ļ���2 ����Ϣ 3���ļ������� 4��5��6���ڿͻ����ļ�
	short int length = 0;//������Ϣ�ĳ��ȣ��������ڷ��ļ�����ʱ�����ݲ���1024����ʾ���ն�
	WIN32_FIND_DATA wfd;//�ļ����ԣ������ڷ��ļ�ʱ�ĵ�һ����
	char buffer[1025];//�ļ����ݻ���Ϣ���ݻ��ʼ������·��
	char RelativePath[256];//�ļ������·��
};

DWORD dwReadCount = 0;//���ļ������м�����д���ļ��ĸ���
short int state = 0;////�б��ļ��������Ƿ�wfd���Ƿ����ļ�����
WIN32_FIND_DATA wfd;//�ļ����Ե�ַ���棬�����ں������޸ļ̳�
char* bigbuffer = NULL;//�ҵ��˸��õķ�������������
struct sends* sendd;//�ṹ������ָ���ַ���棬�����ں������޸ļ̳�
int waitlist = 1;//�����ã������ɹ�TCP������
int now = 0;//û�õ�δ���ã�ͬ�������ɹ�TCP������ʹ��
CString strRelativePath;//���·����ַ���棬�����ں������޸ļ̳�
CFile fie;//�ļ����Ե�ַ���棬�����ں������޸ļ̳У����ڿͻ������ļ���
CFile* file = &fie;//�ļ�����ָ���ַ���棬�����ں������޸ļ̳У����ڿͻ������ļ���
struct sends** senddbuffer = new struct sends* [2048];//�絽��sendd�Ĵ洢��ַ��������



BOOL CreateDirectoryRecursively(CString strAbsolutePath) {

//���룺�ļ��еľ���·��
// ������ļ����Ƿ񴴽��ɹ�
// ���ܣ��ݹ鴴���þ���·���ϵ��ļ���

	CString strFolderPath = strAbsolutePath;// �ļ���·��
	CString strNewFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\')); // ��ȡ��һ��Ŀ¼
	if (!PathIsDirectory(strNewFolderPath))
	{
		// �����һ��Ŀ¼�����ڣ��ݹ鴴����һ��Ŀ¼
		if (!CreateDirectoryRecursively(strNewFolderPath)) {
			return FALSE;
		}
	}
	BOOL bCreateFolder = CreateDirectory(strFolderPath, NULL); // �����ļ���
	if (bCreateFolder)
	{
		// �����ļ��гɹ�
		//AfxMessageBox(_T("�ļ��д����ɹ����ļ�����")+ strFolderPath);
		return TRUE;
	}
	else
	{
		// �����ļ���ʧ��
		AfxMessageBox(_T("�ļ��д���ʧ�ܣ�") + strFolderPath);
		return FALSE;
	}
}
void chuli(struct sends* sendd, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath, int* now, CFile** file) {
//���룺�յ��Ľṹ��ָ��ȣ����඼��Ϊ�������ں����ڵ��޸���һ�ε��øú�����������������
//���ܣ�����type��ͬ�����յ���sendd
	if ((sendd->type) % 10 == 2) {	
		wchar_t message[1024] = { 0 };
		//MessageBox(NULL,_T("�ͻ�Ҫ������"), _T("��ܰ��ʾ"), MB_OK);
		CMFCApplication3Dlg* pMainDlg = (CMFCApplication3Dlg*)AfxGetMainWnd();
		memcpy(message, sendd->buffer + 1, 1024);
		//MessageBox(NULL, message, _T("��ܰ��ʾ"), MB_OK);
		pMainDlg->m_chat.InsertItem(0, _T("�����ְ�"), 0);
		pMainDlg->m_chat.SetItemText(0, 1, message);
	}
	else if ((sendd->type) % 10 == 1) {	// type:1 �ļ� state=0��
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
			// �ϲ�·��
			CString strAbsolutePath = strExeDir + nowpath;
			(*file)->Open(strAbsolutePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
			strAbsolutePath.ReleaseBuffer();
			if (wfd->nFileSizeLow == 0)//����0kb�ļ�
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
		else if (*state == 1) {//�ļ�������д���ļ�����
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
	else if ((sendd->type) % 10 == 3)	// type3:���ļ���
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
		// �ϲ�·��
		CString strAbsolutePath;
		strAbsolutePath = strExeDir + *strRelativePath;
		if (PathIsDirectory(strAbsolutePath))
		{
			CString strMsg;
			//strMsg.Format(_T("%s �ļ����Ѿ����ڣ�"), strAbsolutePath);
			//AfxMessageBox(strMsg);
			*now += 10;
			if (*now > 20480)
				*now = 0;
			return;
		}
		if (CreateDirectoryRecursively(strAbsolutePath) == FALSE)
		{
			strAbsolutePath.ReleaseBuffer();
			MessageBox(NULL, str + _T("��ע����ע��") + strAbsolutePath, _T("��ܰ��ʾ"), MB_OK);
			DWORD dwError = GetLastError();
			CString strErrorMsg;
			strErrorMsg.Format(_T("�ļ��д���ʧ�ܣ�������룺%d"), dwError);
			AfxMessageBox(strErrorMsg);
			return;
		}
		//MessageBox(NULL, _T("�ļ��н����ˣ�·����")+ strAbsolutePath, _T("��ܰ��ʾ"), MB_OK);
	}
	*now += 10;
	if (*now > 20480)
		*now = 0;
}

void listlizhao(int* now, struct sends** senddbuffer, int* waitlist, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath, CFile** file) {
	//�����ã�����Ϊ�յ��İ����������������յ�����˳�����ڸ�����������������
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

	// ��ȡ����
	char szBuffer[1880];
	int nBytesRead = 0;
	while (nBytesRead < sizeof(szBuffer))//ֻ�л�������1880�����ݣ�һ��sendd��size������sendd���գ�����TCP���Ӳ�����������������ӵ����
	{
		int nRead = Receive(szBuffer + nBytesRead, sizeof(szBuffer) - nBytesRead);
		if (nRead == SOCKET_ERROR)
		{
			// �������󣬴������
			int nError = GetLastError();
			MessageBox(NULL, _T("��ȡ������ʧ��"), _T("��ܰ��ʾ"), MB_OK);
			return;
		}
		nBytesRead += nRead;
	}

	struct sends* sendd = new struct sends;//����sendd
	sendd = (struct sends*)szBuffer;

	//����Ϊ Ϊʵ�ֹ涨Ҫ��� ר��type
	if (sendd->type == 4) {//�յ�������please input the...
		MessageBox(NULL, _T("�������ع��ܣ����롰�ļ�·����·���������ٻ�����"), _T("��ܰ��ʾ"), MB_OK);
	}
	else if (sendd->type == 5) {//�յ�����������������û�пͻ�������ļ���
		MessageBox(NULL, _T("���ع��ܹرգ�Ϲ���ļ�������������"), _T("��ܰ��ʾ"), MB_OK);
	}
	else if (sendd->type == 6) {//�յ������������ļ�
		
		sendd->type = 1;
		chuli(sendd, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &now, &file);
		now-=10;
	}



	//����Ϊ��Ҫ������ɷ����ļ��ļ���ʱ���յ��İ�
	else if (sendd->type <now || sendd->type >now + 10)//��˳����յ����Ϊ���İ��ˣ��򲻻��������Աػ����else
	{
		senddbuffer[waitlist - 1] = sendd;//�洢��ǰ����İ�
		waitlist++;
		if (waitlist >= 2047) {
			MessageBox(NULL, _T("������Ҳ̫��˰ɣ�������Ϊ�����ֵ�����ร"), _T("��ܰ��ʾ"), MB_OK);
			return;
		}
		//chuli(sendd,bigbuffer,&state,&wfd,&dwReadCount, &strRelativePath);
		listlizhao(&now, senddbuffer, &waitlist, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &file);//�ݹ鴦��ȴ��б��з���Ҫ���sendd
		return;
	}
	else {//�����յ���sendd
		chuli(sendd, &bigbuffer, &state, &wfd, &dwReadCount, &strRelativePath, &now, &file);
	}
}

void CClientSocket::OnClose(int nErrorCode)
{
	// ���ӶϿ�����
	MessageBox(NULL, _T("�������ػ����ף�>>>>"), _T("��ܰ��ʾ"), MB_OK);
	CSocket::OnClose(nErrorCode);
}
