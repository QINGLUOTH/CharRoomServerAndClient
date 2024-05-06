#include "chatroomwindow.h"
#include "ui_chatroomwindow.h"

#include <QTextEdit>
#include <QLabel>
#include <qpushbutton.h>
#include <QGridLayout>
#include <utils.h>
#include <stdio.h>
#include <map>
#include <QTimer>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include <QComboBox>
#include <QFontComboBox>

ChatRoomWindow::ChatRoomWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatRoomWindow)
{
    ui->setupUi(this);

    this->contacts = new map<QPushButton*, util::Contact*>();
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
    this->scrollArea = NULL;
    this->groupPushButton = new QPushButton("群聊");
    this->contactPushButton = new QPushButton("联系人");
    this->refreshPushButton = new QPushButton("刷新");
    this->addPushButton = new QPushButton("添加", this);
    this->upFilePushButton = new QPushButton("上传文件", this);
    this->downloadFilePushButton = new QPushButton("下载文件", this);
    this->shareFileFilePushButton = new QPushButton("分享文件", this);
    connect(this->upFilePushButton, &QPushButton::clicked, [this](){
        // 开启模态窗口
        QDialog qd = QDialog(this);
        qd.setFixedSize(200, 130);
        // 上传文件按钮
        QPushButton qp("获取文件", &qd);
        QPushButton upFile("上传文件", &qd);
        upFile.move(100, 0);
        QString upFileName;
        connect(&qp, &QPushButton::clicked, [this, &upFileName](){
            // 文件窗口

            // 获取要上传文件的名字
            // getOpenFileName文件对话框
            // 参数
            // 1. 绑定的页面
            // 2. 对话框的名字
            // 3. 打开的初始路径
            // 4. 显示的文件的条件
            upFileName = QFileDialog::getOpenFileName(
                                            this, "打开文件", "C:\\", "()").toStdString().c_str();
        });

        connect(&upFile, &QPushButton::clicked, [this, &upFileName, &qd](){
            if (upFileName != ""){
                // 打开文件
                QFile* qf = new QFile(upFileName);
                if (!qf->exists()){
                    QMessageBox::critical(this, "错误", "文件不存在");
                    qd.close();
                    return;
                }
                // 设置为只读模式
                qf->open(QIODeviceBase::ReadOnly);
                //        int date[] = {1, 2, 3, 4, 5};  // 数组创建方式

                // 发送第一波数据注册信息
                QString upFileNametemp = upFileName.replace(',', "，");
                upFileNametemp = upFileNametemp.replace('[', "{");
                upFileNametemp = upFileNametemp.replace(']', "}");
                upFileNametemp = upFileNametemp.replace(':', "：");
                Message* m2 = new Message(this, "127.0.0.1", 8880);
                string message = QString("upFileData[fileName:").toStdString() +
                                 upFileNametemp.toStdString();
                message += ",saveUserId:" + this->ws->id->toStdString();
                message += "]";
                m2->sendMessageToServer(message);
                // 设置为只读模式

                // 发送第二波数据保存文件
//                m2->sendMessageToServerLen(data, readNum, );
                connect(m2->socket, &QTcpSocket::readyRead, [m2, this, &qd, upFileName, qf](){
                    string returnData = m2->acceptMessage();
                    qDebug().noquote() << returnData;
                    if (returnData != "ok"){
                        if (returnData.at(0) == 'i' &&
                            returnData.at(1) == 'd' &&
                            returnData.at(2) == ':'){
                            // 保存成功
                            QMessageBox::question(this, "message", QString(("保存成功 " + returnData).c_str()));
                            qd.close();
                            qf->close();
                            delete qf;
                            delete m2;
                            return;
                        }
                    }


                    // 指针创建方式
//                    char* data = new char[1024];

                    // 读取文件
                    QByteArray readNum = qf->read(1024);
                    qDebug().noquote() << readNum.size();
                    if (readNum.size() == 0){
                        m2->sendMessageToServer("ok");
                        return;
                    }
                    m2->sendMessageToServerLen(readNum.data(), readNum.size());
                });
            }
        });
        qd.exec();
    });

    connect(this->downloadFilePushButton, &QPushButton::clicked, [this](){
        QDialog qd = QDialog(this);
        QPushButton path("设置下载路径", &qd);
        path.move(90, 50);
        QString folderUrl;
        QPushButton qp("下载", &qd);
        connect(&path, &QPushButton::clicked, [this, &folderUrl](){
            folderUrl =
                QFileDialog::getExistingDirectoryUrl(this, "获取路径").
                toLocalFile().toStdString().c_str();
        });
        qp.move(0, 50);
        qd.setFixedSize(500, 100);
        Message* m2 = new Message(this, "127.0.0.1", 8880);
        string message = QString("getFileInfo[userId:").toStdString() +
                         this->ws->id->toStdString();
        message += "]";
        m2->sendMessageToServer(message);
        QComboBox comboBox(&qd);
        comboBox.setFixedSize(500, 50);
        connect(m2->socket, &QTcpSocket::readyRead, [m2, this, &qd, &comboBox](){
            string data = m2->acceptMessage();
            map<string, string>* datas = util::analysisString(data);
            for (int var = 0; var < datas->size(); ++var) {
                string name = (*datas).at((QString("file") + QString::number(var)).toStdString());
                comboBox.addItem(name.c_str());
            }
            delete datas;
            delete m2;
        });
        connect(&qp, &QPushButton::clicked, [this, &comboBox, &qd, &folderUrl](){
            if (folderUrl == ""){
                QMessageBox::critical(this, "错误", "请设置下载文件夹");
                return;
            }
            QString qs = comboBox.currentText();
            Message* m2 = new Message(this, "127.0.0.1", 8880);
            string message = QString("downloadFile[name:").toStdString() + qs.toStdString();
            message += "]";
            m2->sendMessageToServer(message);
            FILE* file = fopen((folderUrl.toStdString() + string("/") + qs.toStdString()).c_str(), "w");
            connect(m2->socket, &QTcpSocket::readyRead, [m2, qs, &qd, this, file](){
                QByteArray data = m2->acceptMessageData();
                if (data.size() == 2 && data == "ok"){
                    QMessageBox::question(this, "message", "下载成功");
                    qd.close();
                    fclose(file);
                }
                fputs(data, file);
                m2->sendMessageToServer("no");
            });
        });

        // 显示字体下拉框
//        QFontComboBox fontComboBox;
        qd.exec();
    });

    connect(this->shareFileFilePushButton, &QPushButton::clicked, [this](){
        QDialog qd = QDialog(this);
        qd.exec();
    });

    connect(addPushButton, &QPushButton::clicked, [this](){
        // 添加用户界面
        QDialog qd = QDialog(this);
        QPushButton qButtonContact = QPushButton(&qd);
        qButtonContact.setText("contact");
        qButtonContact.move(QPoint(0, 0));
        QPushButton qButtonGroup = QPushButton(&qd);
        qButtonGroup.setText("group");
        qButtonGroup.move(QPoint(100, 0));

        QLineEdit ql(&qd);
        QLabel qlabel(&qd);
        qlabel.move(QPoint(0, 50));
        qlabel.setText("userId: ");
        ql.move(50, 50);
        qd.setFixedSize(200, 130);
        connect(&qButtonContact, &QPushButton::clicked, [&ql, &qd, this](){
            // 添加
            string str = ql.text().toStdString();
            if(!(str.size() > 0)){
                return;
            }
            Message* m2 = new Message(this, "127.0.0.1", 8880);
            string message = QString("addContact[id:").toStdString();
            message += this->ws->id->toStdString() + ",addContactId:";
            message += str + "]";
            m2->sendMessageToServer(message);
            connect(m2->socket, &QTcpSocket::readyRead, [m2, this, &qd](){
                qd.close();
                this->updateThisWindowContact();
                delete m2;
            });
        });

        connect(&qButtonGroup, &QPushButton::clicked, [&ql, &qd, this](){
            // 添加
            string str = ql.text().toStdString();
            if(!(str.size() > 0)){
                return;
            }
            Message* m2 = new Message(this, "127.0.0.1", 8880);
            string message = QString("addGroup[id:").toStdString();
            message += this->ws->id->toStdString() + ",name:";
            message += str + "]";
            m2->sendMessageToServer(message);
            connect(m2->socket, &QTcpSocket::readyRead, [m2, this, &qd](){
                qd.close();
                this->updateThisWindowContact();
                delete m2;
            });
        });
        // 这行代码执行结束后会阻塞, 模态消息对话框
        qd.exec();
//        qd->show();  // 这行代码执行后不会阻塞, 所以必须使用new的方式创建对象, 不让在函数结束后对象会被销毁
    });
    this->groups = nullptr;
    this->contacts = nullptr;
    connect(this->groupPushButton, &QPushButton::clicked, [this](){
        this->updateThisWindowGroup();
    });

    connect(this->contactPushButton, &QPushButton::clicked, [this](){
        this->updateThisWindowContact();
    });

    scrollAreaWidgetContents = NULL;
    current = NULL;
}

