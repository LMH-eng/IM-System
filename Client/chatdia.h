#ifndef CHATDIA_H
#define CHATDIA_H

#include <QWidget>

namespace Ui {
class ChatDia;
}

class ChatDia : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDia(QWidget *parent = nullptr);
    ~ChatDia();

public:
    void showChatMsg(QString);

private slots:
    void on_pb_send_clicked();

    void on_pb_clear_clicked();

signals:
    //给上层发送聊天内容
    void signals_sendMsg(QString);

private:
    Ui::ChatDia *ui;
};

#endif // CHATDIA_H
