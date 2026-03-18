#pragma once
#include"iNet.h"
class TcpClient :public INet {
public:
	TcpClient(iNetMediator* p);
	~TcpClient();
	//初始化网络（加载库，创建套接字，绑定，连接服务器，创建接受数据的线程）
	bool initNet();
	//关闭网络
	void unInitNet();
	//发送数据
	//udp:sendto(),ip地址决定数据发给谁，ulong类型
	//tcp:send(),socket决定数据发给谁，uint类型
	//发送的数据一直在变化 要发送的数据，长度，  发给谁
	bool sendData(char* data, int len, long to);
	//接收数据
	void recvData();
	static unsigned __stdcall recvThread(void* lpVoid);
};