#pragma once
//#include <Python.h>
#pragma push_macro("slots")
#undef slots
#include <Python.h>
//#include "python.h"
#pragma pop_macro("slots")
#include <QDebug>
#include <numpy/arrayobject.h>
#include "PyThreadStateLock.h"

void PythonInit()
{
    if (!Py_IsInitialized()) {
        // 1.初始化Python解释器，这是调用操作的第一步
        Py_Initialize();
        if (!Py_IsInitialized()) {
            qDebug("Initial Python failed!");
        } else {
            PyEval_InitThreads();

            PyRun_SimpleString("import sys");
            PyRun_SimpleString(
                "sys.path.append('D:/projects/Vision_Tools_Set/MTFTool/mtfCaculation')");
            PyEval_ReleaseThread(PyThreadState_Get());
        }
    }
}

QVector<int> callPython(const std::vector<std::vector<std::vector<double>>>& img,
                        const std::vector<std::vector<double>>& information,
                        const std::string& saveFileName, const std::string& imgFileName,
                        const double& pixelSize)
{
    class PyThreadStateLock PyThreadLock;
    Py_SetPythonHome(L"/Users/wanncye/opt/anaconda3/envs/qt/bin");
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/Users/wanncye/Desktop/MTF/mtfCaculation')");


    PyObject* pModule = PyImport_ImportModule("example");
    if (pModule == nullptr) {
        qDebug() << "Module example not found";
        // return false;
    }

    PyObject* pFunc = PyObject_GetAttrString(pModule, "test_main");

    if (pFunc == nullptr) {
        qDebug() << "Module test_main not found";
        // return false;
    }

    Py_ssize_t Len = img.size();
    PyObject* imgArray = PyTuple_New(Len);
    for (Py_ssize_t i = 0; i < Len; i++) {
        Py_ssize_t rowLen = img[i].size();
        PyObject* rowItem = PyTuple_New(rowLen);
        for (Py_ssize_t j = 0; j < rowLen; j++) {
            Py_ssize_t colLen = img[i][j].size();
            PyObject* item = PyTuple_New(colLen);
            for (Py_ssize_t k = 0; k < colLen; k++)
                PyTuple_SET_ITEM(item, k, PyFloat_FromDouble(img[i][j][k]));
            PyTuple_SET_ITEM(rowItem, j, item);
        }
        PyTuple_SET_ITEM(imgArray, i, rowItem);
    }

    Py_ssize_t rowLen = information.size();
    PyObject* informationArray = PyTuple_New(rowLen);
    for (Py_ssize_t i = 0; i < rowLen; i++) {
        Py_ssize_t colLen = information[i].size();
        PyObject* item = PyTuple_New(colLen);
        for (Py_ssize_t j = 0; j < colLen; j++)
            PyTuple_SET_ITEM(item, j, PyFloat_FromDouble(information[i][j]));
        PyTuple_SET_ITEM(informationArray, i, item);
    }

    PyObject* fileName = PyTuple_New(2);
    PyTuple_SET_ITEM(fileName, 0, PyBytes_FromString(saveFileName.data()));
    PyTuple_SET_ITEM(fileName, 1, PyBytes_FromString(imgFileName.data()));

    PyObject* pixelWidth = PyTuple_New(1);
    PyTuple_SET_ITEM(pixelWidth, 0, PyFloat_FromDouble(pixelSize));

    PyObject* pArgs = PyTuple_New(4);
    PyTuple_SetItem(pArgs, 0, imgArray);
    PyTuple_SetItem(pArgs, 1, informationArray);
    PyTuple_SetItem(pArgs, 2, fileName);
    PyTuple_SetItem(pArgs, 3, pixelWidth);
    PyObject* pRet = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    QVector<int> errRoiId;
    if (PyList_Check(pRet)) {
        int sizeOfList = PyList_Size(pRet);
        for (int i = 0; i < sizeOfList; i++) {
            PyObject* listItem = PyList_GetItem(pRet, i);
            errRoiId.push_back(PyLong_AsLong(listItem));
            Py_DECREF(listItem);
        }
    }
    qDebug() << "errRoiId.size:" << errRoiId.size();
    // int res = 0;
    // PyArg_Parse(pRet, "i", &res);
    Py_DECREF(pRet);
    // Py_Finalize();

    return errRoiId;
}
