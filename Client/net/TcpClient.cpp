
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "TcpClient.h"
#include"../mediator/TcpClientMediator.h"
TcpClient::TcpClient(iNetMediator* p)
{
	m_pMediator = p;
}

TcpClient::~TcpClient()
{
}
//初始化网络（加载库，创建套接字，绑定，连接服务器，创建接受数据的线程）
bool TcpClient::initNet()
{
	//1.加载库
	WORD version = MAKEWORD(2, 2);
	WSADATA data = {};
	int err = WSAStartup(version, &data);
	if (0 != err)
	{
		cout << "WSAStartup fail" << endl;
		return false;
	}
	if (2 != HIBYTE(data.wVersion) || 2 != LOBYTE(data.wVersion))
	{
		cout << "WSAStartup version error" << endl;
		return false;
	}
	else {
		cout << "WSAStartup version success" << endl;
	}
	//2.创建套接字
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_sock)
	{
		cout << "socket error" << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "socket success" << endl;
	}
	// 3. 连接服务器
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_TCP_PORT);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	err = connect(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (SOCKET_ERROR == err) {
		cout << "connect error: " << WSAGetLastError() << endl;
		return false;
	}
	cout << "connect success" << endl;
	m_handle = (HANDLE)_beginthreadex(nullptr,0,&recvThread,this,0,nullptr);
	return true;
}
//接受数据的线程 调用recvdata函数 静态函数不能调用非静态函数
unsigned __stdcall TcpClient::recvThread(void* lpVoid)
{
	TcpClient* pThis = (TcpClient*)lpVoid;
	pThis->recvData();
	return 1;
}
//接受数据的线程函数（调用recvdata函数）
//关闭网络(回收线程资源，关闭套接字，卸载库)
void TcpClient::unInitNet()
{
	m_bRunning = false;
	if (m_handle)
	{
		if (WAIT_TIMEOUT == WaitForSingleObject(m_handle, 5000)) {
			TerminateThread(m_handle, -1);
		}
		CloseHandle(m_handle);
		m_handle = nullptr;
	}

	//2.关闭套接字
	if (m_sock && INVALID_SOCKET != m_sock)
	{
		closesocket(m_sock);
	}
	//3.卸载库
	WSACleanup();
}

bool TcpClient::sendData(char* data, int len, long to)
{
	//判断传入参数是否合法
	if (!data || len < 1)
	{
		cout << "TcpClient::sendData paramater error" << endl;
		return false;
	}
	//2.先发数据长度len
	int nSendNum = send(m_sock,(char*)&len, sizeof(int),0);
	if (nSendNum == SOCKET_ERROR) {
		cout << "TcpClient send error: " << WSAGetLastError() << endl;
		return false;
	}
	//3.再发数据内容
	nSendNum = send(m_sock, data,len, 0);
	if (nSendNum == SOCKET_ERROR) {
		cout << "TcpClient send error: " << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
void TcpClient::recvData()
{   //记录一个包中累计长度的大小
	int nOffset = 0;
    //保存数据长度
	int packLen = 0;
	int nRecvNum = 0;
	while (m_bRunning)
	{
		nOffset = 0;		
		//1.接收数据长度
		nRecvNum = recv(m_sock, (char*)&packLen, sizeof(int), 0);
		if (nRecvNum > 0)
		{
			//2.按照数据长度packLen申请空间
			char* pPack = new char[packLen];
			//接受数据内容
			//循环接受一个包的数据内容
			while (packLen>0) {//循环退出的条件是剩余空间为0，packLen记录剩余空间的大小
				nRecvNum = recv(m_sock, pPack+nOffset, packLen, 0);
				if (nRecvNum > 0)
				{
					//成功接收一个包的一部分数据
					packLen -= nRecvNum;
                    nOffset += nRecvNum;//新加的
				}
				else {
					cout << "TcpClient::recvData error:" << WSAGetLastError() << endl;
					break;
				}
			}
            m_pMediator->transmiteData(pPack, nOffset, m_sock);
		}
		else {
			cout << "TcpClient::recvData error:" << WSAGetLastError() << endl;
			break;
		}
		
	}
}
