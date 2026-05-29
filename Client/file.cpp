#include "client.h"
#include "file.h"
#include "ui_file.h"
#include "uploader.h"
#include <QFileDialog>
#include <QInputDialog>
#include <qmessagebox.h>

void File::flushFile()
{
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::updateFileList(QList<FileInfo *> pFileInfoList)
{
    foreach (FileInfo * pFileInfo, m_pFileInfoList)
    {
        delete pFileInfo;
    }
    m_pFileInfoList.clear();
    m_pFileInfoList = pFileInfoList;

    ui->listWidget->clear();
    foreach (FileInfo * pFileInfo, pFileInfoList)
    {
        QListWidgetItem* pItem = new QListWidgetItem;
        if (pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }
        else
        {
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        pItem->setText(pFileInfo->caName);
        ui->listWidget->addItem(pItem);
    }
}

void File::uploadFile()
{
    Uploader* uploader = new Uploader(m_strUploadFilePath);
    connect(uploader, &Uploader::errorMsg, this, &File::uploadErrorBox, Qt::QueuedConnection);
    connect(uploader, &Uploader::uploadPDU, &Client::getInstance(), &Client::sendMsg, Qt::QueuedConnection);
    uploader->start();
}

File::File(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::File)
{
    ui->setupUi(this);
    m_strUserPath = QString("%1/%2").arg(Client::getInstance().m_strRootPath).arg(Client::getInstance().m_strLoginName);
    m_strCurPath = m_strUserPath;
    flushFile();
}

File::~File()
{
    delete ui;
}

void File::uploadErrorBox(const QString &msg)
{
    QMessageBox::information(this, "提示", msg);
}

void File::on_mkdir_PB_clicked()
{
    QString strDirName = QInputDialog::getText(this, "新建文件夹", "新建文件夹名：");
    if (strDirName.isEmpty() || strDirName.toStdString().size() > 32)
    {
        QMessageBox::information(this, "提示", "文件夹名长度非法");
        return ;
    }
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_MKDIR_REQUEST;
    memcpy(pdu->caData, strDirName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_flush_PB_clicked()
{
    flushFile();
}

void File::on_del_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if (!pItem)
    {
        return ;
    }
    int ret = QMessageBox::question(this, "删除文件", QString("是否确认删除文件%1？").arg(pItem->text()));
    if (ret != QMessageBox::Yes)
    {
        return ;
    }
    QString strPath = QString("%1/%2").arg(m_strCurPath).arg(pItem->text());
    PDU* pdu = mkPDU(strPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_DEL_FILE_REQUEST;
    memcpy(pdu->caMsg, strPath.toStdString().c_str(), strPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_rename_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if (!pItem)
    {
        return ;
    }
    QString strNewName = QInputDialog::getText(this, "重命名", "新文件名：");
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_RENAME_FILE_REQUEST;
    memcpy(pdu->caData, pItem->text().toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strNewName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    foreach (FileInfo* pFileInfo, m_pFileInfoList)
    {
        if (pFileInfo->caName == item->text() && pFileInfo->iFileType != 0)
        {
            return ;
        }
    }
    m_strCurPath = QString("%1/%2").arg(m_strCurPath).arg(item->text());
    flushFile();
}

void File::on_return_PB_clicked()
{
    if (m_strCurPath == m_strUserPath)
    {
        return ;
    }
    int index = m_strCurPath.lastIndexOf('/');
    m_strCurPath.remove(index, m_strCurPath.size() - index);
    flushFile();
}

void File::on_upload_PB_clicked()
{
    m_strUploadFilePath.clear();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    qDebug() << "m_strUploadFilePath" << m_strUploadFilePath;
    if (m_strUploadFilePath.isEmpty())
    {
        return ;
    }
    int index = m_strUploadFilePath.lastIndexOf('/');
    QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - index - 1);
    QFile file(m_strUploadFilePath);
    qint64 iFileSize = file.size();
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_UPLOAD_FILE_INIT_REQUEST;
    memcpy(pdu->caData, strFileName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, &iFileSize, sizeof(qint64));
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}
