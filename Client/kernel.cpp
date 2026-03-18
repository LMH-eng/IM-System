#include "kernel.h"
#include<QDebug>
#include"net/def.h"
#include<QMessageBox>
#include<QTextCodec>
#include "./mediator/TcpClientMediator.h"

#include <vector> //后加
#define SEND_BUF_TO_CLIENT(TYPE,NAME)\
    {\
    int len =sizeof(TYPE);\
    char *pbuf = new char[len];\
    memcpy(pbuf,&NAME,len);\
    emit signals_sendDataToClient(pbuf,len,0);\
    }


Kernel::Kernel(QObject *parent) : QObject(parent),m_pLoginDia(new LoginDia),m_pMediator(new TcpClientMediator),m_pMainWidget(nullptr)
{

    m_pArrProtFun[DEF_REGISTER_RS - DEF_BASE_PACKAGETYPE] = &Kernel::dealRegisterRs;
    m_pArrProtFun[DEF_LOGIN_RS - DEF_BASE_PACKAGETYPE] = &Kernel::dealLoginRs;
    m_pArrProtFun[DEF_FRIEND_INFO - DEF_BASE_PACKAGETYPE] = &Kernel::dealFriendInfo;
    m_pArrProtFun[DEF_CHATMSG_RS - DEF_BASE_PACKAGETYPE] = &Kernel::dealChatmsgRs;
    m_pArrProtFun[DEF_CHATMSG_RQ - DEF_BASE_PACKAGETYPE] = &Kernel::dealChatmsgRq;
    m_pArrProtFun[DEF_ADD_FRIEND_RS - DEF_BASE_PACKAGETYPE] = &Kernel::dealAddFriendRs;
    m_pArrProtFun[DEF_ADD_FRIEND_RQ - DEF_BASE_PACKAGETYPE] = &Kernel::dealAddFriendRq;
    m_pArrProtFun[DEF_FRIEND_OFFLINE - DEF_BASE_PACKAGETYPE] = &Kernel::dealFriendOffline;
    m_pArrProtFun[DEF_HEARTBEAT - DEF_BASE_PACKAGETYPE] = &Kernel::dealHeartbeatRs; //后加 绑定心跳响应处理函数
    //绑定退出进程的信号和曹函数
    connect(m_pLoginDia,&LoginDia::signals_closeProcess,this,&Kernel::slots_closeProcess);
    connect(this,SIGNAL(signals_sendDataToClient(char*,int,long)),this,SLOT(slots_recvServerData(char*,int,long)));
    //绑定注册的
    connect(m_pLoginDia,SIGNAL(signals_sendRegisterInfo(QString,QString,QString)),this,SLOT(slots_recvRegisterInfo(QString,QString,QString)));
    //绑定登陆的
    connect(m_pLoginDia,SIGNAL(signals_sendLoginInfo(QString,QString)),this,SLOT(slots_recvLoginInfo(QString,QString)));
    m_pAddFriTimer = new QTimer;//创建定时器
    m_pAddFriTimer->setSingleShot(true);//设定此定时器，是单次生效
    connect(m_pAddFriTimer,SIGNAL(timeout()),this,SLOT(slots_dealAddFriendTimer()));
    //m_pOfflineTimer = new QTimer;//创建定时器（已禁用）
    //m_pOfflineTimer->setSingleShot(true);//设定此定时器，是单次生效
    //connect(m_pOfflineTimer,SIGNAL(timeout()),this,SLOT(slots_dealOfflineTimer()));
    //后加 创建心跳定时器（循环触发）
    m_pHeartbeatTimer = new QTimer;
    m_pHeartbeatTimer->setSingleShot(false);
    connect(m_pHeartbeatTimer, SIGNAL(timeout()), this, SLOT(slots_sendHeartbeat()));
    m_myUserId = 0; //后加 初始化用户ID
    m_pLoginDia->show();//显示登录，注册窗口
    //打开网络
    if(!m_pMediator->openNet())
    {
        QMessageBox::about(m_pLoginDia,"提示","打开网络失败");
        exit(1);
    }
    //绑定把接收到的数据转发给kernel
    connect(m_pMediator,SIGNAL(signals_recvServerData(char* , int , long)),this,SLOT(slots_recvServerData(char* , int , long)));

}
void  Kernel::utf8ToGb2312(QString src,char*dst,int len){
    QTextCodec* dc=QTextCodec::codecForName("gb2312");
    QByteArray ba=dc->fromUnicode(src);
    strcpy_s(dst,len,ba.data());

}
QString Kernel::gb2312ToUtf8(char* src){
     QTextCodec* dc=QTextCodec::codecForName("gb2312");
     return dc->toUnicode(src);

}
//统一接收服务端返回的数据
void Kernel::slots_recvServerData(char* pbuf ,int len,long form){
    qDebug()<<"slots_recvServerData 收到了服务端返回的数据：";
   // qDebug()<<pbuf;
//   取出协议类型
  packageType type = *(packageType*)pbuf;
    if(type>= DEF_BASE_PACKAGETYPE){//初步判断是否合法
        //m_pArrProtFun[type-DEF_BASE_PACKAGETYPE];取出下标对应的函数（绑定的函数）
       (this->*m_pArrProtFun[type-DEF_BASE_PACKAGETYPE])(pbuf,len,form);
  }
   delete []pbuf;
   pbuf = nullptr;
}
void Kernel::slots_recvLoginInfo(QString tel,QString pass){
    qDebug()<<"slots_recvLoginInfo"<<tel<<","<<pass;

    PROT_LOGIN_RQ loginRq;
    strcpy_s(loginRq.tel,DEF_TEL_LEN,tel.toStdString().c_str());
    strcpy_s(loginRq.pass,DEF_PASS_LEN,pass.toStdString().c_str());

m_pMediator->sendData((char*)&loginRq,sizeof(loginRq),67);

}

