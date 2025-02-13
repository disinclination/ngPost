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

#include "PostingJob.h"
#include "NgPost.h"
#include "NntpConnection.h"
#include "nntp/NntpArticle.h"
#include "nntp/NntpFile.h"
#include "nntp/NntpServerParams.h"
#ifdef __USE_HMI__
#include "hmi/PostingWidget.h"
#endif
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <cmath>

PostingJob::PostingJob(NgPost *ngPost,
                       const QString &nzbFilePath,
                       const QFileInfoList &files,
                       PostingWidget *postWidget,
                       const QList<QString> &grpList,
                       const std::string &from,
                       bool obfuscateArticles,
                       bool obfuscateFileName,
                       const QString &tmpPath,
                       const QString &rarPath,
                       const QString &rarArgs,
                       uint rarSize,
                       bool useRarMax,
                       uint par2Pct,
                       bool doCompress,
                       bool doPar2,
                       const QString &rarName,
                       const QString &rarPass,
                       bool keepRar,
                       bool delFilesAfterPost,
                       bool overwriteNzb,
                       QObject *parent)
    : QObject(parent)
    , _ngPost(ngPost)
    , _files(files)
    , _postWidget(postWidget)
    ,

    _extProc(nullptr)
    , _compressDir(nullptr)
    , _limitProcDisplay(false)
    , _nbProcDisp(42)
    ,

    _tmpPath(tmpPath)
    , _rarPath(rarPath)
    , _rarArgs(rarArgs)
    , _rarSize(rarSize)
    , _useRarMax(useRarMax)
    , _par2Pct(par2Pct)
    , _doCompress(doCompress)
    , _doPar2(doPar2)
    , _rarName(rarName)
    , _rarPass(rarPass)
    , _keepRar(keepRar)
    , _splitArchive(false)
    ,

    _nntpConnections()
    , _closedConnections()
    ,

    _nzbName(QFileInfo(nzbFilePath).fileName())
    , _filesToUpload()
    , _filesInProgress()
    , _filesFailed()
    , _nbFiles(0)
    , _nbPosted(0)
    ,

    _nzbFilePath(nzbFilePath)
    , _nzb(nullptr)
    , _nzbStream()
    , _nntpFile(nullptr)
    , _file(nullptr)
    , _part(0)
    , _timeStart()
    , _totalSize(0)
    , _pauseTimer()
    , _pauseDuration(0)
    ,

    _nbConnections(0)
    , _nbThreads(_ngPost->_nbThreads)
    , _nbArticlesUploaded(0)
    , _nbArticlesFailed(0)
    , _uploadedSize(0)
    , _nbArticlesTotal(0)
    , _stopPosting(0x0)
    , _noMoreFiles(0x0)
    , _postStarted(false)
    , _packed(false)
    , _postFinished(false)
    , _obfuscateArticles(obfuscateArticles)
    , _obfuscateFileName(obfuscateFileName)
    , _delFilesAfterPost(delFilesAfterPost ? 0x1 : 0x0)
    , _originalFiles(!postWidget || delFilesAfterPost || obfuscateFileName ? files : QFileInfoList())
    , _secureDiskAccess()
    , _posters()
    , _overwriteNzb(overwriteNzb)
    , _grpList(grpList)
    , _from(from)
    , _use7z(false)
    , _isPaused(false)
    , _resumeTimer()
    , _isActiveJob(false)
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    , _immediateSize(0)
    , _immediateSpeedTimer()
    , _immediateSpeed("0 B/s")
    , _useHMI(_ngPost->useHMI())
#endif
{
#ifdef __DEBUG__
    qDebug() << "[PostingJob] >>>> Construct " << this;
#endif
    connect(this, &PostingJob::startPosting, this, &PostingJob::onStartPosting, Qt::QueuedConnection);
    connect(this, &PostingJob::stopPosting, this, &PostingJob::onStopPosting, Qt::QueuedConnection);
    connect(this,
            &PostingJob::postingStarted,
            _ngPost,
            &NgPost::onPostingJobStarted,
            Qt::QueuedConnection);
    connect(this, &PostingJob::packingDone, _ngPost, &NgPost::onPackingDone, Qt::QueuedConnection);
    connect(this,
            &PostingJob::postingFinished,
            _ngPost,
            &NgPost::onPostingJobFinished,
            Qt::QueuedConnection);
    connect(this,
            &PostingJob::noMoreConnection,
            _ngPost,
            &NgPost::onPostingJobFinished,
            Qt::QueuedConnection);

    //    connect(this, &PostingJob::scheduleNextArticle, this, &PostingJob::onPrepareNextArticle, Qt::QueuedConnection);

#ifdef __USE_HMI__
    if (_postWidget) {
        connect(this,
                &PostingJob::filePosted,
                _postWidget,
                &PostingWidget::onFilePosted,
                Qt::QueuedConnection);
        connect(this,
                &PostingJob::archiveFileNames,
                _postWidget,
                &PostingWidget::onArchiveFileNames,
                Qt::QueuedConnection);
        connect(this,
                &PostingJob::articlesNumber,
                _postWidget,
                &PostingWidget::onArticlesNumber,
                Qt::QueuedConnection);
        connect(this,
                &PostingJob::postingFinished,
                _postWidget,
                &PostingWidget::onPostingJobDone,
                Qt::QueuedConnection);
        connect(this,
                &PostingJob::noMoreConnection,
                _postWidget,
                &PostingWidget::onPostingJobDone,
                Qt::QueuedConnection);
    }
#endif

    connect(&_resumeTimer, &QTimer::timeout, this, &PostingJob::onResumeTriggered);
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    if (_useHMI)
        connect(&_immediateSpeedTimer,
                &QTimer::timeout,
                this,
                &PostingJob::onImmediateSpeedComputation,
                Qt::QueuedConnection);
#endif

    if (ngPost->debugMode())
        _log(NntpConnection::sslSupportInfo());
}

