#ifndef FILE_H
#define FILE_H

#include "protocol.h"

#include <QWidget>
#include <qlistwidget.h>

namespace Ui
{
    class File;
}

class File : public QWidget
{
    Q_OBJECT

public:
    QString m_strUserPath;
    QString m_strCurPath;
    QString m_strUploadFilePath;
    QList<FileInfo*> m_pFileInfoList;
    void flushFile();
    void updateFileList(QList<FileInfo*> pFileInfoList);
    void uploadFile();
    explicit File(QWidget *parent = nullptr);
    ~File();

public slots:
    void uploadErrorBox(const QString &msg);

private slots:
    void on_mkdir_PB_clicked();

    void on_flush_PB_clicked();

    void on_del_PB_clicked();

    void on_rename_PB_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_return_PB_clicked();

    void on_upload_PB_clicked();

private:
    Ui::File *ui;
};

#endif // FILE_H
