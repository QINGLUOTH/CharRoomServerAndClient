#ifndef CHATROOMWINDOWS_H
#define CHATROOMWINDOWS_H
#include <QMainWindow>
#include <QStackedWidget>
#include <string>

struct Windows {
    QMainWindow* mainWindow;
    QMainWindow* loginWindow;
    QMainWindow* createWindow;
    QMainWindow* chatRoomWindow;
    QStackedWidget* stackedWidget;

    int loginWindowIndex;
    int createWindowIndex;
    int chatRoomWindowIndex;

    QString* id;
    std::string* name;

    // 背景图片路径
    QString img;
};
#endif // CHATROOMWINDOWS_H
