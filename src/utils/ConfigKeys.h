#ifndef CONFIGKEYS_H
#define CONFIGKEYS_H

#include <QString>

class ConfigKeys {
public:
    static const QString LANG;
    static const QString NZBPATH;
    static const QString MONITOR_SEC_DELAY_SCAN;
    static const QString INPUTDIR;
    static const QString POST_HISTORY;
    static const QString GROUPS;
    static const QString GROUP_POLICY;
    static const QString GEN_FROM;
    static const QString DISP_PROGRESS;
    static const QString ARTICLE_SIZE;
    static const QString RETRY;
    static const QString NZB_RM_ACCENTS;
    static const QString RESUME_WAIT;
    static const QString SOCK_TIMEOUT;
    static const QString PREPARE_PACKING;
    static const QString LOG_IN_FILE;
    static const QString TMP_DIR;
    static const QString TMP_RAM_RATIO;
    static const QString RAR_PATH;
    static const QString RAR_SIZE;
    static const QString RAR_MAX;
    static const QString PAR2_PCT;
    static const QString LENGTH_NAME;
    static const QString LENGTH_PASS;
    static const QString SERVER_HOST;
    static const QString SERVER_PORT;
    static const QString SERVER_SSL;
    static const QString SERVER_USER;
    static const QString SERVER_PASS;
    static const QString SERVER_CONNECTION;
    static const QString SERVER_ENABLED;
    static const QString SERVER_NZBCHECK;

};

// QStringLiteral is preferred over the QString constructor with string literals because it creates the string at compile time, optimizing memory usage and eliminating unnecessary runtime overhead associated with dynamic memory allocation. 
// This results in faster execution and more efficient memory management, especially beneficial for static or constant strings used throughout the application.

const QString ConfigKeys::LANG = QStringLiteral("EN");
const QString ConfigKeys::NZBPATH = QStringLiteral("/tmp");
const QString ConfigKeys::MONITOR_SEC_DELAY_SCAN = QStringLiteral("1");
const QString ConfigKeys::INPUTDIR = QStringLiteral("/tmp");
const QString ConfigKeys::POST_HISTORY = QStringLiteral("/nzb/ngPost_history.csv");
const QString ConfigKeys::GROUPS = QStringLiteral("alt.binaries.test,alt.binaries.misc");
const QString ConfigKeys::GROUP_POLICY = QStringLiteral("EACH_POST");
const QString ConfigKeys::GEN_FROM = QStringLiteral("true");
const QString ConfigKeys::DISP_PROGRESS = QStringLiteral("BAR");
const QString ConfigKeys::ARTICLE_SIZE = QStringLiteral("716800");
const QString ConfigKeys::RETRY = QStringLiteral("5");
const QString ConfigKeys::NZB_RM_ACCENTS = QStringLiteral("true");
const QString ConfigKeys::RESUME_WAIT = QStringLiteral("30");
const QString ConfigKeys::SOCK_TIMEOUT = QStringLiteral("30");
const QString ConfigKeys::PREPARE_PACKING = QStringLiteral("true");
const QString ConfigKeys::LOG_IN_FILE = QStringLiteral("true");
const QString ConfigKeys::TMP_DIR = QStringLiteral("/tmp");
const QString ConfigKeys::TMP_RAM_RATIO = QStringLiteral("1.1");
const QString ConfigKeys::RAR_PATH = QStringLiteral("/usr/bin/rar");
const QString ConfigKeys::RAR_SIZE = QStringLiteral("42");
const QString ConfigKeys::RAR_MAX = QStringLiteral("99");
const QString ConfigKeys::PAR2_PCT = QStringLiteral("8");
const QString ConfigKeys::LENGTH_NAME = QStringLiteral("22");
const QString ConfigKeys::LENGTH_PASS = QStringLiteral("15");
const QString ConfigKeys::SERVER_HOST = QStringLiteral("news.newshosting.com");
const QString ConfigKeys::SERVER_PORT = QStringLiteral("443");
const QString ConfigKeys::SERVER_SSL = QStringLiteral("true");
const QString ConfigKeys::SERVER_USER = QStringLiteral("myUser");
const QString ConfigKeys::SERVER_PASS = QStringLiteral("myPass");
const QString ConfigKeys::SERVER_CONNECTION = QStringLiteral("30");
const QString ConfigKeys::SERVER_ENABLED = QStringLiteral("true");
const QString ConfigKeys::SERVER_NZBCHECK = QStringLiteral("false");

#endif