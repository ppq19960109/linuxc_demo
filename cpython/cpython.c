#include <stdio.h>
#include <string.h>
#include <Python.h>

int pytest(void)
{
    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("print('Python3 start')");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

    //导入python源文件
    PyObject *pname = NULL;
    pname = PyUnicode_FromString("pytest");

    PyObject *pmodule = NULL;
    pmodule = PyImport_Import(pname);
    if (pmodule == NULL)
    {
        printf("can not find .py\n");
        return -1;
    }
    //调用函数
    PyObject *pfunc = PyObject_GetAttrString(pmodule, "add");
    if (!pfunc)
    {
        printf("can not find function trans()\n");
        return -1;
    }

    PyObject *pArgs = NULL;
    pArgs = PyTuple_New(2);

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", 1));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", "hello"));

    //调用函数
    PyObject *pRetVal = PyObject_CallObject(pfunc, pArgs);
    int ret;
    char *str;
    int val[5];
    // PyArg_Parse
    PyArg_ParseTuple(pRetVal, "is(ii)", &ret, &str, &val[0], &val[1]);
    printf("ret %d,%s,%d,%d\n", ret, str, val[0], val[1]);

    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return 0;
}

int hy_conversion(char *modelId, char *key, char *dir, char *value, int valueLen, char *out, int *outLen)
{
    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("print('hy_conversion Python3 start')");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

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
    char func[64];
    strcpy(func, key);
    strcpy(&func[strlen(key)], dir);
    printf("%s.py Function name:%s\n", modelId, func);
    PyObject *pfunc = PyObject_GetAttrString(pmodule, func);
    if (!pfunc)
    {
        printf("can not find function %s\n", func);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    pArgs = PyTuple_New(2);

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", value));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", valueLen));
    //调用函数
    PyObject *pRetVal = PyObject_CallObject(pfunc, pArgs);
    if (!pRetVal)
    {
        printf("PyObject_CallObject error\n");
        goto fail_pRetVal;
    }
    char *str;
    // PyArg_Parse
    PyArg_ParseTuple(pRetVal, "si", &str, outLen);
    printf("function RetVal:%s,%d\n", str, *outLen);
    strcpy(out, str);

    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return 0;

fail_pRetVal:
    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
fail_pfunc:
    Py_DECREF(pfunc);
fail_pmodule:
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return -1;
}

