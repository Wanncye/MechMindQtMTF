QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += charts
QT += core
QT += gui

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
    ProfilerChartView.cpp \
    danleimtfcalc.cpp \
    labelpaintertool.cpp \
    lnxmtfprototype.cpp \
    main.cpp \
    mainwindow.cpp \
    mtfCaculation/callPythonScripts.cpp \
    mtfcalc.cpp \
    ROISelectionWindow.cpp

HEADERS += \
    ProfilerChartView.h \
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


# MAC的环境
#macx: LIBS += -L$$PWD/../../opt/anaconda3/envs/qt/lib/ -lpython3.7m

#INCLUDEPATH += $$PWD/../../opt/anaconda3/envs/qt/include/python3.7m
#INCLUDEPATH += $$PWD/../../opt/anaconda3/envs/qt/lib/python3.7/site-packages/numpy/core/include
#DEPENDPATH += $$PWD/../../opt/anaconda3/envs/qt/include

# windows下的环境
win32:CONFIG(release, debug|release): LIBS += -LC:/Users/mech-mind/anaconda3/envs/qt/libs/ -lpython37
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/Users/mech-mind/anaconda3/envs/qt/libs/ -lpython37d
else:unix: LIBS += -LC:/Users/mech-mind/anaconda3/envs/qt/libs/ -lpython37

INCLUDEPATH += C:/Users/mech-mind/anaconda3/envs/qt/include
INCLUDEPATH += C:/Users/mech-mind/anaconda3/envs/qt/Lib/site-packages/numpy/core/include
DEPENDPATH += C:/Users/mech-mind/anaconda3/envs/qt/libs


DISTFILES += \
    mtfCaculation/example.py \
    mtfCaculation/mtf.py
