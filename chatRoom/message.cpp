#include "message.h"

Message::Message(QMainWindow* parent, std::string ip, int port)
{
    socket = NULL;
    socket = new QTcpSocket();
    // 设置随着什么窗口销毁而销毁
    socket->setParent(parent);
    // 监听
    socket->connectToHost(QHostAddress(ip.c_str()), port);
}

void Message::bind(std::string ip, int port){
    // 监听
    socket->connectToHost(QHostAddress(ip.c_str()), port);
}

void Message::close(){
    this->socket->close();
}

Message::~Message(){
    this->close();
    delete this->socket;
}

bool Message::sendMessageToServer(std::string message){
    this->socket->write(message.c_str(), message.length());
    return true;
}

bool Message::sendMessageToServerLen(std::string message, int len){
    this->socket->write(message.c_str(), len);
    return true;
}

QByteArray Message::acceptMessageData(){
    QByteArray localReadAll = this->socket->readAll();
    return localReadAll;
}

std::string Message::acceptMessage(){
    QByteArray localReadAll = this->socket->readAll();
    return localReadAll.toStdString();
}
