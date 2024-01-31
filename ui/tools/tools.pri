INCLUDEPATH += $$PWD

#include($$PWD/basewindow/basewindow.pri)

HEADERS += \
    $$PWD/ExcelReader.h \
    $$PWD/ModbusClient.h \
    $$PWD/configmanage.h \
    $$PWD/convert.h \
    $$PWD/globalhook.h \
    $$PWD/log.h

SOURCES += \
    $$PWD/configmanage.cpp \
    $$PWD/globalhook.cpp \
    $$PWD/log.cpp \

