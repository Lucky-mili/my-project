#ifndef SERVER_H
#define SERVER_H

#include <QTcpSocket>
#include <QWidget>

class Server : public QWidget
{
    Q_OBJECT

public:
    QString m_strIP;
    quint16 m_usPort;
    QString m_strRootPath;
    static Server& getInstance();
    ~Server();
    void loadConfig();

private:
    Server(QWidget *parent = nullptr);
};
#endif // SERVER_H
