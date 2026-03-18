#include "chatdia.h"
#include "ui_chatdia.h"

#include<QMessageBox>
#include<QTime>
#include<QDebug>

ChatDia::ChatDia(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatDia)
{
    ui->setupUi(this);
}

ChatDia::~ChatDia()
{
    qDebug()<<"~ChatDia()";
    delete ui;
}

//发送聊天内容的槽函数
void ChatDia::on_pb_send_clicked()
{
    //获取聊天内容
    QString msg =ui->te_inputmsg->toPlainText();
    if(msg.isEmpty()){
        QMessageBox::information(this,"提示","发送的信息不能为空");
        return;
    }

    //获取的是带有格式的文本
    msg = ui->te_inputmsg->toHtml();

    //
    ui->pb_chatmsg->append(QString("<font color='gray'>我[%1]:</font><p><font font-weight:bold color='black'size='5'>%2</font></p>")
                            .arg(QTime::currentTime().toString("hh:mm:ss")).arg(msg));

    //给上层发送连天内容
    emit signals_sendMsg(msg);

    //将输入的聊天内容清空
    ui->te_inputmsg->clear();
}

void ChatDia::showChatMsg(QString msg){
    ui->pb_chatmsg->append(msg);
}

//清空，输入的聊天信息
void ChatDia::on_pb_clear_clicked()
{
    ui->te_inputmsg->clear();
}

