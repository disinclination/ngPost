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

#include "FileUploader.h"
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
FileUploader::FileUploader(QNetworkAccessManager &netMgr, const QString &nzbFilePath)
    : QObject()
    , _netMgr(netMgr)
    , _reply(nullptr)
    , _nzbFilePath(nzbFilePath)
    , _nzbFile(nzbFilePath)
    , _nzbUrl()
{}

FileUploader::~FileUploader()
{
    if (_nzbFile.isOpen()) {
#ifdef __DEBUG__
        qDebug() << "Deleting FileUploader for " << _nzbFile.fileName();
#endif
        _nzbFile.close();
    }

    if (_reply)
        delete _reply;
}

void FileUploader::startUpload(const QUrl &serverUrl)
{
    if (_nzbFile.open(QIODevice::ReadOnly)) {
        QString protocol = serverUrl.scheme(); // always lowercase
        if (protocol == "ftp") {
            _nzbUrl = QUrl(QString("%1/%2").arg(serverUrl.url()).arg(_nzbFilePath.fileName()));
#ifdef __DEBUG__
            qDebug() << "FileUploader FTP url: " << _nzbUrl.url();
#endif

            _reply = _netMgr.put(QNetworkRequest(_nzbUrl), &_nzbFile);
        } else if (protocol.startsWith("http")) {
            _nzbUrl = serverUrl;

#ifdef __DEBUG__
            qDebug() << "FileUploader POST on url: " << _nzbUrl.url();
#endif
            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QString fileKey("file"), fileName = QFileInfo(_nzbFilePath).fileName();
            fileName.replace('"', '\'');
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QString("form-data; name=\"%1\"; filename=\"%2\"")
                                   .arg(fileKey)
                                   .arg(fileName));
            filePart.setBodyDevice(&_nzbFile);
            multiPart->append(filePart);

            QNetworkRequest req(_nzbUrl);
            req.setRawHeader("User-Agent", "ngPost C++ app");

            _reply = _netMgr.post(req, multiPart);

            multiPart->setParent(_reply); // multiPart deleted on the destruction of reply
        } else {
            emit error(tr("Error uploading nzb to %1: Protocol not supported").arg(url()));
            emit readyToDie();
        }
        if (_reply)
            QObject::connect(_reply,
                             &QNetworkReply::finished,
                             this,
                             &FileUploader::onUploadFinished);
    } else {
        emit error(tr("Error uploading file: can't open file ").arg(_nzbFile.fileName()));
        emit readyToDie();
    }
}

void FileUploader::onUploadFinished()
{
    qDebug() << "FileUploader reply: " << _reply->readAll();
    if (_reply->error())
        emit error(tr("Error uploading nzb to %1: %2").arg(url()).arg(_reply->errorString()));
    else
        emit log(tr("nzb %1 uploaded to %2\n").arg(_nzbFilePath.fileName()).arg(url()));

    emit readyToDie();
}
