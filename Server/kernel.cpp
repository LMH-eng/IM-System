#include "kernel.h"
#include"net/def.h"
#include"mediator/TcpServerMediator.h"
#include <vector> // 动态数组
#include <mysql.h> // MySQL转义函数

// UTF-8转GBK编码（新加的）
static std::string utf8ToGbk(const char* utf8Str)
{
	if (!utf8Str || strlen(utf8Str) == 0) return "";
	
	// UTF-8 -> 宽字符
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wlen];
	MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wstr, wlen);
	
	// 宽字符 -> GBK
	int gbkLen = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* gbkStr = new char[gbkLen];
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, gbkStr, gbkLen, NULL, NULL);
	
	std::string result(gbkStr);
	delete[] wstr;
	delete[] gbkStr;
	return result;
}

// GBK转UTF-8编码（新加的）
static std::string gbkToUtf8(const char* gbkStr)
{
	if (!gbkStr || strlen(gbkStr) == 0) return "";
	
	// GBK -> 宽字符
	int wlen = MultiByteToWideChar(CP_ACP, 0, gbkStr, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wlen];
	MultiByteToWideChar(CP_ACP, 0, gbkStr, -1, wstr, wlen);
	
	// 宽字符 -> UTF-8
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* utf8Str = new char[utf8Len];
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8Str, utf8Len, NULL, NULL);
	
	std::string result(utf8Str);
	delete[] wstr;
	delete[] utf8Str;
	return result;
}

kernel* kernel::m_pKernel = nullptr;
kernel::kernel():m_pMediator(nullptr)
{
	m_pKernel = this;
	setProtpcolVer();
}

kernel::~kernel()
{
}

