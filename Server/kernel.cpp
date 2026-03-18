#include "kernel.h"
#include"net/def.h"
#include"mediator/TcpServerMediator.h"
#include <vector> //后加
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
	//初始化数组
	memset(m_pArrProtFun, 0, sizeof(m_pArrProtFun));
	m_pArrProtFun[DEF_REGISTER_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealRegisterRq;
	m_pArrProtFun[DEF_LOGIN_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealLoginRq;
	m_pArrProtFun[DEF_CHATMSG_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealChatRq;
	m_pArrProtFun[DEF_FRIEND_OFFLINE - DEF_BASE_PACKAGETYPE] = &kernel::dealOfflineRq;
	m_pArrProtFun[DEF_ADD_FRIEND_RQ - DEF_BASE_PACKAGETYPE] = &kernel::dealAddFriendRq;
	m_pArrProtFun[DEF_ADD_FRIEND_RS - DEF_BASE_PACKAGETYPE] = &kernel::dealAddFriendRs;
	m_pArrProtFun[DEF_HEARTBEAT - DEF_BASE_PACKAGETYPE] = &kernel::dealHeartbeatRq; //后加 绑定心跳请求处理函数
	//把处理函数的地址保存到数组中
}

bool kernel::startServer()
{//打开网络
	m_pMediator = new TcpServerMediator;
	if (!m_pMediator->openNet()) {
		cout << "打开网络失败" << endl;
		return false;
	}
	//连接数据库
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
{//关闭网络
	if (m_pMediator) {
		m_pMediator->closeNet();
		delete m_pMediator;
		m_pMediator = nullptr;
		//断开数据库
		m_sql.DisConnect();
}
}
//接受所有数据
void kernel::dealData(char* data, int len, long from)
{
	cout << __func__ << endl;
	//   取出协议类型
	packageType type = *(packageType*)data;
	//计算数组下标
	int index = type - DEF_BASE_PACKAGETYPE;
	if (index >= 0 && index < _PROTOCOL_COUNT) {//初步判断是否合法
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
//处理注册请求
void kernel::dealRegisterRq(char* data, int len, long from)
{
	cout << __func__ << endl;

	
	PROT_REGISTER_INFO_RQ* rq = (PROT_REGISTER_INFO_RQ*)data;
	//1.从数据库根据昵称查询
	//cout <<"rq:" << rq->name<<" "<<rq->packType<<" "<<rq->pass<<" "<<rq->tel << endl;
	list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql,"select name from t_user where name='%s';",rq->name);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql,1,lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
	cout <<"sql:"<< sql << endl;
	//2.判断查询结果是否为空
	PROT_REGISTER_INFO_RS rs;
	if (0 ==lstStr.size() )
	{//查询结果为空说明昵称未被注册
		//3.从数据中根据电话号查询
		sprintf_s(sql, "select tel from t_user where tel='%s';", rq->tel);
		{lock_guard<mutex> lock(m_dbMutex); // 后加的
		if (!m_sql.SelectMySql(sql, 1, lstStr)) {
			cout << "查询数据库失败：" << sql << endl;
			return;
		}
		} // 后加的
		//4.判断查询结果是否为空
		if (0 == lstStr.size())
		{//查询结果为空说明电话哈号未被注册
			//5.把用户信息写入t_uer表中
			sprintf_s(sql, "insert into t_user(name,tel,pass,iconid,feeling)values('%s','%s','%s',5,'学习类');", rq->name,rq->tel,rq->pass);
			{lock_guard<mutex> lock(m_dbMutex); // 后加的
			if (!m_sql.UpdateMySql(sql)) {
				cout << "插入数据库失败：" << sql << endl;
				return;
			}
			} // 后加的
			//6.注册成功就是给rs赋值
			rs.result = DEF_REGISTER_SUC;
		}
		else {//查询结果不为空，说明电话号已被注册
			//7.注册失败DEF_REGISTER_TEL_EXISTS
			rs.result = DEF_REGISTER_TEL_EXISTS;
		}

	}
	else {
		//查询结果不为空说明昵称已被注册
		//8查询失败
		rs.result = DEF_REGISTER_NAME_EXISTS;

	}
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);
}

void kernel::dealLoginRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_LOGIN_RQ* rq = (PROT_LOGIN_RQ*)data;
// 1、从数据库根据电话号查询密码
    list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql, "select pass,id from t_user where tel='%s';", rq->tel);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql, 2, lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
// 2、判断查询结果是否为空
	PROT_LOGIN_RS rs;
	if (0 ==lstStr.size() ) {
		// 查询结果为空，说明手机号未注册
		// 3、登录失败DEF_LOGIN_NOTEXIST
		rs.result = DEF_LOGIN_NOTEXIST;
	}
	else {
		// 4、从查询结果中取出密码
		string pass = lstStr.front();
		lstStr.pop_front();
		//取出用户id
		int userId= stoi(lstStr.front());
		lstStr.pop_front();
		// 5、判断密码是否正确，跟用户当前输入的密码
		if (pass==rq->pass ) {
			// 6、密码相等，登录成功DEF_LOGIN_SUC
			rs.result = DEF_LOGIN_SUC;
			rs.userId = userId;
			//保存用户id和socket
			m_mutex.lock(); // 后加的
			m_mapIdToSocket[userId]=from;
			m_mapIdToLastActive[userId] = time(nullptr); //后加 记录用户最后活跃时间
			m_mutex.unlock(); // 后加的
			// 8、把登录结果返回给客户端
			m_pMediator->sendData((char*)&rs, sizeof(rs), from);
			//给客户端发送登录用户的信息和她好友的信息
			sendUserInfoAndFriendInfo(userId);
			return;
		}
		else {
			// 7、密码不相等，登录失败DEF_LOGIN_PASS_ERR
			rs.result = DEF_LOGIN_PASS_ERR;
			
		}
	}
	// 8、把登录结果返回给客户端
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);

}

