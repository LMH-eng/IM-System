#include "TcpServerMediator.h"
#include"../net/TcpServer.h"
#include"../kernel.h"
TcpServerMediator::TcpServerMediator()
{
	m_pNet = new TcpServer(this);
}

TcpServerMediator::~TcpServerMediator()
{
	if (m_pNet)
	{
		m_pNet->unInitNet();
		delete m_pNet;
		m_pNet = nullptr;
	}
}

bool TcpServerMediator::openNet()
{
	return m_pNet->initNet();
}

void TcpServerMediator::closeNet()
{
	return m_pNet->unInitNet();
}

bool TcpServerMediator::sendData(char* data, int len, long to)
{
	return m_pNet->sendData(data, len, to);
}

void TcpServerMediator::transmiteData(char* data, int len, long from)
{
	if (kernel::m_pKernel)
	{
		kernel::m_pKernel->dealData(data, len, from);
	}

}
