QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

RC_ICONS = appicon.ico

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
    dboperate.cpp \
    main.cpp \
    mytcpserver.cpp \
    mytcpsocket.cpp \
    protocol.cpp \
    tcpserver.cpp \
    conditionquery.cpp \
    recordlog.cpp \
    querylog.cpp \
    rsakey.cpp \
    padding.cpp \
    aes.cpp \
    filethread.cpp

HEADERS += \
    dboperate.h \
    mytcpserver.h \
    mytcpsocket.h \
    protocol.h \
    tcpserver.h \
    conditionquery.h \
    recordlog.h \
    querylog.h \
    rsakey.h \
    padding.h \
    aes.h \
    filethread.h

FORMS += \
    tcpserver.ui \
    conditionquery.ui \
    querylog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    config.qrc \
    icon.qrc

DISTFILES +=

INCLUDEPATH += F:/OpenSSL-Win64/include
LIBS += F:/OpenSSL-Win64/lib/libeay32.lib F:/OpenSSL-Win64/lib/ssleay32.lib