PostingJob::~PostingJob()
{
#ifdef __DEBUG__
    _log("Destructing PostingJob");
    qDebug() << "[PostingJob] <<<< Destruction " << this;
#endif

    if (_ngPost->debugMode())
        _log("Deleting PostingJob");

    if (_compressDir) {
        if (hasPostFinishedSuccessfully())
            _cleanCompressDir();
        delete _compressDir;
        _compressDir = nullptr;
    }

    if (_extProc)
        _cleanExtProc();

    qDeleteAll(_filesFailed);
    qDeleteAll(_filesInProgress);
    qDeleteAll(_filesToUpload);
    qDeleteAll(_nntpConnections);
    qDeleteAll(_closedConnections);
    qDeleteAll(_posters);

    if (_nzb)
        delete _nzb;
    if (_file)
        delete _file;
}

void PostingJob::pause()
{
    _log("Pause posting...");
    for (NntpConnection *con : _nntpConnections)
        emit con->killConnection();

    _isPaused = true;
    _pauseTimer.start();
}

void PostingJob::resume()
{
    _log("Resume posting...");
    for (NntpConnection *con : _nntpConnections)
        emit con->startConnection();

    _isPaused = false;
    _pauseDuration += _pauseTimer.elapsed();
}

QString PostingJob::sslSupportInfo()
{
    return NntpConnection::sslSupportInfo();
}

bool PostingJob::supportsSsl()
{
    return NntpConnection::supportsSsl();
}

void PostingJob::onResumeTriggered()
{
    if (_isPaused) {
        _log(tr("Try to resume posting"));
        _nntpConnections.swap(_closedConnections);
        _ngPost->resume();
    }
}

#ifdef __COMPUTE_IMMEDIATE_SPEED__
void PostingJob::onImmediateSpeedComputation()
{
    QString power = " ";
    int immediateSpeedDurationMs = NgPost::immediateSpeedDurationMs();
    double bandwidth = 1000. * _immediateSize / immediateSpeedDurationMs;
    if (bandwidth > 1024) {
        bandwidth /= 1024;
        power = "k";
    }
    if (bandwidth > 1024) {
        bandwidth /= 1024;
        power = "M";
    }

    _immediateSpeed = QString("%1 %2B/s").arg(bandwidth, 6, 'f', 2).arg(power);
    _immediateSize = 0;
    _immediateSpeedTimer.start(immediateSpeedDurationMs);
}
#endif

void PostingJob::onStartPosting(bool isActiveJob)
{
    _isActiveJob = isActiveJob;
#ifdef __DEBUG__
    qDebug() << "[MB_TRACE][Issue#82][PostingJob::onStartPosting] job: " << this
             << ", file: " << nzbName() << " (isActive: " << isActiveJob << ")";
#endif
#ifdef __USE_HMI__
    if (_postWidget)
        _log(tr("<h3>Start Post #%1: %2</h3>").arg(_postWidget->jobNumber()).arg(_nzbName));
    else
#endif
        _log(QString("\n\n[%1] %2: %3").arg(timestamp()).arg(tr("Start posting")).arg(_nzbName));

    if (_doCompress) {
#ifdef __USE_TMP_RAM__
        if (_ngPost->useTmpRam()) {
            qint64 sourceSize = 0;
            for (const QFileInfo &fi : _files)
                sourceSize += NgPost::recursiveSize(fi);

            double sourceSizeWithRatio = _ngPost->ramRatio() * sourceSize,
                   availableSize = static_cast<double>(_ngPost->ramAvailable());
            if (sourceSizeWithRatio < availableSize) {
                _tmpPath = _ngPost->_ramPath;
                _log(tr("Using TMP_RAM path as temporary folder. Post size: %1")
                         .arg(humanSize(static_cast<double>(sourceSize))));
            } else {
                _error(tr("Couldn't use TMP_RAM as there is not enough space: %1 available for a "
                          "Post with ratio of %2")
                           .arg(humanSize(availableSize))
                           .arg(humanSize(sourceSizeWithRatio)));
            }
        }
#endif

#ifdef __DEBUG__
        _log("[PostingJob::onStartPosting] Starting compression...");
#endif
        if (!startCompressFiles(_rarPath, _tmpPath, _rarName, _rarPass, _rarSize))
            emit postingFinished();
    } else if (_doPar2) {
#ifdef __USE_TMP_RAM__
        if (_ngPost->useTmpRam()) {
            qint64 sourceSize = 0;
            for (const QFileInfo &fi : _files)
                sourceSize += NgPost::recursiveSize(fi);

            double par2Size = (_ngPost->ramRatio() - 1) * sourceSize,
                   availableSize = static_cast<double>(_ngPost->ramAvailable());
            if (par2Size < availableSize) {
                _tmpPath = _ngPost->_ramPath;
                _log(tr("Using TMP_RAM path as temporary folder for par2. Post size: %1")
                         .arg(humanSize(static_cast<double>(sourceSize))));
            } else {
                _error(tr("Couldn't use TMP_RAM as there is not enough space: %1 available for a "
                          "par2 volume using TMP_RAM_RATIO of %2")
                           .arg(humanSize(availableSize))
                           .arg(humanSize(par2Size)));
            }
        }
#endif
        if (!startGenPar2(_tmpPath, _rarName, _par2Pct))
            emit postingFinished();
    } else {
        _packed = true;
        _postFiles();
    }
}

