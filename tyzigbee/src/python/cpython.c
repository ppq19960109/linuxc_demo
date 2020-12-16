#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "logFunc.h"
#include "commonFunc.h"
#include "base64.h"
#include "python3.6m/Python.h"

int hyLinkConver(const char *modelId, const char *key, const char *dir, char *in, int inLen, char *out, int *outLen)
{
    char hkey[24] = {0};
    char *pos = strchr(key, '_');
    if (pos != NULL)
    {
        strncpy(hkey, key, pos - key + 1);
    }
    else
    {
        strcpy(hkey, key);
    }

    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyRun_SimpleString("sys.path.append('./pyfile')");
    //导入python源文件
    PyObject *pname = NULL;
    pname = PyUnicode_FromString(modelId);

    PyObject *pmodule = NULL;
    pmodule = PyImport_Import(pname);
    if (pmodule == NULL)
    {
        printf("can not find %s.py\n", modelId);
        goto fail_pmodule;
    }
    //调用函数
    char func[33];
    sprintf(func, "%s%s", hkey, dir);
    printf("%s.py Function name:%s\n", modelId, func);
    PyObject *pfunc = PyObject_GetAttrString(pmodule, func);
    if (pfunc == NULL)
    {
        printf("can not find function %s\n", func);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    PyObject *pRetVal = NULL;

    pArgs = PyTuple_New(2);

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", in));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", inLen));
    //调用函数
    pRetVal = PyObject_CallObject(pfunc, pArgs);
    if (pRetVal == NULL)
    {
        printf("PyObject_CallObject error\n");
        goto fail_pRetVal;
    }

    // PyArg_Parse
    char *pRsp;
    PyArg_ParseTuple(pRetVal, "si", &pRsp, outLen);

    strcpy(out, pRsp);

    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return 0;

fail_pRetVal:
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
fail_pfunc:
    Py_DECREF(pmodule);
fail_pmodule:
    Py_DECREF(pname);
    Py_Finalize();
    return -1;
}

int pythonTest()
{
    printf("main start\n");

    char in[16] = {"哈"};
    char out[16] = {0};
    int outLen = 0;
    hyLinkConver("_TZE200_twuagcv5", "SceName_", "down", in, strlen(in), out, &outLen);
    printf("down:%s,%d\n", out, outLen);

    int decodelen = strlen(out);
    void *decode_out = malloc(BASE64_DECODE_OUT_SIZE(decodelen));
    int decode_outlen = base64_decode(out, decodelen, decode_out);
    logPrintfHex("base64_decode:", decode_out, decode_outlen);

    int encodelen = decode_outlen;
    void *encode_out = malloc(BASE64_ENCODE_OUT_SIZE(encodelen));
    int encode_outlen = base64_encode(decode_out, encodelen, encode_out);
    logDebug("encode_out:%s,%d", encode_out, encode_outlen);

    hyLinkConver("_TZE200_twuagcv5", "SceName_", "up", out, strlen(out), out, &outLen);
    printf("up:%s,%d,%d\n", out, outLen, strlen(out));
    free(decode_out);
    free(encode_out);
    return 0;
}