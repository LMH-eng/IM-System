#pragma once
#include<map>
#include<iostream>
#include<mutex> //后加
#include<ctime> //后加
#include"MySQL/CMySql.h"
#include"net/def.h"
#include"mediator/iNetMediator.h"
using namespace std;
class kernel {
public:
	kernel();
	~kernel();

	//初始化和存储处理函数数组
	void setProtpcolVer();
	//打开服务器
	bool startServer();
	//关闭服务器
	void closeServer();
	//接受所有数据
	void dealData(char* data, int len, long from);
	//处理注册请求
	void dealRegisterRq(char* data, int len, long from);
	//处理登录请求
	void dealLoginRq(char* data, int len, long from);
	//给客户端发送登录用户的信息和她好友的信息
	void sendUserInfoAndFriendInfo(int userId);
	//根据id查询用户信息
	void getInfoById(int id, PROT_FRIEND_INFO* info);
	//处理聊天请求
	void dealChatRq(char* data, int len, long from);
	//处理下线请求
	void dealOfflineRq(char* data, int len, long from);
	//处理添加好友
	void dealAddFriendRq(char* data, int len, long from);
	//处理添加好友回复
	void dealAddFriendRs(char* data, int len, long from);
	void dealHeartbeatRq(char* data, int len, long from); //后加 处理心跳请求
	void checkHeartbeatTimeout(); //后加 检查超时用户并通知好友下线
private:
	iNetMediator* m_pMediator;
public:
	static kernel* m_pKernel;
	CMySql m_sql;
	//定义函数指针数组
	using PROFUN = void (kernel::*)(char*, int, long);
	PROFUN m_pArrProtFun[_PROTOCOL_COUNT];//类成员函数指针数组
	//保存用户id和socket(登陆成功的时候保存，下线的时候删除)
	map<int, SOCKET>m_mapIdToSocket;
	map<int, time_t> m_mapIdToLastActive; //后加 保存用户最后活跃时间
	mutex m_mutex; // 后加的
	mutex m_dbMutex; // 后加的 数据库操作专用锁
};