#include "Poster.h"
void PostingJob::_postFiles()
{
    _postStarted = true;

#ifdef __USE_HMI__
    if (_postWidget) // in case we were in Pending mode
        _postWidget->setPosting();
#endif

    if (_doCompress) {
        QStringList archiveNames;
        _files.clear();
        for (const QFileInfo &file : _compressDir->entryInfoList(QDir::Files, QDir::Name)) {
            _files << file;
            archiveNames << file.absoluteFilePath();
            if (_ngPost->debugMode())
                _ngPost->_log(QString("  - %1").arg(file.fileName()));
        }
        emit archiveFileNames(archiveNames);
    } else if (_doPar2) {
        QStringList archiveNames;
        for (const QFileInfo &file : _files)
            archiveNames << file.absoluteFilePath();

        for (const QFileInfo &file : _compressDir->entryInfoList(QDir::Files, QDir::Name)) {
            _files << file;
            archiveNames << file.absoluteFilePath();
            if (_ngPost->debugMode())
                _ngPost->_log(QString("  - %1").arg(file.fileName()));
        }
        emit archiveFileNames(archiveNames);
    }
    _initPosting();

    if (_nbThreads > QThread::idealThreadCount())
        _nbThreads = QThread::idealThreadCount();

    int nbPosters = _nbThreads / 2, nbCon = _createNntpConnections();
    if (nbPosters < 1)
        nbPosters = 1;
    if (!nbCon) {
        _error(tr("Error: there are no NntpConnection..."));
        emit postingFinished();
        return;
    }

    if (!_nzb->open(QIODevice::WriteOnly)) {
        _error(tr("Error: Can't create nzb output file: %1").arg(_nzbFilePath));
        emit postingFinished();
        return;
    } else {
        QString tab = _ngPost->space();
        _nzbStream.setDevice(_nzb);
        _nzbStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   << "<!DOCTYPE nzb PUBLIC \"-//newzBin//DTD NZB 1.1//EN\" "
                      "\"http://www.newzbin.com/DTD/nzb/nzb-1.1.dtd\">\n"
                   << "<nzb xmlns=\"http://www.newzbin.com/DTD/2003/nzb\">\n";

        if (!_rarPass.isEmpty() || _ngPost->_meta.size()) {
            _nzbStream << tab << "<head>\n";
            for (auto itMeta = _ngPost->_meta.cbegin(); itMeta != _ngPost->_meta.cend(); ++itMeta)
                _nzbStream << tab << tab << "<meta type=\"" << itMeta.key() << "\">"
                           << itMeta.value() << "</meta>\n";
            if (!_rarPass.isEmpty())
                _nzbStream << tab << tab << "<meta type=\"password\">" << _rarPass << "</meta>\n";
            _nzbStream << tab << "</head>\n\n";
        }
        _nzbStream << MB_FLUSH;
    }

    _timeStart.start();

    //    QMutexLocker lock(&_secureArticles); // start the connections but they must wait _prepareArticles

    if (nbPosters > nbCon)
        nbPosters = nbCon; // we can't have more thread than available connections

    _posters.reserve(nbPosters);
    int nbConPerPoster = static_cast<int>(std::floor(nbCon / nbPosters));
    int nbExtraCon = nbCon - nbConPerPoster * nbPosters;
#ifdef __DEBUG__
    qDebug() << "[PostingJob::_postFiles] nbFiles: " << _filesToUpload.size()
             << ", nbPosters: " << nbPosters << ", nbCons: " << nbCon
             << " => nbCon per Poster: " << nbConPerPoster << " (nbExtraCon: " << nbExtraCon << ")";
#endif

    int conIdx = 0;
    for (ushort posterIdx = 0; posterIdx < nbPosters; ++posterIdx) {
        Poster *poster = new Poster(this, posterIdx);
        _posters.append(poster);
        poster->lockQueue(); // lock queue so the connection will wait before starting building Articles

        for (int i = 0; i < nbConPerPoster; ++i)
            poster->addConnection(_nntpConnections.at(conIdx++));

        if (nbExtraCon-- > 0)
            poster->addConnection(_nntpConnections.at(conIdx++));

        poster->startThreads();
    }

    // Prepare 2 Articles for each connections
    _preparePostersArticles();

#ifdef __COMPUTE_IMMEDIATE_SPEED__
    _immediateSpeedTimer.start(NgPost::immediateSpeedDurationMs());
#endif

    for (Poster *poster : _posters)
        poster->unlockQueue();

    emit postingStarted();
}

void PostingJob::onStopPosting()
{
    if (_extProc) {
        _log(tr("killing external process..."));
        _extProc->terminate();
        _extProc->waitForFinished();
    } else {
        _finishPosting();
        emit postingFinished();
    }
}

void PostingJob::onDisconnectedConnection(NntpConnection *con)
{
    if (MB_LoadAtomic(_stopPosting))
        return; // we're destructing all the connections

    if (!con->hasNoMoreFiles())
        _error(tr("Error: disconnected connection: #%1\n").arg(con->getId()));

    if (_nntpConnections.removeOne(con)) {
        con->resetErrorCount(); // In case we will resume if we loose all
        _closedConnections.append(con);

        if (_nntpConnections.isEmpty()) {
            if (con->hasNoMoreFiles()) {
                _finishPosting();
                if (!_postFinished)
                    emit noMoreConnection();
            } else {
                _error(tr("we lost all the connections..."));
                if (_ngPost->_tryResumePostWhenConnectionLost) {
                    int sleepDurationInSec = _ngPost->waitDurationBeforeAutoResume();
                    _log(tr("Sleep for %1 sec before trying to reconnect").arg(sleepDurationInSec));
                    _ngPost->pause();
                    _resumeTimer.start(sleepDurationInSec * 1000);
                } else {
                    _finishPosting();
                    if (!_postFinished)
                        emit noMoreConnection();
                }
            }
        }
    }
}

