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

#endif // CONFIGKEYS_H