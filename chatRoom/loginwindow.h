#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>

#include "chatRoomWindows.h"
#include "message.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    Windows* ws;
    void paintEvent(QPaintEvent* qpe);
    void init();
    bool loginSendMessage(std::string name, std::string password, Message* m);

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
