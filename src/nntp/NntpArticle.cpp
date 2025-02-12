/*
 * Copyright (c) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
 * Copyright (c) 2025 disinclination
 * Licensed under the GNU General Public License v3.0
 */

#include "NntpArticle.h"
#include "NntpConnection.h"
#include "NgPost.h"
#include "nntp/NntpFile.h"
#include "nntp/Nntp.h"
#include "utils/Yenc.h"
#include <cstring>
#include <sstream>
#include <random>

ushort NntpArticle::sNbMaxTrySending = 5;

NntpArticle::NntpArticle(NntpFile *file, uint part, qint64 pos, qint64 bytes,
                         const std::string *from, bool obfuscateArticles):
    _nntpFile(file), _part(part),
    _id(QUuid::createUuid()),
    _from(from),
    _subject(nullptr),
    _body(nullptr),
    _filePos(pos), _fileBytes(bytes),
    _nbTrySending(0),
    _msgId(),
    _obfuscateArticles(obfuscateArticles)
{
    file->addArticle(this);
    connect(this, &NntpArticle::posted, _nntpFile, &NntpFile::onArticlePosted, Qt::QueuedConnection);
    connect(this, &NntpArticle::failed, _nntpFile, &NntpFile::onArticleFailed, Qt::QueuedConnection);

    if (!obfuscateArticles)
    {
        std::stringstream ss;
        ss << _nntpFile->nameWithQuotes().toStdString() << " (" << part << "/" << _nntpFile->nbArticles() << ")";

        std::string subject = ss.str();
        std::strncpy(_subject, subject.c_str(), sizeof(_subject) - 1); _subject[sizeof(_subject) - 1] = '\0';
    }
}

std::string generateRandomString(int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    std::string randomString;
    randomString.reserve(length);
    for (int i = 0; i < length; ++i) {
        randomString += alphanum[dist(engine)];
    }

    return randomString;
}

int generateRandomStringLength(int start, int end) {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> dist(start, end);
    return dist(engine);
}

void NntpArticle::yEncBody(const char data[])
{
    quint32 crc32    = 0xFFFFFFFF;
    uchar  *yencBody = new uchar[_fileBytes*2];
    Yenc::encode(data, _fileBytes, yencBody, crc32);

    std::stringstream ss;
    std::string filename;

    if (_obfuscateArticles)
    {
        filename = generateRandomString(generateRandomStringLength(32, 62));
    }
    else
    {
        filename = _nntpFile->fileName();
    }

    ss << "=ybegin part=" << _part << " total=" << _nntpFile->nbArticles() << " line=128"
       << " size=" << _nntpFile->fileSize() << " name=" << filename << Nntp::ENDLINE
       << "=ypart begin=" << _filePos + 1 << " end=" << _filePos + _fileBytes << Nntp::ENDLINE
       << yencBody << Nntp::ENDLINE
       << "=yend size=" << _fileBytes << " pcrc32=" << std::hex << crc32 << Nntp::ENDLINE
       << "." << Nntp::ENDLINE;

    delete[] yencBody;

    std::string body = ss.str();
    std::strncpy(_body, body.c_str(), sizeof(_body) - 1); _body[sizeof(_body) - 1] = '\0';
}

NntpArticle::~NntpArticle()
{
    freeMemory();
}

QString NntpArticle::str() const
{
    if (_msgId.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        _msgId = _id.toString(sMsgIdFormat);
#else
        _msgId = _id.toString();
#endif
    return QString("%5 - Article #%1/%2 <id: %3, nbTrySend: %4>").arg(
                _part).arg(_nntpFile->nbArticles()).arg(_msgId).arg(
                _nbTrySending).arg(_nntpFile->name());
}

bool NntpArticle::tryResend()
{
    if (_nbTrySending < sNbMaxTrySending)
    {
        _id = QUuid::createUuid();
        return true;
    }
    else
        return false;
}

void NntpArticle::write(NntpConnection *con, const std::string &idSignature)
{
    ++_nbTrySending;
    con->write(header(idSignature).c_str());
    con->write(_body);
}

std::string NntpArticle::header(const std::string &idSignature) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    QByteArray msgId = _id.toByteArray(sMsgIdFormat);
#else
    QByteArray msgId = _id.toByteArray();
#endif
    std::stringstream ss;
    ss << "From: "        << (_from == nullptr ? NgPost::randomStdFrom() : *_from)    << Nntp::ENDLINE
       << "Newsgroups: "  << _nntpFile->groups()  << Nntp::ENDLINE
       << "Subject: "     << (_subject == nullptr ? msgId.constData() : _subject) << Nntp::ENDLINE
       << "Message-ID: <" << msgId.constData() << "@" << idSignature << ">" << Nntp::ENDLINE
       << Nntp::ENDLINE;
    _msgId = QString("%1@%2").arg(QString::fromUtf8(msgId.constData()), QString::fromStdString(idSignature));
    return ss.str();
}

void NntpArticle::dumpToFile(const QString &path, const std::string &articleIdSignature)
{
    QString fileName = QString("%1/%2_%3.yenc").arg(path, QString::fromStdString(_nntpFile->fileName()), QString::number(_part));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[NntpArticle::dumpToFile] error creating file " << fileName;
        return;
    }

    file.write(header(articleIdSignature).c_str());
    file.write(_body);
    file.close();
}