void Kernel::dealLoginRs(char*pbuf,int len ,long form){
    qDebug()<<"dealLoginRs";

    PROT_LOGIN_RS* pLoginRs = (PROT_LOGIN_RS*)pbuf;
    if(pLoginRs->result == DEF_LOGIN_SUC){
        qDebug()<<"登陆成功";

        //隐藏登录框
        m_pLoginDia->hide();

        m_pMainWidget = new MainWidget;

        //先存储自己的ID
        m_pMainWidget->setMyId(pLoginRs->userId);
        m_myUserId = pLoginRs->userId; //后加 保存用户ID用于发送心跳
        //显示主聊天窗口
        m_pMainWidget->show();

        {//保证m_pMainWidget创建了以后，才能绑定
            connect(m_pMainWidget,SIGNAL(signals_sendMsgToKernel(int,QString)),this,SLOT(slots_recvMsgToServer(int,QString)));

            connect(m_pMainWidget,SIGNAL(signals_addFriend(QString)),this,SLOT(slots_addFriend(QString)));

            connect(m_pMainWidget,SIGNAL(signals_notifyOffline()),this,SLOT(slots_dealMyOffline()));
        }

        //开始启动好友下线的定时器
        //m_pOfflineTimer->start(8000);//（已禁用）

        //后加 启动心跳定时器，每30秒发送一次心跳
        m_pHeartbeatTimer->start(DEF_HEARTBEAT_INTERVAL);


    }else if(pLoginRs->result == DEF_LOGIN_NOTEXIST){
        QMessageBox::information(nullptr,"提示","用户不存在");
    }else if(pLoginRs->result == DEF_LOGIN_PASS_ERR){
        QMessageBox::information(nullptr,"提示","密码错误");
    }
}

void Kernel::dealFriendInfo(char *pbuf, int len, long form){
    qDebug()<<"dealFriendInfo";
    PROT_FRIEND_INFO* pFriendInfo = (PROT_FRIEND_INFO*)pbuf;
    QString name=gb2312ToUtf8(pFriendInfo->name);
        QString feeling=gb2312ToUtf8(pFriendInfo->feeling);
    if(pFriendInfo->userId == m_pMainWidget->getMyId()){//是自己的信息
        //设置自己的信息
        m_pMainWidget->setMySelfInfo(pFriendInfo->iconId,name,feeling);
    }else{//朋友的信息
        m_pMainWidget->setFriendInfo(pFriendInfo->userId,pFriendInfo->iconId,name,feeling,pFriendInfo->status);

    }
}


void Kernel::slots_recvRegisterInfo(QString name,QString tel,QString pass){
    qDebug()<<"slots_recvRegisterInfo"<<name<<","<<tel<<","<<pass;
    //定义注册请求的协议结构
    PROT_REGISTER_INFO_RQ registerInfoRq;
    utf8ToGb2312(name,registerInfoRq.name,sizeof(registerInfoRq.name));
    //strcpy_s(registerInfoRq.name,DEF_NAME_LEN,name.toStdString().c_str());
    strcpy_s(registerInfoRq.tel,DEF_TEL_LEN,tel.toStdString().c_str());
    strcpy_s(registerInfoRq.pass,DEF_PASS_LEN,pass.toStdString().c_str());
 qDebug()<<name<<tel<<pass;
    //registerInfoRq打包，发送给服务端，默认注册成功
m_pMediator->sendData((char*)&registerInfoRq,sizeof(registerInfoRq),21);

    //计算大小
    //int len = sizeof (PROT_REGISTER_INFO_RS);
    //申请同等大小的空间
    //char *pbuf =new char[len];
    //拷贝到缓冲区中
   // memcpy(pbuf,&registerInfoRs,len);

   // emit signals_sendDataToClient(pbuf,len,0);

    //SEND_BUF_TO_CLIENT(PROT_REGISTER_INFO_RS,registerInfoRs)
}