void PostingJob::onNntpFileStartPosting()
{
    NntpFile *nntpFile = static_cast<NntpFile *>(sender());
    if (!_ngPost->useHMI())
        _log(QString("[%1][%2: %3] >>>>> %4")
                 .arg(timestamp())
                 .arg(tr("avg. speed"))
                 .arg(avgSpeed())
                 .arg(nntpFile->name()));
}

void PostingJob::onNntpFilePosted()
{
    NntpFile *nntpFile = static_cast<NntpFile *>(sender());
    _totalSize += static_cast<quint64>(nntpFile->fileSize());
    ++_nbPosted;
    if (_postWidget)
        emit filePosted(nntpFile->path(), nntpFile->nbArticles(), nntpFile->nbFailedArticles());

    if (_ngPost->_dispFilesPosting && !_ngPost->useHMI())
        _log(QString("[%1][%2: %3] <<<<< %4")
                 .arg(timestamp())
                 .arg(tr("avg. speed"))
                 .arg(avgSpeed())
                 .arg(nntpFile->name()));

    nntpFile->writeToNZB(_nzbStream, QString::fromStdString(_from));
    _filesInProgress.remove(nntpFile);
    emit nntpFile->scheduleDeletion();
    if (_nbPosted == _nbFiles) {
#ifdef __DEBUG__
        _log(QString(
                 "All files have been posted => closing Job (nb article uploaded: %1, failed: %2)")
                 .arg(_nbArticlesUploaded)
                 .arg(_nbArticlesFailed));
#endif

        _postFinished = true;
        _finishPosting();

        emit postingFinished();
    }
}

void PostingJob::onNntpErrorReading()
{
    NntpFile *nntpFile = static_cast<NntpFile *>(sender());
    ++_nbPosted;
    if (_postWidget)
        emit filePosted(nntpFile->path(), nntpFile->nbArticles(), nntpFile->nbArticles());

    if (_ngPost->_dispFilesPosting && !_ngPost->useHMI())
        _log(tr("[avg. speed: %1] <<<<< %2").arg(avgSpeed()).arg(nntpFile->name()));

    _filesInProgress.remove(nntpFile);
    _filesFailed.insert(nntpFile);
    if (_nbPosted == _nbFiles) {
#ifdef __DEBUG__
        _log(QString(
                 "All files have been posted => closing Job (nb article uploaded: %1, failed: %2)")
                 .arg(_nbArticlesUploaded)
                 .arg(_nbArticlesFailed));
#endif

        _postFinished = true;
        _finishPosting();

        emit postingFinished();
    }
}

void PostingJob::_log(const QString &aMsg, bool newline) const
{
    emit _ngPost->log(aMsg, newline);
}

void PostingJob::_error(const QString &error) const
{
    emit _ngPost->error(error);
}

int PostingJob::_createNntpConnections()
{
    _nbConnections = 0;
    for (NntpServerParams *srvParams : _ngPost->_nntpServers) {
        if (srvParams->enabled)
            _nbConnections += srvParams->nbCons;
    }

    _nntpConnections.reserve(_nbConnections);
    int conIdx = 0;
    for (NntpServerParams *srvParams : _ngPost->_nntpServers) {
        if (srvParams->enabled) {
            for (int k = 0; k < srvParams->nbCons; ++k) {
                NntpConnection *nntpCon = new NntpConnection(_ngPost, ++conIdx, *srvParams);
                connect(nntpCon, &NntpConnection::log, _ngPost, &NgPost::onLog, Qt::QueuedConnection);
                connect(nntpCon,
                        &NntpConnection::error,
                        _ngPost,
                        &NgPost::onError,
                        Qt::QueuedConnection);
                connect(nntpCon,
                        &NntpConnection::errorConnecting,
                        _ngPost,
                        &NgPost::onErrorConnecting,
                        Qt::QueuedConnection);
                connect(nntpCon,
                        &NntpConnection::disconnected,
                        this,
                        &PostingJob::onDisconnectedConnection,
                        Qt::QueuedConnection);
                _nntpConnections.append(nntpCon);
            }
        }
    }

    if (_ngPost->useHMI())
        _log(tr("Number of available Nntp Connections: %1").arg(_nbConnections));
    else
        _log(QString("[%1] %2: %3")
                 .arg(timestamp())
                 .arg(tr("Number of available Nntp Connections"))
                 .arg(_nbConnections));

    return _nbConnections;
}

void PostingJob::_preparePostersArticles()
{
    if (_ngPost->debugFull())
        _log("PostingJob::_prepareArticles");

    for (int i = 0; i < _ngPost->sNbPreparedArticlePerConnection; ++i) {
        if (MB_LoadAtomic(_noMoreFiles))
            break;
        for (Poster *poster : _posters) {
            if (!poster->prepareArticlesInAdvance())
                break;
        }
    }
}

void PostingJob::_delOriginalFiles()
{
    for (const QFileInfo &fi : _originalFiles) {
        QString path = fi.absoluteFilePath();
        _log(tr("Deleting posted %1: %2").arg(fi.isDir() ? tr("folder") : tr("file")).arg(path));
        if (fi.isDir()) {
            QDir dir(path);
            dir.removeRecursively();
        } else
            QFile::remove(path);
    }
}