void kernel::setProtpcolVer()
{
	// 初始化协议处理函数数组
	memset(m_pArrProtFun, 0, sizeof(m_pArrProtFun));
	m_pArrProtFun[DEF_REGISTER_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealRegisterRq;
	m_pArrProtFun[DEF_LOGIN_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealLoginRq;
	m_pArrProtFun[DEF_CHATMSG_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealChatRq;
	m_pArrProtFun[DEF_FRIEND_OFFLINE - DEF_BASE_PACKAGETYPE] = &kernel::dealOfflineRq;
	m_pArrProtFun[DEF_ADD_FRIEND_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealAddFriendRq;
	m_pArrProtFun[DEF_ADD_FRIEND_RS - DEF_BASE_PACKAGETYPE] = &kernel::dealAddFriendRs;
	m_pArrProtFun[DEF_HEARTBEAT - DEF_BASE_PACKAGETYPE] = &kernel::dealHeartbeatRq;
}
bool kernel::startServer()
{
	// 开启网络
	m_pMediator = new TcpServerMediator;
	if (!m_pMediator->openNet()) {
		cout << "开启网络失败" << endl;
		return false;
	}
	// 连接数据库
	char ip[] = "127.0.0.1";
	char name[] = "root";
	char pass[] = "1a2b3c4d";
	char db[] = "20241019im";
	if (!m_sql.ConnectMySql(ip,name,pass,db)) {
		cout << "连接数据库失败" << endl;
		return false;
	}
	else {
		cout << "连接数据库成功" << endl;
	}
	return true;
}
void kernel::closeServer()
{
	// 关闭网络
	if (m_pMediator) {
		m_pMediator->closeNet();
		delete m_pMediator;
		m_pMediator = nullptr;
		// 关闭数据库
		m_sql.DisConnect();
	}
}
// 处理接收到的数据
void kernel::dealData(char* data, int len, long from)
{
	cout << __func__ << endl;
	// 获取协议类型
	packageType type = *(packageType*)data;
	// 计算协议处理数组下标
	int index = type - DEF_BASE_PACKAGETYPE;
	if (index >= 0 && index < _PROTOCOL_COUNT) {
		PROFUN pFun = m_pArrProtFun[index];
		if (pFun) {
			(this->*pFun)(data, len, from);

		}
		else {
			cout << "type2:" << type << endl;
		}
	}
	else {
		cout << "type1:" << type << endl;
	}
	
	delete[]data;
	data = nullptr;
	
}
// 处理注册请求
void kernel::dealRegisterRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	
	PROT_REGISTER_INFO_RQ* rq = (PROT_REGISTER_INFO_RQ*)data;
	// 1.查询用户名是否存在
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql,"select name from t_user where name='%s';",rq->name);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql,1,lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	cout <<"sql:"<< sql << endl;
	// 2.判断用户名是否存在
	PROT_REGISTER_INFO_RS rs;
	if (0 ==lstStr.size() )
	{
		// 用户名不存在，继续检查电话号码
		// 3.查询该电话号码是否存在
		sprintf_s(sql, "select tel from t_user where tel='%s';", rq->tel);
		{
			lock_guard<mutex> lock(m_dbMutex);
			if (!m_sql.SelectMySql(sql, 1, lstStr)) {
				cout << "查询数据库失败" << sql << endl;
				return;
			}
		}
		// 4.判断电话号码是否存在
		if (0 == lstStr.size())
		{
			// 电话号码也不存在，可以注册
			// 5.将用户写入t_user表
			sprintf_s(sql, "insert into t_user(name,tel,pass,iconid,feeling)values('%s','%s','%s',5,'新用户');", rq->name,rq->tel,rq->pass);
			{
				lock_guard<mutex> lock(m_dbMutex);
				if (!m_sql.UpdateMySql(sql)) {
					cout << "注册写入失败" << sql << endl;
					return;
				}
			}
			// 6.返回注册成功rs
			rs.result = DEF_REGISTER_SUC;
		}
		else {
			// 电话号码已存在
			// 7.返回错误DEF_REGISTER_TEL_EXISTS
			rs.result = DEF_REGISTER_TEL_EXISTS;
		}

	}
	else {
		// 用户名已存在
		// 8.返回错误
		rs.result = DEF_REGISTER_NAME_EXISTS;

	}
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);
}

void kernel::dealLoginRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_LOGIN_RQ* rq = (PROT_LOGIN_RQ*)data;
	// 1.根据电话号码查询用户信息
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql, "select pass,id from t_user where tel='%s';", rq->tel);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 2, lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	// 2.判断用户是否存在
	PROT_LOGIN_RS rs;
	if (0 ==lstStr.size() ) {
		// 用户不存在
		// 3.返回错误DEF_LOGIN_NOTEXIST
		rs.result = DEF_LOGIN_NOTEXIST;
	}
	else {
		// 4.用户存在，取出密码
		string pass = lstStr.front();
		lstStr.pop_front();
		// 取出用户id
		int userId= stoi(lstStr.front());
		lstStr.pop_front();
		// 5.判断密码是否正确
		if (pass==rq->pass ) {
			// 6.密码正确，返回DEF_LOGIN_SUC
			rs.result = DEF_LOGIN_SUC;
			rs.userId = userId;
			// 映射用户id和socket
			m_mutex.lock();
			m_mapIdToSocket[userId]=from;
			m_mapIdToLastActive[userId] = time(nullptr); // 记录用户最后活跃时间
			m_mutex.unlock();
			// 8.发送登录结果给客户端
			m_pMediator->sendData((char*)&rs, sizeof(rs), from);
			// 发送用户信息和好友信息给客户端
			sendUserInfoAndFriendInfo(userId);
			// 发送离线消息
			sendOfflineMsgs(userId);
			// 发送离线好友申请（新加的）
			sendOfflineFriendReqs(userId);
			return;
		}
		else {
			// 7.密码错误，返回DEF_LOGIN_PASS_ERR
			rs.result = DEF_LOGIN_PASS_ERR;
			
		}
	}
	// 8.发送登录结果给客户端
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);

}

