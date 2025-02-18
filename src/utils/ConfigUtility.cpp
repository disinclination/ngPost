/*
 * Copyright (c) 2025 disinclination
 * Licensed under the GNU General Public License v3.0
 */

#include "ConfigUtility.h"

QString ConfigUtility::GetConfigPath() {
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString configFilePath = QDir(appDirPath).filePath(ConfigUtility::ConfigName);

    return configFilePath;
}

void ConfigUtility::UpdateField(const QString& key, const QString& value) {
    QString fileContent = GetCurrentConfig();
    QStringList lines = fileContent.split("\n");
    QMap<QString, QString> currentConfig;

    for (const QString& line : lines) {
        
        QString trimmedLine = line.trimmed();

        if (trimmedLine.isEmpty() || trimmedLine.startsWith("#")) {
            continue;
        }

        QStringList keyValue = trimmedLine.split("=", Qt::SkipEmptyParts);

        if (keyValue.size() == 2) {
            currentConfig[keyValue[0].trimmed()] = keyValue[1].trimmed();
        }
    }

    currentConfig[key] = value;

    WriteConfig(currentConfig);
}

QString ConfigUtility::GetCurrentConfig() {
    QString configPath = GetConfigPath();
    QFile configFile(configPath);
  
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open config file:" << configFile.errorString();
        return QString(); // Return an empty QString on error
    }
  
    QString fileContent;
    QTextStream in(&configFile);
    fileContent = in.readAll();
  
    configFile.close();
  
    return fileContent;
}

QString ConfigUtility::GetCurrentValue(const QString& key) {
    QString fileContent = GetCurrentConfig();
    QStringList lines = fileContent.split("\n");
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.startsWith(key + "=")) {
            QStringList keyValue = trimmedLine.split("=", Qt::SkipEmptyParts);
            return keyValue.size() > 1 ? keyValue[1].trimmed() : QString();
        }
    }
    return QString();
}

void ConfigUtility::WriteConfig(const QMap<QString, QString>& configMap) {
    QString configPath = GetConfigPath();
    QFile configFile(configPath);

    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open config file for writing:" << configFile.errorString();
        return;
    }

    QTextStream out(&configFile);
    for (auto it = configMap.constBegin(); it != configMap.constEnd(); ++it) {
        out << it.key() << " = " << it.value() << "\n";
    }

    configFile.close();
}