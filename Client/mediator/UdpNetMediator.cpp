#include "UdpNetMediator.h"
#include"../net/UdpNet.h"
UdpMediator::UdpMediator()
{
	m_pNet = new UdpNet(this);
}

UdpMediator::~UdpMediator()
{
	if (m_pNet)
	{
		m_pNet->unInitNet();
		delete m_pNet;
		m_pNet = nullptr;
	}
}

bool UdpMediator::openNet()
{
	return m_pNet->initNet();
}

void UdpMediator::closeNet()
{
	return m_pNet->unInitNet();
}

bool UdpMediator::sendData(char* data, int len, long to)
{
	return m_pNet->sendData(data,len,to);
}

void UdpMediator::transmiteData(char* data, int len, long from)
{
	//测试代码：打印出接收到的数据
	cout << " UdpMediator::transmiteData:" << data << endl;

}