void Kernel::dealRegisterRs(char* pbuf, int len, long form){
    qDebug()<<"dealRegisterRs";
    PROT_REGISTER_INFO_RS* pRegisterInfoRs = (PROT_REGISTER_INFO_RS*)pbuf;
    if(pRegisterInfoRs->result == DEF_REGISTER_SUC){
            QMessageBox::information(nullptr,"提示","注册成功");
    }else if(pRegisterInfoRs->result == DEF_REGISTER_NAME_EXISTS){
            QMessageBox::information(nullptr,"提示","注册失败，昵称已被注册");
    }else if(pRegisterInfoRs->result == DEF_REGISTER_TEL_EXISTS){
        QMessageBox::information(nullptr,"提示","注册失败，电话号码已被注册");
}


}

//接收和某个朋友聊天的内容，并发送给服务端
void Kernel::slots_recvMsgToServer(int friendId,QString msg){
   qDebug()<<"id"<<friendId<<",msg"<<msg;


   PROT_CHATMSG_RQ chatmsgRq(m_pMainWidget->getMyId(),friendId);
   strcpy_s(chatmsgRq.chatmsg,DEF_CHATMSG_LEN,msg.toStdString().c_str());

   //发送给服务器
   m_pMediator->sendData((char*)&chatmsgRq,sizeof(chatmsgRq),21);

}


void Kernel::dealChatmsgRs(char*pbuf,int len ,long form){
    qDebug()<<"dealChatmsgRs";

    PROT_CHATMSG_RS* pChatMsgRs =(PROT_CHATMSG_RS*)pbuf;
    if(pChatMsgRs->result == DEF_CHATMSG_SUC){
        m_pMainWidget->setFriendChatMsg(pChatMsgRs->formid,"已送达");
    }else if(pChatMsgRs->result == DEF_CHATMSG_FAIL){
        m_pMainWidget->setFriendChatMsg(pChatMsgRs->formid,"已下线");
    }
}

void Kernel::dealChatmsgRq(char*pbuf,int len ,long form){
    qDebug()<<"dealChatmsgRq";

    PROT_CHATMSG_RQ * pChatmsgRq = (PROT_CHATMSG_RQ * )pbuf;

    m_pMainWidget->setFriendChatMsg(pChatmsgRq->myid,
                                    QString("<p><font font-weight:bold color='black'size='5'>%2</font></p>").arg(pChatmsgRq->chatmsg));
}

//添加好友。向客户端添加请求
void Kernel::slots_addFriend(QString friName){

    PROT_ADD_FRIEND_RQ addFriRq(m_pMainWidget->getMyId());
    strcpy_s(addFriRq.myname,DEF_NAME_LEN,m_pMainWidget->getMyName().toStdString().c_str());
    //strcpy_s(addFriRq.friendname,DEF_NAME_LEN,friName.toStdString().c_str());
    utf8ToGb2312(friName,addFriRq.friendname,sizeof(addFriRq.friendname));
   //发给服务器
    m_pMediator->sendData((char*)&addFriRq,sizeof(addFriRq),34);

}


void Kernel::dealAddFriendRs(char*pbuf,int len ,long form){
    qDebug()<<"dealAddFriendRs";
    PROT_ADD_FRIEND_RS* pAddFriRs = (PROT_ADD_FRIEND_RS*)pbuf;
    //转码好友名字
    QString friendName=gb2312ToUtf8(pAddFriRs->fromname);
    if(pAddFriRs->result == DEF_ADD_FRIEND_ACCEPT){
        QMessageBox::information(m_pMainWidget,"提示",QString("%1 接受了你的好友申请").arg(pAddFriRs->fromname));

    }else if(pAddFriRs->result == DEF_ADD_FRIEND_REJECT){
        QMessageBox::information(m_pMainWidget,"提示",QString("%1 拒绝了你的好友申请").arg(pAddFriRs->fromname));
    }else if(pAddFriRs->result == DEF_ADD_FRIEND_OFFLINE){
        QMessageBox::information(m_pMainWidget,"提示",QString("%1 用户已经下线了，请稍后重试").arg(friendName));
    }else if(pAddFriRs->result == DEF_ADD_FRIEND_NOTEXIST){
        QMessageBox::information(m_pMainWidget,"提示",QString("%1 用户不存在").arg(friendName));
    }
}


