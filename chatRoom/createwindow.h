#ifndef CREATEWINDOW_H
#define CREATEWINDOW_H

#include <QMainWindow>

#include "chatRoomWindows.h"

namespace Ui {
class CreateWindow;
}

class CreateWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CreateWindow(QWidget *parent = nullptr);
    ~CreateWindow();
    Windows* ws;
    void paintEvent(QPaintEvent* qpe);
    void init();
private:
    Ui::CreateWindow *ui;
};

#endif // CREATEWINDOW_H