void refreshGroupMessage(ChatRoomWindow* thisChat, util::Group* group){
    Message* m1 = new Message(thisChat, "127.0.0.1", 8880);
    string message = QString("getGroupMessageBySendId[groupId:").toStdString();
    message += group->groupId + "]";
    m1->sendMessageToServer(message);
    thisChat->connect(m1->socket, &QTcpSocket::readyRead, [m1, thisChat](){
        // 接受到显示的消息
        string mes = m1->acceptMessage();
        string showMes = "";
        vector<string>* v1 = util::analysisStringByKeysVector(mes);

        for (int i = 0; i < v1->size(); ++i) {
            showMes += v1->at(i) + "\n";
        }

        // 显示
        thisChat->qte2->setText(showMes.c_str());
        auto show = thisChat->qte2->textCursor();
        show.movePosition(QTextCursor::End);
        thisChat->qte2->setTextCursor(show);
        delete m1;
    });
}

void refreshContactMessage(ChatRoomWindow* thisChat, util::Contact* con){
    Message* m1 = new Message(thisChat, "127.0.0.1", 8880);
    string message = QString("getMessageBySendId[messageId:").toStdString();
    message += con->messageId + "]";
    m1->sendMessageToServer(message);
    thisChat->connect(m1->socket, &QTcpSocket::readyRead, [m1, thisChat](){
        string data = m1->acceptMessage();
        auto datas = util::analysisStringByKeysVector(data);
        string str = "";
        auto vectorData = datas->begin();
        for(int i = 0; i < datas->size(); i++, vectorData++){
            str += *vectorData + "\n";
        }
        thisChat->qte2->setText(str.c_str());
        auto show = thisChat->qte2->textCursor();
        show.movePosition(QTextCursor::End);
        thisChat->qte2->setTextCursor(show);
        delete m1;
    });
}

