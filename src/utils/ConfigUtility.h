/*
 * Copyright (c) 2025 disinclination
 * Licensed under the GNU General Public License v3.0
 */

 #include <QCoreApplication>
 #include <QDir>
 #include <QFile>
 #include <QTextStream>
 #include <QDebug>
 #include <QString>

 class ConfigUtility {
    public:
        QString GetCurrentConfig();
        void SetNewConfig(QString config);
        void UpdateField(const QString& key, const QString& value);
        QString GetCurrentValue(const QString& key);
    
    private:
        QString GetConfigPath();
        const QString ConfigName = "ngPost.conf";
        void WriteConfig(const QMap<QString, QString>& configMap);
};