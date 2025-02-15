//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/disinclination/ngPost
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================

#ifndef FILEUPLOADER_H
#define FILEUPLOADER_H
#include <QFile>
#include <QFileInfo>
#include <QUrl>
class QNetworkAccessManager;
class QNetworkReply;

class FileUploader : public QObject
{
    Q_OBJECT

private:
    QNetworkAccessManager &_netMgr;
    QNetworkReply *_reply;
    QFileInfo _nzbFilePath;
    QFile _nzbFile;
    QUrl _nzbUrl;

public:
    FileUploader(QNetworkAccessManager &netMgr, const QString &nzbFilePath);
    ~FileUploader();

    void startUpload(const QUrl &serverUrl);

signals:
    void readyToDie();
    void error(const QString &msg);
    void log(const QString &msg, bool newline = true);

private slots:
    void onUploadFinished();

private:
    inline QString url() const;
};

QString FileUploader::url() const
{
    if (_nzbUrl.isEmpty())
        return QString();
    else
        return _nzbUrl.toString(QUrl::RemovePassword | QUrl::RemovePath);
}

#endif // FILEUPLOADER_H
