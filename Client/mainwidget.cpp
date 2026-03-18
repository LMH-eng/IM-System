#include "mainwidget.h"
#include "ui_mainwidget.h"

#include<QDebug>
#include<QInputDialog>
#include<QMessageBox>

#define ADD_FRI  "添加好友"
#define SYS_SET  "系统设置"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    m_pMenu = new QMenu;

    //为菜单添加行为
    m_pMenu->addAction(ADD_FRI);
    m_pMenu->addAction(SYS_SET);

    //绑定菜单的行为触发
    connect(m_pMenu,SIGNAL(triggered(QAction *)),this,SLOT(slots_dealMenuTrigger(QAction *)));


    //去掉最大化，最小化，关闭等
    this->setWindowFlags( this->windowFlags()
                          &~ Qt::WindowMinimizeButtonHint &~ Qt::WindowMaximizeButtonHint  &~Qt::WindowCloseButtonHint
    );

}

MainWidget::~MainWidget()
{
    qDebug()<<"~MainWidget()";

    //回收菜单
    if(m_pMenu){
        delete  m_pMenu;
        m_pMenu = nullptr;
    }

    //遍历回收朋友

    for(frienditem*& pitem : m_mapIdToFriend){
        pitem->close();
        delete pitem;
        pitem = nullptr;
    }

    delete ui;
}

void MainWidget::setMySelfInfo(int iconid,QString name,QString feeling){
    ui->l_name->setText(name);
    ui->le_feeling->setText(feeling);

    ui->pb_img->setIcon(QIcon(QString(":/tx/%1.png").arg(iconid)));
}


void MainWidget::setFriendInfo(int userid, int iconid, QString name, QString feeling, int status){


    if(m_mapIdToFriend.count(userid)>0){
        //列表里如果有好友就更新状态
        frienditem*pFriItem=m_mapIdToFriend[userid];
        //设置朋友信息
        pFriItem->setInfo(userid,iconid,name,feeling,status);
    }
    else{//列表没有这个好友
    //创建FriendItem
    frienditem* pFriItem = new frienditem;

    //设置朋友信息
    pFriItem->setInfo(userid,iconid,name,feeling,status);

    QListWidgetItem* pListItem = new QListWidgetItem;

    ui->lw_friendList->addItem(pListItem);

    ui->lw_friendList->setItemWidget(pListItem,pFriItem);

    //将高度保持一致
    QRect rect = pFriItem->geometry();
    pListItem->setSizeHint(QSize(rect.width(),rect.height()));

    //添加到朋友链表中，方便后续查找使用
    m_mapIdToFriend[userid]=pFriItem;

    //要保证，Friend Item创建出来以后，才能绑定
        connect(pFriItem,SIGNAL(signals_sendIdMsg(int,QString)),this,SIGNAL(signals_sendMsgToKernel(int,QString)));

    }
}

void MainWidget::setFriendChatMsg(int friendid,QString msg){
    //需要找到指定的朋友，并在聊天框设置回复的内容

        if(m_mapIdToFriend.count(friendid)>0){//匹配到了
            frienditem* pFriItem=m_mapIdToFriend[friendid];
            pFriItem->setChatMsg(msg);

        }


}

//当点击菜单的某个行为时触发
void MainWidget::slots_dealMenuTrigger(QAction * paction){
    if(paction->text() == ADD_FRI){
        qDebug()<<"点击了添加好友";

        //弹出弹出框，输入好友昵称

        QString name = QInputDialog::getText(this,"添加好友","请输入好友昵称").trimmed();
        if(name.isEmpty()){
            QMessageBox::information(this,"提示","好友昵称不能为空");
            return;
        }

        //是否是自己
        if(name == ui->l_name->text()){
            QMessageBox::information(this,"提示","不能添加自己");
            return;
        }

        //遍历好友链表，判断是否已经是好友了
        for(frienditem* pItem : m_mapIdToFriend){
            if(pItem->getFriendName()== name){
                QMessageBox::information(this,"提示","已经是你好友了");
                return;
            }
        }


        emit signals_addFriend(name);


    }else if(paction->text() == SYS_SET){
        qDebug()<<"点击了系统设置";
    }

}

//当点击了菜单按钮时弹出菜单
void MainWidget::on_pb_menu_clicked()
{
    //获取鼠标点，基于屏幕位置
    QPoint po = QCursor::pos();

    //获取菜单的大小
    QSize size = m_pMenu->sizeHint();

    //重新设置y坐标的值
    po.setY(po.y()-size.height());

    m_pMenu->exec(po);

}

QString MainWidget::getMyName(){

    return ui->l_name->text();
}

void MainWidget::setFriendOffline(int friendId){

    if(m_mapIdToFriend.count(friendId)>0){
        //列表里如果有好友就更新状态
        frienditem*pFriItem=m_mapIdToFriend[friendId];
        //设置朋友信息
        pFriItem->setFriendOffline();
    }
}

//关闭窗口，程序下线
void MainWidget::on_pb_quit_clicked()
{
    qDebug() << "on_pb_quit_clicked 触发, myId:" << m_myId;//心跳机制 -- 调试输出
    //通知kernel 下线了
    emit signals_notifyOffline();
}

