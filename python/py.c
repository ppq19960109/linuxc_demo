
#include <stdio.h>
#include <Python.h>

int main(int argc, char **argv)
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
    int ret,ret2;
    char* str;
    int val[5];
    // PyArg_Parse
    PyArg_ParseTuple(pRetVal, "is(ii)", &ret, &str,&val[0],&val[1]);
    printf("ret %d,%s,%d,%d\n",ret,str,val[0],val[1]);
    Py_DECREF(pRetVal);

    Py_Finalize();
    return 0;
}

int main1()
{
    printf("main start");
  
    return 0;
}