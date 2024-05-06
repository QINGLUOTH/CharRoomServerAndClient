#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QFileDialog>
#include <QPainter>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <string>
#include <QString>
#include <QMessageBox>
#include "message.h"
#include <map>
#include <string>
#include <vector>
#include "chatroomwindow.h"
#include "utils.h"

bool LoginWindow::loginSendMessage(std::string name, std::string password, Message* m){

    return true;
}

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    connect(ui->actionclose, &QAction::triggered, [this](){
        this->ws->mainWindow->close();
    });

    connect(ui->actionSetBackgroundPicture, &QAction::triggered, [this](){
        QString name = QFileDialog::getOpenFileName(this, "打开文件", "C:\\", "(*.png *.jpg *.jpeg)");
        this->ws->img = name;
        QFile qf("J:\\img.txt");
        qf.open(QIODeviceBase::WriteOnly);
        qf.write(name.toStdString().c_str());
        qf.close();
        // 设置背景图片
        this->update();
    });

    connect(ui->login, &QPushButton::clicked, [this](){
        std::string acountName = this->ui->name->text().toStdString();
        std::string acountPassword = this->ui->password->text().toStdString();

        if(acountName == "" || acountPassword == ""){
            qDebug().noquote() << "登录失败";
            QMessageBox::critical(this, "critical", "登录失败");
            return;
        }
        // 发送信息
        Message* m = new Message(this, "127.0.0.1", 8880);
        m->sendMessageToServer("login[name:" + acountName + "," + "password:" + acountPassword + "]");
        connect(m->socket, &QTcpSocket::readyRead, [m, this, acountName](){
            using namespace std;
            map<string, string>* m1 = util::analysisString(m->acceptMessage());
            bool is = ((*(m1))["isLogin"] == "true");
            if (is){
                delete ws->id;
                ws->id = new QString(acountName.c_str());
                qDebug() << "ws-id = " << ws->id;
                qDebug().noquote() << "登录成功";
                QMessageBox::about(this, "about", "登录成功");
                this->ws->id = new QString(acountName.c_str());
                this->ws->stackedWidget->setCurrentIndex(this->ws->chatRoomWindowIndex);
                this->ws->name = new string((*(m1))["username"]);
                qDebug().noquote() << (*(m1))["username"];
                ((ChatRoomWindow*) this->ws->chatRoomWindow)->updateThisWindowContact();
            }else {
                qDebug().noquote() << "登录失败";
                QMessageBox::critical(this, "critical", "登录失败");
            }
        });
    });

    connect(ui->create, &QPushButton::clicked, [this](){
        // 注册按钮, 跳转界面
        this->ws->stackedWidget->setCurrentIndex(this->ws->createWindowIndex);
    });
}

void LoginWindow::init(){
    QFile qf("J:\\img.txt");
    qf.open(QIODeviceBase::ReadWrite);
    QString name = qf.readAll();
    this->ws->img = name;
    qf.close();
    this->update();
}

void LoginWindow::paintEvent(QPaintEvent* qpe){
    QPixmap qpm;
    qpm.load(this->ws->img);

    // 设置背景图片
    QPainter qp(this);
    //    qpm.scaled(this->width() * 0.5, this->height() * 0.5);  // 缩放
    qp.drawPixmap(0, 0, this->width(), this->height(), qpm);
}


LoginWindow::~LoginWindow()
{
    delete ui;
}