NntpArticle *PostingJob::_readNextArticleIntoBufferPtr(const QString &threadName, char **bufferPtr)
{
    //qDebug() << "[PostingJob::readNextArticleIntoBufferPtr] " << threadName;
    if (!_nntpFile) {
        _nntpFile = _getNextFile();
        if (!_nntpFile) {
            _noMoreFiles = 0x1;
#ifdef __DEBUG__
            _log(tr("[%1] No more file to post...").arg(threadName));
#endif
            return nullptr;
        }
    }

    if (!_file) {
        _file = new QFile(_nntpFile->path());
        if (_file->open(QIODevice::ReadOnly)) {
            if (_ngPost->debugFull())
                _log(tr("[%1] starting processing file %2").arg(threadName).arg(_nntpFile->path()));
            _part = 0;
        } else {
            if (_ngPost->debugMode())
                _error(
                    tr("[%1] Error: couldn't open file %2").arg(threadName).arg(_nntpFile->path()));
            else
                _error(tr("Error: couldn't open file %1").arg(_nntpFile->path()));
            emit _nntpFile->errorReadingFile();
            delete _file;
            _file = nullptr;
            _nntpFile = nullptr;
            return _readNextArticleIntoBufferPtr(threadName,
                                                 bufferPtr); // Check if we have more files
        }
    }

    if (_file) {
        qint64 pos = _file->pos();
        qint64 bytesRead = _file->read(*bufferPtr, _ngPost->articleSize());
        if (bytesRead > 0) {
            (*bufferPtr)[bytesRead] = '\0';
            if (_ngPost->debugFull())
                _log(tr("[%1] we've read %2 bytes from %3 (=> new pos: %4)")
                         .arg(threadName)
                         .arg(bytesRead)
                         .arg(pos)
                         .arg(_file->pos()));
            ++_part;
            NntpArticle *article = new NntpArticle(_nntpFile,
                                                   _part,
                                                   pos,
                                                   bytesRead,
                                                   _obfuscateArticles ? nullptr : &_from,
                                                   _obfuscateArticles);
            return article;
        } else {
            if (_ngPost->debugFull())
                _log(tr("[%1] finished processing file %2").arg(threadName).arg(_nntpFile->path()));

            _file->close();
            delete _file;
            _file = nullptr;
            _nntpFile = nullptr;
        }
    }

    return _readNextArticleIntoBufferPtr(threadName,
                                         bufferPtr); // if we didn't have an Article, check next file
}

void PostingJob::_initPosting()
{
    // initialize buffer and nzb file
    if (!_overwriteNzb) {
        QFileInfo fi(_nzbFilePath);
        if (fi.exists()) {
            QString baseName = fi.completeBaseName();
            ushort nbDuplicates = 1;
            do {
                _nzbFilePath = QString("%1/%2_%3.nzb")
                                   .arg(fi.absolutePath())
                                   .arg(baseName)
                                   .arg(nbDuplicates++);
                fi = QFileInfo(_nzbFilePath);
            } while (fi.exists());

            _nzbFilePath = fi.absoluteFilePath();
        }
    }
    _nzb = new QFile(_nzbFilePath);
    _nbFiles = static_cast<uint>(_files.size());

    int numPadding = 1;
    double padding = static_cast<double>(_nbFiles) / 10.;
    while (padding > 1.) {
        ++numPadding;
        padding /= 10.;
    }

    // initialize the NntpFiles (active objects)
    _filesToUpload.reserve(static_cast<int>(_nbFiles));
    uint fileNum = 0;
    int nbGroups = _grpList.size();
    for (const QFileInfo &file : _files) {
        NntpFile *nntpFile = new NntpFile(this,
                                          file,
                                          ++fileNum,
                                          _nbFiles,
                                          numPadding,
                                          _obfuscateArticles && _ngPost->groupPolicyPerFile()
                                                  && nbGroups > 1
                                              ? QStringList(_grpList.at(std::rand() % nbGroups))
                                              : _grpList);
        connect(nntpFile,
                &NntpFile::allArticlesArePosted,
                this,
                &PostingJob::onNntpFilePosted,
                Qt::QueuedConnection);
        connect(nntpFile,
                &NntpFile::errorReadingFile,
                this,
                &PostingJob::onNntpErrorReading,
                Qt::QueuedConnection);
        if (_ngPost->_dispFilesPosting && _ngPost->debugMode())
            connect(nntpFile,
                    &NntpFile::startPosting,
                    this,
                    &PostingJob::onNntpFileStartPosting,
                    Qt::QueuedConnection);

        _filesToUpload.enqueue(nntpFile);
        _nbArticlesTotal += nntpFile->nbArticles();
    }
    emit articlesNumber(_nbArticlesTotal);
}

void PostingJob::_finishPosting()
{
#ifdef __DEBUG__
    qDebug() << "[MB_TRACE][PostingJob::_finishPosting]";
#endif
    _stopPosting = 0x1;

    if (_timeStart.isValid() && _postFinished) {
        _nbArticlesUploaded = _nbArticlesTotal; // we might not have processed the last onArticlePosted
        _uploadedSize = _totalSize;
    }

    if (_ngPost->debugMode())
        _log("Finishing posting...");

    _ngPost->_finishPosting(); // to update progress bar

    // 1.: print stats
    if (_timeStart.isValid())
        _printStats();

    for (NntpConnection *con : _nntpConnections)
        emit con->killConnection();

    qApp->processEvents();

    // 4.: stop and wait for all threads
    // 2.: close nzb file
    _closeNzb();

    // 3.: close all the connections (they're living in the _threadPool)
    for (Poster *poster : _posters)
        poster->stopThreads();

    if (_ngPost->debugMode())
        _log("All posters stopped...");

#ifdef __DEBUG__
    _log("All connections are closed...");
#endif

    // 5.: print out the list of files that havn't been posted
    // (in case of disconnection)
    int nbPendingFiles = _filesToUpload.size() + _filesInProgress.size() + _filesFailed.size();
    if (nbPendingFiles) {
        _error(tr("ERROR: there were %1 on %2 that havn't been posted:")
                   .arg(nbPendingFiles)
                   .arg(_nbFiles));
        bool isDebugMode = _ngPost->debugMode();
        for (NntpFile *file : _filesInProgress) {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fInProgress%1)").arg(file->missingArticles());
            _error(msg);
        }
        for (NntpFile *file : _filesToUpload) {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fToUpload%1)").arg(file->missingArticles());
            _error(msg);
        }
        for (NntpFile *file : _filesFailed) {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fFailed%1)").arg(file->missingArticles());
            _error(msg);
        }
        _error(
            tr("you can try to repost only those and concatenate the nzb with the current one ;)"));
    } else if (_postFinished && MB_LoadAtomic(_delFilesAfterPost))
        _delOriginalFiles();
}

