#ifndef LOGINDIA_H
#define LOGINDIA_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDia; }
QT_END_NAMESPACE

class LoginDia : public QDialog
{
    Q_OBJECT

public:
    LoginDia(QWidget *parent = nullptr);
    ~LoginDia();
//重写父类的关闭事件
    void closeEvent(QCloseEvent* event);
private slots:
    void on_pb_register_clicked();

    void on_pushButton_2_clicked();

    void on_pb_login_clicked();

    void on_pb_clear_clicked();

signals:
    void signals_sendLoginInfo(QString tel,QString pass);
    void signals_sendRegisterInfo(QString name,QString tel,QString pass);
    void signals_closeProcess();

private:
    Ui::LoginDia *ui;
};
#endif // LOGINDIA_H
