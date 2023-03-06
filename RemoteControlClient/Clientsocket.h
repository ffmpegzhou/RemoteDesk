
#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <iostream>
#include <vector>


#pragma pack(push)
#pragma pack(1)              //改变字节对齐

class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}    //初始化成员列表
	
	/*打包函数*/
	CPacket(WORD nCMD, const BYTE* pData, size_t nSize)       //
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCMD;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) //求和
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack)/*副本构造函数, 以便对Cpacket类的对象进行赋值传递。当对象中有诸如指针、
		动态数组等数据类型时，使用默认构造函数可能出错，此时需要用户自定义副本构造函数（深拷贝）*/
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	/*解析数据包，参数分别是：
	pdata:要发送的数据
	nSize:数据长度*/

	CPacket(const BYTE* pData, size_t& nSize)        //
	{
		size_t i = 0;                              //i用来标记当前读取位置
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)         //寻找到指定的包头
			{
				sHead = *(WORD*)(pData + i);
				i += 2;                              //仅有包头长度的特殊情况，后移2字节
				break;
			}
		}
		if (i + 8 > nSize)      //小于一个基本包长（length,cmd,sum4+2+2），数据可能不全。解析失败,返回
		{
			nSize = 0;
			return;
		}

		nLength = *(DWORD*)(pData + i);      //读取包数据长度
		i += 4;
		if (nLength + i > nSize)      //包未完全解析（仅读取部分内容）,返回
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);        //读取命令
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);      //截掉多余的4个字符（length-cmd-sum），为数据包真实长度
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)//利用按位与进行和校验(取低8位的值)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)         //如果解析成功
		{
			nSize = i;     //head2 + length4 + sizeof（Strdata）
			return;
		}
		nSize = 0;        //否则解析失败，nSize置零
	}

	~CPacket() {}
	CPacket& operator=(const CPacket& pack)     //对等号"="进行运算符重载
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size()        //获得包数据大小
	{
		return nLength + 6;
	}
	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)(pData) = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;			 //头部，固定位FEFF（经验值），2字节
	DWORD nLength;		//包长度（从控制命令开始，到和校验结束），4字节
	WORD sCmd;			//控制命令，2字节
	std::string strData;  //数据
	WORD sSum;				//和校验，2字节
	std::string strOut;     //整个包数据


protected:
private:

};

#pragma pack(pop)    //还原对齐状态

typedef struct MouseEvent//鼠标事件结构体
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、移动、双击
	WORD nButton; //左右键、滚轮
	POINT ptXY; //坐标

}MOUSEEV, * PMOUSEEV;

//文件信息结构体
typedef struct file_info
{
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;           //是否有效
	BOOL IsDirectory;       //是否为目录
	BOOL HasNext;         //是否还有后续文件
	char szFileName[256];   //文件名
}FILEINFO, * PFILEINFO;


/*错误提示*/
std::string GetErrorInformation(int wasErrCode);


class CClientsocket
{
	/*用于比较大或复杂的对象，只初始化一次，应该还有一个private的构造函数，
		使得不能用new来实例化对象，只能调用getInstance方法来得到对象
		，而getInstance保证了每次调用都返回相同的对象。*/
public:
	static CClientsocket* getInstance()
	{
		if (m_instance == NULL)//静态函数无this指针
		{
			m_instance = new CClientsocket;
		}

		return m_instance;

	}
	bool InitSocket(int nIP, int nPort)
	{
		if (m_sock != INVALID_SOCKET)CloseSocket();
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1)
		{
			return FALSE;
		}
		sockaddr_in serv_adr;    //before bind
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		TRACE("addr %08X  nip  %08X\r\n", inet_addr("127.0.0.1 "), nIP);
		serv_adr.sin_addr.s_addr = htonl(nIP);
		serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1 ");
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("指定的ip无效,或不存在！");
			return false;
		}

		//connect
		int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox("连接失败！");  //消息框内部显示的文本
			TRACE("连接失败：%d\r\n", WSAGetLastError());  // 用于捕捉调试
			return false;
		}
		return TRUE;
	}
	/*&处理收发字符。接收服务端发来的数据，并且将数据放入缓冲区m_packet中。
	* 注意index用静态变量，为了在循环中保留它的值。
	* 也要注意buffer大小需要灵活调整，避免溢出导致卡死
	*/
#define BUFFERSIZE 4096000    

	int DealCommand()
	{
		if (m_sock == -1)return -1;
		char* buffer = m_buffer.data();
		//char* buffer = new char[BUFFERSIZE];
	   // memset(buffer, 0, BUFFERSIZE);
		static size_t index = 0;
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFERSIZE - index, 0); //实际接收的长度
			if (len == 0)TRACE("%d\r\n", WSAGetLastError());     
			if (((int)len <= 0) && ((int)index <= 0))   
			{
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)			 //解析成功
			{
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, size_t nSize)
	{
		if (m_sock == -1) return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack)
	{
		TRACE("msock=%d\r\n", m_sock);
		if (m_sock == -1)return false;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
	}

	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)
			|| (m_packet.sCmd == 9))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& GetPacket()        //返回命令
	{
		return m_packet;
	}

	void CloseSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}


private:
	std::vector<char> m_buffer;   //用动态数组代替char[]数组
	SOCKET m_sock;
	CPacket m_packet;
	CClientsocket& operator=(const CClientsocket& ss) {}     // 赋值
	CClientsocket(const CClientsocket& ss) //副本构造函数
	{
		m_sock = ss.m_sock;
	}

	CClientsocket()
	{
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境,检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		//m_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_buffer.resize(BUFFERSIZE);   //初始化设置buffer大小
		memset(m_buffer.data(), 0, BUFFERSIZE);
	}
	~CClientsocket()
	{
		closesocket(m_sock);
		WSACleanup();         //释放资源，终止Winsock 2 DLL (Ws2_32.dll) 的使用
	}
	//初始化socket
	BOOL InitSockEnv()
	{
		WSADATA data;       //这个结构存储被WSAStartup函数调用后返回的Windows Sockets数据。
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)  //Windows Sockets Asynchronous，Windows异步套接字的启动命令
		{
			return FALSE;//TODO:返回值处理

		}
		return TRUE;

	}
	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			CClientsocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CClientsocket* m_instance;


	class CHelper
	{
	public:
		CHelper() {
			CClientsocket::getInstance();
		}

		~CHelper()
		{
			CClientsocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

//extern CClientsocket server;     //声明外部变量
