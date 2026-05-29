#include "OperateDB.h"
#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OperateDB::getInstance().connectSQL();
    Server::getInstance();
//    w.show();
    return a.exec();
}