void kernel::sendUserInfoAndFriendInfo(int userId)
{
	cout << __func__ << endl;
	// 根据用户id获取用户信息
	PROT_FRIEND_INFO userInfo;
	getInfoById(userId, &userInfo);
	// 检查用户是否仍然在线
	m_mutex.lock();
	bool userOnline = m_mapIdToSocket.count(userId) > 0;
	SOCKET userSocket = m_mapIdToSocket[userId];
	m_mutex.unlock();
	if (userOnline) {
		m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), userSocket);
	}
	// 根据用户id查询好友id列表
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql, "select idB from t_friend where idA='%d';", userId);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 1, lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	// 遍历好友id列表
	int friendId = 0;
	PROT_FRIEND_INFO friendInfo;
	while (lstStr.size() > 0) {
		// 取出好友id
		friendId = stoi(lstStr.front());
		lstStr.pop_front();
		// 根据好友id获取好友信息
		getInfoById(friendId, &friendInfo);
		// 发送好友信息给用户
		m_mutex.lock();
		bool userOnline2 = m_mapIdToSocket.count(userId) > 0;
		SOCKET userSocket2 = m_mapIdToSocket[userId];
		m_mutex.unlock();
		if (userOnline2) {
			m_pMediator->sendData((char*)&friendInfo, sizeof(friendInfo), userSocket2);
		}
	}
	// 判断好友是否在线，如果在线发送用户信息给好友
	m_mutex.lock();
	bool friendOnline2 = m_mapIdToSocket.count(friendId) > 0;
	SOCKET friendSocket2 = m_mapIdToSocket[friendId];
	m_mutex.unlock();
	if (friendOnline2)
	{
		m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), friendSocket2);
	}
	
}
// 根据id获取用户信息
void kernel::getInfoById(int id, PROT_FRIEND_INFO* info)
{
	cout << __func__ <<"id:"<<id<< endl;
	info->userId = id;
	// 判断用户是否在线，即判断map中是否存在该用户id
	m_mutex.lock();
	bool online = m_mapIdToSocket.count(id) > 0;
	m_mutex.unlock();
	if (online) {
		// 在线
		info->status = DEF_STATUS_ONLINE;
	}
	else {
		// 离线
		info->status = DEF_STATUS_OFFLINE;
	}
	// 从数据库中查询用户信息，根据用户id查询
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql, "select name,iconid,feeling from t_user where id='%d';", id);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 3, lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	// 判断查询结果是否为3
	if (3 == lstStr.size()) {
		// 取出name
		strcpy_s(info->name, sizeof(info->name), lstStr.front().c_str());
		lstStr.pop_front();
		// 取出iconid
		info->iconId = stoi(lstStr.front());
		lstStr.pop_front();
		// 取出签名
		strcpy_s(info->feeling, sizeof(info->feeling), lstStr.front().c_str());
		lstStr.pop_front();
	}
	else {
		cout << "查询数据失败sql:" << sql << endl;
	}

}

// 发送离线消息
void kernel::sendOfflineMsgs(int userId)
{
	cout << __func__ << " userId: " << userId << endl;
	list<string> lstStr;
	char sql[1024] = "";
	sprintf_s(sql, "select id, from_id, message from t_offline_msg where to_id='%d';", userId);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 3, lstStr)) {
			cout << "查询离线消息失败" << endl;
			return;
		}
	}
	
	// 遍历并发送离线消息
	while (lstStr.size() >= 3) {
		int msgId = stoi(lstStr.front());
		lstStr.pop_front();
		int fromId = stoi(lstStr.front());
		lstStr.pop_front();
		string msg = lstStr.front();
		lstStr.pop_front();
		
		// 构造聊天消息
		PROT_CHATMSG_RQ* rq = new PROT_CHATMSG_RQ(fromId, userId);
		strncpy_s(rq->chatmsg, sizeof(rq->chatmsg), msg.c_str(), msg.length());
		
		// 检查用户是否仍在线
		m_mutex.lock();
		bool online = m_mapIdToSocket.count(userId) > 0;
		SOCKET userSocket = 0;
		if (online) {
			userSocket = m_mapIdToSocket[userId];
		}
		m_mutex.unlock();
		
		if (online) {
			m_pMediator->sendData((char*)rq, sizeof(PROT_CHATMSG_RQ), userSocket);
			cout << "发送离线消息, from: " << fromId << " to: " << userId << endl;
		}
		delete rq;
		
		// 删除已发送的离线消息
		char delSql[256] = "";
		sprintf_s(delSql, "delete from t_offline_msg where id=%d;", msgId);
		{
			lock_guard<mutex> lock(m_dbMutex);
			m_sql.UpdateMySql(delSql);
		}
	}
}

