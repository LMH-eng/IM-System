#ifndef FRIENDITEM_H
#define FRIENDITEM_H

#include <QWidget>
#include"chatdia.h"

namespace Ui {
class frienditem;
}

class frienditem : public QWidget
{
    Q_OBJECT

public:
    explicit frienditem(QWidget *parent = nullptr);
    ~frienditem();

private:
    Ui::frienditem *ui;
    int friendId;//储存朋友的ID
    int m_iconId;
    int m_status;//记录朋友的状态,以后可能会用上

    ChatDia* m_pChatDia;

public:
    void setInfo(int userid, int iconid, QString name, QString feeling, int status);
    int getFriendid(){return friendId;}
    void setChatMsg(QString msg);
    QString getFriendName();
    int getFriendId(){return friendId;}

    void setFriendOffline();
private slots:
    void on_pb_friimg_clicked();
    void slots_recvMsgAndSend(QString);

signals:
    void signals_sendIdMsg(int,QString);
};

#endif // FRIENDITEM_H
