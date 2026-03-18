#pragma once
#include"iNetMediator.h"
class UdpMediator:public iNetMediator {
public:
	UdpMediator();
	~UdpMediator();
	//初始化网络
	 bool openNet();
	//关闭网络
	void closeNet();
	//发送数据
	//udp:sendto(),ip地址决定数据发给谁，ulong类型
	//tcp:send(),socket决定数据发给谁，uint类型
	//发送的数据一直在变化 要发送的数据，长度，  发给谁
	bool sendData(char* data, int len, long to) ;
	//转发数据给核心处理器
	//data:要转发的数据
	//len：数据的长度
	//from:数据从哪来
	//udp:sendto(),ip地址决定数据从哪来，ulong类型
	//tcp:send(),socket决定数据从那个客户端来，uint类型
	void transmiteData(char* data, int len, long from);

};