void ChatRoomWindow::updateThisWindowGroup(){
    if (isUpdate == 1){
        return;
    }

    isUpdate = 1;
    if (this->contacts != nullptr){
        auto begin = this->contacts->begin();
        for (int i = 0; i < this->contacts->size(); ++i, begin++) {
            std::pair<QPushButton*, util::Contact*> button = *(begin);
            gridLayout->removeWidget(button.first);
            delete button.first;
            delete button.second;
        }
        delete contacts;
        contacts = nullptr;
    }

    string message = "";
    Message* m2 = new Message(this, "127.0.0.1", 8880);
    message = QString("UpdateContactsGroup[id:").toStdString();
    message += this->ws->id->toStdString() + "]";
    m2->sendMessageToServer(message);
    this->currentMessage = this->temp;

    connect(m2->socket, &QTcpSocket::readyRead, [m2, this](){
        std::string contactsStr = m2->acceptMessage();
        map<string, string>* contacts = util::analysisString(contactsStr);
        // 更新组
        map<QPushButton*, util::Group*>* mapGroup = new map<QPushButton*, util::Group*>();
        int contactsLen = QString((*(contacts))["length"].c_str()).toInt();
        for (int i = 0; i < contactsLen; ++i) {
            util::Group* group = new util::Group;
            group->userId = (*(contacts))["group" + QString::number(i).toStdString() + "userId"];
            group->groupId = (*(contacts))["group" + QString::number(i).toStdString() + "groupId"];
            group->groupName = (*(contacts))["group" + QString::number(i).toStdString() + "groupName"];

            QPushButton* pushButton = new QPushButton(QString(group->groupName.c_str()), this);

            gridLayout->addWidget(pushButton);
            mapGroup->insert(pair<QPushButton*, util::Group*>(pushButton, group));

            connect(pushButton, &QPushButton::clicked, [this, pushButton, group](){
                // 更新聊天数据到聊天面板上
                if(this->current != NULL){
                    delete this->current;
                }
                auto p = new pair<QPushButton*, std::string>(pushButton, "group");
                this->current = p;
                this->currentMessage = pushButton;

                // 发送消息
                refreshGroupMessage(this, group);
            });
        }
        delete m2;
        this->groups = mapGroup;
    });
}

