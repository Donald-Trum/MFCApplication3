#pragma once
#include <afxsock.h>
class CClientSocket :
	public CSocket
{
public:
	virtual void OnReceive(int nErrorCode);
//	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};

