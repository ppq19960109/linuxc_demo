#include "pn512.h"
#include "ISO14443A.h"
#include "ISO14443B.h"
#include "pn.h"

static unsigned char key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static unsigned char IC_key[6] = {0xE2, 0xF9, 0xB4, 0xEF, 0xCD, 0x78};
static unsigned char IC_key_V201[6] = {0xF9, 0xE2, 0xEF, 0xB4, 0x78, 0xCD};

static unsigned char pcdData[16] = {1, 2, 3, 4, 5, 6, 7, 99};
static unsigned char pcdData_read[16] = {0};
void ReaderCard(void) {
    unsigned char Temp[3] = {0};
    unsigned char UID[4] = {0};
    if (PcdRequest(PICC_REQALL, Temp) == MI_OK)  //选卡
    {
        if (Temp[0] == 0x04 && Temp[1] == 0x00)
            printf("MFOne-S50");
        else if (Temp[0] == 0x02 && Temp[1] == 0x00)
            printf("MFOne-S70");
        else if (Temp[0] == 0x44 && Temp[1] == 0x00)
            printf("MF-UltraLight");
        else if (Temp[0] == 0x08 && Temp[1] == 0x00)
            printf("MF-Pro");
        else if (Temp[0] == 0x44 && Temp[1] == 0x03)
            printf("MF Desire");
        else
            printf("Unknown");
        if (PcdAnticoll(UID) == MI_OK) { /*防冲撞*/
            printf("\r\nID:%02x %02x %02x %02x\n", UID[0], UID[1], UID[2], UID[3]);
        };
    }
}

void PN512_READ_CPU() {
    PcdRequest_IDCard();  //读取身份证UID
}

void PN512_READ_IC() {
    // unsigned char status;
    //	unsigned char Temp[3]={0};unsigned char UID[4] ={0};
    memset(SelectedSnr, 0, sizeof(SelectedSnr));
    memset(TagType, 0, sizeof(TagType));
    if (MI_OK == PcdRequest(PICC_REQALL, TagType)) { /*扫描ID*/
        if (MI_OK == PcdAnticoll(SelectedSnr)) {
            printf("\r\nID:%02x %02x %02x %02x\n", SelectedSnr[0], SelectedSnr[1], SelectedSnr[2], SelectedSnr[3]);
            printf("\r\nCard_Type:%02x %02x %02x\n", TagType[0], TagType[1], TagType[2]);
            if (MI_OK == PcdSelect(SelectedSnr)) {
                if (MI_OK == PcdAuthState(0x60, 2, key, SelectedSnr)) {
                    //										PcdWrite(2,pcdData);
                    PcdRead(2, pcdData_read);
                    printf("\r\n pcdData_read:%02x %02x %02x\n", pcdData_read[0], pcdData_read[1], pcdData_read[2]);
                }
            }
        }
    }
}

void PN512_write_IC() {
    // unsigned char status;
    //	unsigned char Temp[3]={0};unsigned char UID[4] ={0};
    memset(SelectedSnr, 0, sizeof(SelectedSnr));
    memset(TagType, 0, sizeof(TagType));
    if (MI_OK == PcdRequest(PICC_REQALL, TagType)) { /*扫描ID*/
        if (MI_OK == PcdAnticoll(SelectedSnr)) {
            printf("\r\nID:%02x %02x %02x %02x\n", SelectedSnr[0], SelectedSnr[1], SelectedSnr[2], SelectedSnr[3]);
            printf("\r\nCard_Type:%02x %02x %02x\n", TagType[0], TagType[1], TagType[2]);
            if (MI_OK == PcdSelect(SelectedSnr)) {
                if (MI_OK == PcdAuthState(0x60, 2, key, SelectedSnr)) {
                    PcdWrite(2, pcdData);
                }
            }
        }
    }
}