void getCurrentTime(string* timeCurrentStr){
    time_t tt = time(NULL);

    char*timeCurrent = new char[100];
    struct tm* ttm = gmtime(&tt);
    sprintf(timeCurrent, "y: %d m: %d d: %d w: %d h: %d M: %d s: %d",
            ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday,
            ttm->tm_wday, ttm->tm_hour - 4, ttm->tm_min, ttm->tm_sec);
    *timeCurrentStr += timeCurrent;
}

void ChatRoomWindow::updateThisWindowContact(){
    if (isUpdate == 0){
        return;
    }

    isUpdate = 0;

    if (this->groups != nullptr){
        auto begin = this->groups->begin();
        for (int i = 0; i < this->groups->size(); ++i, begin++) {
            std::pair<QPushButton*, util::Group*> button =*(begin);
            gridLayout->removeWidget(button.first);
            delete button.first;
            delete button.second;
        }
        delete groups;
        groups = nullptr;
    }

    Message* m1 = new Message(this, "127.0.0.1", 8880);
    string message = QString("updateContacts[id:").toStdString();
    message += this->ws->id->toStdString() + "]";
    m1->sendMessageToServer(message);
    this->currentMessage = temp;

    connect(m1->socket, &QTcpSocket::readyRead, [m1, this](){
        std::string contactsStr = m1->acceptMessage();
        map<string, string>* contacts = util::analysisString(contactsStr);
        map<QPushButton*, util::Contact*>* newContacts = new map<QPushButton*, util::Contact*>();
        int contactsLen = QString((*(contacts))["length"].c_str()).toInt();
        for (int i = 0; i < contactsLen; ++i) {
            util::Contact* con = new util::Contact;
            con->contactId = (*(contacts))["contact" + QString::number(i).toStdString() + "contactId"];
            con->userId = QString(((*(contacts))["contact" + QString::number(i).toStdString() + "userId"]).c_str()).toStdString();
            con->messageId = (*(contacts))["contact" + QString::number(i).toStdString() + "messageId"];
            con->contactName = (*(contacts))["contact" + QString::number(i).toStdString() + "contactName"];

            QPushButton* pushButton = new QPushButton(QString(con->contactName.c_str()), this);

            gridLayout->addWidget(pushButton);
            newContacts->insert(pair<QPushButton*, util::Contact*>(pushButton, con));

            connect(pushButton, &QPushButton::clicked, [this, pushButton, con](){
                // 更新聊天数据到聊天面板上
                if(this->current != NULL){
                    delete this->current;
                }
                auto p = new pair<QPushButton*, std::string>(pushButton, "contact");
                this->current = p;
                this->currentMessage = pushButton;

                // 发送消息, 更新聊天消息
                this->contactId = con->contactId;


                refreshContactMessage(this, con);
            });
        }
        this->contacts = newContacts;
        delete m1;
    });
}