void PostingJob::_closeNzb()
{
    if (_nzb) {
        if (_nzb->isOpen()) {
            _nzbStream << "</nzb>\n";
            _nzb->close();
            _ngPost->doNzbPostCMD(this);
        }
        delete _nzb;
        _nzb = nullptr;
    }
}

void PostingJob::_printStats() const
{
    QString size = postSize();

    int duration = static_cast<int>(_timeStart.elapsed() - _pauseDuration);
    double sec = duration / 1000;

    QString msgEnd("\n"), ts = QString("[%1] ").arg(timestamp());
    if (!_ngPost->useHMI())
        msgEnd += ts;
    msgEnd += tr("Upload size: %1 in %2 (%3 sec) \
                 => average speed: %4 (%5 connections on %6 threads)\n")
                  .arg(size)
                  .arg(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss.zzz"))
                  .arg(sec)
                  .arg(avgSpeed())
                  .arg(_nntpConnections.size() + _closedConnections.size())
                  .arg(_posters.size() * 2);

    if (_nbArticlesFailed > 0)
        msgEnd += tr("%1 / %2 articles FAILED to be uploaded (even with %3 retries)...\n")
                      .arg(_nbArticlesFailed)
                      .arg(_nbArticlesTotal)
                      .arg(NntpArticle::nbMaxTrySending());

    if (_nzb) {
        if (!_ngPost->useHMI())
            msgEnd += ts;
        msgEnd += tr("nzb file: %1\n").arg(_nzbFilePath);
        if (_doCompress) {
            if (!_ngPost->useHMI())
                msgEnd += ts;
            msgEnd += tr("file: %1, rar name: %2").arg(_nzbName).arg(_rarName);
            if (!_rarPass.isEmpty())
                msgEnd += tr(", rar pass: %1").arg(_rarPass);
        }
        if (!_ngPost->_urlNzbUpload)
            msgEnd += "\n";
    }

    _log(msgEnd);
}

bool PostingJob::startCompressFiles(const QString &cmdRar,
                                    const QString &tmpFolder,
                                    const QString &archiveName,
                                    const QString &pass,
                                    uint volSize)
{
    if (!_canCompress())
        return false;

    // 1.: create archive temporary folder
    QString archiveTmpFolder = _createArchiveFolder(tmpFolder, archiveName);
    if (archiveTmpFolder.isEmpty())
        return false;

    _extProc = new QProcess(this);
    connect(_extProc,
            &QProcess::readyReadStandardOutput,
            this,
            &PostingJob::onExtProcReadyReadStandardOutput,
            Qt::DirectConnection);
    connect(_extProc,
            &QProcess::readyReadStandardError,
            this,
            &PostingJob::onExtProcReadyReadStandardError,
            Qt::DirectConnection);
    connect(_extProc,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            &PostingJob::onCompressionFinished,
            Qt::QueuedConnection);

    _use7z = false;
    if (_rarPath.contains("7z")) {
        _use7z = true;
        if (_rarArgs.isEmpty())
            _rarArgs = _ngPost->sDefault7zOptions;
    } else {
        if (_rarArgs.isEmpty())
            _rarArgs = _ngPost->sDefaultRarExtraOptions;
    }

    // 2.: create rar args (rar a -v50m -ed -ep1 -m0 -hp"$PASS" "$TMP_FOLDER/$RAR_NAME.rar" "${FILES[@]}")
    //    QStringList args = {"a", "-idp", "-ep1", compressLevel, QString("%1/%2.rar").arg(archiveTmpFolder).arg(archiveName)};
    QStringList args = _rarArgs.split(" ");
    if (!args.contains("a"))
        args.prepend("a");
    if (!_use7z && !args.contains("-idp"))
        args << "-idp";
    if (!pass.isEmpty()) {
        if (_use7z) {
            if (!args.contains("-mhe=on"))
                args << "-mhe=on";
            args << QString("-p%1").arg(pass);
        } else
            args << QString("-hp%1").arg(pass);
    }
    if (volSize > 0 || _useRarMax) {
        if (_useRarMax) {
            qint64 postSize = 0;
            for (const QFileInfo &fileInfo : _files) {
                if (fileInfo.isDir())
                    postSize += _dirSize(fileInfo.absoluteFilePath());
                else
                    postSize += fileInfo.size();
            }
            postSize /= 1024 * 1024; // to get it in MB
            if (volSize > 0) {
                if (postSize / volSize > _ngPost->_rarMax)
                    volSize = static_cast<uint>(postSize / _ngPost->_rarMax) + 1;
            } else
                volSize = static_cast<uint>(postSize / _ngPost->_rarMax) + 1;

#ifdef __DEBUG__
            qDebug() << tr("postSize: %1 MB => volSize: %2").arg(postSize).arg(volSize);
#endif
            if (_ngPost->debugMode())
                _log(tr("postSize: %1 MB => volSize: %2").arg(postSize).arg(volSize));
        }
        args << QString("-v%1m").arg(volSize);
        _splitArchive = true;
    }

#if defined(Q_OS_WIN)
    if (archiveTmpFolder.startsWith("//"))
        archiveTmpFolder.replace(QRegularExpression("^//"), "\\\\");
#endif
    args << QString("%1/%2.%3").arg(archiveTmpFolder, archiveName, _use7z ? "7z" : "rar");

    if (_obfuscateFileName) {
        _files.clear();
        for (const QFileInfo &fileInfo : _originalFiles) {
            QString obfuscatedName = QString("%1/%2").arg(fileInfo.absolutePath(),
                                                          _ngPost->randomPass(_ngPost->_lengthName)),
                    fileName = fileInfo.absoluteFilePath();

            if (QFile::rename(fileName, obfuscatedName)) {
                _obfuscatedFileNames.insert(obfuscatedName, fileName);
                _files << QFileInfo(obfuscatedName);
            } else {
                _files << fileInfo;
                _error(tr("Couldn't rename file %1").arg(fileName));
            }
        }
    }

    bool hasDir = false;
    if (_files.size()
        == 1) { // Only remove rar root folder if there is only ONE folder AND RAR_NO_ROOT_FOLDER is set
        const QFileInfo &fileInfo = _files.first();
        QString path = fileInfo.absoluteFilePath();
#if defined(Q_OS_WIN)
        if (path.startsWith("//"))
            path.replace(QRegularExpression("^//"), "\\\\");
#endif
        if (fileInfo.isDir()) {
            hasDir = true;
            if (_ngPost->removeRarRootFolder())
                path += "/";
        }
        args << path;
    } else {
        for (const QFileInfo &fileInfo : _files) {
            QString path = fileInfo.absoluteFilePath();
#if defined(Q_OS_WIN)
            if (path.startsWith("//"))
                path.replace(QRegularExpression("^//"), "\\\\");
#endif
            if (fileInfo.isDir())
                hasDir = true;
            args << path;
        }
    }

    if (hasDir && !args.contains("-r"))
        args << "-r";

    // 3.: launch rar
    if (_ngPost->debugMode() || !_postWidget)
        _log(QString("[%1] %2: %3 %4\n")
                 .arg(timestamp())
                 .arg(tr("Compressing files"))
                 .arg(cmdRar)
                 .arg(args.join(" ")));
    else
        _log(QString("%1...\n").arg(tr("Compressing files")));
    _limitProcDisplay = false;
    _extProc->start(cmdRar, args);

#ifdef __DEBUG__
    _log("[PostingJob::_compressFiles] compression started...");
#endif

    return true;
}

void PostingJob::onCompressionFinished(int exitCode)
{
    if (_ngPost->debugMode())
        _log(tr("=> rar exit code: %1\n").arg(exitCode));
    else
        _log("\n");

#ifdef __DEBUG__
    _log("[PostingJob::_compressFiles] compression finished...");
#endif

    if (_obfuscateFileName && !MB_LoadAtomic(_delFilesAfterPost)) {
        for (auto it = _obfuscatedFileNames.cbegin(), itEnd = _obfuscatedFileNames.cend();
             it != itEnd;
             ++it)
            QFile::rename(it.key(), it.value());
    }

    if (exitCode != 0) {
        _error(tr("Error during compression: %1").arg(exitCode));
        _cleanCompressDir();
        emit postingFinished();
    } else {
        if (_doPar2) {
            disconnect(_extProc,
                       static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                           &QProcess::finished),
                       this,
                       &PostingJob::onCompressionFinished);
            if (!startGenPar2(_tmpPath, _rarName, _par2Pct)) {
                _cleanCompressDir();
                emit postingFinished();
            }
        } else {
            _packed = true;
            _cleanExtProc();
            if (this == _ngPost->_activeJob)
                _postFiles();

            emit packingDone();
        }
    }
}

