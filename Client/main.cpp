#include "logindia.h"

#include <QApplication>
#include"kernel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //LoginDia w;
    //w.show();

    Kernel kernel;

    return a.exec();
}
//QT使用UTF-8编码方式，vs使用的是GB2312编码方式
//转码都是在客户端，发给vs之前转码成GB2312,qt收到的转码成UTF8
//有三种类型存字符串：char*,std::string,QString
//UTF-8编码方式的字符串用QString保存，GB2312编码方式的字符串用char*保存
//std::string=>.c_str();
//QString=>toStdString.c_str();
//什么样的数据需要转码
//1.数据可能包括中文；2.vs需要处理的数据
//char s[]="qwe";
//string x;
//x=s;
//QString y;
//y=s;