void kernel::sendUserInfoAndFriendInfo(int userId)
{
	cout << __func__ << endl;
	//根据自己的id查询自己的信息
	PROT_FRIEND_INFO userInfo;
	getInfoById(userId, &userInfo);
	//把自己的信息发给客户端
	m_mutex.lock(); // 后加的
	bool userOnline = m_mapIdToSocket.count(userId) > 0; // 后加的
	SOCKET userSocket = m_mapIdToSocket[userId]; // 后加的
	m_mutex.unlock(); // 后加的
	if (userOnline) { // 后加的
	//if (m_mapIdToSocket.count(userId) > 0) {
		m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), userSocket); // 后加的
		//m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[userId]);
	}
	//根据自己的id查询好友id列表
	list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql, "select idB from t_friend where idA='%d';", userId);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql, 1, lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
	//遍历好友id列表
	int friendId = 0;
	PROT_FRIEND_INFO friendInfo;
	while (lstStr.size() > 0) {
    //取出好友id
		friendId = stoi(lstStr.front());
		lstStr.pop_front();
	//根据好友id查询好友的信息
		getInfoById(friendId, &friendInfo);
//把好友信息发回客户端
		m_mutex.lock(); // 后加的
		bool userOnline2 = m_mapIdToSocket.count(userId) > 0; // 后加的
		SOCKET userSocket2 = m_mapIdToSocket[userId]; // 后加的
		m_mutex.unlock(); // 后加的
		if (userOnline2) { // 后加的
		//if (m_mapIdToSocket.count(userId) > 0) {
			m_pMediator->sendData((char*)&friendInfo, sizeof(friendInfo), userSocket2); // 后加的
			//m_pMediator->sendData((char*)&friendInfo, sizeof(friendInfo), m_mapIdToSocket[userId]);
		}
	}
	//判断好友是否在线把自己的信息发给好友
	m_mutex.lock(); // 后加的
	bool friendOnline2 = m_mapIdToSocket.count(friendId) > 0; // 后加的
	SOCKET friendSocket2 = m_mapIdToSocket[friendId]; // 后加的
	m_mutex.unlock(); // 后加的
	if (friendOnline2) // 后加的
	//if (m_mapIdToSocket.count(friendId) > 0)
	{
		m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), friendSocket2); // 后加的
		//m_pMediator->sendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[friendId]);
	}
		
		

	
}
//根据id查询用户信息
void kernel::getInfoById(int id, PROT_FRIEND_INFO* info)
{
	cout << __func__ <<"id:"<<id<< endl;
	info->userId = id;
	//判断用户是否在线（判断map中是否有这个用户的id）
	m_mutex.lock(); // 后加的
	bool online = m_mapIdToSocket.count(id) > 0; // 后加的
	m_mutex.unlock(); // 后加的
	if (online) { // 后加的
	//if (m_mapIdToSocket.count(id) > 0) {
		//在线
		info->status = DEF_STATUS_ONLINE;
	}
	else {
		//不在线
		info->status = DEF_STATUS_OFFLINE;
	}
	//从数据库查询用户的昵称，头像id和签名
	list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql, "select name,iconid,feeling from t_user where id='%d';", id);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql, 3, lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
	//判断查询结果的个数是否是3
	if (3 == lstStr.size()) {
		//取出name
		strcpy_s(info->name, sizeof(info->name), lstStr.front().c_str());
		lstStr.pop_front();
		//取出iconid
		info->iconId = stoi(lstStr.front());
		lstStr.pop_front();
		//取出签名
		strcpy_s(info->feeling, sizeof(info->feeling), lstStr.front().c_str());
		lstStr.pop_front();
	}
	else {
		cout << "查询数据库sql:" << sql << endl;
	}

}