bool PostingJob::startGenPar2(const QString &tmpFolder, const QString &archiveName, uint redundancy)
{
    if (!_canGenPar2())
        return false;

    QStringList args;
    if (_ngPost->_par2Args.isEmpty())
        args << "c" << "-l" << "-m1024" << QString("-r%1").arg(redundancy);
    else
        args << _ngPost->_par2Args.split(" ");

    bool useParPar = _ngPost->useParPar();
    QString archiveTmpFolder = QString("%1/%2").arg(tmpFolder, archiveName);

    // we've already compressed => we gen par2 for the files in the archive folder
    if (_extProc) {
        if (useParPar && args.last().trimmed() != "-o")
            args << "-o";
        args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
        if (useParPar)
            args << "-R" << archiveTmpFolder;
        else {
            if (_use7z)
                args << QString("%1/%2.7z%3")
                            .arg(archiveTmpFolder, archiveName, _splitArchive ? "*" : "");
            else
                args << QString("%1/%2*rar").arg(archiveTmpFolder, archiveName);

            if (_ngPost->_par2Args.isEmpty() && (_ngPost->debugMode() || !_postWidget))
                args << "-q"; // remove the progressbar bar
        }
    } else { // par2 generation only => can't use folders or files from different drive (Windows)
        QString basePath = _files.first().absolutePath();
        if (useParPar) {
            args << "-f" << "basename";
            if (args.last().trimmed() != "-o")
                args << "-o";
            args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
        } else {
#if defined(Q_OS_WIN)
            QString basePathWin(basePath);
            basePathWin.replace("/", "\\");
            if (_ngPost->useMultiPar())
                args << QString("/d%1").arg(basePathWin);
            else
                args << "-B" << basePathWin;
            QString par2File = QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
            par2File.replace("/", "\\");
            args << par2File;
#else
            args << "-B" << basePath;
            args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
#endif
        }

        for (const QFileInfo &fileInfo : _files) {
            if (fileInfo.isDir()) {
                _error(tr("you can't post folders without compression..."));
                return false;
            }
            QString path = fileInfo.absoluteFilePath();
#if defined(Q_OS_WIN)
            if (!useParPar && basePath != fileInfo.absolutePath()) {
                //                _error(tr("Due to par2 software limitation and to avoid unnecessary copies, all files must be in the same folder..."));
                _error(tr("only ParPar allows to generate par2 for files from different folders... "
                          "you should consider using it ;)"));
                return false;
            }
            path.replace("/", "\\");
#endif
            args << path;
        }

        QString archiveTmpFolder = _createArchiveFolder(tmpFolder, archiveName);
        if (archiveTmpFolder.isEmpty())
            return false;

        _extProc = new QProcess(this);
        connect(_extProc,
                &QProcess::readyReadStandardOutput,
                this,
                &PostingJob::onExtProcReadyReadStandardOutput,
                Qt::DirectConnection);
        connect(_extProc,
                &QProcess::readyReadStandardError,
                this,
                &PostingJob::onExtProcReadyReadStandardError,
                Qt::DirectConnection);
    }

    connect(_extProc,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            &PostingJob::onGenPar2Finished,
            Qt::QueuedConnection);

    if (_ngPost->debugMode() || !_postWidget)
        _log(QString("[%1] %2: %3 %4")
                 .arg(timestamp())
                 .arg(tr("Generating par2"))
                 .arg(_ngPost->_par2Path)
                 .arg(args.join(" ")));
    else
        _log(QString("%1...\n").arg(tr("Generating par2")));
    if (!_ngPost->useParPar())
        _limitProcDisplay = true;
    _nbProcDisp = 0;
    _extProc->start(_ngPost->_par2Path, args);

    return true;
}

