#include "client.h"
#include "index.h"
#include "protocol.h"
#include "reshandler.h"
#include "ui_client.h"

#include <QFile>
#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    loadConfig();
    m_prh = new Reshandler;
    m_socket.connectToHost(QHostAddress(m_strIP), m_usPort);
    connect(&m_socket, &QTcpSocket::connected, this, &Client::showConnect);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Client::recvMsg);
}

Client::~Client()
{
    delete ui;
    delete m_prh;
}

void Client::loadConfig()
{
    QFile file(":/connect.config");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "打开文件失败";
        return ;
    }
    QByteArray baData = file.readAll();
    QString strData = QString(baData);
    QStringList strList = strData.split("\r\n");
    m_strIP = strList[0];
    m_usPort = strList[1].toUShort();
    m_strRootPath = strList[2];
    qDebug() << "ip" << m_strIP << "port" << m_usPort << "strRootPath" << m_strRootPath;

    file.close();
}

Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

void Client::sendMsg(PDU *pdu)
{
    m_socket.write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "sendmsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData + 32" << pdu->caData + 32
             << "pdu->caMsg" << pdu->caMsg;
    free(pdu);
    pdu = NULL;
}

PDU *Client::readMsg()
{
    qDebug() << "recvMsg 接收消息长度" << m_socket.bytesAvailable();
    uint uiPDULen = 0;
    m_socket.read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    m_socket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    return pdu;
}

void Client::handleMsg(PDU *pdu)
{
    qDebug() << "handlemsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData + 32" << pdu->caData + 32
             << "pdu->caMsg" << pdu->caMsg;
    m_prh->pdu = pdu;
    switch (pdu->uiType)
    {
        case ENUM_TYPE_REGIST_RESPOND:
        {
            m_prh->regist();
            break;
        }
        case ENUM_TYPE_LOGIN_RESPOND:
        {
            m_prh->login();
            break;
        }
        case ENUM_TYPE_FIND_USER_RESPOND:
        {
            m_prh->findUser();
            break;
        }
        case ENUM_TYPE_ONLINE_USER_RESPOND:
        {
            m_prh->onlineUser();
            break;
        }
        case ENUM_TYPE_ADD_FRIEND_RESPOND:
        {
            m_prh->addFriend();
            break;
        }
        case ENUM_TYPE_ADD_FRIEND_RESEND:
        {
            m_prh->addFriendResend();
            break;
        }
        case ENUM_TYPE_ADD_FRIEND_AGREE_RESPOND:
        {
            m_prh->addFriendAgree();
            break;
        }
        case ENUM_TYPE_FLUSH_FRIEND_RESPOND:
        {
            m_prh->flushFriend();
            break;
        }
        case ENUM_TYPE_MKDIR_RESPOND:
        {
            m_prh->mkdir();
            break;
        }
        case ENUM_TYPE_FLUSH_FILE_RESPOND:
        {
            m_prh->flushFile();
            break;
        }
        case ENUM_TYPE_DEL_FILE_RESPOND:
        {
            m_prh->delFile();
            break;
        }
        case ENUM_TYPE_RENAME_FILE_RESPOND:
        {
            m_prh->renameFile();
            break;
        }
        case ENUM_TYPE_UPLOAD_FILE_INIT_RESPOND:
        {
            m_prh->uploadFileInit();
            break;
        }
        case ENUM_TYPE_UPLOAD_FILE_DATA_RESPOND:
        {
            m_prh->uploadFileData();
            break;
        }
        default:
            break;
    }
}

void Client::recvMsg()
{
    qDebug() << "recvMsg 接收消息长度" << m_socket.bytesAvailable();

    QByteArray data = m_socket.readAll();
    buffer.append(data);
    while (buffer.size() >= int(sizeof(PDU)))
    {
        PDU* pdu = (PDU*)buffer.data();
        if (buffer.size() < int(pdu->uiTotalLen))
        {
            break ;
        }
        handleMsg(pdu);
        buffer.remove(0, pdu->uiTotalLen);
    }
}

void Client::showConnect()
{
    qDebug() << "连接成功";
}

//void Client::on_send_PB_clicked()
//{
//    QString strMsg = ui->input_LE->text();
//    PDU* pdu = mkPDU(strMsg.toStdString().size());
//    pdu->uiType = ENUM_TYPE_REGIST_REQUEST;
//    memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.toStdString().size());
//    m_socket.write((char*)pdu, pdu->uiTotalLen);
//    qDebug() << "sendmsg pdu->uiTotalLen" << pdu->uiTotalLen
//             << "pdu->uiMsgLen" << pdu->uiMsgLen
//             << "pdu->uiType" << pdu->uiType
//             << "pdu->caData" << pdu->caData
//             << "pdu->caMsg" << pdu->caMsg;
//    free(pdu);
//    pdu = NULL;
//}

void Client::on_rigist_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    if (strName.isEmpty() || strPwd.isEmpty() || strName.toStdString().size() > 32 || strPwd.toStdString().size() > 32)
    {
        QMessageBox::critical(this, "注册", "用户名或密码过长");
        return ;
    }
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_TYPE_REGIST_REQUEST;
    sendMsg(pdu);
}

void Client::on_login_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    if (strName.isEmpty() || strPwd.isEmpty() || strName.toStdString().size() > 32 || strPwd.toStdString().size() > 32)
    {
        QMessageBox::critical(this, "登录", "用户名或密码过长");
        return ;
    }
    m_strLoginName = strName;
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_TYPE_LOGIN_REQUEST;
    sendMsg(pdu);
}
