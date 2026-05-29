#include "client.h"
#include "index.h"
#include "reshandler.h"
#include "string.h"

#include <QMessageBox>

Reshandler::Reshandler()
{

}

void Reshandler::regist()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret)
    {
        QMessageBox::information(&Client::getInstance(), "提示", "注册成功");
    }
    else
    {
        QMessageBox::information(&Client::getInstance(), "提示", "注册失败");
    }
}

void Reshandler::login()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    if (ret)
    {
        Index::getInstance().show();
        Client::getInstance().hide();
    }
    else
    {
        QMessageBox::information(&Client::getInstance(), "提示", "登录失败");
    }
}

void Reshandler::findUser()
{
    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    qDebug() << "finduser ret" << ret;
    if (ret == 0)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    }
    if (ret == 1)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户在线");
    }
    if (ret == 2)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不存在");
    }
    if (ret == -1)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "查找失败");
    }
}

void Reshandler::onlineUser()
{
    uint uiSize = pdu->uiMsgLen/32;
    char caTmp[32] = {'\0'};
    QStringList userList;
    for (uint i = 0; i < uiSize; i++)
    {
        memcpy(caTmp, pdu->caMsg + i * 32, 32);
        userList.append(caTmp);
    }
    Index::getInstance().getFriend()->m_pOnlineUser->updateLW(userList);
}

void Reshandler::addFriend()
{
    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    qDebug() << "addFriend ret" << ret;
    if(ret == 0)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    }
    if(ret == -2)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户已经是好友");
    }
    if(ret == -1)
    {
        QMessageBox::information(&Index::getInstance(), "提示", "服务器错误：联系开发人员");
    }
}

void Reshandler::addFriendResend()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    int ret = QMessageBox::question(&Index::getInstance(), "添加好友", QString("是否同意 %1 的添加好友请求？").arg(caName));
    if (ret != QMessageBox::Yes)
    {
        return ;
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, pdu->caData, 64);
    respdu->uiType = ENUM_TYPE_ADD_FRIEND_AGREE_REQUEST;
    Client::getInstance().sendMsg(respdu);
}

void Reshandler::addFriendAgree()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "addFriendAgree ret" << ret;
    if (ret)
    {
        QMessageBox::information(&Client::getInstance(), "提示", "添加好友成功");
    }
    else
    {
        QMessageBox::information(&Client::getInstance(), "提示", "添加好友失败");
    }
}

void Reshandler::flushFriend()
{
    QStringList friendList;
    int iSize = pdu->uiMsgLen / 32;
    char caTmp[32] = {'\0'};
    for (int i = 0; i < iSize; i++)
    {
        memcpy(caTmp, pdu->caMsg + i * 32, 32);
        friendList.append(caTmp);
    }
    Index::getInstance().getFriend()->update_LW(friendList);
}

void Reshandler::chat()
{
    Chat* c = Index::getInstance().getFriend()->m_pChat;
    if (c->isHidden())
    {
        c->show();
    }
    char caChatName[32] = {'\0'};
    memcpy(caChatName, pdu->caData, 32);
    c->updateShow_TE(QString("%1 : %2").arg(caChatName).arg(pdu->caMsg));
    c->m_strChatName = caChatName;
}

void Reshandler::mkdir()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "mkdir ret" << ret;
    if (ret)
    {
        Index::getInstance().getFile()->flushFile();
    }
    else
    {
        QMessageBox::information(&Index::getInstance(), "提示", "创建文件夹失败");
    }
}

void Reshandler::flushFile()
{
    int iCount = pdu->uiMsgLen / sizeof(FileInfo);

    QList<FileInfo*> pFileInfoList;
    for (int i = 0; i < iCount; i++)
    {
        FileInfo* pFileInfo = new FileInfo;
        memcpy(pFileInfo, pdu->caMsg + i * sizeof(FileInfo), sizeof(FileInfo));
        pFileInfoList.append(pFileInfo);
    }
    Index::getInstance().getFile()->updateFileList(pFileInfoList);
}

void Reshandler::delFile()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "delFile ret" << ret;
    if (ret)
    {
        Index::getInstance().getFile()->flushFile();
    }
    else
    {
        QMessageBox::information(&Index::getInstance(), "提示", "删除文件失败");
    }
}

void Reshandler::renameFile()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "renameFile ret" << ret;
    if (ret)
    {
        Index::getInstance().getFile()->flushFile();
    }
    else
    {
        QMessageBox::information(&Index::getInstance(), "提示", "重命名文件失败");
    }
}

void Reshandler::uploadFileInit()
{
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "uploadFileInit ret" << ret;
    if (ret)
    {
        Index::getInstance().getFile()->uploadFile();
    }
    else
    {
        QMessageBox::information(&Index::getInstance(), "提示", "上传文件失败");
    }
}

void Reshandler::uploadFileData()
{
    QMessageBox::information(&Index::getInstance(), "提示", "上传文件完成");
    Index::getInstance().getFile()->flushFile();
}