// 发送离线好友申请（新加的）
void kernel::sendOfflineFriendReqs(int userId)
{
	cout << __func__ << " userId: " << userId << endl;
	list<string> lstStr;
	char sql[1024] = "";
	sprintf_s(sql, "select id, from_id, from_name from t_offline_friend_req where to_id='%d';", userId);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 3, lstStr)) {
			return;
		}
	}
	
	while (lstStr.size() >= 3) {
		int reqId = stoi(lstStr.front());
		lstStr.pop_front();
		int fromId = stoi(lstStr.front());
		lstStr.pop_front();
		string fromName = lstStr.front();
		lstStr.pop_front();
		
		// 构造添加好友请求（新加的）
		PROT_ADD_FRIEND_RQ* rq = new PROT_ADD_FRIEND_RQ(fromId);
		// GBK转UTF-8后发送给客户端（新加的）
		std::string utf8Name = gbkToUtf8(fromName.c_str());
		strncpy_s(rq->myname, sizeof(rq->myname), utf8Name.c_str(), utf8Name.length());
		
		m_mutex.lock();
		bool online = m_mapIdToSocket.count(userId) > 0;
		SOCKET userSocket = 0;
		if (online) {
			userSocket = m_mapIdToSocket[userId];
		}
		m_mutex.unlock();
		
		if (online) {
			m_pMediator->sendData((char*)rq, sizeof(PROT_ADD_FRIEND_RQ), userSocket);
			cout << "发送离线好友申请, from: " << fromId << " to: " << userId << endl;
		}
		delete rq;
		
		// 删除已发送的离线好友申请（新加的）
		char delSql[256] = "";
		sprintf_s(delSql, "delete from t_offline_friend_req where id=%d;", reqId);
		{
			lock_guard<mutex> lock(m_dbMutex);
			m_sql.UpdateMySql(delSql);
		}
	}
}

void kernel::dealChatRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_CHATMSG_RQ* rq = (PROT_CHATMSG_RQ*)data;
	// 1.判断好友是否在线
	m_mutex.lock();
	bool online = m_mapIdToSocket.count(rq->friendid) > 0;
	SOCKET friendSocket = 0;
	if (online) {
		friendSocket = m_mapIdToSocket[rq->friendid];
	}
	m_mutex.unlock();
	if (online) {
		// 好友在线，直接转发消息
		m_pMediator->sendData(data, len, friendSocket);
	}
	else {
		// ============ 原代码（修改前）：只返回失败，不保存离线消息 ============
		// PROT_CHATMSG_RS rs;
		// rs.result = DEF_CHATMSG_FAIL;
		// rs.formid = rq->friendid;
		// rs.destid = rq->myid;
		// m_pMediator->sendData((char*)&rs, sizeof(rs), from);
		// 
		// TODO: 发给A后B离线，B登录后，之前发给B的消息，B应该收到，所以要存到数据库里
		// 需要一张离线消息表：保存Aid,Bid，消息内容，时间
		// B上线后，查询数据库，发现有B的离线消息，发送给B，然后删除数据库中B的离线消息
		// =======================================================================
		
		// ============ 新代码（修改后）：保存离线消息到数据库 ============
		// 好友离线，保存离线消息到数据库
		char sql[4096] = "";
		// 转义消息中的特殊字符，防止SQL注入
		char escapedMsg[DEF_CHATMSG_LEN * 2] = "";
		mysql_escape_string(escapedMsg, rq->chatmsg, strlen(rq->chatmsg));
		sprintf_s(sql, "insert into t_offline_msg(from_id, to_id, message) values(%d, %d, '%s');", 
			rq->myid, rq->friendid, escapedMsg);
		{
			lock_guard<mutex> lock(m_dbMutex);
			if (!m_sql.UpdateMySql(sql)) {
				cout << "保存离线消息失败" << sql << endl;
			} else {
				cout << "离线消息已保存, from: " << rq->myid << " to: " << rq->friendid << endl;
			}
		}
		// 返回好友离线状态给发送者
		PROT_CHATMSG_RS rs;
		rs.result = DEF_CHATMSG_FAIL; // 好友离线
		rs.formid = rq->friendid;
		rs.destid = rq->myid;
		m_pMediator->sendData((char*)&rs, sizeof(rs), from);
	}
}

