#ifndef MESSAGE_H
#define MESSAGE_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QMainWindow>

#define printerror(line, error) line + "行error: " + error


class Message
{
public:
    Message(QMainWindow* parent, std::string ip, int port);
    bool sendMessageToServer(std::string message);
    bool sendMessageToServerLen(std::string message, int len);
    std::string acceptMessage();
    QByteArray acceptMessageData();
    QTcpSocket* socket;
    void bind(std::string ip, int port);
    void close();
    ~Message();
};

#endif // MESSAGE_H
