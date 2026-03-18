#ifndef KERNEL_H
#define KERNEL_H
#include <QObject>
#include"logindia.h"
#include"mainwidget.h"
#include"mediator/TcpClientMediator.h"
#include<QTimer> //后加

class Kernel : public QObject
{
    Q_OBJECT
public:
    explicit Kernel(QObject *parent = nullptr);

signals:

public:
    LoginDia * m_pLoginDia;//登录框
    MainWidget * m_pMainWidget;
    TcpClientMediator* m_pMediator;
    using PROFUN = void (Kernel::*)(char*,int,long);
    PROFUN m_pArrProtFun[20];//类成员函数指针数组，绑定服务端回复的协议和处理函数
    QTimer * m_pAddFriTimer;//模拟别人添加我为好友时的定时器
    //QTimer * m_pOfflineTimer;//模拟好友下线（已禁用）
    QTimer * m_pHeartbeatTimer; //后加 心跳定时器
    int m_myUserId; //后加 当前用户ID
public:
    //UTF8转码
    void utf8ToGb2312(QString src,char*dst,int len);
    //GB2312转码  空间里有数据数据必须是一个真实的字符串后面不需要有长度
    QString gb2312ToUtf8(char* src);
    //各种处理函数
    void dealRegisterRs(char*pbuf,int len ,long form);
    void dealLoginRs(char*pbuf,int len ,long form);
    void dealFriendInfo(char*pbuf,int len ,long form);
    void dealChatmsgRs(char*pbuf,int len ,long form);
    void dealChatmsgRq(char*pbuf,int len ,long form);
    void dealAddFriendRs(char*pbuf,int len ,long form);
    void dealAddFriendRq(char*pbuf,int len ,long form);
    void dealFriendOffline(char*pbuf,int len ,long form);
    void dealHeartbeatRs(char*pbuf,int len ,long form); //后加 处理心跳响应
public slots:
    void slots_recvLoginInfo(QString,QString);
    void slots_recvRegisterInfo(QString,QString,QString);
    void slots_recvMsgToServer(int,QString);
    void slots_addFriend(QString);//添加好友的槽函数
    void slots_dealAddFriendTimer();//模拟定时器，别人添加我为好友
    //void slots_dealOfflineTimer();//模拟好友下线时的定时器（已禁用）
    void slots_dealMyOffline();
    void slots_closeProcess();
    void slots_sendHeartbeat(); //后加 发送心跳包
public slots:
    void slots_recvServerData(char* pbuf,int len,long form);

signals:
    void signals_sendDataToClient(char* pbuf,int len,long form/*网路服务端用到数据*/);

};

#endif // KERNEL_H
