#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "TcpServer.h"
#include"../mediator/TcpServerMediator.h"
TcpServer::TcpServer(iNetMediator* p)
{
	m_pMediator=p;

}
TcpServer::~TcpServer()
{
}
//
bool TcpServer::initNet()
{
	//1.加载库
	WORD version = MAKEWORD(2, 2);
	WSADATA data = {};
	int err = WSAStartup(version, &data);
	if (0 != err) {
		cout << " WSAStartup fail" << endl;
		return false;
	}
	if (2 != HIBYTE(data.wVersion) || 2 != LOBYTE(data.wVersion))
	{
		cout << " WSAStartup version error" << endl;
		return false;
	}
	else {
		cout << " WSAStartup version success" << endl;
	}
	//2.创建套接字ipv4 tcp
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_sock)
	{
		cout << "socket error" << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "socket success" << endl;
	}
	//3.绑定ip和端口
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_TCP_PORT);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	err = bind(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (SOCKET_ERROR == err)
	{
		cout << "bind error" << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "bind success" << endl;
	}
	//4.监听
	err = listen(m_sock, _LISTEN_MAX_LENGTH/*等待连接的队列的最大长度*/);
	if (SOCKET_ERROR == err)
	{
		cout << "listen error" << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "listen success" << endl;
	}
	HANDLE handle = (HANDLE)_beginthreadex(nullptr, 0, &acceptThread, this, 0, nullptr);
	if (handle) {
		m_listHandle.push_back(handle);
	}
	return true;
}
unsigned __stdcall TcpServer::acceptThread(void* lpVoid)
{
	TcpServer* pThis=(TcpServer*)lpVoid;//从void*转回去
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in addrClient = {};
	HANDLE handle = nullptr;
	int s = sizeof(addrClient);
	//保存线程id
	unsigned int threadId = 0;
	while (pThis->m_bRunning) {
		//接受连接
		sock = accept(pThis->m_sock, (sockaddr*)&addrClient, &s);
		if (INVALID_SOCKET == sock)
		{
			cout << "accept error:" << WSAGetLastError() << endl;
		}
		else {
			//接受连接成功打印客户端IP地址
			cout << "client ip:" << inet_ntoa(addrClient.sin_addr) << endl;
			//给连接成功的客户端创建一个接收数据的线程
			handle = (HANDLE)_beginthreadex(nullptr, 0, &recvThread, pThis, 0, &threadId);
			//保存线程句柄
			if (handle) {
				pThis->m_listHandle.push_back(handle);
			}
			
			//保存连接成功的客户端的socket
			pThis->m_mapThreadIdToSocket[threadId] = sock;
		}
	}
	return 1;
}
////接受数据的线程 调用recvdata函数
unsigned __stdcall TcpServer::recvThread(void* lpVoid)
{
	TcpServer* pThis = (TcpServer*)lpVoid;
	pThis->recvData();
	return 0;
}
void TcpServer::unInitNet()
{
	m_bRunning = false;
	
	HANDLE handle = nullptr;
	for (auto ite = m_listHandle.begin(); ite != m_listHandle.end();)
	{
		//取出保存的句柄
		handle = *ite;
		//回收句柄
		if (handle)
	{
		if (WAIT_TIMEOUT == WaitForSingleObject(handle, 5000)) {
			TerminateThread(handle, -1);
		}
		CloseHandle(handle);
		handle = nullptr;
		
	}
		//把无效节点从list移除
		ite=m_listHandle.erase(ite);
	}
	//2.关闭套接字
	if (m_sock && INVALID_SOCKET != m_sock)
	{
		closesocket(m_sock);
	}
	SOCKET s = INVALID_SOCKET;
	for (auto ite = m_mapThreadIdToSocket.begin(); ite != m_mapThreadIdToSocket.end();) {
		//取出当前节点保存的socket
		s = ite->second;
		if (s && INVALID_SOCKET != s)
		{
			closesocket(s);
		}
		//把无效节点从map中删除 erase会返回下一个有效节点
		ite = m_mapThreadIdToSocket.erase(ite);
	}
	//3.卸载库
	WSACleanup();
}

bool TcpServer::sendData(char* data, int len, long to)
{
	//判断传入参数是否合法
	if (!data || len < 1)
	{
		cout << "TcpClient::sendData paramater error" << endl;
		return false;
	}
	//2.先发数据长度len
	int nSendNum = send(to, (char*)&len, sizeof(int), 0);
	if (nSendNum == SOCKET_ERROR) {
		cout << "Tcp server send error: " << WSAGetLastError() << endl;
		return false;
	}
	//3.再发数据内容
	nSendNum = send(to, data, len, 0);
	if (nSendNum == SOCKET_ERROR) {
		cout << "Tcp server send error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

void TcpServer::recvData()
{
	//休眠一会为了保证accept thread把SOCKET保存到map
	Sleep(5);
	//去除当前线程对用的socket
	//获取线程
	unsigned int threadId = GetCurrentThreadId();
	//根据线程id获取socket
	SOCKET s = INVALID_SOCKET;
	if (m_mapThreadIdToSocket.find(threadId) != m_mapThreadIdToSocket.end())
	{
		s = m_mapThreadIdToSocket[threadId];
	}
	else
	{
		//map中没有保存当前线程的socket
		cout << "TcpServer::recvData socket error" << endl;
		return;
	}
	//记录一个包中累计长度的大小
	int nOffset = 0;
	//保存数据长度
	int packLen = 0;
	int nRecvNum = 0;
	while (m_bRunning)
	{
		nOffset = 0;
		//1.接收数据长度
		nRecvNum = recv(s, (char*)&packLen, sizeof(int), 0);
		if (nRecvNum > 0)
		{
			//2.按照数据长度packLen申请空间
			char* pPack = new char[packLen];
			//接受数据内容
			//循环接受一个包的数据内容
			while (packLen > 0) {//循环退出的条件是剩余空间为0，packLen记录剩余空间的大小
				nRecvNum = recv(s, pPack + nOffset, packLen, 0);
				if (nRecvNum > 0)
				{
					//成功接收一个包的一部分数据
					packLen -= nRecvNum;
					nOffset += nRecvNum;
				}
				else {
					cout << "TcpServer::recvData error:" << WSAGetLastError() << endl;
					break;
				}
			}
			//todo空间写满接受一个包的数据成功，接受数据内容成功把数据的空间地址传给中介者
			//测试代码打印接收到的数据 应用层最大64K 传输层最大1500字节容易接受不完全
			m_pMediator->transmiteData(pPack, nOffset, s);
		}
		else {
			cout << "TcpServer::recvData error:" << WSAGetLastError() << endl;
			break;
		}

	}
}




