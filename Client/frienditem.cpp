#include "frienditem.h"
#include "ui_frienditem.h"
#include"net/def.h"
#include"QBitmap"
#include<QDebug>
#include<QTime>

frienditem::frienditem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frienditem),m_pChatDia(nullptr)
{
    ui->setupUi(this);
}

frienditem::~frienditem()
{
    qDebug()<<"~frienditem()";

    if(m_pChatDia){
        m_pChatDia->close();
        delete m_pChatDia;
        m_pChatDia = nullptr;
    }
    delete ui;
}

void frienditem::setInfo(int userid, int iconid, QString name, QString feeling, int status){
    friendId =userid;
    m_iconId =iconid;
    m_status =status;
    if(status == DEF_STATUS_ONLINE){//在线，显示彩色图
        ui->pb_friimg->setIcon(QIcon(QString(":/tx/%1.png").arg(iconid)));
    }else if(status == DEF_STATUS_OFFLINE){//离线，显示黑白图
       ui->pb_friimg->setIcon(QBitmap(QString(":/tx/%1.png").arg(iconid)));

    }

    ui->l_name->setText(name);
    ui->l_feeling->setText(feeling);

}

void frienditem::on_pb_friimg_clicked()
{
    //当点击了头像，应当弹出聊天框
    if(!m_pChatDia){//如果第一次点击头像，指针为空则创建
        m_pChatDia = new ChatDia;

        //设置窗口标题

        m_pChatDia->setWindowTitle(QString("与 %1 的聊天").arg(ui->l_name->text()));

        {//在m_pChatDia不为空(创建了对象后)，才能绑定
            connect(m_pChatDia,SIGNAL(signals_sendMsg(QString)),this,SLOT(slots_recvMsgAndSend(QString)));

        }
    }

    m_pChatDia->show();
}

void frienditem::slots_recvMsgAndSend(QString msg){
    qDebug()<<"slots_recvMsgAndSend"<<msg;

    //给上层传递内容，（朋友的id，聊天内容）
    emit signals_sendIdMsg(friendId,msg);
}

void frienditem::setChatMsg(QString msg){
    if(!m_pChatDia){//如果第一次点击头像，指针为空则创建
        m_pChatDia = new ChatDia;

        //设置窗口标题

        m_pChatDia->setWindowTitle(QString("与 %1 的聊天").arg(ui->l_name->text()));

        {//在m_pChatDia不为空(创建了对象后)，才能绑定
            connect(m_pChatDia,SIGNAL(signals_sendMsg(QString)),this,SLOT(slots_recvMsgAndSend(QString)));

        }
    }

    QString friName = ui->l_name->text();

    m_pChatDia->showChatMsg(QString("<font color = 'gray'>%1[%2]:%3</font>")
                            .arg(friName).arg(QTime::currentTime().toString("hh:mm:ss")).arg(msg));
}
QString frienditem::getFriendName(){
    return ui->l_name->text();
}

void frienditem::setFriendOffline(){

    ui->pb_friimg->setIcon(QBitmap(QString(":/tx/%1.png").arg(m_iconId)));
    m_status =DEF_STATUS_OFFLINE;

}
