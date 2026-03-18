#pragma once
#ifndef __DEF_H__
#define __DEF_H__
//udp端口号
#define _UDP_PORT (2345)
#define _TCP_PORT (4321)
#define _LISTEN_MAX_LENGTH 100
#define _PROTOCOL_COUNT  (20)
#define DEF_BASE_PACKAGETYPE  1000
//注册
#define DEF_REGISTER_RQ  (DEF_BASE_PACKAGETYPE+0)
#define DEF_REGISTER_RS  (DEF_BASE_PACKAGETYPE+1)

//登录
#define DEF_LOGIN_RQ  (DEF_BASE_PACKAGETYPE+2)
#define DEF_LOGIN_RS  (DEF_BASE_PACKAGETYPE+3)

//朋友的信息
#define DEF_FRIEND_INFO  (DEF_BASE_PACKAGETYPE+4)

//聊天请求和请求结果的回复
#define DEF_CHATMSG_RQ  (DEF_BASE_PACKAGETYPE+5)
#define DEF_CHATMSG_RS  (DEF_BASE_PACKAGETYPE+6)

//添加好友的请求和回复
#define DEF_ADD_FRIEND_RQ  (DEF_BASE_PACKAGETYPE+7)
#define DEF_ADD_FRIEND_RS  (DEF_BASE_PACKAGETYPE+8)

//朋友下线
#define DEF_FRIEND_OFFLINE  (DEF_BASE_PACKAGETYPE+9)

#define DEF_HEARTBEAT  (DEF_BASE_PACKAGETYPE+10) //后加
#define DEF_HEARTBEAT_INTERVAL  30000  //后加 心跳间隔30秒
#define DEF_HEARTBEAT_TIMEOUT   90000  //后加 超时时间90秒

#define DEF_NAME_LEN  30
#define DEF_TEL_LEN   20
#define DEF_PASS_LEN  20
#define DEF_FEELING_LEN  100

//注册结果
#define DEF_REGISTER_SUC  0
#define DEF_REGISTER_NAME_EXISTS 1
#define DEF_REGISTER_TEL_EXISTS 2

//登陆返回的结果
#define DEF_LOGIN_SUC      0
#define DEF_LOGIN_NOTEXIST 1
#define DEF_LOGIN_PASS_ERR 2

//用户的状态，在线，离线
#define DEF_STATUS_ONLINE    0
#define DEF_STATUS_OFFLINE   1

#define DEF_CHATMSG_LEN    (1024*8)

//聊天请求回复的结果
#define DEF_CHATMSG_SUC     0
#define DEF_CHATMSG_FAIL    1

//添加好友请求回复的结果
#define DEF_ADD_FRIEND_ACCEPT       0
#define DEF_ADD_FRIEND_REJECT       1
#define DEF_ADD_FRIEND_OFFLINE      2
#define DEF_ADD_FRIEND_NOTEXIST     3

//---------------------------------------------

using packageType = unsigned int;


//注册 请求
struct PROT_REGISTER_INFO_RQ {
    packageType packType;//协议包类型
    char name[DEF_NAME_LEN];
    char tel[DEF_TEL_LEN];
    char pass[DEF_PASS_LEN];

    PROT_REGISTER_INFO_RQ() :packType(DEF_REGISTER_RQ)
    {}
};

struct PROT_REGISTER_INFO_RS {
    packageType packType;
    int result;//注册结果

    PROT_REGISTER_INFO_RS(int _result) :packType(DEF_REGISTER_RS), result(_result)
    {}
};

//登录
struct PROT_LOGIN_RQ {
    packageType packType;
    char tel[DEF_TEL_LEN];
    char pass[DEF_PASS_LEN];

    PROT_LOGIN_RQ() :packType(DEF_LOGIN_RQ) {

    }
};

//登陆回复
struct PROT_LOGIN_RS {
    packageType packType;
    int result;//登陆的结果
    int userId;//唯一标识的某个用户

    PROT_LOGIN_RS(int _result, int _userId)
        :packType(DEF_LOGIN_RS), result(_result), userId(_userId)
    {}
};

//朋友的信息（也包含自己的信息）
struct PROT_FRIEND_INFO {
    packageType packType;
    int userId;
    int iconId;
    int status;
    char name[DEF_NAME_LEN];
    char feeling[DEF_FEELING_LEN];

    PROT_FRIEND_INFO(int _userId, int _iconId, int _status)
        :packType(DEF_FRIEND_INFO), userId(_userId), iconId(_iconId), status(_status)
    {}
};

//聊天请求
struct PROT_CHATMSG_RQ {
    packageType packType;
    int myid;
    int friendid;
    char chatmsg[DEF_CHATMSG_LEN];
    PROT_CHATMSG_RQ(int _myid, int _friendid) :packType(DEF_CHATMSG_RQ), myid(_myid), friendid(_friendid)
    {}
};

//发送聊天请求的回复结果
struct PROT_CHATMSG_RS {
    packageType packType;
    int formid;
    int destid;
    int result;//发送本次聊天的结果
    PROT_CHATMSG_RS(int _formid, int _destid, int _result) :
        packType(DEF_CHATMSG_RS), formid(_formid), destid(_destid), result(_result) {}

};

//添加好友的请求
struct PROT_ADD_FRIEND_RQ {
    packageType packType;
    int myid;
    char myname[DEF_NAME_LEN];
    char friendname[DEF_NAME_LEN];

    PROT_ADD_FRIEND_RQ(int _myid) :packType(DEF_ADD_FRIEND_RQ), myid(_myid)
    {}
};

//对于添加好友请求的回复
struct PROT_ADD_FRIEND_RS {
    packageType packType;
    int fromid;
    char fromname[DEF_NAME_LEN];
    int destid;
    char destname[DEF_NAME_LEN];
    int result;
    PROT_ADD_FRIEND_RS(int _fromid, int _destid, int _result)
        :packType(DEF_ADD_FRIEND_RS), fromid(_fromid), destid(_destid), result(_result)
    {}
};

//好友下线，发送的请求
struct PROT_FRIEND_OFFLINE {
    packageType packType;
    int myid;

    PROT_FRIEND_OFFLINE(int _myid) :packType(DEF_FRIEND_OFFLINE), myid(_myid)
    {}
};

//后加 心跳包结构体
struct PROT_HEARTBEAT {
    packageType packType;
    int userId;

    PROT_HEARTBEAT() : packType(DEF_HEARTBEAT), userId(0) {}
    PROT_HEARTBEAT(int _userId) : packType(DEF_HEARTBEAT), userId(_userId) {}
};

#endif
