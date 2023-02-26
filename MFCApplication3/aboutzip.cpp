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
	//AfxMessageBox(_T("开始压缩文件夹内文件"));
	if (zip)
	{
		CFileFind finder;
		BOOL bWorking = finder.FindFile(zipPath + _T("\\*.*"));
		//AfxMessageBox(_T("压缩操作对象有效"));
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			CString strPath = finder.GetFilePath();
			if (finder.IsDirectory() && !finder.IsDots())
			{
				// 文件夹递归调用
				_compressedFolder(zip, finder.GetFilePath(), filePath);
			}
			else if (!finder.IsDirectory() && !finder.IsDots())
			{
				// 文件添加到文件压缩文件
				CString relatePath = _getRelateFolder(strPath, filePath);
				relatePath += finder.GetFileName();
				ZipAdd(zip, relatePath, strPath);
				//ZipAdd(zip, finder.GetFileName(), strPath);
				//AfxMessageBox(_T("压入文件：") + strPath + _T("，相对路径：") + relatePath);
			}
		}
	}
	else
	{
		AfxMessageBox(_T("压缩对象无效"));
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
