#pragma once
#include<map>
#include<iostream>
#include<mutex> // 互斥锁
#include<ctime> // 时间
#include"MySQL/CMySql.h"
#include"net/def.h"
#include"mediator/iNetMediator.h"
using namespace std;
class kernel {
public:
	kernel();
	~kernel();

	// 初始化协议处理函数数组
	void setProtpcolVer();
	// 开启服务器
	bool startServer();
	// 关闭服务器
	void closeServer();
	// 处理接收到的数据
	void dealData(char* data, int len, long from);
	// 处理注册请求
	void dealRegisterRq(char* data, int len, long from);
	// 处理登录请求
	void dealLoginRq(char* data, int len, long from);
	// 发送用户信息和好友信息给客户端
	void sendUserInfoAndFriendInfo(int userId);
	// 根据id获取用户信息
	void getInfoById(int id, PROT_FRIEND_INFO* info);
	// 处理聊天请求
	void dealChatRq(char* data, int len, long from);
	// 处理下线请求
	void dealOfflineRq(char* data, int len, long from);
	// 处理添加好友请求
	void dealAddFriendRq(char* data, int len, long from);
	// 处理添加好友回复
	void dealAddFriendRs(char* data, int len, long from);
	void dealHeartbeatRq(char* data, int len, long from);
	void checkHeartbeatTimeout();
	void sendOfflineMsgs(int userId); // 发送离线消息
	void sendOfflineFriendReqs(int userId); // 发送离线好友申请（新加的）
private:
	iNetMediator* m_pMediator;
public:
	static kernel* m_pKernel;
	CMySql m_sql;
	// 定义函数指针类型
	using PROFUN = void (kernel::*)(char*, int, long);
	PROFUN m_pArrProtFun[_PROTOCOL_COUNT];// 协议处理函数数组
	// 映射用户id和socket(用户上线，存入映射；用户下线，删除映射)
	map<int, SOCKET>m_mapIdToSocket;
	map<int, time_t> m_mapIdToLastActive; // 记录用户最后活跃时间
	mutex m_mutex; // 互斥锁
	mutex m_dbMutex; // 数据库互斥锁
};
