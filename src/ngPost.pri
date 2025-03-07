QT += core network

DEFINES += "APP_VERSION=\"5.0\""

INCLUDEPATH += $$PWD
TARGET = ngPost
TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle

DEFINES += __USE_CONNECTION_TIMEOUT__
DEFINES += __COMPUTE_IMMEDIATE_SPEED__

DEFINES += __USE_TMP_RAM__

DEFINES += __RELEASE_ARTICLES_WHEN_CON_FAILS__


# macro for debuging posting on multiple provides (no need anymore)
#DEFINES -= __DISP_ARTICLE_SERVER__


TRANSLATIONS = lang/ngPost_en.ts lang/ngPost_fr.ts lang/ngPost_es.ts lang/ngPost_de.ts\
               lang/ngPost_nl.ts lang/ngPost_pt.ts lang/ngPost_zh.ts

win32: {
    LIBS += -luser32 -ladvapi32
    RC_ICONS = ngPost.ico
    RC_FILE = resources/version.rc
    # Include console only if not using HMI (GUI)
    CONFIG += console
}

macx: {
    ICON = ngPost.icns
    CONFIG += app_bundle

    # Ensure par2 and parpar exist externally before adding them
    system(test -f /usr/local/bin/par2) {
        ExtraFiles.files = /usr/local/bin/par2 /usr/local/bin/parpar $$PWD/LICENSE
        ExtraFiles.path = Contents/MacOS
        QMAKE_BUNDLE_DATA += ExtraFiles
    }
}

CONFIG(debug, debug|release) :{
    DEFINES += __DEBUG__

    DEFINES += LOG_CONNECTION_STEPS
    DEFINES -= LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
    DEFINES += LOG_NEWS_AUTH
    DEFINES -= LOG_NEWS_DATA
    DEFINES += LOG_CONSTRUCTORS

    DEFINES -= __SAVE_ARTICLES__
}
else {
    # In release mode, remove all qDebugs !
    DEFINES += QT_NO_DEBUG_OUTPUT
}



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
        ArticleBuilder.cpp \
        FileUploader.cpp \
        FoldersMonitorForNewFiles.cpp \
        NgPost.cpp \
        NntpCheckCon.cpp \
        NntpConnection.cpp \
        NzbCheck.cpp \
        Poster.cpp \
        PostingJob.cpp \
        main.cpp \
        nntp/Nntp.cpp \
        nntp/NntpArticle.cpp \
        nntp/NntpFile.cpp \
        utils/CmdOrGuiApp.cpp \
        utils/Yenc.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ArticleBuilder.h \
    FileUploader.h \
    FoldersMonitorForNewFiles.h \
    NgPost.h \
    NntpCheckCon.h \
    NntpConnection.h \
    NzbCheck.h \
    Poster.h \
    PostingJob.h \
    nntp/Nntp.h \
    nntp/NntpArticle.h \
    nntp/NntpFile.h \
    nntp/NntpServerParams.h \
    utils/CmdOrGuiApp.h \
    utils/Macros.h \
    utils/PureStaticClass.h \
    utils/Yenc.h



RESOURCES += \
    resources/resources.qrc \
    resources/version.rc

