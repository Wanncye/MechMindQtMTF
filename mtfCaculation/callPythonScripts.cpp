#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#include <QDebug>
#include <numpy/arrayobject.h>
#include "PyThreadStateLock.h"

#define print(val) qDebug() << #val << val

void PythonInit()
{
    if (!Py_IsInitialized()) {
        // 1.初始化Python解释器，这是调用操作的第一步
        Py_SetPythonHome(L"C:/Users/mech-mind/anaconda3/envs/qt");
        Py_Initialize();
        if (!Py_IsInitialized()) {
            print("Initial Python failed!");
        } else {
            print("Initial sucess!");
            PyEval_InitThreads();
            PyRun_SimpleString("import sys");
            PyRun_SimpleString("sys.path.append('D:/MechMindQtMTF/mtfCaculation')");
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
    //    Py_SetPythonHome(L"/Users/wanncye/opt/anaconda3/envs/qt/bin"); // MAC
    Py_SetPythonHome(L"C:/Users/mech-mind/anaconda3/envs/qt"); // Windows
    Py_Initialize();

    PyRun_SimpleString("import sys");
    //    PyRun_SimpleString("sys.path.append('/Users/wanncye/Desktop/MTF/mtfCaculation')"); // MAC
    PyRun_SimpleString("sys.path.append('D:/MechMindQtMTF/mtfCaculation')"); // Windows

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
    Py_DECREF(pRet);

    return errRoiId;
}

QVector<int> callPythonReturnMTFData(const std::vector<std::vector<std::vector<double>>>& img,
                                     const std::vector<std::vector<double>>& information,
                                     const std::string& saveFileName,
                                     const std::string& imgFileName, const double& pixelSize,
                                     std::vector<std::vector<double>>& mtfData,
                                     std::vector<std::vector<double>>& mtfControlData)
{
    class PyThreadStateLock PyThreadLock;
    print("callPythonReturnMTFData");
    //    Py_SetPythonHome(L"/Users/wanncye/opt/anaconda3/envs/qt/bin"); // MAC
    Py_SetPythonHome(L"C:/Users/mech-mind/anaconda3/envs/qt"); // Windows
    Py_Initialize();

    PyRun_SimpleString("import sys");
    //    PyRun_SimpleString("sys.path.append('/Users/wanncye/Desktop/MTF/mtfCaculation')"); // MAC
    PyRun_SimpleString("sys.path.append('D:/MechMindQtMTF/mtfCaculation')"); // Windows

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
    print(Len);
    print(imgArray);
    for (Py_ssize_t i = 0; i < Len; i++) {
        Py_ssize_t rowLen = img[i].size();
        PyObject* rowItem = PyTuple_New(rowLen);
        for (Py_ssize_t j = 0; j < rowLen; j++) {
            Py_ssize_t colLen = img[i][j].size();
            PyObject* item = PyTuple_New(colLen);
            for (Py_ssize_t k = 0; k < colLen; k++) {
                PyTuple_SET_ITEM(item, k, PyFloat_FromDouble(img[i][j][k]));
            }
            print("before rowItem PyTuple_SET_ITEM");
            print(j);
            PyTuple_SET_ITEM(rowItem, j, item);
            print("after rowItem PyTuple_SET_ITEM");
        }
        print("before imgArray PyTuple_SET_ITEM");
        PyTuple_SET_ITEM(imgArray, i, rowItem);
        print("after imgArray PyTuple_SET_ITEM");
    }
    print("--------debug-----------");
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
    PyObject *errorRet = nullptr, *dataRet = nullptr;
    if (PyTuple_Check(pRet)) {
        print("return is tuple");
        errorRet = PyTuple_GetItem(pRet, 0);
        int sizeOfList = PyList_Size(errorRet);
        for (int i = 0; i < sizeOfList; i++) {
            PyObject* listItem = PyList_GetItem(errorRet, i);
            errRoiId.push_back(PyLong_AsLong(listItem));
            Py_DECREF(listItem);
        }
        print(errRoiId.size());

        dataRet = PyTuple_GetItem(pRet, 1);
        int roiNum = PyList_Size(dataRet);
        print(roiNum);
        mtfData.resize(roiNum);
        mtfControlData.resize(roiNum);
        for (int i = 0; i < roiNum; ++i) {
            PyObject* roiMTFData = PyList_GetItem(dataRet, i);
            int roiMTFLen = PyList_Size(roiMTFData);
            int controlInformationLen = 11;
            std::vector<double> controlVec(controlInformationLen, 0.);
            for (int j = 0; j < controlInformationLen; ++j) {
                PyObject* listElement = PyList_GetItem(roiMTFData, j);
                controlVec[j] = PyFloat_AsDouble(listElement);
                Py_DECREF(listElement);
            }
            mtfControlData[i] = controlVec;
            print(mtfControlData.size());
            std::vector<double> roiMTFVec(roiMTFLen - controlInformationLen, 0.);
            for (int j = controlInformationLen; j < roiMTFLen; ++j) {
                PyObject* listElement = PyList_GetItem(roiMTFData, j);
                roiMTFVec[j - controlInformationLen] = PyFloat_AsDouble(listElement);
                Py_DECREF(listElement);
            }
            mtfData[i] = roiMTFVec;
            Py_DECREF(roiMTFData);
        }
        print(mtfData.size());
    } else if (PyList_Check(pRet)) {
        // list的情况
        print("return is List");
    } else if (PyNumber_Check(pRet)) {
        // 数字的情况
        print("return is Number");
    }
    Py_DECREF(pRet);
    Py_DECREF(errorRet);
    Py_DECREF(dataRet);
    Py_DECREF(imgArray);
    print("---------------------return callPythonReturnMTFData-------------------");
    return errRoiId;
}
