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

#ifndef STASH_H
#define STASH_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QPointer>

#include <kis_types.h>

#include "o2requestor.h"
#include "o2deviantart.h"

class KoProgressUpdater;
struct Submission {

    /// The ID of the stash entry. This may be empty (will be empty if isFolder is true)
    QString id;
    /// If this is a folder, the ID of the folder in question. If this is not a folder,
    /// the ID of the folder the submission is contained within
    QString folderId;
    /// Whether or not this submission is a folder
    bool isFolder;
    QString title;
    QString artist_comments;
    QStringList keywords;
    QString original_url;
    QString deviant_category;
    QString original;
    QString fullview;
    QString thumb150;
    QString thumb200H;
    QString thumb300W;
    QString description;
    QString size;
    QString thumbFolder;
    QList<Submission> contents;
};
Q_DECLARE_METATYPE(QList<Submission>);

class Stash : public QObject {

    Q_OBJECT
    /// Whether or not the stash is ready to take submission calls (force an update on this status by using the test call)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    /// The number of bytes available in the stash. Update by calling updateAvailableSpace
    Q_PROPERTY(int availableSpace READ availableSpace NOTIFY availableSpaceChanged)
    /// The current list of submissions. Update this list by calling delta, and update a single item by calling fetch on a submission's id
    Q_PROPERTY(QList<Submission> submissions READ submissions NOTIFY submissionsChanged)

public:

    Stash(O2DeviantART *deviant, QObject *parent = 0);
    ~Stash();

    bool ready() const;
    QList<Submission> submissions() const;
    int availableSpace();

    enum Call {
        None,
        Placebo,
        Submit,
        Update,
        Move,
        RenameFolder,
        UpdateAvailableSpace,
        Delta,
        Fetch
    };


public slots:

    /// Do the placebo call to make sure the connection to deviant art works
    void testCall();

    /// Upload the given image to deviantart as PNG)
    void submit(KisImageWSP image, const QString& filename, const QString& title, const QString& comments, const QStringList& keywords, const QString& folder);
    /// Overloaded submit call, which takes a QObject of a view rather than an image.
    /// This allows us to call the function from QML. Do not call this from C++ without
    /// good reason (one which you don't have, as the view will always enable you to
    /// use the other function)
    void submit(QObject* view, const QString& filename, const QString& title, const QString& comments, const QStringList& keywords, const QString& folder);

    /// Update the given item
    void update(const QString &stashid, const QString &title, const QString comments, const QStringList& keywords);

    /// Move the given stash to the specified folder
    void move(const QString &stashid, const QString folder);

    /// Rename the specified folder
    void renameFolder(const QString &folderId, const QString &folder);

    /// updates the available space variable
    void updateAvailableSpace();

    /// updates the list of folders and submissions
    void delta();

    /// fetches folder or submission data. This works both for folders and submissions
    void fetch(const QString &id);


private slots:
    void slotFinished(int id, QNetworkReply::NetworkError error, const QByteArray &data);
    void slotUploadProgress(int id, qint64 bytesSent, qint64 bytesTotal);

signals:
    void readyChanged();
    void availableSpaceChanged();
    void submissionsChanged();

    void callFinished(Stash::Call call, bool result);
    void callError(QString error);
    void uploadProgress(int id, qint64 bytesSent, qint64 bytesTotal);
    void newSubmission(qulonglong stashId, QString folder, int folderId);

private:
    QMap<int, Call> m_callMap;

    bool m_ready;
    QNetworkAccessManager m_networkAccessManager;
    O2Requestor *m_requestor;
    QList<Submission> m_submissions;
    int m_bytesAvailable;
    KoProgressUpdater* m_progressUpdater;
    QPointer<KoUpdater> m_progressSubtask;

    void testCallFinished(const QByteArray& data);
    void submitCallFinished(const QByteArray& data);
    void updateCallFinished(const QByteArray& data);
    void moveCallFinished(const QByteArray& data);
    void renameFolderCallFinished(const QByteArray& data);
    void updateAvailableSpaceCallFinished(const QByteArray& data);
    void deltaCallFinished(const QByteArray& data);
    void fetchCallFinished(const QByteArray& data);
};

#endif