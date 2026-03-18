#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include"frienditem.h"
#include<QMap>
#include<QMenu>
using namespace std;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
private:
    Ui::MainWidget *ui;

    int m_myId;//唯一识别用户的ID

    QMap<int,frienditem*> m_mapIdToFriend; //储存朋友的链表

    QMenu * m_pMenu;//菜单的指针

public:
    void setMyId(int id){ m_myId = id;}
    int getMyId(){return m_myId;}

    void setMySelfInfo(int iconid,QString name,QString feeling);

    void setFriendInfo(int userid,int iconid,QString name,QString feeling,int status);

    void setFriendChatMsg(int friendid,QString msg);
    QString getMyName();

    void setFriendOffline(int friendId);

signals:
    void signals_sendMsgToKernel(int,QString);
    void signals_addFriend(QString);

    void signals_notifyOffline();

public slots:
    void slots_dealMenuTrigger(QAction *);
private slots:
    void on_pb_menu_clicked();
    void on_pb_quit_clicked();
};

#endif // MAINWIDGET_H
