#include "pch.h"
#include"zip.h"
CString _getRelateFolder(const CString& compressedFile, const CString& zipFile);
CString _getRelateFolder(const CString& compressedFile, const CString& zipFile)
{
	CString result = _T("");
	CFileFind compressedFileFind, zipFileFind;
	if (compressedFileFind.FindFile(compressedFile) && zipFileFind.FindFile(zipFile))
	{
		CString compressedDir = compressedFileFind.GetRoot();
		CString zipDir = zipFileFind.GetRoot();
		compressedDir.Replace(zipDir, _T(""));
		result = compressedDir;
	}
	return result;
}
bool _compressedFolder(const HZIP& zip, const CString& zipPath, const CString& filePath)
{
	bool ret = false;
	//AfxMessageBox(_T("��ʼѹ���ļ������ļ�"));
	if (zip)
	{
		CFileFind finder;
		BOOL bWorking = finder.FindFile(zipPath + _T("\\*.*"));
		//AfxMessageBox(_T("ѹ������������Ч"));
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			CString strPath = finder.GetFilePath();
			if (finder.IsDirectory() && !finder.IsDots())
			{
				// �ļ��еݹ����
				_compressedFolder(zip, finder.GetFilePath(), filePath);
			}
			else if (!finder.IsDirectory() && !finder.IsDots())
			{
				// �ļ���ӵ��ļ�ѹ���ļ�
				CString relatePath = _getRelateFolder(strPath, filePath);
				relatePath += finder.GetFileName();
				ZipAdd(zip, relatePath, strPath);
				//ZipAdd(zip, finder.GetFileName(), strPath);
				//AfxMessageBox(_T("ѹ���ļ���") + strPath + _T("�����·����") + relatePath);
			}
		}
	}
	else
	{
		AfxMessageBox(_T("ѹ��������Ч"));
	}

	return ret;
}
bool compressedFile(const CString& zipPath, const CString& filePath)
{
	CFileFind find;
	if (!PathFileExists(zipPath))
		return false;
	HZIP hz = CreateZip(filePath, 0);
	if (GetFileAttributes(zipPath) == FILE_ATTRIBUTE_DIRECTORY)
		_compressedFolder(hz, zipPath, filePath);
	else
	{
		CString fileName = zipPath.Right(zipPath.ReverseFind('\\'));
		ZipAdd(hz, fileName, zipPath);
	}

	CloseZip(hz);
	return true;
}
