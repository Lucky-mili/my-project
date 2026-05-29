#ifndef RESHANDLER_H
#define RESHANDLER_H

#include "protocol.h"



class Reshandler
{
public:
    PDU* pdu;
    Reshandler();
    void regist();
    void login();
    void findUser();
    void onlineUser();
    void addFriend();
    void addFriendResend();
    void addFriendAgree();
    void flushFriend();
    void chat();
    void mkdir();
    void flushFile();
    void delFile();
    void renameFile();
    void uploadFileInit();
    void uploadFileData();
};

#endif // RESHANDLER_H
