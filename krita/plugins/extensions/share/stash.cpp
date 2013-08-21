/*
 * Copyright (c) 2012 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <stash.h>

#include <QCoreApplication>
#include <QUrl>
#include <QBuffer>
#include <QMetaClassInfo>
#include <qjson/parser.h>
#include <KTemporaryFile>
#include <QImageWriter>
#include <complex>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <kis_view2.h>
#include <KoUpdater.h>

Stash::Stash(O2DeviantART *deviant, QObject *parent)
    : QObject(parent)
    , m_ready(0)
    , m_bytesAvailable(-1)
    , m_progressUpdater(0)
    , m_progressSubtask(0)
{
    connect(deviant, SIGNAL(linkedChanged()), SLOT(testCall()));
    m_requestor = new O2Requestor(&m_networkAccessManager, deviant, this);
    connect(m_requestor, SIGNAL(finished(int,QNetworkReply::NetworkError,QByteArray)), SLOT(slotFinished(int,QNetworkReply::NetworkError,QByteArray)));
    connect(m_requestor, SIGNAL(uploadProgress(int,qint64,qint64)), SIGNAL(uploadProgress(int,qint64,qint64)));
    connect(m_requestor, SIGNAL(uploadProgress(int,qint64,qint64)), SLOT(slotUploadProgress(int,qint64,qint64)));
}

Stash::~Stash()
{

}

bool Stash::ready() const
{
    return m_ready;
}

QList<Submission> Stash::submissions() const
{
    return m_submissions;
}

int Stash::availableSpace()
{
    if(m_bytesAvailable == -1)
        updateAvailableSpace();
    return m_bytesAvailable;
}

void Stash::testCall()
{
    QUrl url("https://www.deviantart.com/api/draft15/placebo");
    QNetworkRequest request(url);
    m_callMap[m_requestor->get(request)] = Placebo;
}

void Stash::submit(KisImageWSP image, const QString &filename, const QString &title, const QString &comments, const QStringList &keywords, const QString &folder)
{
    QUrl url("https://www.deviantart.com/api/draft15/stash/submit");
    url.addQueryItem("title", title);
    url.addQueryItem("artist_comments", comments);
    url.addQueryItem("keywords", keywords.join(","));
    if(!folder.isEmpty())
        url.addQueryItem("folderid", folder);
    QNetworkRequest request(url);

    //QString bound="margin"; //name of the boundary
    //according to rfc 1867 we need to put this string here:
    QByteArray data(QString("--margin\r\n").toAscii());
    data.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
    data.append("submit\r\n");   //our script's name, as I understood. Please, correct me if I'm wrong
    data.append("--margin\r\n");   //according to rfc 1867
    data.append(QString("Content-Disposition: form-data; name=\"uploaded\"; filename=\"%1\"\r\n").arg(filename).toAscii());  //name of the input is "uploaded" in my form, next one is a file name.
    data.append("Content-Type: image/jpeg\r\n\r\n"); //data type

    QByteArray tmpData;
    QBuffer buffer(&tmpData, this);
    buffer.open(QIODevice::WriteOnly);
    QImage imagedata = image->convertToQImage(image->bounds(), image->profile());
    imagedata.save(&buffer, "PNG");
    data.append(tmpData);

    data.append("\r\n");
    data.append("--margin--\r\n");  //closing boundary according to rfc 1867
    request.setRawHeader(QString("Content-Type").toAscii(),QString("multipart/form-data; boundary=margin").toAscii());
    request.setRawHeader(QString("Content-Length").toAscii(), QString::number(data.length()).toAscii());

    m_callMap[m_requestor->post(request, data)] = Submit;
}

void Stash::submit(QObject* view, const QString& filename, const QString& title, const QString& comments, const QStringList& keywords, const QString& folder)
{
    KisView2* tmpView = qobject_cast<KisView2*>(view);
    if(tmpView) {
        m_progressUpdater = tmpView->createProgressUpdater();
        m_progressUpdater->start(100, i18n("Uploading to Sta.sh"));
        m_progressSubtask = m_progressUpdater->startSubtask(1, i18n("Uploading to Sta.sh"));
        m_progressSubtask->setRange(0, 1);
        submit(tmpView->image(), filename, title, comments, keywords, folder);
    }
}

void Stash::update(const QString &stashid, const QString &title, const QString comments, const QStringList& keywords)
{

}


void Stash::move(const QString &stashid, const QString folder)
{

}


void Stash::renameFolder(const QString &folderId, const QString &folder)
{

}


void Stash::updateAvailableSpace()
{
    QUrl url("https://www.deviantart.com/api/draft15/stash/space");
    QNetworkRequest request(url);
    m_callMap[m_requestor->get(request)] = UpdateAvailableSpace;
}


void Stash::delta()
{
    // TODO remember to store the results and include the cursor if we have one from previously...
    QUrl url("https://www.deviantart.com/api/draft15/stash/delta");
    QNetworkRequest request(url);
    m_callMap[m_requestor->get(request)] = Delta;
}


void Stash::fetch(const QString &id)
{

}

void Stash::slotFinished(int id, QNetworkReply::NetworkError error, const QByteArray &data)
{
    Call currentCall = m_callMap[id];
    if(error != QNetworkReply::NoError) {
        emit callFinished(currentCall, false);
        QString errorName = QNetworkReply::staticMetaObject.enumerator( QNetworkReply::staticMetaObject.indexOfEnumerator("NetworkError") ).valueToKey(error);
        emit callError(QString("Error %1 submitting artwork: %2").arg(error).arg(errorName));
        return;
    }
    switch(currentCall)
    {
        case Placebo:
            testCallFinished(data);
            break;
        case Submit:
            submitCallFinished(data);
            break;
        case Update:
            updateCallFinished(data);
            break;
        case Move:
            moveCallFinished(data);
            break;
        case RenameFolder:
            renameFolderCallFinished(data);
            break;
        case UpdateAvailableSpace:
            updateAvailableSpaceCallFinished(data);
            break;
        case Delta:
            deltaCallFinished(data);
            break;
        case Fetch:
            fetchCallFinished(data);
            break;
        default:
            break;
    }
    m_callMap.remove(id);
}

void Stash::testCallFinished(const QByteArray& data)
{
    QJson::Parser parser;
    qDebug() << Q_FUNC_INFO << data;
    bool ok(false);
    QVariantMap result = parser.parse(data, &ok).toMap();
    if(ok && result.contains("status")) {
        m_ready = (result.value("status").toString() == QLatin1String("success"));
    }
    else {
        m_ready = false;
    }
    emit readyChanged();
    emit callFinished(Placebo, m_ready);
}

void Stash::submitCallFinished(const QByteArray& data)
{
    QJson::Parser parser;
    bool ok(false);
    QVariantMap result = parser.parse(data, &ok).toMap();
    if(ok && result.contains("status")) {
        if(result.value("status").toString() == QLatin1String("success")) {
            emit newSubmission(result.value("stashid").toULongLong(), result.value("folder").toString(), result.value("folderid").toInt());
        }
        emit callFinished(Submit, true);
    }
    emit callFinished(Submit, false);
    emit callError(QString("Unknown error submitting new artwork: %1").arg(QString(data)));
    if(m_progressUpdater) {
        m_progressUpdater->deleteLater();
        m_progressUpdater = 0;
        m_progressSubtask = 0;
    }
    /// TODO reenable this when sta.sh decides to not be a derp
    //updateAvailableSpace();
}

void Stash::updateCallFinished(const QByteArray& data)
{

}

void Stash::moveCallFinished(const QByteArray& data)
{

}

void Stash::renameFolderCallFinished(const QByteArray& data)
{

}

void Stash::updateAvailableSpaceCallFinished(const QByteArray& data)
{
    QJson::Parser parser;
    bool ok(false);
    QVariantMap result = parser.parse(data, &ok).toMap();
    if(ok && result.contains("available_space")) {
        m_bytesAvailable = result.value("available_space").toInt();
        emit availableSpaceChanged();
        emit callFinished(UpdateAvailableSpace, true);
    }
    emit callFinished(UpdateAvailableSpace, false);
    emit callError(QString("Unknown error updating the available space: %1").arg(QString(data)));
}

void Stash::deltaCallFinished(const QByteArray& data)
{
    QJson::Parser parser;
    bool ok(false);
    QVariantMap result = parser.parse(data, &ok).toMap();
    if(!ok)
        emit callFinished(Delta, false);

    // TODO handle has_more and cursor
    m_submissions.clear();
    if(result.contains("entries")) {
        foreach(const QVariant& var, result.value("entries").toList()) {
            QVariantMap entry = var.toMap();
            Submission sub;
            sub.id = entry.value("stashid").toString();
            sub.folderId = entry.value("folderid").toString();
            QVariantMap meta = entry.value("metadata").toMap();
            sub.artist_comments = meta.value("artist_comments").toString();
            sub.description = meta.value("description").toString();
            sub.isFolder = meta.value("is_folder").toBool();
            sub.title = meta.value("title").toString();
            m_submissions.append(sub);
            qDebug() << "-----------------------\n" << meta;
        }
    }
    emit callFinished(Delta, true);
    emit submissionsChanged();
}

void Stash::fetchCallFinished(const QByteArray& data)
{

}

void Stash::slotUploadProgress(int id, qint64 bytesSent, qint64 bytesTotal)
{
    if(m_progressUpdater) {
        if(m_progressSubtask->max == 1) {
            m_progressSubtask->setRange(0, bytesTotal);
        }
        m_progressSubtask->setValue(bytesSent);
    }
}

#include "stash.moc"