void PostingJob::onGenPar2Finished(int exitCode)
{
    if (_ngPost->debugMode())
        _log(tr("=> par2 exit code: %1\n").arg(exitCode));
    else
        _log("\n");

    _cleanExtProc();

    if (exitCode != 0) {
        _error(tr("Error during par2 generation: %1").arg(exitCode));
        _cleanCompressDir();
        emit postingFinished();
    } else {
        _packed = true;
        if (this == _ngPost->_activeJob)
            _postFiles();

        emit packingDone();
    }
}

void PostingJob::_cleanExtProc()
{
    delete _extProc;
    _extProc = nullptr;
    if (_ngPost->debugFull())
        _log(tr("External process deleted."));
}

void PostingJob::_cleanCompressDir()
{
    if (!_keepRar)
        _compressDir->removeRecursively();
    if (_ngPost->debugMode())
        _log(tr("Compressed files deleted."));
}

QString PostingJob::_createArchiveFolder(const QString &tmpFolder, const QString &archiveName)
{
    QString archiveTmpFolder = QString("%1/%2").arg(tmpFolder).arg(archiveName);
    _compressDir = new QDir(archiveTmpFolder);
    if (_compressDir->exists()) {
        _error(tr("The temporary directory '%1' already exists... (either remove it or change the "
                  "archive name)")
                   .arg(archiveTmpFolder));
        delete _compressDir;
        _compressDir = nullptr;
        return QString();
    } else {
        if (!_compressDir->mkpath(".")) {
            _error(tr("Couldn't create the temporary folder: '%1'...").arg(archiveTmpFolder));
            delete _compressDir;
            _compressDir = nullptr;
            return QString();
        }
    }
    return archiveTmpFolder;
}

void PostingJob::onExtProcReadyReadStandardOutput()
{
    if (_ngPost->debugMode())
        _log(_extProc->readAllStandardOutput(), false);
    else if (_isActiveJob) {
        if (!_limitProcDisplay || ++_nbProcDisp % 42 == 0)
            _log("*", false);
    }
}

void PostingJob::onExtProcReadyReadStandardError()
{
    _error(_extProc->readAllStandardError());
}

bool PostingJob::_checkTmpFolder() const
{
    if (_tmpPath.isEmpty()) {
        _error(tr("NO_POSSIBLE_COMPRESSION: You must define the temporary directory..."));
        return false;
    }

    QFileInfo fi(_tmpPath);
    if (!fi.exists() || !fi.isDir() || !fi.isWritable()) {
        _error(tr("ERROR: the temporary directory must be a WRITABLE directory..."));
        return false;
    }

    return true;
}

qint64 PostingJob::_dirSize(const QString &path)
{
    qint64 size = 0;
    QDir dir(path);
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::System
                                                 | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fi.isDir())
            size += _dirSize(fi.absoluteFilePath());
        else
            size += fi.size();
    }
    return size;
}

bool PostingJob::_canCompress() const
{
    //1.: the _tmp_folder must be writable
    if (!_checkTmpFolder())
        return false;

    //2.: check _rarPath is executable
    QFileInfo fi(_rarPath);
    if (!fi.exists() || !fi.isFile() || !fi.isExecutable()) {
        _error(tr("ERROR: the RAR path is not executable..."));
        return false;
    }

    return true;
}

bool PostingJob::_canGenPar2() const
{
    //1.: the _tmp_folder must be writable
    if (!_checkTmpFolder())
        return false;

    //2.: check _ is executable
    QFileInfo fi(_ngPost->_par2Path);
    if (!fi.exists() || !fi.isFile() || !fi.isExecutable()) {
        _error(tr("ERROR: par2 is not available..."));
        return false;
    }

    return true;
}
