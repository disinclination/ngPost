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

#include "NntpFile.h"
#include "PostingJob.h"
#include "NgPost.h"
#include "NntpArticle.h"
#include <cmath>
#include <QTextStream>
#include <QDebug>

NntpFile::NntpFile(PostingJob *postingJob, const QFileInfo &file,
                   uint num, uint nbFiles, int padding,
                   const QList<QString> &grpList):
    QObject(),
    _postingJob(postingJob),
    _file(file), _num(num), _nbFiles(nbFiles), _padding(padding),
    _grpList(grpList), _groups(grpList.join(",").toStdString()),
    _nbAticles(static_cast<uint>(std::ceil(static_cast<float>(file.size())/NgPost::articleSize()))),
    _articles(),
    _posted(), _failed()
{
#if defined(__DEBUG__) && defined(LOG_CONSTRUCTORS)
    qDebug() << "Creation NntpFile: " << file.absoluteFilePath()
             << " size: " << file.size()
             << " article size: " << NgPost::articleSize()
             << " => nbArticles: " << _nbAticles;
#endif
    _articles.reserve(static_cast<int>(_nbAticles));
    connect(this, &NntpFile::scheduleDeletion, this, &QObject::deleteLater, Qt::QueuedConnection);
}

NntpFile::~NntpFile()
{
#if defined(__DEBUG__) && defined(LOG_CONSTRUCTORS)
    qDebug() << "Destruction nntpFile: " << _file.absoluteFilePath();
#endif
    qDeleteAll(_articles);
}

void NntpFile::onArticlePosted(quint64 size)
{
    _postingJob->articlePosted(size);
    NntpArticle *article = static_cast<NntpArticle*>(sender());
    uint part = article->_part;
#ifdef __DEBUG__
    if (_posted.contains(part) || _failed.contains(part))
        qCritical() << "[NntpFile::onArticlePosted] DUPLICATE article #" << part
                    << " for file: " << name();

    qDebug() << "[NntpFile::onArticlePosted] " << name()
             << ": posted: " << _posted.size() << " / " << _nbAticles
             << " (nb FAILED: " << _failed.size() << ")"
             << " article part " << part
             << ", id: " << article->id();
#endif
    _posted.insert(part);
    article->freeMemory(); // free resources

    if (_posted.size() + _failed.size() == static_cast<int>(_nbAticles))
        emit allArticlesArePosted();
}

void NntpFile::onArticleFailed(quint64 size)
{
    _postingJob->articleFailed(size);
    NntpArticle *article = static_cast<NntpArticle*>(sender());
    uint part = article->_part;
#ifdef __DEBUG__
    if (_posted.contains(part) || _failed.contains(part))
        qCritical() << "[NntpFile::onArticleFailed] DUPLICATE article #" << part
                    << " for file: " << name();

    qDebug() << "[NntpFile::onArticleFailed] " << name()
             << ": posted: " << _posted.size() << " / " << _nbAticles
             << " (nb FAILED: " << _failed.size() << ")"
             << " article part " << part
             << ", id: " << article->id();
#endif
    _failed.insert(part);
    article->freeMemory(); // free resources

    if (_posted.size() + _failed.size() == static_cast<int>(_nbAticles))
        emit allArticlesArePosted();
}

#include <string>
#include <QDateTime>
void NntpFile::writeToNZB(QTextStream &stream, const QString &from)
{
    //    <file poster="NewsUP &lt;NewsUP@somewhere.cbr&gt;" date="1565026184" subject="[1/846] - &quot;1w7NbOvYC2E8D5oYeXROyp4FZAaxEOmK&quot; ">
    //        <groups>
    //          <group>alt.binaries.superman</group>
    //          <group>alt.binaries.paxer</group>
    //          <group>alt.binaries.xylo</group>
    //          <group>alt.binaries.kleverig</group>
    //        </groups>
    //        <segments>
    //          <segment bytes="716800" number="1">part52of36.5M2EYjx9pGk-gkmn6rAa_tc@powerpost2000AA.local</segment>



//    file poster="ngPost@somewhere.com" date="1565794820357" subject="Odin_3.09.3.zip">
//      <groups>    </groups>    <segments>      <segment bytes="716800" number="1">{83f5c794-1b1a-4bc3-88cb-87dff4060f2e}</segment>
//        <segment bytes="287882" number="2">{6f2e54e9-6a84-4eed-a811-5d9ebd0069e4}</segment>
//      <segments>  </file>
//    file poster="ngPost@somewhere.com" date="1565794820357" subject="nginx-1.15.1.tar.gz">
//      <groups>    </groups>    <segments>      <segment bytes="716800" number="1">{1c502e51-ab9c-4ec6-bd0a-17377ed8a0a7}</segment>
//        <segment bytes="307286" number="2">{ebae6f34-37e7-4df2-91b8-850f82f3cc47}</segment>
//      <segments>  </file>
    if (_nbAticles)
    {
        QString tab = NgPost::space();
        stream << tab << "<file poster=\"" << from << "\""
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
               << " date=\"" << QDateTime::currentSecsSinceEpoch() << "\""
#else
               << " date=\"" << QDateTime::currentMSecsSinceEpoch()/1000 << "\""
#endif
               << QString(" subject=\"[%1/%2] - &quot;").arg(_num, _padding, 10,  QChar('0')).arg(_nbFiles)
//               << " subject=\""  << "[" << _num << "/" << _nbFiles << "] - &quot;"
               << NgPost::escapeXML(_file.fileName())
               << "&quot; yEnc (1/"<< _nbAticles << ") " << _file.size() << "\">\n";

        stream << tab << tab << "<groups>\n";
        for (const QString &grp : _grpList)
            stream << tab << tab << tab << "<group>" << grp << "</group>\n";
        stream << tab << tab << "</groups>\n";

        stream << tab << tab << "<segments>\n";
        for (NntpArticle *article : _articles)
        {
            stream << tab << tab << tab << "<segment"
                   << " bytes=\""  << article->_fileBytes << "\""
                   << " number=\"" << article->_part << "\">"
                   << article->_msgId
                   << "</segment>\n";
        }
        stream << tab << tab << "</segments>\n";


        stream << tab << "</file>\n" << MB_FLUSH;
    }
}

QString NntpFile::missingArticles() const
{
    QSet<uint> allArticles;
    allArticles.reserve(static_cast<int>(_nbAticles));
    for (uint i = 1; i <= _nbAticles ; ++i)
        allArticles << i;

    allArticles.subtract(_posted);
    allArticles.subtract(_failed);

    if (allArticles.isEmpty())
        return QString();
    else
    {
        QString str(" missing Articles: ");
        for (uint part : allArticles)
            str += QString("%1 ").arg(part);
        return str;
    }
}
