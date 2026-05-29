#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "protocol.h"

#include <QFile>
#include <qstring.h>

class MsgHandler
{
public:
    PDU* pdu;
    qint64 m_iUploadFileSize;
    qint64 m_iUploadFileReceived;
    QFile m_iUploadFile;
    MsgHandler();
    PDU* regist();
    PDU* login(QString& m_strLoginName);
    PDU* findUser();
    PDU* onlineUser();
    PDU* addFriend();
    PDU* addFriendAgree();
    PDU* flushFriend();
    PDU* chat();
    PDU* mkdir();
    PDU* flushFile();
    PDU* delFile();
    PDU* renameFile();
    PDU* uploadFileInit();
    PDU* uploadFileData();
};

#endif // MSGHANDLER_H
