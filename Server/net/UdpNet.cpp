#include "UdpNet.h"
#include"../mediator/UdpNetMediator.h"
UdpNet::UdpNet(iNetMediator* p)
{
	m_pMediator =p;
}

UdpNet::~UdpNet()
{
}
//初始化网络（加载库，创建套接字，绑定，创建接受数据的线程）
bool UdpNet::initNet()
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
	if (2 == HIBYTE(data.wVersion) && 2 == LOBYTE(data.wVersion)) {
		cout << "WSAStartup success" << endl;
	}
	else {
		cout << "WSAStartup version error" << endl;
		return false;
	}
	//2.创建套接字UDP
	m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_sock) {
		cout << "socket error: " << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "socket success" << endl;
	}
	//3.绑定ip和端口
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//魔鬼数字：给代码后期阅读和维护带来了非常大的困难，把数字定义成宏
	addr.sin_port = htons(_UDP_PORT);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	err = bind(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (SOCKET_ERROR == err) {
		cout << "bind error: " << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "bind success" << endl;
	}
	//4.创建接受数据的线程
	//create thread和eixtthread是一对，如果线程中使用了c++运行时库（如strcpy）的函数
	//申请空间未释放，eixtthread也不会回收空间，就会造成内存泄漏
	//_beginthreadex和_endthreadex是一对，_endthreadex会先回收空间，然后调用eixtthread
	m_handle=(HANDLE)_beginthreadex(nullptr/*安全级别，空代表默认安全级别*/,
		0/*堆栈大小，0代表使用默认堆栈大小1M*/,
		&recvThread/*线程函数起始地址*/,//静态的函数在没有对象之前就已经存在了
		this/*线程函数的参数列表*/,//调用函数的时候会创建对象 把对象传入线程
		0/*初始化标志位，0代表创建即运行，suspand挂起*/,
		nullptr/*输出参数，操作系统给线程分配的线程id*/);
	return true;
}
//接受数据的线程 调用recvdata函数 静态函数不能调用非静态函数
unsigned __stdcall UdpNet::recvThread(void* lpVoid)
{
	UdpNet* pThis = (UdpNet*)lpVoid;//从void*转回去
	pThis->recvData();
	return 1;
}


//关闭网络(回收线程资源，关闭套接字，卸载库)
void UdpNet::unInitNet()
{
	//1.回收线程资源：操作系统会给线程分配3个资源：线程id，线程句柄，内核对象，引用计数器是2
	//结束线程函数，计数器-1
	m_bRunning = false;
	
	//关闭线程句柄，计数器-1
	if (m_handle)
	{//等一会儿等线程函数运行到判断Bool值的代码就行
		if (WAIT_TIMEOUT == WaitForSingleObject(m_handle, 5000)){
			/*timeout是超时,等待5000ms时间以后线程还没有结束工作*/
			//强制杀死线程（不要一开始就杀死线程）
			TerminateThread(m_handle/*杀死哪个线程添线程的句柄*/, -1/*退出码*/);
		}
        //关闭线程句柄计数器-1
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
//发送数据
bool UdpNet::sendData(char* data, int len, long to)
{   //1.校验传入参数是否合法
	if (!data || len < 1)
	{
		cout << "UdpNet::sendData paramater error" << endl;
		return false;
	}
	//2.发送数据
	sockaddr_in sendTo;
	sendTo.sin_family = AF_INET;
	sendTo.sin_port = htons(_UDP_PORT);
	sendTo.sin_addr.S_un.S_addr = to;
	int nSendNum = sendto(m_sock, data, len, 0, (sockaddr*)&sendTo, sizeof(sendTo));
	if (SOCKET_ERROR == nSendNum)
	{
		cout << "sendto error:" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
//接收数据 放在线程里
void UdpNet::recvData()
{
	char recvBuf[4096] = "";
	int nRecvNum = 0;
	sockaddr_in from = {};
	int size = sizeof(from);
	while (m_bRunning)
	{//接收数据
		nRecvNum = recvfrom(m_sock, recvBuf, sizeof(recvBuf), 0, (sockaddr*)&from, &size);
		if (nRecvNum > 0)
		{
			//接受数据成功，根据接收到的数据长度new一个新空间
			char* pPack = new char[nRecvNum];
			//把接收到的数据拷贝到新空间  新空间 数据 长度
			memcpy(pPack, recvBuf, nRecvNum);
			//todo:把新空间的数据传给中介者类
			m_pMediator->transmiteData(pPack, nRecvNum, from.sin_addr.S_un.S_addr);
		}
		else {
			cout << "UdpNet::recvData error:" << WSAGetLastError() << endl;
			break;
		}
	}
}


