#include "createwindow.h"
#include "ui_createwindow.h"
#include <QPainter>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QMenuBar>
#include <QFileDialog>
#include "message.h"
#include <QMessageBox>
#include <map>
#include <string>
#include <vector>
#include "utils.h"

CreateWindow::CreateWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CreateWindow)
{
    ui->setupUi(this);

    QMenuBar* qmb = new QMenuBar(this);
    QMenu* localAddMenu = qmb->addMenu("开始");
    QAction* close = localAddMenu->addAction("关闭");
    QAction* actionSetBackgroundPicture = localAddMenu->addAction("设置背景");
//    qmb->setGeometry(QRect(0, 0, 800, 25));
    this->setMenuBar(qmb);

    connect(close, &QAction::triggered, [this](){
        this->ws->mainWindow->close();
    });

    connect(actionSetBackgroundPicture, &QAction::triggered, [this](){
        QString name = QFileDialog::getOpenFileName(this, "打开文件", "C:\\", "(*.png *.jpg *.jpeg)");
        this->ws->img = name;
        QFile qf("J:\\img.txt");
        qf.open(QIODeviceBase::WriteOnly);
        qf.write(name.toStdString().c_str());
        qf.close();
        // 设置背景图片
        this->update();
    });

    connect(ui->create, &QPushButton::clicked, [this](){
        std::string acountPassword = this->ui->password->text().toStdString();
        std::string name = this->ui->name->text().toStdString();

        if (acountPassword == "" || name ==""){
            qDebug().noquote() << "注册失败";
            QMessageBox::critical(this, "critical", "注册失败");
            return;
        }

        // 发送信息
        Message* m = new Message(this, "127.0.0.1", 8880);
        m->sendMessageToServer("create[password:" + acountPassword + ",name:" + name + "]");
        connect(m->socket, &QTcpSocket::readyRead, [m, this](){
            map<string, string>* m1 = util::analysisString(m->acceptMessage());
            bool is = ((*(m1))["isCreate"] == "true");
            qDebug().noquote() << (*(m1))["isCreate"];
            if (is){
                qDebug().noquote() << "注册成功";
                QMessageBox::about(this, "about", "注册成功, id" + QString((*(m1))["id"].c_str()));
            } else {
                qDebug().noquote() << "注册失败";
                QMessageBox::critical(this, "critical", "注册失败");
            }
        });
    });
}

void CreateWindow::paintEvent(QPaintEvent* qpe){
    QPixmap qpm;
    qpm.load(this->ws->img);

    // 设置背景图片
    QPainter qp(this);
    //    qpm.scaled(this->width() * 0.5, this->height() * 0.5);  // 缩放
    qp.drawPixmap(0, 0, this->width(), this->height(), qpm);
}

void CreateWindow::init(){
    QFile qf("J:\\img.txt");
    qf.open(QIODeviceBase::ReadWrite);
    QString name = qf.readAll();
    this->ws->img = name;
    qf.close();
    this->update();

    connect(ui->returnLogin, &QPushButton::clicked, [this](){
        this->ws->stackedWidget->setCurrentIndex(this->ws->loginWindowIndex);
    });
}

CreateWindow::~CreateWindow()
{
    delete ui;
}
