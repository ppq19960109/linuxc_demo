
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

int hy_conversion(char *modelId, char *key, char *dir, char *value, int *valueLen)
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
        return -1;
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
        return -1;
    }

    PyObject *pArgs = NULL;
    pArgs = PyTuple_New(2);

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", value));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", *valueLen));
    //调用函数
    PyObject *pRetVal = PyObject_CallObject(pfunc, pArgs);
    if (!pRetVal)
    {
        printf("PyObject_CallObject error\n");
        return -1;
    }

    // PyArg_Parse
    PyArg_ParseTuple(pRetVal, "si", &value, valueLen);
    printf("function RetVal:%s,%d\n", value, *valueLen);

    Py_DECREF(pRetVal);
    Py_DECREF(pArgs);
    Py_DECREF(pfunc);
    Py_DECREF(pmodule);
    Py_DECREF(pname);
    Py_Finalize();
    return 0;
}

int main(int argc, char **argv)
{
    printf("main start\n");
    // pytest();
    char str[16] = "valhi";
    int len = strlen(str);
    hy_conversion("HY123", "name", "up", str, &len);
    printf("hy ret str:%s,len:%d\n", str, len);
    return 0;
}