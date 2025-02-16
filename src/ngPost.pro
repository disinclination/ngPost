CONFIG  += use_hmi

use_hmi {
    QT += gui
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

    DEFINES += __USE_HMI__
}
else {
    QT -= gui
    CONFIG += console
}

include(ngPost.pri)

use_hmi {
SOURCES += \
    hmi/AutoPostWidget.cpp \
    hmi/CheckBoxCenterWidget.cpp \
    hmi/PostingWidget.cpp \
    hmi/SignedListWidget.cpp \
    hmi/MainWindow.cpp

HEADERS += \
    hmi/AutoPostWidget.h \
    hmi/CheckBoxCenterWidget.h \
    hmi/PostingWidget.h \
    hmi/SignedListWidget.h \
    hmi/MainWindow.h

FORMS += \
    hmi/AutoPostWidget.ui \
    hmi/MainWindow.ui \
    hmi/PostingWidget.ui
}

FORMS += \
    hmi/PathSettingsWidget.ui \
    hmi/SettingsWidget.ui

HEADERS += \
    hmi/PathSettingsWidget.h \
    hmi/SettingsWidget.h

SOURCES += \
    hmi/PathSettingsWidget.cpp \
    hmi/SettingsWidget.cpp