void kernel::dealChatRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_CHATMSG_RQ* rq = (PROT_CHATMSG_RQ*)data;
	//1.判断好友是否在线
	m_mutex.lock(); // 后加的
	bool online = m_mapIdToSocket.count(rq->friendid) > 0; // 后加的
	SOCKET friendSocket = m_mapIdToSocket[rq->friendid]; // 后加的
	m_mutex.unlock(); // 后加的
	if (online) { // 后加的
	//if (m_mapIdToSocket.count(rq->friendid) > 0) {
		//在线就把聊天请求转发给好友
		m_pMediator->sendData(data, len, friendSocket); // 后加的
		//m_pMediator->sendData(data, len, m_mapIdToSocket[rq->friendid]);
	}
	else {
		//B不在线,告诉A,B不在线
		PROT_CHATMSG_RS rs;
		rs.result = DEF_CHATMSG_FAIL;
		rs.formid = rq->friendid;
		rs.destid = rq->myid;
		m_pMediator->sendData((char*)&rs, sizeof(rs), from);
		//正常流程：A和B聊天，B当前不在线，把聊天请求的内容保存到数据库
		//创建一个表：保存Aid,Bid,聊天内容，发送时间
		//B登录成功之后遍历这个表，取出数据转发给B把发送成功的数据从表中删除
	}
}

void kernel::dealOfflineRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_FRIEND_OFFLINE* rq = (PROT_FRIEND_OFFLINE*)data;
	//根据下线用户的id查询好友id列表
	list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql, "select idB from t_friend where idA='%d';", rq->myid);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql, 1, lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
	//遍历好友id列表
	int friendId = 0;
	PROT_FRIEND_INFO friendInfo;
	while (lstStr.size() > 0) {
		//取出好友id
		friendId = stoi(lstStr.front());
		lstStr.pop_front();
		//判断好友是否在线给在线好友转发下线请求
		m_mutex.lock(); // 后加的
		bool friendOnline = m_mapIdToSocket.count(friendId) > 0; // 后加的
		SOCKET friendSocket = m_mapIdToSocket[friendId]; // 后加的
		m_mutex.unlock(); // 后加的
		if (friendOnline) { // 后加的
		//if (m_mapIdToSocket.count(friendId) > 0) {
			m_pMediator->sendData(data, len, friendSocket); // 后加的
			//m_pMediator->sendData(data, len, m_mapIdToSocket[friendId]);
				
		}

	}
	//从map中取出下线用户的socket关闭，回收空间
	m_mutex.lock(); // 后加的
	bool userExists = m_mapIdToSocket.count(rq->myid) > 0; // 后加的
	if (userExists) { // 后加的
	//if (m_mapIdToSocket.count(rq->myid)>0) {
		//取出socket关闭
		SOCKET s = m_mapIdToSocket[rq->myid];
		closesocket(s);
		//把无效节点从map移除
		m_mapIdToSocket.erase(rq->myid);
	}
	m_mutex.unlock(); // 后加的
}
//添加好友
void kernel::dealAddFriendRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_ADD_FRIEND_RQ* rq = (PROT_ADD_FRIEND_RQ*)data;
	//1.根据好友昵称查询好友id
	list<string> lstStr;//装查询结果
	char sql[1024] = "";//装sql语句
	sprintf_s(sql, "select id from t_user where name='%s';", rq->friendname);
	{lock_guard<mutex> lock(m_dbMutex); // 后加的
	if (!m_sql.SelectMySql(sql, 1, lstStr)) {
		cout << "查询数据库失败：" << sql << endl;
		return;
	}
	} // 后加的
	//2.判断查询结果是否为空
	if (0 == lstStr.size()) {
		//3.没有用户添加失败
		PROT_ADD_FRIEND_RS rs(rq->myid, 0, DEF_ADD_FRIEND_NOTEXIST);
		strcpy_s(rs.fromname, sizeof(rs.fromname), rq->friendname);
		//把添加结果返回给发起请求的客户端
		m_pMediator->sendData((char*)&rs, sizeof(rs), from);
	}
	else {
		//4.取出好友id
		int friendId = stoi(lstStr.front());
		lstStr.pop_front();
		//5.判断好友是否在线
		m_mutex.lock(); // 后加的
		bool friendOnline = m_mapIdToSocket.count(friendId) > 0; // 后加的
		SOCKET friendSocket = m_mapIdToSocket[friendId]; // 后加的
		m_mutex.unlock(); // 后加的
		if (friendOnline) { // 后加的
		//if (m_mapIdToSocket.count(friendId)> 0) {
			//6.好友在线把添加好友请求转发给好友
			m_pMediator->sendData(data, len, friendSocket); // 后加的
			//m_pMediator->sendData(data, len, m_mapIdToSocket[friendId]);
		}
		else {
			//7.好友不在线添加好友失败
			PROT_ADD_FRIEND_RS rs(rq->myid, friendId, DEF_ADD_FRIEND_OFFLINE);
			strcpy_s(rs.fromname, sizeof(rs.fromname), rq->friendname);
		//正常流程：A添加B，B当前不在线，把请求保存到数据库
		//创建一个表：保存Aid,Bid,添加请求，请求发送时间
		//B登录成功之后遍历这个表，取出数据转发给B把发送成功的数据从表中删除
			//把添加结果返回给发起请求的客户端
			m_pMediator->sendData((char*)&rs, sizeof(rs), from);
		}
	}
}

