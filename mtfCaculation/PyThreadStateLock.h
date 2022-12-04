#ifndef PYTHREADSTATELOCK_H
#define PYTHREADSTATELOCK_H

//将全局解释器锁和线程的相关操作用类封装
#include "Python.h"

class PyThreadStateLock
{
public:
    PyThreadStateLock(void) { gstate = PyGILState_Ensure(); }
    ~PyThreadStateLock(void) { PyGILState_Release(gstate); }

private:
    PyGILState_STATE gstate;
};

#endif // PYTHREADSTATELOCK_H
