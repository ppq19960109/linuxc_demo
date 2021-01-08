#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "cpython.h"
#include "base64.h"

#include "zigbeeManage.h"

#include "hylinkRecv.h"



int hylinkValueConversion(ConverDesc *converDesc)
{
    if (converDesc->inLen == 0)
        return -1;
    if (strcmp(converDesc->dir, STR_UP) == 0) //up
    {
        if (converDesc->hyKeyPrivate == 0)
        {
            sprintf((char *)converDesc->out, "%d", *converDesc->in);
            converDesc->outLen = strlen((char *)converDesc->out);
        }
        else
        {
            if (strcmp(converDesc->hyKey, "SceName_") == 0 || strcmp(converDesc->hyKey, "ScePhoto_") == 0)
            {
                int num = (converDesc->in[0] << 8) + converDesc->in[1];
                char buf[8] = {0};
                sprintf(buf, "%d", num);
                strcat(converDesc->hyKey, buf);
                converDesc->in += 2;
                converDesc->inLen -= 2;
            }

            void *encodeOut = malloc(BASE64_ENCODE_OUT_SIZE(converDesc->inLen));
            int encodeOutlen = base64_encode(converDesc->in, converDesc->inLen, encodeOut);
            logDebug("base64_encode encodeOut:%s", encodeOut);

            hyLinkConver(converDesc->modelId, converDesc->hyKey, converDesc->dir, (char *)encodeOut, encodeOutlen, (char *)converDesc->out, &converDesc->outLen);
            free(encodeOut);
        }
    }
    else //down
    {
        if (converDesc->hyKeyPrivate == 0)
        {
            long num;
            strToNum((char *)converDesc->in, 10, &num);
            *converDesc->out = num;
            converDesc->outLen = 1;
        }
        else
        {
            char privatekeyLen = 0;
            if (strncmp(converDesc->hyKey, "SceName_", strlen("SceName_")) == 0 || strncmp(converDesc->hyKey, "ScePhoto_", strlen("ScePhoto_")) == 0)
            {
                char *pos = strchr(converDesc->hyKey, '_');
                long num;
                strToNum(pos + 1, 10, &num);
                converDesc->out[0] = num >> 8;
                converDesc->out[1] = num & 0xff;
                converDesc->out += 2;
                privatekeyLen = 2;
            }
            logDebug("in %s,%d", converDesc->in, converDesc->inLen);
            hyLinkConver(converDesc->modelId, converDesc->hyKey, converDesc->dir, (char *)converDesc->in, converDesc->inLen, (char *)converDesc->out, &converDesc->outLen);
            logDebug("out %s,%d", converDesc->out, converDesc->outLen);

            void *decodeOut = malloc(BASE64_DECODE_OUT_SIZE(converDesc->outLen));
            int decodeOutlen = base64_decode((char *)converDesc->out, converDesc->outLen, decodeOut);
            logPrintfHex("base64_decode:", decodeOut, decodeOutlen);
            memcpy(converDesc->out, decodeOut, decodeOutlen);
            converDesc->outLen = decodeOutlen + privatekeyLen;
            free(decodeOut);
        }
    }

    return converDesc->outLen;
}
