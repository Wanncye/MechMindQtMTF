QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11, sdk_no_version_check
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    danleimtfcalc.cpp \
    labelpaintertool.cpp \
    lnxmtfprototype.cpp \
    main.cpp \
    mainwindow.cpp \
    mtfcalc.cpp \
    ROISelectionWindow.cpp

HEADERS += \
    danleimtfcalc.h \
    danleimtfcalc.h \
    labelpaintertool.h \
    lnxmtfprototype.h \
    mainwindow.h \
    mtfcalc.h \
    ROISelectionWindow.h\
    mtfCaculation/PyThreadStateLock.h \
    mtfCaculation/callPythonScripts.h

FORMS += \
    danleimtfcalc.ui \
    danleimtfcalc.ui \
    ROISelectionWindow.ui \
    lnxmtfprototype.ui \
    mainwindow.ui \
    mtfcalc.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



macx: LIBS += -L$$PWD/../../opt/anaconda3/envs/qt/lib/ -lpython3.7m

INCLUDEPATH += $$PWD/../../opt/anaconda3/envs/qt/include/python3.7m
INCLUDEPATH += $$PWD/../../opt/anaconda3/envs/qt/lib/python3.7/site-packages/numpy/core/include

DEPENDPATH += $$PWD/../../opt/anaconda3/envs/qt/include

DISTFILES += \
    mtfCaculation/example.py \
    mtfCaculation/mtf.py