void Kernel::slots_dealAddFriendTimer(){
    qDebug()<<"slots_dealAddFriendTimer";

    //模拟ccc添加张三

    PROT_ADD_FRIEND_RQ addFriRq(122);
    strcpy_s(addFriRq.myname,DEF_NAME_LEN,"ccc");
    strcpy_s(addFriRq.friendname,DEF_NAME_LEN,"张三");

    qDebug()<<QString("服务端收到了%1 [%2] 添加 %3 的好友请求，将申请转发给%4")
              .arg(addFriRq.myid).arg(addFriRq.myname).arg(addFriRq.friendname).arg(addFriRq.friendname);


    SEND_BUF_TO_CLIENT(PROT_ADD_FRIEND_RQ,addFriRq)
}

void Kernel::dealAddFriendRq(char*pbuf,int len ,long form){
    PROT_ADD_FRIEND_RQ* pAddFriRq = (PROT_ADD_FRIEND_RQ* )pbuf;

    QMessageBox::StandardButton but = QMessageBox::information(m_pMainWidget,"提示",
                             QString("收到了来自%1的好友申请,是否同意？").arg(pAddFriRq->myname),
                             QMessageBox::Yes|QMessageBox::No);

    if( but == QMessageBox::Yes){//接受申请

        PROT_ADD_FRIEND_RS addFriRs(m_pMainWidget->getMyId(),pAddFriRq->myid,DEF_ADD_FRIEND_ACCEPT);
        strcpy_s(addFriRs.fromname,DEF_NAME_LEN,m_pMainWidget->getMyName().toStdString().c_str());
        strcpy_s(addFriRs.destname,DEF_NAME_LEN,pAddFriRq->myname);

        //发送给服务端
m_pMediator->sendData((char*)&addFriRs,sizeof(addFriRs),32);

    }else{//拒绝申请
        PROT_ADD_FRIEND_RS addFriRs(m_pMainWidget->getMyId(),pAddFriRq->myid,DEF_ADD_FRIEND_REJECT);
        strcpy_s(addFriRs.fromname,DEF_NAME_LEN,m_pMainWidget->getMyName().toStdString().c_str());
        strcpy_s(addFriRs.destname,DEF_NAME_LEN,pAddFriRq->myname);

        //发送给服务端
   m_pMediator->sendData((char*)&addFriRs,sizeof(addFriRs),32);


    }


}

//8秒后，小红下线（已禁用）
//void Kernel::slots_dealOfflineTimer(){
//    qDebug()<<"slots_dealOfflineTimer";
//
//    //发送下线的请求
//    PROT_FRIEND_OFFLINE friOff(113);
//    SEND_BUF_TO_CLIENT(PROT_FRIEND_OFFLINE,friOff)
//}

void Kernel::dealFriendOffline(char*pbuf,int len ,long form){
    qDebug()<<"dealFriendOffline";
    PROT_FRIEND_OFFLINE* pfriOff =(PROT_FRIEND_OFFLINE*)pbuf;
    m_pMainWidget->setFriendOffline(pfriOff->myid);
}

void Kernel::slots_dealMyOffline(){
    PROT_FRIEND_OFFLINE myoff(m_pMainWidget->getMyId());
    //发送给服务端
    m_pMediator->sendData((char*)&myoff,sizeof(myoff),12);
    slots_closeProcess();


}
void Kernel::slots_closeProcess(){
    qDebug()<<__func__;
    //关闭窗口，推出程序

    if(m_pLoginDia){
        m_pLoginDia->hide();//关闭窗口
        delete m_pLoginDia;
        m_pLoginDia = nullptr;
    }

    if(m_pMainWidget){
        m_pMainWidget->hide();//关闭窗口
        delete m_pMainWidget;
        m_pMainWidget = nullptr;
    }

    if(m_pAddFriTimer){
        delete m_pAddFriTimer;
        m_pAddFriTimer = nullptr;
    }

    //if(m_pOfflineTimer){//（已禁用）
    //    delete m_pOfflineTimer;
    //    m_pOfflineTimer = nullptr;
    //}
    //后加 停止并释放心跳定时器
    if(m_pHeartbeatTimer){
        m_pHeartbeatTimer->stop();
        delete m_pHeartbeatTimer;
        m_pHeartbeatTimer = nullptr;
    }
    if(m_pMediator){
        m_pMediator->closeNet();
        delete m_pMediator;
       m_pMediator = nullptr;
    }
    //退出进程
    exit(0);
}

//后加 发送心跳包
void Kernel::slots_sendHeartbeat(){
    if(m_myUserId == 0) return;

    PROT_HEARTBEAT heartbeat(m_myUserId);
    m_pMediator->sendData((char*)&heartbeat, sizeof(heartbeat), 0);
}

//后加 处理心跳响应
void Kernel::dealHeartbeatRs(char* pbuf, int len, long form){
    // 收到服务端的心跳响应，说明连接正常
}
