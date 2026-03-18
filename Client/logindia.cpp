#include "logindia.h"
#include "ui_logindia.h"
#include<QMessageBox>
#include<QDebug>

LoginDia::LoginDia(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDia)
{
    ui->setupUi(this);
}

LoginDia::~LoginDia()
{
    qDebug()<<"~LoginDia()";
    delete ui;
}
//重写父类的关闭事件
void LoginDia::closeEvent(QCloseEvent* event){
    //通知kernel
}
//注册按钮
void LoginDia::on_pb_register_clicked()
{
    //先获取注册信息
    QString name = ui->le_name->text().trimmed();
    QString tel = ui->le_register_tel->text().trimmed();
    QString pass = ui->le_register_pass->text().trimmed();
    QString passAgain = ui->le_pass_again->text().trimmed();

    //检验 是否为空
    if(name.isEmpty()||tel.isEmpty()||pass.isEmpty()||passAgain.isEmpty()){
        QMessageBox::warning(this,"警告","注册信息不能为空");
        return;
    }

    //tel 11位数字
    if(tel.size()!=11){
        QMessageBox::warning(this,"警告","电话号码太长或太短");
        return;
    }

    for(int i=0;i<11;++i){
        if(tel[i]<'0'||tel[i]>'9'){
            QMessageBox::warning(this,"警告","电话号码只能为数字");
            return;
        }
    }

    //判断两次输入密码是否一致
    if(pass!=passAgain){
        QMessageBox::warning(this,"警告","两次输入密码不一致，请重新输入");
        return;
    }
    //字符串校验（客户端和服务端都要校验，规则要一致）
    //1.是否为空或者是全空格
    //2.校验长度是否合法
    //3.校验内容是否符合要求（电话号码全为数字，）
    //xss攻击
    //发送注册信息
    //qDebug()<<name<<tel<<pass;
    emit signals_sendRegisterInfo(name,tel,pass);
}


void LoginDia::on_pushButton_2_clicked()
{
    ui->le_name->clear();
    ui->le_register_tel->clear();
    ui->le_register_pass->clear();
    ui->le_pass_again->clear();
}


void LoginDia::on_pb_login_clicked()
{
    QString tel = ui->le_tel->text().trimmed();
    QString pass = ui->le_pass->text().trimmed();

    //检验 是否为空
    if(tel.isEmpty()||pass.isEmpty()){
        QMessageBox::warning(this,"警告","注册信息不能为空");
        return;
    }

    //tel 11位数字
    if(tel.size()!=11){
        QMessageBox::warning(this,"警告","电话号码太长或太短");
        return;
    }

    for(int i=0;i<11;++i){
        if(tel[i]<'0'||tel[i]>'9'){
            QMessageBox::warning(this,"警告","电话号码只能为数字");
            return;
        }
    }

    //发送登录信息
    emit signals_sendLoginInfo(tel,pass);
}


void LoginDia::on_pb_clear_clicked()
{
    ui->le_tel->clear();
    ui->le_pass->clear();
}

