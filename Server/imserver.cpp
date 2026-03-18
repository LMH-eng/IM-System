#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<iostream>
#include<Windows.h>

#include"kernel.h"

using namespace std;
int main()
{  
	kernel ker;
	if (!ker.startServer())
	{
		cout << "打开服务器失败" << endl;
		return 1;
	}
	

	
	while (true)
	{
		
			Sleep(5000);
			cout << "server running";
}


	
	return 0;
}
//im聊天系统（c/s模型）
//注册
//登录
//显示好友列表
//聊天
//添加好友
//下线

//客户端qt
//ui界面类
//kernel核心处理类（组织要发送的数据，处理接收到的数据）
//中介者类（发送数据，接收数据，打开网络，关闭网络）
//网络类（发送数据，接收数据，初始化网络类，关闭网络）

//服务端vs
//数据库类（连接数据库，更新数据库，查询数据库，断开连接）
//核心处理类（组织要发送的数据，处理接收到的数据）
//中介者类（发送数据，接收数据，打开网络，关闭网络）
//网络类（发送数据，接收数据，初始化网络类，关闭网络）

//网络类（支持udp和tcp协议）
//inet父类（发送数据，接收数据，初始化网络类，关闭网络）
//udp子类
//TCP client子类
//TCP server子类

//三种变量：
//局部
//类成员变量
//全局变量，静态类成员变量

//编译问题
//未定义标识符：没加头文件
//已有主体：函数重复实现
//无法解析的外部符号：找不到函数的实现  
//自己写的函数没实现---在cpp实现函数   库中的函数找不到实现---没有导入依赖库

//数据结构STL
//数组:空间连续，已知下标查找快，增加和删除复杂
//链表：空间不连续，增加和删除方便，查找慢
//队列：先进先出
//栈：先进后出
//map：key-value键值对，一对一，查找快
//set：key=value,查找快