void kernel::dealOfflineRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_FRIEND_OFFLINE* rq = (PROT_FRIEND_OFFLINE*)data;
	// 根据用户id查询好友id列表
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql, "select idB from t_friend where idA='%d';", rq->myid);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 1, lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	// 遍历好友id列表
	int friendId = 0;
	PROT_FRIEND_INFO friendInfo;
	while (lstStr.size() > 0) {
		// 取出好友id
		friendId = stoi(lstStr.front());
		lstStr.pop_front();
		// 判断好友是否在线，如果在线发送下线通知给好友
		m_mutex.lock();
		bool friendOnline = m_mapIdToSocket.count(friendId) > 0;
		SOCKET friendSocket = m_mapIdToSocket[friendId];
		m_mutex.unlock();
		if (friendOnline) {
			m_pMediator->sendData(data, len, friendSocket);
		}

	}
	// 从map中删除该用户socket映射关系
	m_mutex.lock();
	bool userExists = m_mapIdToSocket.count(rq->myid) > 0;
	if (userExists) {
		// 取出socket
		SOCKET s = m_mapIdToSocket[rq->myid];
		closesocket(s);
		// 删除无效映射map
		m_mapIdToSocket.erase(rq->myid);
		m_mapIdToLastActive.erase(rq->myid);
	}
	m_mutex.unlock();
}
// 处理添加好友
void kernel::dealAddFriendRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_ADD_FRIEND_RQ* rq = (PROT_ADD_FRIEND_RQ*)data;
	// 1.根据好友名字查询好友id
	list<string> lstStr;// 存储结果
	char sql[1024] = "";// sql语句
	sprintf_s(sql, "select id from t_user where name='%s';", rq->friendname);
	{
		lock_guard<mutex> lock(m_dbMutex);
		if (!m_sql.SelectMySql(sql, 1, lstStr)) {
			cout << "查询数据库失败" << sql << endl;
			return;
		}
	}
	// 2.判断好友是否存在
	if (0 == lstStr.size()) {
		// 3.好友不存在
		PROT_ADD_FRIEND_RS rs(rq->myid, 0, DEF_ADD_FRIEND_NOTEXIST);
		strcpy_s(rs.fromname, sizeof(rs.fromname), rq->friendname);
		// 发送好友不存在的结果给客户端
		m_pMediator->sendData((char*)&rs, sizeof(rs), from);
	}
	else {
		// 4.取出好友id
		int friendId = stoi(lstStr.front());
		lstStr.pop_front();
		// 5.判断好友是否在线
		m_mutex.lock();
		bool friendOnline = m_mapIdToSocket.count(friendId) > 0;
		SOCKET friendSocket = m_mapIdToSocket[friendId];
		m_mutex.unlock();
		if (friendOnline) {
			// 6.好友在线，转发添加好友请求给好友
			m_pMediator->sendData(data, len, friendSocket);
		}
		else {
			// ============ 原代码（修改前）：好友离线，返回错误 ============
			// PROT_ADD_FRIEND_RS rs(rq->myid, friendId, DEF_ADD_FRIEND_OFFLINE);
			// strcpy_s(rs.fromname, sizeof(rs.fromname), rq->friendname);
			// // 发送好友离线的结果给客户端
			// m_pMediator->sendData((char*)&rs, sizeof(rs), from);
			// =============================================================

			// ============ 新代码（修改后）：保存离线好友申请到数据库 ============
			// 保存离线好友申请到数据库（新加的）
			char insertSql[1024] = "";
			// UTF-8转GBK（新加的）
			std::string gbkName = utf8ToGbk(rq->myname);
			sprintf_s(insertSql, "insert into t_offline_friend_req(from_id, to_id, from_name) values(%d, %d, '%s');", 
				rq->myid, friendId, gbkName.c_str());
			{
				lock_guard<mutex> lock(m_dbMutex);
				if (!m_sql.UpdateMySql(insertSql)) {
					cout << "保存离线好友申请失败" << insertSql << endl;
				} else {
					cout << "离线好友申请已保存, from: " << rq->myid << " to: " << friendId << endl;
				}
			}
			// 返回好友离线状态给申请者（新加的）
			PROT_ADD_FRIEND_RS rs(rq->myid, friendId, DEF_ADD_FRIEND_OFFLINE);
			strcpy_s(rs.fromname, sizeof(rs.fromname), rq->friendname);
			m_pMediator->sendData((char*)&rs, sizeof(rs), from);
			// ===================================================================
		}
	}
}

