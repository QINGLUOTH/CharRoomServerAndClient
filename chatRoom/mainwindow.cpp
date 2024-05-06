#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <LoginWindow.h>
#include <QStackedWidget>

#include "createwindow.h"
#include "chatRoomWindows.h"
#include "chatroomwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(600, 500);
    this->setWindowTitle("聊天室");
    // 使用QStackedWidget实现跳转页面
    QStackedWidget* qStackedWidget = new QStackedWidget(this);
    qStackedWidget->setFixedSize(this->width(), this->height());

    // 设置存储结构体
    this->ws = new Windows;

    // 登录页面
    LoginWindow* localLoginWindow = new LoginWindow();
    CreateWindow* createWindow = new CreateWindow();
    ChatRoomWindow* chatRoomWindow = new ChatRoomWindow();

    ws->loginWindow = localLoginWindow;
    ws->mainWindow = this;
    ws->createWindow = createWindow;
    ws->chatRoomWindow = chatRoomWindow;
    ws->id = new QString("");

    createWindow->ws = ws;
    this->ws = ws;
    localLoginWindow->ws = ws;
    chatRoomWindow->ws = ws;

    localLoginWindow->setFixedSize(this->width(), this->height());
    createWindow->setFixedSize(this->width(), this->height());
    chatRoomWindow->setFixedSize(this->width(), this->height());

    // 设置页面
    ws->loginWindowIndex = qStackedWidget->addWidget(ws->loginWindow);
    ws->createWindowIndex = qStackedWidget->addWidget(ws->createWindow);
    ws->chatRoomWindowIndex = qStackedWidget->addWidget(ws->chatRoomWindow);
    ws->mainWindow->setWindowTitle("登录聊天室");
    ws->stackedWidget = qStackedWidget;

    localLoginWindow->init();
    createWindow->init();
    chatRoomWindow->init();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->ws->id;
    delete this->ws;
}

