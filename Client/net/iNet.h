#pragma once
#include"def.h"
#include<Winsock2.h>
#include<iostream>
#include<process.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;
//先声明后使用先声明这个类使用类名去定义变量编译一会就编译到这个类了
class iNetMediator;
class INet {
public:
	INet():m_sock(INVALID_SOCKET),m_handle(NULL), m_bRunning(true), m_pMediator(NULL){}
	virtual ~INet() {}
	//初始化网络
	virtual bool initNet() = 0;
	//关闭网络
	virtual void unInitNet() = 0;
	//发送数据
	//udp:sendto(),ip地址决定数据发给谁，ulong类型
    //tcp:send(),socket决定数据发给谁，uint类型
	//发送的数据一直在变化 要发送的数据，长度，  发给谁
	virtual bool sendData(char* data,int len,long to) = 0;
	//接收数据
	virtual void recvData() = 0;	
protected:
	SOCKET m_sock;
    HANDLE m_handle;//线程句柄
	bool m_bRunning;
	iNetMediator* m_pMediator;
};
