#include "TcpClientMediator.h"
#include"../net/TcpClient.h"
TcpClientMediator::TcpClientMediator()
{
	m_pNet = new TcpClient(this);
}

TcpClientMediator::~TcpClientMediator()
{
	if (m_pNet)
	{
		m_pNet->unInitNet();
		delete m_pNet;
		m_pNet = nullptr;
	}
}

bool TcpClientMediator::openNet()
{
	return m_pNet->initNet();
}

void TcpClientMediator::closeNet()
{
	return m_pNet->unInitNet();
}

bool TcpClientMediator::sendData(char* data, int len, long to)
{
	return m_pNet->sendData(data, len, to);
}

void TcpClientMediator::transmiteData(char* data, int len, long from)
{
	//测试代码打印接收到的数据 应用层最大64K 传输层最大1500字节容易接受不完全
	cout << " TcpClientMediator::transmiteData:" << data << endl;
}
