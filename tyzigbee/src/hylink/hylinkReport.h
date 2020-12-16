#ifndef _HYLINKREPORT_H_
#define _HYLINKREPORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char DeviceId[24];
        char ModelId[24];
        char Key[24];
        char Value[16];
        // void *private;
    } HylinkReportData;
    typedef struct
    {
        char Command;
        int FrameNumber;
        char Type[12];
        char GatewayId[16];
        HylinkReportData *Data;
        unsigned char DataSize;
    } HylinkReport;

    int hylinkReportFunc(HylinkReport *hylinkReport);
#ifdef __cplusplus
}
#endif
#endif