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
    //转发数据给核心处理类
    Q_EMIT signals_recvServerData(data, len, from);
}
//qt信号和槽机制：两个类之间用来通知和发送数据（加参数）
//1.两个类都直接或间接继承自Qobject 并且有Q_OBJECT宏
//2.在发通知或者数据点类的头文件中使用signals声明信号，返回值是void，名字一般以signals_开头，参数列表
//信号不是函数，不需要在cpp实现，需要在发通知或者数据的地方使用emit或者Q_EMIT信号名（参数列表）;发送
//3.在接受数据的类的头文件中，使用public/protected/private slots声明槽函数，槽函数的参数和返回值要与信号一致
//在cpp中实现槽函数（收到数据以后要做的事）
//4.在接受数据的类里面，发送数据对象new出来的下面，连接信号和槽函数










