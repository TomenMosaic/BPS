QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
QT += network websockets
QT += sql
QT += script
QT += axcontainer
QT += serialbus

TARGET      = bps
TEMPLATE    = app

HEADERS     += \
            global.h \
            head.h

SOURCES     += main.cpp \
    global.cpp

RESOURCES   += other/main.qrc
RESOURCES   += $$PWD/../core_qss/qss.qrc

RC_FILE += $$PWD/appicon.rc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/form
include ($$PWD/form/form.pri)

#INCLUDEPATH += $$PWD/model
#include ($$PWD/model/model.pri)

INCLUDEPATH += $$PWD/../core_base
include ($$PWD/../core_base/core_base.pri)

INCLUDEPATH += $$PWD/tools
include ($$PWD/tools/tools.pri)

INCLUDEPATH += $$PWD/bll
include ($$PWD/bll/bll.pri)

INCLUDEPATH += $$PWD/../algorithm
include ($$PWD/../algorithm/algorithm.pri)

# QXlsx
INCLUDEPATH += $$PWD/../plugin/QXlsx-1.4.6/QXlsx/header
include ($$PWD/../plugin/QXlsx-1.4.6/QXlsx/QXlsx.pri)

# qt speesh
QT += texttospeech

# boost stacktrace
# LIBS += -ldbghelp -lole32
INCLUDEPATH += D:\Projects\boost_1_84_0
LIBS += -LE:\Projects\boost_1_84_0\stage\lib
INCLUDEPATH += D:\Projects\libbacktrace-master
LIBS += -LD:\Projects\libbacktrace-master\.libs -lbacktrace
## unix: DEFINES += BOOST_STACKTRACE_USE_ADDR2LINE
# DEFINES += BOOST_STACKTRACE_USE_BACKTRACE


# QMAKE_CXXFLAGS += -g


