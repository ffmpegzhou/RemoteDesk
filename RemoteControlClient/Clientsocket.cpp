#include "pch.h"
#include "Clientsocket.h"

CClientsocket* CClientsocket::m_instance = NULL;      //�ÿ�
CClientsocket::CHelper CClientsocket::m_helper;              //ȷ�����캯��ȫ��Ψһ
CClientsocket* pclient = CClientsocket::getInstance();

std::string GetErrorInformation(int wasErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wasErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}