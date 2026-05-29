#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include "msghandler.h"
#include "protocol.h"

#include <QObject>
#include <qtcpsocket.h>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    ~MyTcpSocket();
    QString m_strLoginName;
    MsgHandler* m_pmh;
    QByteArray buffer;
    void sendMsg(PDU* pdu);
    PDU* readMsg();
    PDU* handleMsg(PDU* pdu);

public slots:
    void recvMsg();
    void clientOffline();
};

#endif // MYTCPSOCKET_H