void ChatRoomWindow::paintEvent(QPaintEvent* qpe){
//    QPixmap qpm;
//    qpm.load(this->ws->img);

//    // 设置背景图片
//    QPainter qp(this);
//    //    qpm.scaled(this->width() * 0.5, this->height() * 0.5);  // 缩放
//    qp.drawPixmap(0, 0, this->width(), this->height(), qpm);
}

void getMessage(string* messagePtr, string* AnalysisDataPtr, string* typePtr, int num) {
    for (int i = 0; i < (*AnalysisDataPtr).length(); i++){
        if ((*AnalysisDataPtr).at(i) == '['){
            for (int j = i + 1; j < (*AnalysisDataPtr).length(); j++){
                if ((*AnalysisDataPtr).at(j) == ']' && num == 2){
                    break;
                }
                (*messagePtr) += (*AnalysisDataPtr)[j];
            }
            return;
        }

        (*typePtr) += (*AnalysisDataPtr).at(i);
    }
}

static vector<string>* analysisStringMessage(string AnalysisData, string* type1) {
    // 解析数据
    vector<string>* m3 = new vector<string>;
    vector<string> v1;
    string temp = AnalysisData;
    string type = "";
    string message = "";

    // returnUserMessage[message[" + message + "],sendId:" + contactId + ",userId:" + userId + "]
    getMessage(&message, &AnalysisData, type1, 1);


    AnalysisData = message;
    message = "";

    getMessage(&message, &AnalysisData, &type, 2);
    m3 = util::analysisStringByKeysVector(message);
    return m3;
};

void ChatRoomWindow::init(){
//    QFile qf("J:\\img.txt");
//    qf.open(QIODeviceBase::ReadWrite);
//    QString name = qf.readAll();
//    this->ws->img = name;
//    qf.close();
//    this->update();
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("scrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setGeometry(QRect(0, 0 + 26, this->width()*0.3, this->height() - this->menuBar()->height()));

    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 115, 225));
    gridLayout = new QGridLayout(scrollAreaWidgetContents);

    gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setObjectName("gridLayout");
    gridLayout->addWidget(this->addPushButton);
    gridLayout->addWidget(this->refreshPushButton);
    gridLayout->addWidget(contactPushButton);
    gridLayout->addWidget(groupPushButton);
    gridLayout->addWidget(upFilePushButton);
    gridLayout->addWidget(downloadFilePushButton);
    gridLayout->addWidget(shareFileFilePushButton);
    scrollArea->setWidget(scrollAreaWidgetContents);