int hy_conversion_updown(char *modelId, char *key, char *dir, char *str, int strLen, char *num, int *numLen)
{
    Py_Initialize(); //开始Python解释器

    // PyRun_SimpleString("print('hy_conversion Python3 start')");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

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
    char func[32];
    sprintf(func, "%s%s", key, dir);
    // printf("%s.py Function name:%s\n", modelId, func);
    PyObject *pfunc = PyObject_GetAttrString(pmodule, func);
    if (!pfunc)
    {
        printf("can not find function %s\n", func);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    PyObject *pRetVal = NULL;
    int i;
    int buf;
    if (strcmp("up", dir) == 0)
    {
        pArgs = PyTuple_New(2);
        buf = 0;
        for (i = 0; i < strLen; ++i)
        {
            buf += str[i] << 8 * i;
        }
        PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", buf));
        PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", strLen));
        //调用函数
        pRetVal = PyObject_CallObject(pfunc, pArgs);
        if (!pRetVal)
        {
            printf("PyObject_CallObject error\n");
            goto fail_pRetVal;
        }

        char *out;
        // PyArg_Parse
        PyArg_ParseTuple(pRetVal, "si", &out, numLen);
        // printf("function RetVal:%s,%d\n", out, *strLen);
        strcpy(num, out);
    }
    else if (strcmp("down", dir) == 0)
    {
        pArgs = PyTuple_New(2);

        PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", str));
        PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", strLen));
        //调用函数
        pRetVal = PyObject_CallObject(pfunc, pArgs);
        if (!pRetVal)
        {
            printf("PyObject_CallObject error\n");
            goto fail_pRetVal;
        }

        // PyArg_Parse
        PyArg_ParseTuple(pRetVal, "ii", &buf, numLen);

        for (i = 0; i < *numLen; ++i)
        {
            num[i] = buf >> i;
        }
        // printf("function RetVal:%d,%d\n", num[0],*numLen);
    }
    else
    {
        goto fail_pfunc;
    }

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

int hy_conversion2_updown(char *modelId, char *dir, char *in, int inLen, char *inout, int *inoutLen, char *out, int *outLen)
{
    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

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

    PyObject *pfunc = PyObject_GetAttrString(pmodule, dir);
    if (!pfunc)
    {
        printf("can not find function %s\n", dir);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    PyObject *pRetVal = NULL;
    PyObject *PyList = NULL;
    if (strcmp("up", dir) == 0)
    {
        PyList = PyList_New(inLen);
        for (int i = 0; i < inLen; i++)
            PyList_SetItem(PyList, i, PyLong_FromLong(in[i]));

        pArgs = PyTuple_New(1);

        PyTuple_SetItem(pArgs, 0, PyList);

        //调用函数
        pRetVal = PyObject_CallObject(pfunc, pArgs);
        if (!pRetVal)
        {
            printf("PyObject_CallObject error\n");

            goto fail_pRetVal;
        }

        if (PyTuple_Check(pRetVal))
        {                                           //检查是否为List对象
            int SizeOfList = PyTuple_Size(pRetVal); //List对象的大小，这里SizeOfList = 2
            printf("返回的结果result：\n");
            for (int i = 0; i < SizeOfList; i++)
            {
                PyObject *ListItem = PyTuple_GetItem(pRetVal, i); //获取List对象中的每一个元素
                int NumOfItems = PyList_Size(ListItem);           //List对象子元素的大小，这里NumOfItems = 3
                char *pyout;
                if (i == 0)
                {
                    *inoutLen = NumOfItems;
                    pyout = inout;
                }
                else
                {
                    *outLen = NumOfItems;
                    pyout = out;
                }

                for (int j = 0; j < NumOfItems; j++)
                {
                    PyObject *Item = PyList_GetItem(ListItem, j); //遍历List对象中子元素中的每个元素
                    int result;
                    PyArg_Parse(Item, "i", &result); //i表示转换成int型变量
                    printf("%x ", result);
                    pyout[j] = result;
                    // Py_DECREF(Item); //释放空间
                }
                Py_DECREF(ListItem); //释放空间
            }
            printf("\n");
        }
    }
    else if (strcmp("down", dir) == 0)
    {
        PyList = PyList_New(inLen);
        for (int i = 0; i < inLen; i++)
            PyList_SetItem(PyList, i, PyLong_FromLong(in[i]));

        PyObject *PyList2 = PyList_New(inLen);
        for (int i = 0; i < *inoutLen; i++)
            PyList_SetItem(PyList2, i, PyLong_FromLong(inout[i]));
        pArgs = PyTuple_New(2);

        PyTuple_SetItem(pArgs, 0, PyList);
        PyTuple_SetItem(pArgs, 1, PyList2);

        //调用函数
        pRetVal = PyObject_CallObject(pfunc, pArgs);
        if (!pRetVal)
        {
            printf("PyObject_CallObject error\n");
            goto fail_pRetVal;
        }

        if (PyTuple_Check(pRetVal))
        {                                           //检查是否为List对象
            int SizeOfList = PyTuple_Size(pRetVal); //List对象的大小，这里SizeOfList = 2
            printf("返回的结果result：\n");

            PyObject *ListItem = PyTuple_GetItem(pRetVal, 0); //获取List对象中的每一个元素
            int NumOfItems = PyList_Size(ListItem);           //List对象子元素的大小，这里NumOfItems = 3
            char *pyout;

            *outLen = NumOfItems;
            pyout = out;

            for (int j = 0; j < NumOfItems; j++)
            {
                PyObject *Item = PyList_GetItem(ListItem, j); //遍历List对象中子元素中的每个元素
                int result;
                PyArg_Parse(Item, "i", &result); //i表示转换成int型变量
                printf("%x ", result);
                pyout[j] = result;
                // Py_DECREF(Item); //释放空间
            }
            Py_DECREF(ListItem); //释放空间

            printf("\n");
        }
    }
    else
    {
        goto fail_pArgs;
    }
    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return 0;

fail_pRetVal:
    Py_DECREF(pArgs);
fail_pArgs:
    Py_DECREF(pfunc);
fail_pfunc:
    Py_DECREF(pmodule);
fail_pmodule:
    Py_DECREF(pname);
    Py_Finalize();
    return -1;
}

int hy_conversion3_updown(char *modelId, char *key, char *dir, char *str, int strLen, char *strOut, int *strOutLen)
{
    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

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
    char func[32];
    sprintf(func, "%s%s", key, dir);
    // printf("%s.py Function name:%s\n", modelId, func);
    PyObject *pfunc = PyObject_GetAttrString(pmodule, func);
    if (!pfunc)
    {
        printf("can not find function %s\n", func);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    PyObject *pRetVal = NULL;

    pArgs = PyTuple_New(2);

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", str));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", strLen));
    //调用函数
    pRetVal = PyObject_CallObject(pfunc, pArgs);
    if (!pRetVal)
    {
        printf("PyObject_CallObject error\n");
        goto fail_pRetVal;
    }

    char *pRsp;
    // PyArg_Parse
    PyArg_ParseTuple(pRetVal, "si", &pRsp, strOutLen);
    // printf("function RetVal:%s,%d\n", pRsp[5], *strOutLen);
    strcpy(strOut, pRsp);

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

int hy_conversion4_updown(char *modelId, char *dir, char *in, int inLen, char *out, int *outLen)
{
    Py_Initialize(); //开始Python解释器

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

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

    PyObject *pfunc = PyObject_GetAttrString(pmodule, dir);
    if (!pfunc)
    {
        printf("can not find function %s\n", dir);
        goto fail_pfunc;
    }

    PyObject *pArgs = NULL;
    PyObject *pRetVal = NULL;
    PyObject *PyList = NULL;

    PyList = PyTuple_New(inLen);
    for (int i = 0; i < inLen; i++)
        PyTuple_SetItem(PyList, i, PyLong_FromLong(in[i]));

    pArgs = PyTuple_New(1);

    PyTuple_SetItem(pArgs, 0, PyList);

    //调用函数
    pRetVal = PyObject_CallObject(pfunc, pArgs);
    if (!pRetVal)
    {
        printf("PyObject_CallObject error\n");

        goto fail_pRetVal;
    }

    if (PyTuple_Check(pRetVal))
    {
        int result;                             //检查是否为List对象
        int SizeOfList = PyTuple_Size(pRetVal); //List对象的大小，这里SizeOfList = 2
        *outLen = SizeOfList;
        printf("返回的结果result：\n");
        for (int i = 0; i < SizeOfList; i++)
        {
            PyObject *Item = PyTuple_GetItem(pRetVal, i); //获取List对象中的每一个元素
            PyArg_Parse(Item, "i", &result);
            printf("%x ", result);
            out[i] = result;

            // Py_DECREF(Item); //释放空间
        }
        printf("\n");
    }

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

int main(int argc, char **argv)
{
    printf("main start\n");

    char str[10] = {0x56, 0xde, 0x5b, 0xb6};
    int len = 4;
    char num[9] = {0};
    int numlen = 1;

    char out2[9] = {0};
    int out2len = 0;
    // hy_conversion("HY123", "name", "up", str, len, out, &outlen);
    if (1)
    {
        hy_conversion4_updown("TS0002", "up", str, len, out2, &out2len);
        printf("%x,%d,%d,%d\n", (unsigned char)out2[0], (unsigned char)out2[1], (unsigned char)out2[2], out2len);

        // hy_conversion2_updown("TS0002", "up", str, len, out1, &out1len, out2, &out2len);
        // printf("%x,%d,%d,%d\n", (unsigned char)out1[2], out1len, out2[2], out2len);
        // hy_conversion2_updown("TS0002", "down", str, len, out1, &out1len, out2, &out2len);
        // printf("%d,%d\n", out2[2], out2len);

        // hy_conversion3_updown("pytest", "hy_", "down", str, len, num, &numlen);
        // printf("%d,%d,%d,%d,len:%d\n", num[0], num[1], num[2], num[3], numlen);
    }
    else
    {
        hy_conversion_updown("TS0002", "Switch_1_", "down", str, len, num, &numlen);
        int in = 0;
        for (int i = 0; i < numlen; ++i)
        {
            in += num[i] << 8 * i;
        }
        printf("num:%d,numlen:%d\n", in, numlen);
    }
    return 0;
}