void kernel::dealAddFriendRs(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_ADD_FRIEND_RS* rs= (PROT_ADD_FRIEND_RS*)data;
	//1.判断对方是否同意
	if (DEF_ADD_FRIEND_ACCEPT==rs->result) {
		//2.如果同意把好友关系写入数据库
		
		char sql[1024] = "";//装sql语句
		sprintf_s(sql, "insert into t_friend values(%d,%d);",rs->destid,rs->fromid);
		{lock_guard<mutex> lock(m_dbMutex); // 后加的
		if (!m_sql.UpdateMySql(sql)) {
			cout << "查询数据库失败：" << sql << endl;
			return;
		}
		sprintf_s(sql, "insert into t_friend values(%d,%d);", rs->fromid, rs->destid);
		if (!m_sql.UpdateMySql(sql)) {
			cout << "查询数据库失败：" << sql << endl;
			return;
		}
		} // 后加的
		//3.更新双端好友
		sendUserInfoAndFriendInfo(rs->destid);
	}
	//4.把添加结果返回给发起端
	m_mutex.lock(); // 后加的
	SOCKET destSocket = m_mapIdToSocket[rs->destid]; // 后加的
	m_mutex.unlock(); // 后加的
	m_pMediator->sendData(data, len, destSocket); // 后加的
	//m_pMediator->sendData(data, len, m_mapIdToSocket[rs->destid]);
}

//后加 处理心跳请求
void kernel::dealHeartbeatRq(char* data, int len, long from)
{
	cout << __func__ << endl;
	PROT_HEARTBEAT* rq = (PROT_HEARTBEAT*)data;

	// 更新用户最后活跃时间
	m_mutex.lock();
	if (m_mapIdToSocket.count(rq->userId) > 0) {
		m_mapIdToLastActive[rq->userId] = time(nullptr);
		cout << "后加 更新用户活跃时间, userId: " << rq->userId << endl;
	}
	m_mutex.unlock();

	// 发送心跳响应给客户端
	PROT_HEARTBEAT rs(rq->userId);
	m_pMediator->sendData((char*)&rs, sizeof(rs), from);
}

//后加 检查超时用户并通知好友下线（可在服务端定时调用）
void kernel::checkHeartbeatTimeout()
{
	time_t now = time(nullptr);
	vector<int> timeoutUsers;

	// 找出所有超时用户
	m_mutex.lock();
	for (auto& pair : m_mapIdToLastActive) {
		if (now - pair.second > DEF_HEARTBEAT_TIMEOUT / 1000) {
			timeoutUsers.push_back(pair.first);
		}
	}
	m_mutex.unlock();

	// 处理超时用户
	for (int userId : timeoutUsers) {
		cout << "后加 用户超时下线, userId: " << userId << endl;

		// 构造下线通知
		PROT_FRIEND_OFFLINE offline(userId);

		// 查询好友列表并通知
		list<string> lstStr;
		char sql[1024] = "";
		sprintf_s(sql, "select idB from t_friend where idA='%d';", userId);
		{
			lock_guard<mutex> lock(m_dbMutex);
			if (!m_sql.SelectMySql(sql, 1, lstStr)) {
				continue;
			}
		}

		// 通知在线好友
		while (lstStr.size() > 0) {
			int friendId = stoi(lstStr.front());
			lstStr.pop_front();

			m_mutex.lock();
			if (m_mapIdToSocket.count(friendId) > 0) {
				m_pMediator->sendData((char*)&offline, sizeof(offline), m_mapIdToSocket[friendId]);
			}
			m_mutex.unlock();
		}

		// 清理该用户的数据
		m_mutex.lock();
		m_mapIdToSocket.erase(userId);
		m_mapIdToLastActive.erase(userId);
		m_mutex.unlock();
	}
}