//    // 聊天显示框
//    qte = new QLabel(this);
//    QPixmap qpm;
//    qpm.load(this->ws->img);
//    qte->move(QPoint((this->width()*0.3), (0 + 50)));
//    qte->setFixedSize((this->width()*0.7), (this->height()) * 0.7);
//    qte->setPixmap(qpm);
//    // 设置label自己调整图片大小
//    qte->setScaledContents(true);

    // 用户输入聊天内容信息框
    QTextEdit* qte1 = new QTextEdit(this);
    qte1->setObjectName("textEdit1");
    qte1->move(QPoint((this->width()*0.3), (0 + 50 + (this->height()* 0.7))));
    qte1->setFixedSize((this->width()*0.6), (this->height() * 0.3 - 50));

    // 显示消息
    QTextEdit* qte2 = new QTextEdit(this);
    qte2->setObjectName("textEdit2");
    qte2->move(QPoint((this->width()*0.3), (0 + 50)));
    qte2->setFixedSize((this->width()*0.7), (this->height()) * 0.7);
    this->qte2 = qte2;

    // 聊天显示框上面的显示栏
    this->ql = new QLabel(this);
    ql->setPixmap(QPixmap(":/png/white.png"));
    ql->move(this->width()*0.3, 23);
    ql->setFixedSize((this->width()*0.7), 50 - 26);

    // 发送消息按钮
    QPushButton* qpb = new QPushButton(this);
    qpb->setText("send");
    qpb->setFixedSize(this->width() * 0.1, qpb->height());
    qpb->move(this->width() * 0.9, this->height() - qpb->height());

    string date = "";
    getCurrentTime(&date);
    string data = "";

    data += "userId:  ";
    data += this->ws->id->toStdString();
    data += "  time: ";
    data += date;
    QFont qf;
    qf.setFamily("幼圆");
    qf.setPointSize(10);
    ql->setFont(qf);
    this->ql->setText(data.c_str());

    QTimer* qt1 = new QTimer(this);
    qt1->start(1000);  // 每1s执行一次
    this->currentMessage = new QPushButton;
    temp = this->currentMessage;

    connect(qt1, &QTimer::timeout, [&](){
        string data = "";
        string date = "";
        getCurrentTime(&date);
        data += "userId:  ";
        data += this->ws->id->toStdString();
        data += "  time: ";
        data += date;
        this->ql->setText(data.c_str());
        emit this->currentMessage->click();
    });

    connect(qpb, &QPushButton::clicked, [this, qte1, qte2](){
        qte2->setText("");
        if (this->current == NULL){
            // 无法发送消息
            return;
        }

        // 发送消息
        QPushButton* currentButton = this->current->first;
        auto type = this->current->second;
        Message* m1 = new Message(this, "127.0.0.1", 8880);
        // 获取对话框数据
        QString mes = qte1->toPlainText();
        mes = mes.replace(',', "，");
        mes = mes.replace('[', "{");
        mes = mes.replace(']', "}");
        mes = mes.replace(':', "：");
        qte1->setText("");

        if (type == "contact"){
            // 联系人
            // 获取相关数据
            util::Contact* contact = (*(this->contacts))[currentButton];
            string sendMessage = "";
            sendMessage += "sendMessageContact[id:";
            sendMessage += contact->contactId;
            sendMessage += ",sendUser:";
            sendMessage += contact->userId;
            sendMessage += ",message:";
            sendMessage += mes.toStdString();
            sendMessage += "]";
            m1->sendMessageToServer(sendMessage);
        } else {
            // 组
            // 获取相关数据
            util::Group* group = (*(this->groups))[currentButton];
            string sendMessage = "";
            sendMessage += "sendMessageGroup[id:";
            sendMessage += group->groupId;
            sendMessage += ",sendUser:";
            sendMessage += this->ws->id->toStdString();
            sendMessage += ",message:";
            sendMessage += mes.toStdString();
            sendMessage += "]";
            m1->sendMessageToServer(sendMessage);
        }

        // 得到返回的数据
        // 类型:
        // "returnUserMessage[message[" + message + "],sendId:" + contactId + ",userId:" + userId + "]"
        connect(m1->socket, &QTcpSocket::readyRead, [m1, this, qte2](){
            std::string contactsStr = m1->acceptMessage();
            string* type = new string();
            vector<string>* map1 = analysisStringMessage(contactsStr, type);
            // 判断类型
            if (*type == "returnUserMessage"){
                // 显示
                auto data = map1->begin();
                string str = "";
                for(int i = 0; i < map1->size(); i++, data++){
                    str += *data + "\n";
                }

                qte2->setText(QString(str.c_str()));
                auto show = qte2->textCursor();
                show.movePosition(QTextCursor::End);
                qte2->setTextCursor(show);
            }

            delete type;
            delete m1;
        });
    });
}

ChatRoomWindow::~ChatRoomWindow()
{
    delete upFilePushButton;
    delete downloadFilePushButton;
    delete shareFileFilePushButton;
    delete temp;
    delete current;
    if (this->groups != nullptr){
        auto begin = this->groups->begin();
        for (int i = 0; i < this->groups->size(); ++i, begin++) {
            std::pair<QPushButton*, util::Group*> button =*(begin);
            gridLayout->removeWidget(button.first);
            delete button.first;
            delete button.second;
        }
        delete groups;
        groups = nullptr;
    }
    if (this->contacts != nullptr){
        auto begin = this->contacts->begin();
        for (int i = 0; i < this->contacts->size(); ++i, begin++) {
            std::pair<QPushButton*, util::Contact*> button =*(begin);
            gridLayout->removeWidget(button.first);
            delete button.first;
            delete button.second;
        }
        delete contacts;
        contacts = nullptr;
    }
    delete gridLayout;
    delete ui;
}
