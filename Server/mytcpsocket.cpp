#include "OperateDB.h"
#include "mytcpserver.h"
#include "mytcpsocket.h"
#include "protocol.h"

MyTcpSocket::MyTcpSocket()
{
    m_pmh = new MsgHandler();
}

MyTcpSocket::~MyTcpSocket()
{
    delete m_pmh;
}

void MyTcpSocket::sendMsg(PDU *pdu)
{
    if (pdu == NULL)
    {
        return ;
    }
    this->write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "sendmsg respdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData + 32" << pdu->caData + 32
             << "pdu->caMsg" << pdu->caMsg;
    free(pdu);
    pdu = NULL;
}

PDU *MyTcpSocket::readMsg()
{
    qDebug() << "readMsg 读取消息长度" << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    return pdu;
}

PDU *MyTcpSocket::handleMsg(PDU *pdu)
{
    qDebug() << "handlemsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData + 32" << pdu->caData + 32
             << "pdu->caMsg" << pdu->caMsg;
    PDU* respdu = NULL;
    m_pmh->pdu = pdu;
    switch (pdu->uiType)
    {
        case ENUM_TYPE_REGIST_REQUEST:
        {
            respdu = m_pmh->regist();
            break;
        }
        case ENUM_TYPE_LOGIN_REQUEST:
        {
            respdu = m_pmh->login(m_strLoginName);
            break;
        }
        case ENUM_TYPE_FIND_USER_REQUEST:
        {
            respdu = m_pmh->findUser();
            break;
        }
        case ENUM_TYPE_ONLINE_USER_REQUEST:
        {
            respdu = m_pmh->onlineUser();
            break;
        }
        case ENUM_TYPE_ADD_FRIEND_REQUEST:
        {
            respdu = m_pmh->addFriend();
            break;
        }
        case ENUM_TYPE_ADD_FRIEND_AGREE_REQUEST:
        {
            respdu = m_pmh->addFriendAgree();
            break;
        }
        case ENUM_TYPE_FLUSH_FRIEND_REQUEST:
        {
            respdu = m_pmh->flushFriend();
            break;
        }
        case ENUM_TYPE_MKDIR_REQUEST:
        {
            respdu = m_pmh->mkdir();
            break;
        }
        case ENUM_TYPE_FLUSH_FILE_REQUEST:
        {
            respdu = m_pmh->flushFile();
            break;
        }
        case ENUM_TYPE_DEL_FILE_REQUEST:
        {
            respdu = m_pmh->delFile();
            break;
        }
        case ENUM_TYPE_RENAME_FILE_REQUEST:
        {
            respdu = m_pmh->renameFile();
            break;
        }
        case ENUM_TYPE_UPLOAD_FILE_INIT_RESPOND:
        {
            respdu = m_pmh->uploadFileInit();
            break;
        }
        case ENUM_TYPE_UPLOAD_FILE_DATA_RESPOND:
        {
            respdu = m_pmh->uploadFileData();
            break;
        }
        default:
            break;
    }
    return respdu;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << "recvMsg 接收消息长度" << this->bytesAvailable();

    QByteArray data = this->readAll();
    buffer.append(data);
    while (buffer.size() >= int(sizeof(PDU)))
    {
        PDU* pdu = (PDU*)buffer.data();
        if (buffer.size() < int(pdu->uiTotalLen))
        {
            break ;
        }
        PDU* respdu = handleMsg(pdu);
        sendMsg(respdu);
        buffer.remove(0, pdu->uiTotalLen);
    }
}

void MyTcpSocket::clientOffline()
{
    OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    MyTcpServer::getInstance().removeSocket(this);
}
