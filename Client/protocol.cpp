#include "protocol.h"
#include "stdlib.h"
#include "string.h"

PDU *mkPDU(uint uiMsgLen)
{
    uint uiTotalLen = uiMsgLen + sizeof(PDU);
    PDU* pdu = (PDU*)malloc(uiTotalLen);
    if (pdu == NULL)
    {
        exit(1);
    }
    memset(pdu, 0, uiTotalLen);
    pdu->uiTotalLen = uiTotalLen;
    pdu->uiMsgLen = uiMsgLen;
    return pdu;
}