void kernel::dealAddFriendRs(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_ADD_FRIEND_RS* rs= (PROT_ADD_FRIEND_RS*)data;
	// 1.判断是否同意
	if (DEF_ADD_FRIEND_ACCEPT==rs->result) {
		// 2.如果同意，写入数据库表
		
		char sql[1024] = "";// sql语句
		sprintf_s(sql, "insert into t_friend values(%d,%d);",rs->destid,rs->fromid);
		{
			lock_guard<mutex> lock(m_dbMutex);
			if (!m_sql.UpdateMySql(sql)) {
				cout << "添加好友失败" << sql << endl;
				return;
			}
			sprintf_s(sql, "insert into t_friend values(%d,%d);", rs->fromid, rs->destid);
			if (!m_sql.UpdateMySql(sql)) {
				cout << "添加好友失败" << sql << endl;
				return;
			}
		}
		// 3.更新好友信息
		sendUserInfoAndFriendInfo(rs->destid);
	}
	// 4.发送添加好友结果给请求者
	m_mutex.lock();
	SOCKET destSocket = m_mapIdToSocket[rs->destid];
	m_mutex.unlock();
	m_pMediator->sendData(data, len, destSocket);
}

// 处理心跳请求
void kernel::dealHeartbeatRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_HEARTBEAT* rq = (PROT_HEARTBEAT*)data;

	// 更新用户最后活跃时间
	m_mutex.lock();
	if (m_mapIdToSocket.count(rq->userId) > 0) {
		m_mapIdToLastActive[rq->userId] = time(nullptr);
		cout << "收到心跳包, userId: " << rq->userId << endl;
	}
	m_mutex.unlock();

	// 发送心跳响应给客户端
	PROT_HEARTBEAT rs(rq->userId);
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);
}

// 检查心跳超时，如果超时则强制下线
void kernel::checkHeartbeatTimeout()
{
	time_t now = time(nullptr);
	vector<int> timeoutUsers;

	// 遍历所有用户
	m_mutex.lock();
	for (auto& pair : m_mapIdToLastActive) {
		if (now - pair.second > DEF_HEARTBEAT_TIMEOUT / 1000) {
			timeoutUsers.push_back(pair.first);
		}
	}
	m_mutex.unlock();

	// 处理超时用户
	for (int userId : timeoutUsers) {
		cout << "用户心跳超时, userId: " << userId << endl;

		// 构造下线消息
		PROT_FRIEND_OFFLINE offline(userId);

		// 查询好友列表
		list<string> lstStr;
		char sql[1024] = "";
		sprintf_s(sql, "select idB from t_friend where idA='%d';", userId);
		{
			lock_guard<mutex> lock(m_dbMutex);
			if (!m_sql.SelectMySql(sql, 1, lstStr)) {
				continue;
			}
		}

		// 通知好友
		while (lstStr.size() > 0) {
			int friendId = stoi(lstStr.front());
			lstStr.pop_front();

			m_mutex.lock();
			if (m_mapIdToSocket.count(friendId) > 0) {
				m_pMediator->sendData((char*)&offline, sizeof(offline), m_mapIdToSocket[friendId]);
			}
			m_mutex.unlock();
		}

		// 删除用户映射
		m_mutex.lock();
		m_mapIdToSocket.erase(userId);
		m_mapIdToLastActive.erase(userId);
		m_mutex.unlock();
	}
}
