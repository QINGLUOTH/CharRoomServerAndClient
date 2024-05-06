#ifndef CHATROOMWINDOW_H
#define CHATROOMWINDOW_H

#include <QGridLayout>
#include <QMainWindow>
#include "chatRoomWindows.h"
#include <QPainter>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QMenuBar>
#include <QFileDialog>
#include <QScrollArea>
#include <QPushButton>
#include <QtWidgets/QScrollArea>
#include <vector>
#include <map>
#include <QTextEdit>
#include "utils.h"

#include "message.h"
#include "QLabel"

namespace Ui {
class ChatRoomWindow;
}

class ChatRoomWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatRoomWindow(QWidget *parent = nullptr);
    ~ChatRoomWindow();
    Windows* ws;
    QLabel* qte;
    QScrollArea* scrollArea;
    QGridLayout* gridLayout;
    QPushButton* groupPushButton;
    QPushButton* contactPushButton;
    QPushButton* addPushButton;
    QPushButton* upFilePushButton;
    QPushButton* downloadFilePushButton;
    QPushButton* shareFileFilePushButton;
    QWidget* scrollAreaWidgetContents;
    pair<QPushButton*, std::string>* current;
    QPushButton* currentMessage;
    QPushButton* temp;
    QTextEdit* qte2;
    QLabel* ql;
    string contactId;
    int isUpdate = 1;  // 0表示contact, 1表示group
    // 刷新
    QPushButton* refreshPushButton;
    map<QPushButton*, util::Contact*>* contacts;
    map<QPushButton*, util::Group*>* groups;

    void updateThisWindowGroup();
    void updateThisWindowContact();
    void paintEvent(QPaintEvent* qpe);
    void init();
private:
    Ui::ChatRoomWindow *ui;
};

#endif // CHATROOMWINDOW_H
